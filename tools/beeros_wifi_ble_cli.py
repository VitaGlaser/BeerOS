#!/usr/bin/env python3
"""
Minimal BLE client for BeerOS Wi-Fi provisioning service.

This script is intentionally pragmatic: it implements the protocol inferred from
headers in this repository and includes fallbacks for command payload variants.
"""

from __future__ import annotations

import argparse
import asyncio
import json
import re
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

from bleak import BleakClient, BleakScanner

WIFI_SERVICE_UUID = "52a08919-9e40-43e6-acd9-266137021433"
STATUS_UUID = "0f121af2-f3a0-4c2d-a09a-fff2d9b4d611"
COMMAND_UUID = "48d9033a-dd5e-41e6-985c-9aecc16e40c6"
SAVED_NETWORKS_UUID = "4f995cd6-e7e6-43f5-aff9-7ab3bf467927"
SCANNED_NETWORKS_UUID = "c71fa1dd-13c3-4e71-9ab5-d10cbe30b55a"
IDENT_SERVICE_UUID = "4ca5712a-f319-4bfb-8211-1e2337ecaf8f"
IDENT_MANUFACTURER_INFO_UUID = "fd174313-4c5a-42be-b3fb-24e47024982f"
TIME_SERVICE_UUID = "1de02d1d-2b24-4f4b-a0a0-13e3283e39d9"
OTA_SERVICE_UUID = "d6f1d96d-594c-4c53-b1c6-244a1dfde6d8"

KNOWN_BEEROS_SERVICES = {
    WIFI_SERVICE_UUID.lower(),
    IDENT_SERVICE_UUID.lower(),
    TIME_SERVICE_UUID.lower(),
    OTA_SERVICE_UUID.lower(),
}

CMD_DATA_LENGTH = 128
MAX_SCANNED = 32
CACHE_FILE = Path(__file__).with_name(".beeros_ble_cache.json")


class FrameType:
    UNDEFINED = 0
    READ_ITEMS = 1
    READ_PART = 2
    WRITE_ITEMS = 3
    WRITE_PART = 4
    RESET_ITEMS = 5


class WifiCommand:
    NO_COMMAND = 0
    CONFIG_START = 1
    CONFIG_END = 2
    SCAN = 3
    CONNECT = 4
    DISCONNECT = 5


class StaStatus:
    SCANNING = 0
    SCAN_DONE = 1
    CONNECTING = 2
    CONNECTED = 3
    DISCONNECTING = 4
    DISCONNECTED = 5
    ERROR = 6


@dataclass
class WifiStatus:
    rssi: int
    sta_status: int
    config_mode: bool
    connected_ssid: str


@dataclass
class ScannedNetwork:
    rssi: int
    has_password: bool
    ssid: str


@dataclass
class SavedNetwork:
    ssid: str
    password: str


def _load_cached_mac() -> Optional[str]:
    try:
        if not CACHE_FILE.exists():
            return None
        data = json.loads(CACHE_FILE.read_text(encoding="utf-8"))
        mac = (data.get("last_mac") or "").strip().upper()
        return mac if mac else None
    except Exception:
        return None


def _save_cached_mac(mac: str) -> None:
    try:
        CACHE_FILE.write_text(json.dumps({"last_mac": mac.upper()}), encoding="utf-8")
    except Exception:
        pass


def _cstr(data: bytes) -> str:
    return data.split(b"\x00", 1)[0].decode("utf-8", errors="ignore")


def parse_wifi_status(raw: bytes) -> WifiStatus:
    if len(raw) < 36:
        raise ValueError(f"WiFi status payload too short: {len(raw)}")
    rssi = struct.unpack_from("<h", raw, 0)[0]
    sta = raw[2]
    cfg = bool(raw[3])
    ssid = _cstr(raw[4:36])
    return WifiStatus(rssi=rssi, sta_status=sta, config_mode=cfg, connected_ssid=ssid)


def parse_scanned_item(raw: bytes) -> ScannedNetwork:
    if len(raw) < 36:
        raise ValueError(f"Scanned network payload too short: {len(raw)}")
    rssi = struct.unpack_from("<h", raw, 0)[0]
    has_password = bool(raw[2])
    ssid = _cstr(raw[3:35])
    return ScannedNetwork(rssi=rssi, has_password=has_password, ssid=ssid)


def parse_saved_item(raw: bytes) -> SavedNetwork:
    if len(raw) < 96:
        raise ValueError(f"Saved network payload too short: {len(raw)}")
    ssid = _cstr(raw[0:32])
    password = _cstr(raw[32:96])
    return SavedNetwork(ssid=ssid, password=password)


def _device_has_service_uuid(device, service_uuid: str) -> bool:
    uuids = []
    metadata = getattr(device, "metadata", None) or {}
    if isinstance(metadata, dict):
        uuids = metadata.get("uuids") or []
    return any((u or "").lower() == service_uuid.lower() for u in uuids)


def _ascii_from_bytes(data: bytes) -> str:
    return "".join(chr(b) if 32 <= b <= 126 else "\x00" for b in data)


def _extract_serial_candidates(raw: bytes) -> List[str]:
    # Expected serial/UID is typically 16 ASCII chars (letters/digits),
    # often visible as one full chunk in manufacturer data.
    text = _ascii_from_bytes(raw)
    candidates = set()

    for m in re.finditer(r"[A-Za-z0-9]{8,24}", text):
        token = m.group(0)
        if len(token) >= 10:
            candidates.add(token)

    # Also try fixed 16-byte windows directly from binary data.
    if len(raw) >= 16:
        for i in range(0, len(raw) - 15):
            chunk = raw[i : i + 16]
            if all((48 <= b <= 57) or (65 <= b <= 90) or (97 <= b <= 122) for b in chunk):
                candidates.add(chunk.decode("ascii", errors="ignore"))

    # Prefer longer tokens first (typical UID is 16 chars).
    return sorted(candidates, key=lambda x: (-len(x), x))


def _extract_adv_serial_candidates(device) -> List[str]:
    metadata = getattr(device, "metadata", None) or {}
    if not isinstance(metadata, dict):
        return []

    manufacturer_data = metadata.get("manufacturer_data") or {}
    if not isinstance(manufacturer_data, dict):
        return []

    all_candidates = set()

    for raw in manufacturer_data.values():
        if raw is None:
            continue
        data = bytes(raw)
        cands = _extract_serial_candidates(data)
        for c in cands:
            all_candidates.add(c)

    return sorted(all_candidates, key=lambda x: (-len(x), x))


def _pick_best_serial(candidates: List[str]) -> Optional[str]:
    if not candidates:
        return None
    for c in candidates:
        if "MBR" in c.upper():
            return c
    return candidates[0]


def _matches_sn_filters(
    candidates: List[str],
    sn_prefix: Optional[str],
    sn_contains: Optional[str],
    sn_exact: Optional[str],
) -> bool:
    if not (sn_prefix or sn_contains or sn_exact):
        return True
    if not candidates:
        return False

    for sn in candidates:
        upper = sn.upper()
        if sn_prefix and not upper.startswith(sn_prefix.upper()):
            continue
        if sn_contains and sn_contains.upper() not in upper:
            continue
        if sn_exact and upper != sn_exact.upper():
            continue
        return True

    return False


async def discover_devices(
    name_filter: Optional[str],
    timeout: float,
    service_filter: Optional[str],
    sn_prefix: Optional[str],
    sn_contains: Optional[str],
    sn_exact: Optional[str],
) -> None:
    print(f"Scanning BLE devices for {timeout:.1f}s...")
    devices = await BleakScanner.discover(timeout=timeout)

    shown = 0
    for d in devices:
        name = d.name or ""
        adv_sn_candidates = _extract_adv_serial_candidates(d)
        adv_sn = _pick_best_serial(adv_sn_candidates)
        if name_filter and name_filter.lower() not in name.lower():
            continue
        if service_filter and not _device_has_service_uuid(d, service_filter):
            continue
        if not _matches_sn_filters(adv_sn_candidates, sn_prefix, sn_contains, sn_exact):
            continue

        shown += 1
        rssi = getattr(d, "rssi", None)
        rssi_text = f" RSSI={rssi}" if rssi is not None else ""
        sn_text = f" sn='{adv_sn}'" if adv_sn else ""
        if len(adv_sn_candidates) > 1:
            sn_text += f" sn_candidates={adv_sn_candidates}"
        print(f"- {d.address} | name='{name}'{rssi_text}{sn_text}")

    if shown == 0:
        print("No matching BLE devices found.")
        print("Try without --name filter and with longer timeout, e.g. --timeout 15.")


async def _write_with_fallback(client: BleakClient, char_uuid: str, payload: bytes) -> None:
    try:
        await client.write_gatt_char(char_uuid, payload, response=True)
    except Exception:
        await client.write_gatt_char(char_uuid, payload, response=False)


async def dump_gatt(address: str) -> None:
    async with BleakClient(address, timeout=15.0) as client:
        print(f"Connected: {client.is_connected}")
        services = await client.get_services()
        for service in services:
            print(f"Service {service.uuid}")
            for ch in service.characteristics:
                props = ",".join(ch.properties)
                print(f"  Char {ch.uuid} [{props}]")


def _extract_uid_from_manufacturer_info(raw: bytes) -> str:
    # ManufactureInfo layout in firmware:
    # uint64_t cts; char uid[16]; uint8_t environment; (potential padding)
    cands = _extract_serial_candidates(raw)
    if not cands:
        return ""
    for c in cands:
        if "MBR" in c.upper():
            return c
    return cands[0]


async def identify_device(address: str) -> None:
    async with BleakClient(address, timeout=15.0) as client:
        services = await client.get_services()
        service_uuids = {s.uuid.lower() for s in services}
        has_wifi = WIFI_SERVICE_UUID.lower() in service_uuids
        has_ident = IDENT_SERVICE_UUID.lower() in service_uuids
        print(f"Address {address}: wifi_service={has_wifi} ident_service={has_ident}")

        if not has_ident:
            return

        try:
            raw = bytes(await client.read_gatt_char(IDENT_MANUFACTURER_INFO_UUID))
            uid = _extract_uid_from_manufacturer_info(raw)
            if uid:
                print(f"  UID/SN: {uid}")
            else:
                print(f"  Manufacturer raw ({len(raw)} B): {raw.hex()}")
        except Exception as exc:
            print(f"  Manufacturer info read failed: {exc}")


async def probe_uid(
    timeout: float,
    connect_timeout: float,
    max_devices: int,
    total_timeout: float,
    uid_contains: Optional[str],
) -> None:
    print(f"Discovering candidates for {timeout:.1f}s...")
    devices = await BleakScanner.discover(timeout=timeout)
    if not devices:
        print("No BLE devices discovered.")
        return

    devices = list(devices)[: max(1, max_devices)]
    print(
        f"UID probe over up to {len(devices)} device(s), connect-timeout={connect_timeout:.1f}s, total-timeout={total_timeout:.1f}s"
    )

    loop = asyncio.get_running_loop()
    started = loop.time()
    matches = 0

    for idx, d in enumerate(devices, start=1):
        if (loop.time() - started) >= total_timeout:
            print("UID probe stopped by total timeout.")
            break

        address = d.address
        name = d.name or ""
        print(f"[{idx}/{len(devices)}] {address} name='{name}'")

        try:
            async with BleakClient(address, timeout=connect_timeout) as client:
                # Try direct characteristic read first, then fallback to service discovery.
                uid = ""
                try:
                    raw = bytes(await client.read_gatt_char(IDENT_MANUFACTURER_INFO_UUID))
                    uid = _extract_uid_from_manufacturer_info(raw)
                except Exception:
                    pass

                if not uid:
                    try:
                        services = await client.get_services()
                        service_uuids = {s.uuid.lower() for s in services}
                        if IDENT_SERVICE_UUID.lower() in service_uuids:
                            raw = bytes(await client.read_gatt_char(IDENT_MANUFACTURER_INFO_UUID))
                            uid = _extract_uid_from_manufacturer_info(raw)
                    except Exception:
                        pass

                if uid:
                    if uid_contains and uid_contains.upper() not in uid.upper():
                        continue
                    matches += 1
                    print(f"  UID match: {uid}")
        except Exception:
            continue

    if matches == 0:
        print("No UID matches found.")
        print("Try larger --timeout/--max-devices, keep board close, and verify firmware is running BLE.")


async def _detect_reasons_for_address(address: str, connect_timeout: float) -> List[str]:
    reasons: List[str] = []
    async with BleakClient(address, timeout=connect_timeout) as client:
        # Most reliable signal here: status characteristic read, retry a few times.
        for _ in range(3):
            try:
                items = await _read_array_items(client, STATUS_UUID, item_size=36, max_items=1)
                if items:
                    st = parse_wifi_status(items[0])
                    reasons.append(f"status char readable (sta={st.sta_status}, ssid='{st.connected_ssid}')")
                    break
            except Exception:
                await asyncio.sleep(0.15)

        # Service fingerprint.
        try:
            services = await client.get_services()
            service_uuids = {s.uuid.lower() for s in services}
            matched = sorted(service_uuids.intersection(KNOWN_BEEROS_SERVICES))
            if matched:
                reasons.append(f"service match {matched}")
        except Exception:
            pass

        # Direct manufacturer characteristic fingerprint.
        if not reasons:
            try:
                raw = bytes(await client.read_gatt_char(IDENT_MANUFACTURER_INFO_UUID))
                uid = _extract_uid_from_manufacturer_info(raw)
                if uid:
                    reasons.append(f"manufacturer UID {uid}")
            except Exception:
                pass

    return reasons


async def find_device_type(
    timeout: float,
    connect_timeout: float,
    max_devices: int,
    total_timeout: float,
    target_mac: Optional[str],
    stop_on_first: bool,
    use_cache: bool,
) -> None:
    if use_cache and not target_mac:
        cached = _load_cached_mac()
        if cached:
            print(f"Trying cached MAC first: {cached}")
            try:
                reasons = await _detect_reasons_for_address(cached, connect_timeout)
                if reasons:
                    print(f"  BeerOS-like device detected: {cached}")
                    for r in reasons:
                        print(f"    - {r}")
                    if stop_on_first:
                        print("Stopping after cached BeerOS-like match.")
                        return
            except Exception:
                pass

    print(f"Discovering devices for {timeout:.1f}s...")
    devices = await BleakScanner.discover(timeout=timeout)
    if not devices:
        print("No BLE devices discovered.")
        return

    norm_target = (target_mac or "").replace("-", ":").upper()
    if norm_target:
        devices = [d for d in devices if (d.address or "").upper() == norm_target]
        if not devices:
            print(f"Target MAC {norm_target} not seen in discovery window.")
            return

    devices = list(devices)[: max(1, max_devices)]
    print(
        f"Type probe over up to {len(devices)} device(s), connect-timeout={connect_timeout:.1f}s, total-timeout={total_timeout:.1f}s"
    )

    loop = asyncio.get_running_loop()
    started = loop.time()
    found = 0

    for idx, d in enumerate(devices, start=1):
        if (loop.time() - started) >= total_timeout:
            print("Type probe stopped by total timeout.")
            break

        address = d.address
        name = d.name or ""
        print(f"[{idx}/{len(devices)}] Checking {address} name='{name}'")

        try:
            reasons = await _detect_reasons_for_address(address, connect_timeout)
            if reasons:
                found += 1
                print(f"  BeerOS-like device detected: {address}")
                for r in reasons:
                    print(f"    - {r}")
                _save_cached_mac(address)

                if stop_on_first:
                    print("Stopping after first BeerOS-like match.")
                    return

        except Exception:
            continue

    if found == 0:
        print("No BeerOS-like devices detected in this scan.")
        print("If you know device MAC from logs, use --target-mac and increase --timeout.")


async def probe_beeros(
    timeout: float,
    connect_timeout: float,
    max_devices: int,
    stop_on_first: bool,
    total_timeout: float,
    sn_prefix: Optional[str],
    sn_contains: Optional[str],
    sn_exact: Optional[str],
    verbose_services: bool,
) -> None:
    print(f"Discovering candidates for {timeout:.1f}s...")
    devices = await BleakScanner.discover(timeout=timeout)
    if not devices:
        print("No BLE devices discovered.")
        return

    devices = list(devices)[: max(1, max_devices)]
    print(f"Probing up to {len(devices)} device(s), connect-timeout={connect_timeout:.1f}s, total-timeout={total_timeout:.1f}s")

    found = 0
    loop = asyncio.get_running_loop()
    started = loop.time()

    for idx, d in enumerate(devices, start=1):
        if (loop.time() - started) >= total_timeout:
            print("Probe stopped by total timeout.")
            break

        address = d.address
        name = d.name or ""
        adv_sn_candidates = _extract_adv_serial_candidates(d)
        adv_sn = _pick_best_serial(adv_sn_candidates)
        if not _matches_sn_filters(adv_sn_candidates, sn_prefix, sn_contains, sn_exact):
            continue

        sn_text = f" sn='{adv_sn}'" if adv_sn else ""
        print(f"[{idx}/{len(devices)}] Probing {address} name='{name}'{sn_text}")
        try:
            async with BleakClient(address, timeout=connect_timeout) as client:
                services = await client.get_services()
                service_uuids = {s.uuid.lower() for s in services}
                if verbose_services:
                    print(f"  services: {sorted(service_uuids)}")

                matched = sorted(service_uuids.intersection(KNOWN_BEEROS_SERVICES))
                if matched:
                    found += 1
                    print(f"BeerOS candidate: {address} | name='{name}'{sn_text}")
                    print(f"  matched services: {matched}")

                    if IDENT_SERVICE_UUID.lower() in service_uuids:
                        try:
                            raw = bytes(await client.read_gatt_char(IDENT_MANUFACTURER_INFO_UUID))
                            uid = _extract_uid_from_manufacturer_info(raw)
                            if uid:
                                print(f"  UID/SN: {uid}")
                        except Exception:
                            pass

                    if stop_on_first:
                        print("Stopping after first BeerOS candidate.")
                        break
        except Exception:
            # Many devices will reject/timeout; keep probing.
            continue

    if found == 0:
        print("No BeerOS candidate found by GATT UUID probe.")
        print("Check that target is advertising and not already connected to another client.")


async def _read_array_items(
    client: BleakClient,
    char_uuid: str,
    item_size: int,
    max_items: int,
) -> List[bytes]:
    # Ask server to start reading from item 0. value2 is item count for READ_ITEMS.
    header = bytes([FrameType.READ_ITEMS, 0, max_items, 0])
    await _write_with_fallback(client, char_uuid, header)

    result: List[bytes] = []
    seen_indexes = set()

    for _ in range(32):
        packet = bytes(await client.read_gatt_char(char_uuid))
        if len(packet) < 4:
            break

        ftype, item_index, item_count, _ = packet[:4]
        payload = packet[4:]

        if ftype not in (FrameType.READ_ITEMS, FrameType.READ_PART):
            break
        if item_size <= 0:
            break
        if len(payload) % item_size != 0:
            break

        count = len(payload) // item_size
        if count == 0:
            break

        for i in range(count):
            idx = item_index + i
            if idx >= max_items:
                continue
            chunk = payload[i * item_size : (i + 1) * item_size]
            result.append(chunk)
            seen_indexes.add(idx)

        # Stop when wrapped or when we already got enough items.
        if len(seen_indexes) >= max_items:
            break

    return result


async def read_status(address: str) -> None:
    async with BleakClient(address, timeout=15.0) as client:
        items = await _read_array_items(client, STATUS_UUID, item_size=36, max_items=1)
        if not items:
            print("No status data received.")
            return

        status = parse_wifi_status(items[0])
        print(f"status={status.sta_status} rssi={status.rssi} config_mode={status.config_mode} ssid='{status.connected_ssid}'")
        _save_cached_mac(address)


async def read_scanned(address: str) -> None:
    async with BleakClient(address, timeout=15.0) as client:
        items = await _read_array_items(client, SCANNED_NETWORKS_UUID, item_size=36, max_items=MAX_SCANNED)
        networks = [parse_scanned_item(x) for x in items]
        # Device fills empty tail with empty SSID; filter those out.
        networks = [n for n in networks if n.ssid]

        if not networks:
            print("No scanned networks received.")
            return

        for n in networks:
            sec = "secured" if n.has_password else "open"
            print(f"- {n.ssid:32} RSSI={n.rssi:4} {sec}")


async def list_saved_profiles(address: str, show_password: bool) -> None:
    async with BleakClient(address, timeout=15.0) as client:
        items = await _read_array_items(client, SAVED_NETWORKS_UUID, item_size=96, max_items=8)
        profiles = [parse_saved_item(x) for x in items]

        found = 0
        for idx, p in enumerate(profiles):
            if not p.ssid:
                continue
            found += 1
            if show_password:
                pw = p.password
            else:
                pw = "*" * len(p.password) if p.password else ""
            print(f"[{idx}] ssid='{p.ssid}' password='{pw}'")

        if found == 0:
            print("No saved Wi-Fi profiles stored.")


def _build_saved_item(ssid: str, password: str) -> bytes:
    if len(ssid.encode("utf-8")) > 32:
        raise ValueError("SSID is too long (max 32 bytes)")
    if len(password.encode("utf-8")) > 64:
        raise ValueError("Password is too long (max 64 bytes)")

    ssid_slot = ssid.encode("utf-8") + (b"\x00" * (32 - len(ssid.encode("utf-8"))))
    pass_slot = password.encode("utf-8") + (b"\x00" * (64 - len(password.encode("utf-8"))))
    return ssid_slot + pass_slot


async def set_saved_profile(address: str, index: int, ssid: str, password: str) -> None:
    if index < 0 or index > 7:
        raise ValueError("Profile index must be in range 0..7")

    payload = _build_saved_item(ssid, password)
    header = bytes([FrameType.WRITE_ITEMS, index, 1, 0])

    async with BleakClient(address, timeout=15.0) as client:
        await _write_with_fallback(client, SAVED_NETWORKS_UUID, header + payload)
        print(f"Saved profile[{index}] updated: ssid='{ssid}'")


async def send_wifi_command(
    address: str,
    command: int,
    data: bytes = b"",
) -> None:
    data = data[:CMD_DATA_LENGTH]
    data = data + (b"\x00" * (CMD_DATA_LENGTH - len(data)))

    # Try both likely wire formats:
    # 1) u8 command + 128B payload
    # 2) u16 little-endian command + 128B payload
    payload_candidates = [
        bytes([command]) + data,
        struct.pack("<H", command) + data,
    ]

    async with BleakClient(address, timeout=15.0) as client:
        for idx, payload in enumerate(payload_candidates, start=1):
            try:
                await _write_with_fallback(client, COMMAND_UUID, payload)
                print(f"Command format #{idx} sent.")
                return
            except Exception as exc:
                print(f"Command format #{idx} failed: {exc}")

        raise RuntimeError("All command payload formats failed.")


def connect_payload(ssid: str, password: str) -> bytes:
    # Firmware adapter expects ssid\0password\0 in legacy command_data.
    raw = ssid.encode("utf-8") + b"\x00" + password.encode("utf-8") + b"\x00"
    return raw


async def cmd_scan(address: str) -> None:
    await send_wifi_command(address, WifiCommand.SCAN)
    await asyncio.sleep(2.0)
    await read_scanned(address)


async def cmd_connect(address: str, ssid: str, password: str, wait_seconds: int) -> None:
    await send_wifi_command(address, WifiCommand.CONFIG_START)
    await send_wifi_command(address, WifiCommand.CONNECT, connect_payload(ssid, password))

    # Poll state for a short period.
    for _ in range(max(1, wait_seconds)):
        try:
            async with BleakClient(address, timeout=15.0) as client:
                items = await _read_array_items(client, STATUS_UUID, item_size=36, max_items=1)
                if items:
                    st = parse_wifi_status(items[0])
                    print(
                        f"status={st.sta_status} rssi={st.rssi} config_mode={st.config_mode} ssid='{st.connected_ssid}'"
                    )
                    if st.sta_status == StaStatus.CONNECTED:
                        print("Wi-Fi connected.")
                        break
        except Exception as exc:
            print(f"Status read failed: {exc}")
        await asyncio.sleep(1.0)

    await send_wifi_command(address, WifiCommand.CONFIG_END)


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="BeerOS BLE Wi-Fi helper")
    sub = p.add_subparsers(dest="cmd", required=True)

    p_discover = sub.add_parser("discover", help="List BLE devices")
    p_discover.add_argument("--name", default=None, help="Optional device name filter")
    p_discover.add_argument("--timeout", type=float, default=8.0, help="Scan time in seconds")
    p_discover.add_argument(
        "--service",
        default=None,
        help="Optional advertised service UUID filter (some platforms do not expose this)",
    )
    p_discover.add_argument("--sn-prefix", default=None, help="Optional serial prefix filter, e.g. MBR")
    p_discover.add_argument("--sn-contains", default=None, help="Optional serial contains filter, e.g. MBR")
    p_discover.add_argument("--sn-exact", default=None, help="Optional full serial/UID match")

    p_dump = sub.add_parser("dump-gatt", help="List all GATT services/chars")
    p_dump.add_argument("--address", required=True, help="BLE MAC address")

    p_ident = sub.add_parser("identify", help="Read identification data (UID/SN) from one device")
    p_ident.add_argument("--address", required=True, help="BLE MAC address")

    p_probe = sub.add_parser("probe", help="Auto-find BeerOS devices by Wi-Fi service UUID")
    p_probe.add_argument("--timeout", type=float, default=10.0, help="Discover scan time in seconds")
    p_probe.add_argument("--connect-timeout", type=float, default=6.0, help="Per-device connect timeout in seconds")
    p_probe.add_argument("--max-devices", type=int, default=8, help="Maximum discovered devices to probe")
    p_probe.add_argument("--first", action="store_true", help="Stop after first BeerOS candidate")
    p_probe.add_argument("--total-timeout", type=float, default=35.0, help="Hard overall timeout in seconds")
    p_probe.add_argument("--sn-prefix", default=None, help="Optional serial prefix filter, e.g. MBR")
    p_probe.add_argument("--sn-contains", default=None, help="Optional serial contains filter, e.g. MBR")
    p_probe.add_argument("--sn-exact", default=None, help="Optional full serial/UID match")
    p_probe.add_argument("--verbose-services", action="store_true", help="Print all discovered GATT service UUIDs")

    p_uid = sub.add_parser("probe-uid", help="Deep probe: connect and read UID from identification characteristic")
    p_uid.add_argument("--timeout", type=float, default=10.0, help="Discover scan time in seconds")
    p_uid.add_argument("--connect-timeout", type=float, default=4.0, help="Per-device connect timeout in seconds")
    p_uid.add_argument("--max-devices", type=int, default=25, help="Maximum discovered devices to probe")
    p_uid.add_argument("--total-timeout", type=float, default=60.0, help="Hard overall timeout in seconds")
    p_uid.add_argument("--uid-contains", default=None, help="Only print UIDs containing token, e.g. RVSSBHMBRA")

    p_type = sub.add_parser("find-type", help="Actively detect BeerOS device type by GATT fingerprint")
    p_type.add_argument("--timeout", type=float, default=12.0, help="Discover scan time in seconds")
    p_type.add_argument("--connect-timeout", type=float, default=4.0, help="Per-device connect timeout in seconds")
    p_type.add_argument("--max-devices", type=int, default=40, help="Maximum discovered devices to probe")
    p_type.add_argument("--total-timeout", type=float, default=120.0, help="Hard overall timeout in seconds")
    p_type.add_argument("--target-mac", default=None, help="Optional exact MAC to test first, e.g. D0:CF:13:19:CD:E6")
    p_type.add_argument("--first", action="store_true", help="Stop after first BeerOS-like match")
    p_type.add_argument("--no-cache", action="store_true", help="Do not try cached MAC first")

    p_status = sub.add_parser("status", help="Read Wi-Fi status")
    p_status.add_argument("--address", required=True, help="BLE MAC address")

    p_scan = sub.add_parser("wifi-scan", help="Trigger Wi-Fi scan and read results")
    p_scan.add_argument("--address", required=True, help="BLE MAC address")

    p_read_scan = sub.add_parser("read-scanned", help="Read scanned Wi-Fi list without triggering scan")
    p_read_scan.add_argument("--address", required=True, help="BLE MAC address")

    p_list_saved = sub.add_parser("list-saved", help="List saved Wi-Fi profiles")
    p_list_saved.add_argument("--address", required=True, help="BLE MAC address")
    p_list_saved.add_argument("--show-password", action="store_true", help="Show saved passwords in plain text")

    p_set_saved = sub.add_parser("set-saved-profile", help="Set one saved Wi-Fi profile (index 0..7)")
    p_set_saved.add_argument("--address", required=True, help="BLE MAC address")
    p_set_saved.add_argument("--index", type=int, required=True, help="Saved profile index 0..7")
    p_set_saved.add_argument("--ssid", required=True)
    p_set_saved.add_argument("--password", required=True)

    p_conn = sub.add_parser("wifi-connect", help="Connect to Wi-Fi SSID")
    p_conn.add_argument("--address", required=True, help="BLE MAC address")
    p_conn.add_argument("--ssid", required=True)
    p_conn.add_argument("--password", required=True)
    p_conn.add_argument("--wait", type=int, default=12, help="How long to poll status")

    return p


async def main_async(args: argparse.Namespace) -> int:
    if args.cmd == "discover":
        await discover_devices(args.name, args.timeout, args.service, args.sn_prefix, args.sn_contains, args.sn_exact)
        return 0
    if args.cmd == "dump-gatt":
        await dump_gatt(args.address)
        return 0
    if args.cmd == "identify":
        await identify_device(args.address)
        return 0
    if args.cmd == "probe":
        await probe_beeros(
            args.timeout,
            args.connect_timeout,
            args.max_devices,
            args.first,
            args.total_timeout,
            args.sn_prefix,
            args.sn_contains,
            args.sn_exact,
            args.verbose_services,
        )
        return 0
    if args.cmd == "probe-uid":
        await probe_uid(args.timeout, args.connect_timeout, args.max_devices, args.total_timeout, args.uid_contains)
        return 0
    if args.cmd == "find-type":
        await find_device_type(
            args.timeout,
            args.connect_timeout,
            args.max_devices,
            args.total_timeout,
            args.target_mac,
            args.first,
            not args.no_cache,
        )
        return 0
    if args.cmd == "status":
        await read_status(args.address)
        return 0
    if args.cmd == "wifi-scan":
        await cmd_scan(args.address)
        return 0
    if args.cmd == "read-scanned":
        await read_scanned(args.address)
        return 0
    if args.cmd == "list-saved":
        await list_saved_profiles(args.address, args.show_password)
        return 0
    if args.cmd == "set-saved-profile":
        await set_saved_profile(args.address, args.index, args.ssid, args.password)
        return 0
    if args.cmd == "wifi-connect":
        await cmd_connect(args.address, args.ssid, args.password, args.wait)
        return 0

    print(f"Unknown command: {args.cmd}")
    return 2


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    try:
        return asyncio.run(main_async(args))
    except KeyboardInterrupt:
        return 130
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

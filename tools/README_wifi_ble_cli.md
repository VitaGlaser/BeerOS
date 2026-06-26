# BeerOS Wi-Fi BLE CLI (MVP)

Rychly klient pro testovani Wi-Fi konfigurace pres BLE podle protokolu nalezeneho ve firmware.

## Co umi

- najit BLE zarizeni (`discover`)
- vypsat GATT (`dump-gatt`)
- automaticky najit BeerOS podle GATT sluzby (`probe`)
- precist UID/SN z identification service (`identify`)
- precist Wi-Fi status (`status`)
- spustit scan siti a precist vysledky (`wifi-scan`)
- poslat connect prikaz (`wifi-connect`)

## Instalace

```powershell
python -m pip install -r tools/requirements.txt
```

## Pouziti

```powershell
python tools/beeros_wifi_ble_cli.py discover --timeout 12
python tools/beeros_wifi_ble_cli.py discover --timeout 12 --service 52a08919-9e40-43e6-acd9-266137021433
# pokud vis, ze zarizeni skutecne vysila jmeno:
python tools/beeros_wifi_ble_cli.py discover --timeout 12 --name Beer
python tools/beeros_wifi_ble_cli.py probe --timeout 8 --connect-timeout 3 --max-devices 6 --first --total-timeout 20
python tools/beeros_wifi_ble_cli.py probe-uid --timeout 12 --connect-timeout 4 --max-devices 30 --total-timeout 90 --uid-contains RVSSBHMBRA
python tools/beeros_wifi_ble_cli.py find-type --timeout 12 --connect-timeout 4 --max-devices 40 --total-timeout 120
python tools/beeros_wifi_ble_cli.py find-type --timeout 8 --connect-timeout 4 --target-mac D0:CF:13:19:CD:E6
python tools/beeros_wifi_ble_cli.py identify --address AA:BB:CC:DD:EE:FF
python tools/beeros_wifi_ble_cli.py dump-gatt --address AA:BB:CC:DD:EE:FF
python tools/beeros_wifi_ble_cli.py status --address AA:BB:CC:DD:EE:FF
python tools/beeros_wifi_ble_cli.py wifi-scan --address AA:BB:CC:DD:EE:FF
python tools/beeros_wifi_ble_cli.py wifi-connect --address AA:BB:CC:DD:EE:FF --ssid "MojeSit" --password "tajneheslo"
```

## Poznamky

- Firmware zde pouziva custom BLE service UUID:
  - service: `52a08919-9e40-43e6-acd9-266137021433`
  - status: `0f121af2-f3a0-4c2d-a09a-fff2d9b4d611`
  - command: `48d9033a-dd5e-41e6-985c-9aecc16e40c6`
  - scanned networks: `c71fa1dd-13c3-4e71-9ab5-d10cbe30b55a`
- `COMMAND` payload format nebyl v tomto repu primo dohledatelny (implementace je v prebuilt knihovne), proto skript zkousi 2 nejpravdepodobnejsi varianty.
- Pokud je na zarizeni zapnuta dalsi aplikacni autorizace (password attribute), nejdriv bude nutne najit a zapsat spravne heslo do prislusne charakteristiky.
- V tomto firmware muze byt BLE jmeno prazdne (`device_name = ""`), proto filtr `--name` nemusi najit nic.
- Podle firmware se do BLE manufacturer data propaguje `manufactureInfo.uid` (16 znaku), tj. SN/UID.
- Pokud `probe` trva dlouho, sniz `--max-devices` a `--connect-timeout`, nebo pouzij `--first`.
- Pokud se SN neukazuje v advertismentu (bezne na nekterych platformach), pouzij `probe-uid`, ktere cte UID primo z GATT.
- Pokud chces najit "typ zarizeni" bez zavislosti na advertisment SN, pouzij `find-type`.

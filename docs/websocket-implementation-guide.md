# WebSocket Implementation Guide

Krátký návod, jak ve firmware zapojit WebSocket podobně jako v tomto projektu.

## Cíl

- běžný runtime task odesílá data periodicky,
- WebSocket server běží jen když je dostupná síť,
- klient si po připojení vyžádá dostupné kanály,
- zprávy se posílají po kanálech a klient si může vybrat subset.

## 1) Přidej WebSocket manager

Vytvoř manager, který:
- startuje HTTP/WebSocket server,
- registruje endpoint `GET /ws`,
- drží seznam připojených klientů,
- drží subscription masku pro každý klient,
- umí server zastavit, když síť spadne.

Doporučené API:
- `setNetworkConnected(bool connected)`
- `start()`
- `stop()`
- `poll()`

## 2) Napoj start/stop na síť

V connection vrstvě zjisti, jestli je síť dostupná.

- když `network == true`, server spusť,
- když `network == false`, server vypni,
- `poll()` WebSocketu volej jen když je síť dostupná.

Tím máš jeden centrální bod, který řídí celý runtime komunikační tok.

## 3) Použij jednu periodickou smyčku

Nezakládej pro každý kanál vlastní task.
Stačí jedna smyčka, která běží třeba každých `100 ms`:

1. zkontroluje síť,
2. zpracuje WebSocket server,
3. zpracuje MQTT nebo jiné backend odesílání,
4. projde všechny kanály v cyklu.

To je jednodušší na údržbu a snadno se filtruje podle kanálů.

## 4) Zaveď jednotný message model

Pro eventy používej stejnou strukturu zprávy:

```json
{
  "channel": 1,
  "tap_id": 42,
  "time": 1743379201234,
  "volume": 157,
  "state": "tapping"
}
```

Stavy:
- `idle`
- `tapping`
- `finish`

Doporučení:
- `channel` = index kanálu,
- `tap_id` = sekvenční ID čepování,
- `time` = UTC epoch v ms,
- `volume` = aktuální nebo finální objem v ml.

## 5) Po připojení pošli idle zprávu

Hned po connectu pošli klientovi `idle` zprávu.
Ta by měla obsahovat:
- stav `idle`,
- aktuální čas,
- `volume = 0`,
- seznam dostupných kanálů.

Příklad:

```json
{
  "tap_id": 0,
  "time": 1743379200000,
  "volume": 0,
  "state": "idle",
  "available_channels": [0, 1, 3]
}
```

Tohle je dobrý handshake i pro IA/frontendu, protože si hned zjistí, co může sledovat.

## 6) Přidej subscribe protokol

Klient musí umět říct, které kanály chce dostávat.

Minimální varianta:
- `sub:0,2` - přihlásí kanály 0,2
- `get_channels` - vrátí znovu `idle` zprávu s `available_channels`.

Každý klient by měl mít vlastní subscription masku, aby jeden klient nemusel dostávat všechno.

## 7) Broadcastuj po kanálu

Když se změní kanál, pošli zprávu jen klientům, kteří mají ten kanál subscribed.

Pravidlo:
- 1 změna kanálu = 1 WS zpráva,
- když se ve stejném ticku změní 4 kanály, odejdou 4 zprávy,
- `channel` musí být součástí payloadu.

## 8) `tap_id` a sekvence

Použij sekvenční číslo čepování per channel.

Doporučený model:
- při startu eventu: `tap_id = lastSeqNum + 1`,
- při `cepuju` používej stejné ID pro průběžné eventy,
- při `finish` pošli stejné ID z uzavřeného eventu,
- `lastSeqNum` ukládej per channel do NVS nebo do persistent storage.

Tím pádem `tap_id` přežije restart a nepřeskočí při běhu zařízení.

## 9) Doporučený integrační flow pro klienta

1. Připoj se na `ws://<ip>/ws`.
2. Počkej na `idle` zprávu.
3. Z `available_channels` vyber kanály.
4. Pošli `sub:...` .
5. Zpracovávej `tapping` jako live stav.
6. Zpracovávej `finish` jako finální stav.
7. Při reconnectu zopakuj subscribe.

## 10) Co je dobré si pohlídat při přenosu do jiného projektu

- WebSocket server port a endpoint.
- Jak zjistíš dostupnou síť.
- Kde držíš persistent sequence čísla.
- Jaké stavy a payload formát očekává frontend.
- Jestli chceš zprávy per channel nebo batchované.

## 11) Nejmenší implementační checklist

- [ ] WebSocket server manager.
- [ ] Gating přes síť.
- [ ] Jedna periodická poll smyčka.
- [ ] `idle` handshake.
- [ ] Subscribe commandy.
- [ ] Broadcast jen subscribed klientům.
- [ ] Per-channel `tap_id` persistence.
- [ ] Test v browseru přes DevTools Console.

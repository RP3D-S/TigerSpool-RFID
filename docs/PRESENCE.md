# How a TigerSpool declares itself to the account

So that Tiger Studio can list this device, show whether it is online, and tell
which printers it stands in front of.

The shape is **the TigerScale's**, deliberately. A device list, an online dot
and a battery icon should not have to know which product they are looking at,
so the identity, liveness and power fields carry the same names a TigerScale
writes. Only the state block differs, because a spool writer and a scale do
different work.

Reference: `HOW-TIGERSCALE-DECLARES-ITSELF.md`, written from the shipping
TigerScale V3 firmware.

---

## Where it writes

```
users/{uid}/tigerspools/{mac}
```

The collection is **plural**, like every other collection of things under an
account - `scales`, `printers`, `racks`. A user can own several TigerSpools and
each writes its own document; nothing is shared between them.

`{mac}` is the Wi-Fi MAC, lowercase hex, no separators, twelve characters -
`ttcloud::deviceId()`. **The format is frozen.** It is the document id, so
changing it orphans every device already registered under the old one, silently
and with no way to notice from the device.

The device lives under its owner's account. That is the whole access-control
story: a signed-in user may write below their own `users/{uid}`, and no device
allow-list or service account is involved.

### What Firebase has to allow

Until the rules name this path, every beat comes back `403 PERMISSION_DENIED`
and the device is invisible in Studio:

```
match /users/{uid}/tigerspools/{deviceId} {
  allow read, write: if request.auth != null && request.auth.uid == uid;
}
```

The device also declares itself to `pairStart` as `kind: "tigerspool"`,
`model: "TigerSpool"`, with its real firmware version and its mDNS name. It
used to say `bridge` / `TigerTag Bridge` / `cfs_ui`, which was the prototype's
name and a version string that was never a version.

---

## What it writes

One `documents:commit` per beat, with an `updateMask`, so the document is
created on the first beat and updated after - no separate registration step -
and fields outside the mask are left alone. Studio owns `display_name`; a
heartbeat must never trample it.

**Identity, on full beats only**

| field | type |
|---|---|
| `mac` | string, same as the document id |
| `fw_version` | string |
| `mdns_hostname` | string, `tigerspool-xxxx.local` |
| `display_name` | string, `TigerSpool-XXXX` by default |
| `hardware_revision` | null on this board |

**Liveness and power, on every beat**

| field | type |
|---|---|
| `last_heartbeat_at` | **server timestamp**, `REQUEST_TIME` |
| `wifi_signal_dbm` | int, or null when not connected |
| `ip_address` | string |
| `power_source` | `"usb"` or `"battery"` |
| `power_state` | `"active"` or `"screen_off"` |
| `battery_present` | bool |
| `battery_percent` | int, or null with no cell |
| `is_charging` | bool, or null with no cell |

**State, what a TigerSpool is that a scale is not**

| field | type |
|---|---|
| `printers_active` | int - how many printers the user has switched on here |
| `printer_ids` | array of strings - those printers' document ids in the account |
| `last_used_at` | server timestamp, written on the beat after a spool is sent |

`printer_ids` is what lets Studio put the right TigerSpool next to the right
printer. The ids come from the account import and are kept in NVS as one
newline-separated key; a device updated from a firmware that did not keep them
publishes an empty array until its first sync, which is minutes away at worst.

### Three rules that are not decoration

**The timestamp is the server's.** This board has no RTC and no NTP, so any
time it wrote itself would be a guess presented as a fact. `REQUEST_TIME` makes
presence decidable by the server's own clock: online is
`now - last_heartbeat_at < 2 * interval`, whatever the device believes the time
to be.

**Nulls are explicit.** An absent battery writes
`"battery_percent": {"nullValue": "NULL_VALUE"}`. Omitting the field would keep
whatever was there before, so a box that once had a cell would read as still
having one for ever.

**Full beat, then deltas.** The first beat after boot writes everything; after
that only what moved. The full flag is cleared **only on a successful commit**,
so a failed full beat is retried as a full beat rather than leaving Studio with
half a document.

---

## When it beats

| | |
|---|---|
| Awake | every 30 s |
| Screen off | every 5 min |
| A spool written, a cable moved, the screen sleeping or waking, the printer count changing | at once |

The forced beats are the ones a person performs and then watches for. Waiting
five minutes for them reads as an app that has lost the device.

Three refusals in a row hold the next beat for five minutes. A misconfigured
account then costs a handshake now and then, rather than one every thirty
seconds for ever.

## What it costs, and the two traps

A beat is one HTTPS round trip, about a second, run from the main loop. That is
survivable only because the interface has a task of its own: the panel keeps
drawing while the loop is inside the handshake. It is **not** free of the
memory problem - mbedTLS wants a contiguous block of internal RAM - so a beat
is skipped when the largest free block is under 24 KB, and skipped entirely
while the account sync is running. One TLS session at a time.

`power_source` is inferred, not read. This board has no USB-detect line: with
no cell there is nothing else it could be running on, and with one, the charger
holding the rail up is the only evidence available. Studio should treat it as
a hint and `battery_present` / `battery_percent` as the facts.

---

## Not done yet

The command queue - `users/{uid}/tigerspools/{mac}/commands`, polled, with
`pending → ack → in_progress → done | error` - is deliberately left for later.
Presence is what Studio is built around; commands are additive and can land
without changing anything written here.

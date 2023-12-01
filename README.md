# Sway-MÜSLI: Sway - Minimal Ültrafast Status LIne

# Example
`MyWiFi | Qwerty | 80% - Charging | Sun 2021-08-22 12:00:00`

## Features
- Written in C for ültra speed.
- Minimal features:
    - Shows date and time (even seconds!).
    - Shows battery status.
    - Shows wifi connection.

## Installation
1. Clone the repo and run:
```
make
make install # if you want to have the executable in your path
```

2. Add to your sway config file:
```
bar {
    ...
    status_command while sway-musli; do sleep 1; done
    ...
}
```

## Notes
- Depends on `iwd`.
- Assumes your network device is `wlan0`.

## License
MIT

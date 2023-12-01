# Sway-MÜSLI: Sway – Minimal Ültrafast Status LIne

![musli5](https://github.com/sebastiancarlos/sway-musli/assets/88276600/eef1f29a-27db-4a81-ad5d-09c7db590ca8)

## Example
![sway-musli](https://github.com/sebastiancarlos/sway-musli/assets/88276600/b1c82f5e-b2b7-4176-ae95-da5d6ed42d04)

`MyWiFi | Colemak | 65% - Charging | Fri 2023-12-01 17:01:13`


## Features
- Written in C for **ültra speed**.
- Minimal features:
    - Shows date and time (even seconds!).
    - Shows battery status.
    - Shows wifi connection.

## Installation
1. Clone the repo and run:
```bash
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
- I ate müsli with avocado while coding this. 🥣🥑

## License
MIT

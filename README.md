# Sway-MÜSLI: Sway – Minimal Ültrafast Status Line

![musli5](https://github.com/sebastiancarlos/sway-musli/assets/88276600/eef1f29a-27db-4a81-ad5d-09c7db590ca8)

## Example
![sway-musli](https://github.com/sebastiancarlos/sway-musli/assets/88276600/b1c82f5e-b2b7-4176-ae95-da5d6ed42d04)

`MyWiFi | Colemak | 65% - Charging | Fri 2023-12-01 17:01:13`

## Features
- Written in C for **ültra speed**.
  - No dependencies. Uses sockets for communication with Sway and Linux subsystems.
  - Takes one millisecond on my machine.
- Minimal features:
    - Shows date and time (even seconds!).
    - Shows battery status.
    - Shows keyboard layout. 
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
    status_command sway-musli
    ...
}
```

## Usage
```
Usage: sway-musli [-1|--once]
 - Print a stream of status lines to be used with swaybar.
 - If passed -1 or --once, print once and exit.
 - Example sway config:
    ...
    bar {
        status_command sway-musli
    }
    ...
    # Note: Make sure sway-musli is in your PATH.
```

## Notes
- Assumes your network device is `wlan0`. But you can change this in the source file.
- You can take this as a sample code to help you build your personal **ültrafast** Sway status line.
- See also, [i3status](https://manned.org/i3status.1) and [i3blocks](https://github.com/vivien/i3blocks)
- I ate müsli with avocado while coding this. 🥣🥑

## License
MIT

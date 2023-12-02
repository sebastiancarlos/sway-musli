# Sway-MÜSLI: Sway – Minimal Ültrafast Status Line
<p align="center">
  <img src="https://github.com/sebastiancarlos/sway-musli/assets/88276600/4098166f-af80-4a5c-bf0d-363bfede9760" />
</p>

## Example
![sway-musli](https://github.com/sebastiancarlos/sway-musli/assets/88276600/b1c82f5e-b2b7-4176-ae95-da5d6ed42d04)

`MyWiFi | Colemak | 65% - Charging | Fri 2023-12-01 17:01:13`

## Features
- Written in C for **ültra speed**.
  - No dependencies. Uses sockets for communication with Sway and Linux
    subsystems.
  - Takes **1ms** on my machine.
  - Great performance even when running in **60FPS möde**!
  - Support both Sway and i3.
    - Easy to adapt to any status bar which supports text input.
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
Usage: sway-musli [-1|--once] [-f|--fps <FPS>]
 - Print a stream of status lines to be used with swaybar.
 - If passed -1 or --once, print once and exit.
 - If passed -f or --fps, print at most <FPS> times per second.
   - Default is 30 FPS.
 - Example sway config:
    ...
    bar {
        status_command sway-musli
    }
    ...
    # Note: Make sure sway-musli is in your PATH.
```

## FPS?

That's right. We are in high-performance territory now.

By default, `sway-musli` runs at 30 frames per second, but you can change it
with the `--fps` option.

1 FPS is completely acceptable, but you might want more if you add some sort of
dynamic content, or if you want to see keyboard layout changes reflected
immediately.

At around 60FPS, your processor might feel a slight tickle. If your bar is
hidden by default, you might as well run it at 60 FPS because it won't run
while the bar is hidden.

## Notes
- Assumes your network device is `wlan0`. But you can change this in the source
  file.
- You can take this as a sample code to help you build your personal
  **ültrafast** Sway status line.
- See also, [i3status](https://manned.org/i3status.1),
  [i3status-rust](https://github.com/greshake/i3status-rust), and
  [i3blocks](https://github.com/vivien/i3blocks)
- I ate müsli while coding this.
- Contributions welcome!

## License
MIT

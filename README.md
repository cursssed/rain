# rain

ASCII rain for your terminal, written in C with ncurses.
Survives terminal resize without flicker.

<p align="center">
  <img src="https://25midi.com/f/asciirain2.gif" width="100%" alt="ascii-rain demo"/>
</p>

## Dependencies

- ncurses
  - Debian/Ubuntu: `ncurses-dev` or `libncurses-dev`
  - Arch: `ncurses`
  - macOS: `brew install ncurses`
- POSIX `make`
- C11 compiler

## Build

```sh
git clone https://github.com/cursssed/rain
cd rain
make
./rain
```

Press `q` to quit (configurable via `quit_key`).

## Install

```sh
sudo make install              # PREFIX=/usr/local by default
sudo make install PREFIX=/usr  # useful for package builds
```

Installed files:

| File | Destination |
|---|---|
| `rain` | `$PREFIX/bin/rain` |
| `rain.conf.example` | `$PREFIX/share/doc/rain/rain.conf.example` |
| `LICENSE` | `$PREFIX/share/licenses/rain/LICENSE` |

## Configuration

`rain` loads configuration from the first available source, in this order:

1. `--config <path>`
2. `$XDG_CONFIG_HOME/rain/config`
3. `~/.config/rain/config`

A missing config file is not an error - built-in defaults are used instead.

To write a starter config to the default location:

```sh
rain --init-config
```

Add `--force` to overwrite an existing file. See `rain.conf.example` for
documentation of all available keys.

### Available keys

| Key | Default | Description |
|---|---:|---|
| `frame_delay_ms` | `40` | Delay between frames in milliseconds. Lower is faster. |
| `density` | `0.5` | Drop density, measured as drops per terminal column. |
| `speed_min` | `1` | Minimum drop speed in rows per frame. |
| `speed_max` | `5` | Maximum drop speed in rows per frame. |
| `quit_key` | `q` | Single-character quit key. |
| `color_mode` | `manual` | `auto` generates a gradient from `color_base`; `manual` uses `colors`. |
| `color_base` | `#ffffff` | Base color for `auto` mode, in `#rrggbb` format. |
| `colors` | `5-step grayscale` | Comma-separated palette (speed_max entries). Setting this implies `color_mode = manual`. |
| `use_xterm256` | `false` | Quantize colors to the xterm-256 palette instead of using exact RGB. |

## Colors and terminals

By default, `rain` asks the terminal to set exact RGB colors via `init_color`.
This works in ghostty, kitty, wezterm, xterm, and alacritty.

Some terminals report color-changing support but ignore the actual RGB values
(konsole, some SSH/tmux setups). Symptom: drops appear in unexpected ANSI
colors, often blue-ish.

Use this option to avoid terminal palette mutation:

```ini
use_xterm256 = true
```

Colors are then mapped to the fixed xterm-256 palette - more portable, slightly
lower fidelity for dim or low-chroma colors.

## CLI

```text
rain [--config <path>] [--init-config [--force]] [--help] [--version]
```

| Option | Description |
|---|---|
| `--config <path>` | Load configuration from `<path>`. |
| `--init-config` | Write a starter config to the standard location, or to `<path>` if used with `--config`. |
| `--force` | Allow `--init-config` to overwrite an existing file. |
| `--help`, `-h` | Show help and exit. |
| `--version`, `-V` | Show version and exit. |

## Tests and bench

```sh
make test   # run test suite (no real terminal required)
make bench  # frame-loop throughput - prints time and mvaddch call count
```

## Credits

Originally written by Nik, 07.2017.

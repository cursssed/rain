# ascii-rain

Comfy rain for your console, written in C with ncurses.

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

## Install

### System-wide

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

A missing config file is not an error. Built-in defaults are used instead.

To create a documented starter config at the default location:

```sh
rain --init-config
```

By default, `--init-config` refuses to overwrite an existing file. Use `--force`
to replace it:

```sh
rain --init-config --force
```

You can also copy `rain.conf.example` manually from:

```text
$PREFIX/share/doc/rain/rain.conf.example
```

### Available keys

All keys are optional. See `rain.conf.example` for detailed inline
documentation.

| Key | Default | Description |
|---|---:|---|
| `frame_delay_ms` | `30` | Delay between frames in milliseconds. Lower values make the rain faster. |
| `density` | `1.5` | Drop density, measured as drops per terminal column. |
| `speed_min` | `1` | Minimum drop speed in rows per frame. |
| `speed_max` | `5` | Maximum drop speed in rows per frame. |
| `quit_key` | `q` | Single-character quit key. |
| `color_mode` | `auto` | `auto` generates a gradient from `color_base`; `manual` uses `colors`. |
| `color_base` | `#ffffff` | Base color for `auto` mode, in `#rrggbb` format. |
| `colors` | — | Comma-separated manual palette. Setting this switches `color_mode` to `manual`. |
| `use_xterm256` | `false` | Quantize colors to the built-in xterm-256 palette instead of using exact RGB. |

## Colors and terminals

By default, `rain` asks the terminal to set exact RGB colors through
`init_color`. This gives the best palette fidelity and works in terminals such
as:

- ghostty
- kitty
- wezterm
- xterm
- alacritty

Some terminals report color-changing support but ignore the actual RGB palette
changes. This can happen in konsole, some SSH sessions, tmux setups, and older
terminal emulators.

Typical symptom: drops appear in unexpected ANSI colors, often blue-ish, instead
of the configured palette.

Use this option to avoid terminal palette mutation:

```ini
use_xterm256 = true
```

With `use_xterm256 = true`, colors are mapped to the fixed xterm-256 palette.
This avoids OSC-4/custom-palette behavior and is more portable across terminals
that support 256 colors.

The trade-off is slightly lower color fidelity, especially for dim or
low-chroma colors that map into the grayscale ramp.

## CLI

```text
rain [--config <path>] [--init-config [--force]]
```

| Option | Description |
|---|---|
| `--config <path>` | Load configuration from `<path>`. |
| `--init-config` | Write the documented default config to the standard location, or to `<path>` if used with `--config`. |
| `--force` | Allow `--init-config` to overwrite an existing file. |

## Tests

```sh
make test
```

The test suite runs against a ncurses stub, so it does not require a real
terminal.

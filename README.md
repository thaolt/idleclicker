# Idle Clicker

A simple, cross-platform auto-clicker designed for idle games. Built with C and Raylib, it provides a lightweight GUI and a global hotkey to toggle clicking regardless of which window is focused.

![Idle Clicker Screenshot](idleclicker.png)

## Features

- **Cross-Platform**: Supports Linux and Windows.
- **Global Hotkey**: Toggle clicking on/off using the **F8** key even when the app is minimized or in the background.
- **Adjustable Interval**: Change click speed from 50ms to 2000ms.
- **Visual Status**: Clear indication of whether the clicker is active or stopped.

## Controls

- **F8**: Toggle auto-clicking system-wide.
- **+/- Buttons**: Increase or decrease the click interval by 10ms.
- **Quit**: Exit the application.

## Building from Source

### Prerequisites

- **Linux**: `gcc`, `make`, `xxd`, and X11 development libraries (`libx11-dev`, `libxtst-dev`, `libxi-dev`).
- **Docker**: Optional, for cross-compiling or isolated builds.

### Linux Build

To build natively on Linux:

```bash
make
```

The executable will be located at `build/idleclicker`.

To build using Docker (ensures a consistent environment):

```bash
make linux
```

### Windows Build

To build for Windows (using Docker and MinGW):

```bash
make windows
```

The executable will be located at `build/idleclicker.win32.exe`.

## Installation (Linux)

To install the application, desktop entry, and icon:

```bash
sudo make install
```

By default, it installs to `/usr/local`. You can specify a different prefix:

```bash
sudo make install PREFIX=/usr
```

## Attributions

* [Click](https://icons8.com/icon/23547/natural-user-interface) icon by [Icons8](https://icons8.com)
# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

nolifebar is a Unix status bar generator that creates modular status bars using dzen2 as the backend. It's written in C and bash, using autotools for building, and includes Nix flake support for development.

## Build Commands

### Standard Build

Just run `make`. But you can run full command when need 
```bash
autoreconf --install
./configure
make
```

### Testing
```bash
make test                   # Run all tests (bash script tests)
make test-nolifebar-fn-xft  # Test font functionality
```

### Testing Install Target
To test the install target without system-wide installation, use a temporary directory:
```bash
autoreconf --install
./configure --prefix=/tmp/nolifebar-test
make
make install
# This installs to /tmp/nolifebar-test/
```

## Architecture

### Core Components

- **C Programs** (`src/`): Core utilities for X11 integration, multiplexing, and data processing
  - `nolifebar-multiplexer.c` - Merges multiple named pipes and formats output
  - `nolifebar-xtitle.c` - Gets current window title
  - `nolifebar-xworkspaces.c` - Workspace status using XCB
  - `nolifebar-pa-status.c` - PulseAudio volume/mute status
  - `nolifebar-kbd.c` - Keyboard layout and caps/num status
  - `nolifebar-throttle.c` - Throttles STDIN for performance
  - `nolifebar-replacer.c` - Text replacement engine

- **Bash Modules** (`lib/nolifebar/mod_*`): Individual status modules that output data
  - Each module is a self-contained bash script
  - Modules follow a lifecycle: `mod_init_vars` → `mod_init_draw` → `mod_loop` → `mod_draw`
  - Configuration overrides via `MOD_*_init_vars()` functions

- **Core Library** (`lib/nolifebar/functions`): Shared bash functions and dzen2 drawing utilities
  - Drawing functions: `dzen2_draw_stacked_bar`, `dzen2_draw_h_stacked_bars`, `dzen2_draw_flags`
  - Utility functions: color generation, dependencies checking, positioning

### Module System

Each module must:
1. Source `functions` and `$RESOURCE_FILE` (config)
2. Implement `mod_init_vars()`, optional draw functions, and `mod_loop()`
3. Call `mod_run` to start the lifecycle
4. Write output to `$FIFO_FILE`

Configuration is done in `cfg-top-bar.rc` where `CFG_MODULES` defines which modules to load and `MOD_*_init_vars()` functions customize behavior.

### Build System

- **autoconf/automake**: Main build system with conditional compilation
- **configure.ac**: Defines optional components that can be disabled (e.g., `--disable-nolifebar-multiplexer`)
- **Nix flake**: Development environment with all dependencies including custom dzen2 fork
- **Dependencies**: X11, XFT, XCB libraries, optionally PulseAudio

### Key Directories

- `src/` - C source files for core utilities
- `lib/nolifebar/` - Bash modules and core functions
- `bin/` - Shell scripts and entry points
- `share/nolifebar/` - Static resources (icons, etc.)
- `test/` - Test specifications for bash functions

### Configuration

- `cfg-top-bar.rc` - Main configuration file defining modules, themes, and module-specific settings
- Theme system with color replacement via `CFG_REPLACE_TXT`

## Development Tips

- **C Program Development**:
  - Run `make` for testing changes in c programs

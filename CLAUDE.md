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
  - Drawing functions: `dzen2_draw_stacked_bar`, `dzen2_draw_h_stacked_bars`, `dzen2_draw_centered_bars`, `dzen2_draw_flags`
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

## Drawing Functions Reference

The core library provides four main drawing functions for creating dzen2-compatible visual elements:

### `dzen2_draw_stacked_bar`

Creates horizontal stacked progress bars. Used for memory usage, disk usage, etc.

**Function signature:**
```bash
dzen2_draw_stacked_bar result_var [options]
```

**Parameters:**
- `-width N` - Bar width in pixels (default: 100)
- `-height N` - Bar height in pixels (default: 4)
- `-max_value N` - Maximum value for percentage calculation (default: 100)
- `-values "v1,v2,v3"` - Comma-separated values to display
- `-fg_colors "c1,c2,c3"` - Comma-separated foreground colors (default: `CFG_DEFAULT_STACK_COLORS`)
- `-padding N` - Padding between segments (default: 1)
- `-bg color` - Background color (default: `<WIDGET_BG>`)
- `-draw_outline` - Draw outline around the bar

**Example usage:**
```bash
# From mod_mem - memory usage bar
dzen2_draw_stacked_bar mem_bar \
    -width $((width - c_popup_line_height)) \
    -height $((c_popup_line_height / 2)) \
    -draw_outline \
    -fg_colors '<POPUP_BAR1>,<POPUP_BAR2>,<POPUP_BAR3>,<POPUP_BAR4>' \
    -values "${zfs_arc_size},${mem_used_without_arc_buffers},${mem_buf_and_shared},${mem_cached}" \
    -padding 1 \
    -max_value "${mem_total}"
```

### `dzen2_draw_h_stacked_bars`

Creates vertical stacked bars arranged horizontally. Used for CPU usage per core.

**Function signature:**
```bash
dzen2_draw_h_stacked_bars result_var [options]
```

**Parameters:**
- `-bar_width N` - Width of each bar (default: 4)
- `-height N` - Height of bars (default: `CFG_HEIGHT - 2`)
- `-bottom_margin N` - Bottom margin (default: 1)
- `-max_value N` - Maximum value for scaling (default: 100)
- `-stacks N` - Number of stacks per bar (default: 1)
- `-values "v1,v2,v3"` - Comma-separated values
- `-fg_colors "c1,c2,c3"` - Colors for each stack level
- `-bar_padding N` - Padding between bars (default: 1)
- `-stack_padding N` - Padding between stacks (default: 1)
- `-bg color` - Background color (default: `<WIDGET_BG>`)
- `-border_fg color` - Border color (default: `<WIDGET_BORDER>`)
- `-text string` - Text overlay
- `-text_fg color` - Text color (default: `<WIDGET_TEXT_FG>`)
- `-text_fn font` - Text font (default: `<WIDGET_TEXT_FN>`)

**Example usage:**
```bash
# From mod_cpu_bars - CPU core usage
dzen2_draw_h_stacked_bars cores_stats_dzen \
    "${CFG_DEFAULT_STACK_BAR_PARAMS[@]}" \
    -bar_width 3 \
    -text "CPU ${total_cpu_perc}%" \
    "${c_additional_core_params[@]}" \
    -max_value 1000 \
    -stacks 4 \
    -values "$result_cores"
```

### `dzen2_draw_centered_bars`

Creates Y-centered bars with upper and lower sections. Used for network I/O visualization.

**Function signature:**
```bash
dzen2_draw_centered_bars result_var upper_values_array lower_values_array [options]
```

**Parameters:**
- `-bar_width N` - Width of each bar (default: 2)
- `-height N` - Total height (default: `CFG_HEIGHT - 2`)
- `-max_value N` - Maximum value for scaling (default: 100)
- `-fg_colors "c1,c2"` - Colors for lower and upper bars
- `-bar_padding N` - Padding between bars (default: 1)
- `-stack_padding N` - Padding between upper/lower stacks (default: 1)
- `-bg color` - Background color (default: `<WIDGET_BG>`)
- `-border_fg color` - Border color (default: `<WIDGET_BORDER>`)
- `-text string` - Text overlay
- `-text_fg color` - Text color (default: `<WIDGET_TEXT_FG>`)
- `-text_fn font` - Text font (default: `<WIDGET_TEXT_FN>`)

**Example usage:**
```bash
# From mod_network - network I/O history
dzen2_draw_centered_bars output \
    scaled_rx \
    scaled_tx \
    "${CFG_DEFAULT_CENTERED_BARS_PARAMS[@]}" \
    -text "NetIO" \
    "${c_additional_chart_params[@]}" \
    -height "$c_chart_height" \
    -max_value "$log_scale_max_value" \
    -stack_padding "$c_chart_stack_padding"
```

### `dzen2_draw_flags`

Creates flag-style indicators with ON/OFF states. Used for status indicators.

**Function signature:**
```bash
dzen2_draw_flags result_var short_names_dict status_dict keys_array [options]
```

**Parameters:**
- `-label string` - Label text (default: "")
- `-label_size N` - Label width (default: 45)
- `-label_highlight 0|1` - Highlight label when active (default: 0)
- `-color_on color` - Color when any flag is ON (default: `<WIDGET_ACTIVE>`)
- `-color_all_off color` - Color when all flags are OFF (default: `<WIDGET_INACTIVE>`)
- `-color_bg color` - Background color for flags (default: `<WIDGET_FLAG_BG>`)
- `-color_fg color` - Foreground color for flags (default: `<WIDGET_FLAG_FG>`)
- `-name_size N` - Width of each flag (default: 50)
- `-y_padding N` - Vertical padding (default: 2)
- `-fn_main font` - Main font (default: `<FN_MAIN>`)
- `-fn_2rows font` - Two-row font (default: `<FN_2_ROWS>`)
- `-fn_2rows_y_offset N` - Y offset for two-row font (default: 0)
- `-flag_align alignment` - Flag text alignment (default: `_CENTER`)
- `-flag_x_padding N` - Horizontal padding in flags (default: 1)

**Example usage:**
```bash
# From mod_network - network status flags
dzen2_draw_flags metrics \
    g_metric_formatted \
    g_metric_alerts \
    "${selected_metrics}" \
    -label "NET" \
    "${CFG_DEFAULT_DRAW_FLAG_PARAMS[@]}" \
    -name_size 60 \
    -flag_align _LEFT \
    -flag_x_padding 3
```

### Color Theming

All drawing functions support theme integration through color placeholders:
- `<WIDGET_BG>` - Widget background
- `<WIDGET_BORDER>` - Widget border
- `<WIDGET_TEXT_FG>` - Widget text foreground
- `<POPUP_BAR1>`, `<POPUP_BAR2>`, etc. - Sequential bar colors
- `<WIDGET_ACTIVE>`, `<WIDGET_INACTIVE>` - State colors

Colors are resolved through the `nolifebar-replacer` tool using `$CFG_REPLACER_FILE`.

## Development Tips

- **C Program Development**:
  - Run `make` for testing changes in c programs

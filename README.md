A Unix way and easy-to-use tool for generating status bars.

![Build Status](https://github.com/osv/nolifebar/actions/workflows/build-and-test.yml/badge.svg)
![GitHub License](https://img.shields.io/github/license/osv/nolifebar)

# BUILD

    autoreconf --install
    ./configure
    make
    
# INSTALL

    autoreconf --install
    ./configure --prefix=/usr/local
    make
    sudo make install

# Module life cycle

Below is example for module called `mod_time` with `CFG_BACKEND=dzen2`:

Each module must evaluate the config file (`$RESOURCE_FILE`) to apply the configuration.
`MOD_*` is prefix of config functions for certain modules.

Each module must call `mod_run` at the end, which will:

1. Call the next functions if they are defined in the following sequence:

| Function                   | Where It Must Be Defined |
|----------------------------|--------------------------|
| `mod_init_vars`            | Module                   |
| `mod_init_dzen2_vars`      | Module                   |
| `MOD_TIME_init_vars`       | Config file              |
| `MOD_TIME_init_dzen2_vars` | Config file              |
|----------------------------|--------------------------|
| `mod_init_draw`            | Module                   |
| `mod_init_dzen2_draw`      | Module                   |
| `MOD_TIME_init_draw`       | Config file              |
| `MOD_TIME_init_dzen2_draw` | Config file              |

2. Use first exist draw function:

| Function              | Where It Must Be Defined |
|-----------------------|--------------------------|
| `MOD_TIME_dzen2_draw` | Config file              |
| `MOD_TIME_draw`       | Config file              |
| `mod_dzen2_draw`      | Module                   |
| `mod_draw_default`    | Module                   |

3. Call `mod_loop` that must call `mod_draw` for draw.

Example of minimal `time` module:

``` sh
: "${MODULE_CORE_DIR:=$(dirname $0)}"
source "$MODULE_CORE_DIR/functions"
[ -f "$RESOURCE_FILE" ] && source "$RESOURCE_FILE"

mod_init_vars() {
    format="%(%H:%M)T"
}

mod_draw_default() {
    printf "${format}\n" -1
}

mod_loop() {
    while true; do
        mod_draw > "$FIFO_FILE"
        sleep 1
    done
}

mod_run
```

Example of config file

``` sh
MOD_TIME_init_vars() {
    format="Time: %(%H:%M)T"
}
```

# Building

## Requirements

- C11 compiler (`gcc` or `clang`)
- `meson` >= 0.56
- `ninja`

## Build

```sh
meson setup builddir
cd builddir
ninja
```

## Options

| Option           | Default | Description                          |
|------------------|---------|--------------------------------------|
| `systemd_compat` | `true`  | Build systemd `.service` file parser |

```sh
meson setup builddir -Dsystemd_compat=false
```

## Tests

```sh
meson test
```

## Install

Installs `neoinit` to `/sbin/neoinit`.

```sh
ninja install
```

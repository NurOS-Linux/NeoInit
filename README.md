# neoinit

![License](https://img.shields.io/badge/license-GPL--3.0-blue)
![Language](https://img.shields.io/badge/language-C11-informational)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![Build](https://img.shields.io/badge/build-meson-success)

Microkernel-style init system for Linux.

## Quick start

```sh
meson setup builddir && cd builddir && ninja
meson test
ninja install
```

## Documentation

- [Building](docs/building.md)
- [Service format](docs/services.md)
- [Signals](docs/signals.md)

## Authors

See [CONTRIBUTORS.md](CONTRIBUTORS.md).

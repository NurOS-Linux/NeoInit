# raesir

![License](https://img.shields.io/badge/license-GPL--3.0-blue)
![Language](https://img.shields.io/badge/language-C11-informational)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![Build](https://img.shields.io/badge/build-meson-success)

Microkernel-style init system for Linux.

## Requirements

- C11 compiler (`gcc` or `clang`)
- `meson` >= 0.56
- `ninja`

## Building

```sh
meson setup builddir
cd builddir
ninja
```

Run tests:

```sh
meson test
```

Install:

```sh
ninja install
```

Installs `raesir` to `/sbin/raesir` and `servctl` to `/usr/bin/servctl`.

## Build options

| Option               | Default                    | Description                                    |
|----------------------|----------------------------|------------------------------------------------|
| `systemd_compat`     | `true`                     | Build systemd `.service` file parser           |
| `services_dir`       | `/etc/raesir/services`    | Directory to scan for service definitions      |
| `default_path`       | *(system default)*         | Default `PATH` for services                    |
| `container_detection`| `true`                     | Skip reboot and unmount in container environments |

Example — disable systemd compatibility:

```sh
meson setup builddir -Dsystemd_compat=false
```

## Service definitions

Services are loaded from `/etc/raesir/services/` at boot in alphabetical order.

YAML format:

```yaml
name: myservice
description: My service
exec: /usr/bin/myservice --flag
working_dir: /var/lib/myservice
restart: true
type: simple
env:
  - FOO=bar
```

See [docs/services.md](docs/services.md) for full field reference.

## Runtime control

`servctl` communicates with the running init via `/run/raesir.sock`:

```sh
servctl status          # show init status
servctl list            # list all services
servctl start <name>    # start a stopped service
servctl stop <name>     # stop a running service
```

## Signal handling

| Signal    | Effect                          |
|-----------|---------------------------------|
| `SIGCHLD` | Reap zombie processes           |
| `SIGTERM` | Sync, unmount, power off        |
| `SIGINT`  | Sync, unmount, reboot           |
| `SIGHUP`  | Ignored                         |
| `SIGPIPE` | Ignored                         |

The kernel delivers `SIGINT` to PID 1 on Ctrl+Alt+Del.

## Documentation

- [Building](docs/building.md)
- [Service format](docs/services.md)
- [Signals](docs/signals.md)
- [Roadmap](ROADMAP.md)
- [Changelog](CHANGELOG.md)

## Authors

See [CONTRIBUTORS.md](CONTRIBUTORS.md).

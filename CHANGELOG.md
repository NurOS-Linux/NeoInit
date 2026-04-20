# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-04-20

### Added

- PID 1 main loop with `ppoll`-based signal-safe multiplexing
- Essential filesystem mounting (`proc`, `sysfs`, `devtmpfs`, `tmpfs` on `/run`)
- YAML service definitions (`.yaml`, `.yml`)
- systemd `.service` compatibility parser as optional build module (`-Dsystemd_compat=true`)
- runit service directory compatibility parser as optional build module (`-Drunit_compat=true`)
- Process launching with environment variables and working directory
- `simple` and `oneshot` service types
- Automatic service restart on failure with 100 ms throttle
- Zombie reaping via `SIGCHLD`
- Signal handling: `SIGTERM` → poweroff, `SIGINT` → reboot
- Unix domain socket control interface at `/run/neoinit.sock`
- Commands over control socket: `status`, `list`, `start`, `stop`
- Service registry with lookup by name and PID
- `servctl` utility with shell completions (bash, zsh, fish) and a man page
- Container detection via `container` environment variable
- Build options: `services_dir`, `default_path`, `container_detection`, `systemd_compat`, `runit_compat`
- Unit tests for all core modules
- `ROADMAP.md` with planned milestones up to v1.0.0
- `CHANGELOG.md` (this file)

### Fixed

- `oneshot` services now block startup correctly until the process exits before the next service is launched

### Changed

- systemd parser moved from `src/service_sd.c` into `modules/systemd/service_sd.c` as a standalone source module
- `meson.build`: systemd sources now collected into `systemd_module_sources` variable before `subdir('src')`
- `src/meson.build`: `service_sd.c` removed from hardcoded source list; `systemd_module_sources` appended instead

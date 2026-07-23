# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-07-23

### Added

- End-to-end integration test covering TOML, YAML and systemd definitions,
  dependency leveling, registry state and real process launch
- `raesir(8)` man page
- Versioned control socket protocol; `protocol` command reports the current
  version (1)
- Security hardening: control socket mode 0600 and peer credential
  verification via `SO_PEERCRED`

### Fixed

- Restart policy no longer resurrects services stopped manually via
  `servctl stop`

## [0.6.0] - 2026-07-23

### Added

- TTY allocation for console services: `tty` key in TOML/YAML, `TTYPath=` in
  `.service` units; the service gets its own session and controlling terminal
- getty-style respawn for console login services when combined with `restart`

## [0.5.0] - 2026-07-23

### Added

- Each service runs in its own cgroup v2 under `/sys/fs/cgroup/raesir`
- Memory and CPU limits: `memory_max` and `cpu_weight` keys in TOML/YAML,
  `MemoryMax=` and `CPUWeight=` in `.service` units
- Clean cgroup teardown when a service exits or is stopped

## [0.4.0] - 2026-07-23

### Added

- File-based logging to `/var/log/raesir.log` with size-based rotation
- Per-service stdout/stderr capture to `/var/log/raesir/<name>.log`
- Runtime log level filtering via the `loglevel` control command
- Optional structured log output (key=value pairs)

## [0.3.0] - 2026-07-23

### Added

- `servctl restart <name>` with graceful SIGTERM/SIGKILL escalation
- `servctl reload <name>` sends `SIGHUP` to a running service
- Runtime `enable`/`disable` without editing definition files
- `rescan` command loads new service definitions at runtime
- Configurable restart delay (`restart_delay_ms`, `RestartSec=`) and restart
  limit (`restart_max`, `StartLimitBurst=`) per service

## [0.2.0] - 2026-07-23

### Added

- Service dependency declarations: `after` and `requires` fields in YAML;
  `After=`, `Requires=` and `Wants=` in `.service` files
- Topological dependency leveling at startup with cycle detection
- Parallel startup of independent services within the same level
- `wants` weak dependencies that do not block dependents on failure
- Hard `requires` enforcement: dependents are skipped when a required
  dependency failed to start

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
- Unix domain socket control interface at `/run/raesir.sock`
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

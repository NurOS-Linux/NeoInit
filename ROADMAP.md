# Roadmap

## v0.1.0

- [x] PID 1 main loop with `ppoll`-based multiplexing
- [x] Essential filesystem mounting (`proc`, `sysfs`, `devtmpfs`, `run`)
- [x] YAML service definitions
- [x] systemd `.service` compatibility parser (conditional module)
- [x] Process launching with environment and working directory
- [x] `simple` and `oneshot` service types
- [x] Automatic restart on failure
- [x] Zombie reaping via `SIGCHLD`
- [x] Signal handling: `SIGTERM` → poweroff, `SIGINT` → reboot
- [x] Unix socket control interface (`/run/raesir.sock`)
- [x] `servctl` utility: `status`, `list`, `start`, `stop`
- [x] Container detection (`container` env variable)
- [x] Modular build: `modules/systemd` as optional source module

## v0.2.0 — service ordering and parallel startup

- [x] Service dependency declarations (`after`, `requires` in YAML; `After=`, `Requires=`, `Wants=` in `.service`)
- [x] Topological sort at startup
- [x] Parallel startup of independent services
- [x] `wants` weak dependency (failure does not block dependents)

## v0.3.0 — runtime management

- [x] `servctl restart <name>`
- [x] `servctl reload <name>` (send `SIGHUP`)
- [x] Runtime service enable/disable without editing files
- [x] Load new service definitions at runtime without full restart
- [x] Configurable restart delay and max-restart count per service

## v0.4.0 — logging

- [x] File-based logging (`/var/log/raesir.log`) with rotation support
- [x] Per-service stdout/stderr capture to log files
- [x] Log levels configurable at build and runtime
- [x] Optional structured log output (key=value pairs)

## v0.5.0 — cgroups and resource control

- [x] Place each service in its own cgroup v2 hierarchy
- [x] Memory and CPU limits via YAML (`memory_max`, `cpu_weight`)
- [x] Clean cgroup teardown on service stop
- [x] Extend systemd compat: `MemoryMax=`, `CPUWeight=`

## v0.6.0 — TTY and console

- [x] TTY allocation for services that require a terminal (`tty: true`)
- [x] `getty`-style respawn for console login services
- [x] VT switching awareness

## v1.0.0 — stable release

- [x] All v0.x.0 features stable and documented
- [x] Comprehensive integration test suite (real PID 1 in a VM or namespace)
- [x] Man pages for `raesir(8)` and `servctl(1)`
- [x] Stable control socket protocol with versioning
- [x] Security hardening: socket permissions, capability dropping

# Roadmap

## v0.1.0 — current

- [x] PID 1 main loop with `ppoll`-based multiplexing
- [x] Essential filesystem mounting (`proc`, `sysfs`, `devtmpfs`, `run`)
- [x] YAML service definitions
- [x] systemd `.service` compatibility parser (conditional module)
- [x] Process launching with environment and working directory
- [x] `simple` and `oneshot` service types
- [x] Automatic restart on failure
- [x] Zombie reaping via `SIGCHLD`
- [x] Signal handling: `SIGTERM` → poweroff, `SIGINT` → reboot
- [x] Unix socket control interface (`/run/neoinit.sock`)
- [x] `servctl` utility: `status`, `list`, `start`, `stop`
- [x] Container detection (`container` env variable)
- [x] Modular build: `modules/systemd` as optional source module

## v0.2.0 — service ordering and parallel startup

- [ ] Service dependency declarations (`after`, `requires` in YAML; `After=`, `Requires=`, `Wants=` in `.service`)
- [ ] Topological sort at startup
- [ ] Parallel startup of independent services
- [ ] `wants` weak dependency (failure does not block dependents)

## v0.3.0 — runtime management

- [ ] `servctl restart <name>`
- [ ] `servctl reload <name>` (send `SIGHUP`)
- [ ] Runtime service enable/disable without editing files
- [ ] Load new service definitions at runtime without full restart
- [ ] Configurable restart delay and max-restart count per service

## v0.4.0 — logging

- [ ] File-based logging (`/var/log/neoinit.log`) with rotation support
- [ ] Per-service stdout/stderr capture to log files
- [ ] Log levels configurable at build and runtime
- [ ] Optional structured log output (key=value pairs)

## v0.5.0 — cgroups and resource control

- [ ] Place each service in its own cgroup v2 hierarchy
- [ ] Memory and CPU limits via YAML (`memory_max`, `cpu_weight`)
- [ ] Clean cgroup teardown on service stop
- [ ] Extend systemd compat: `MemoryMax=`, `CPUWeight=`

## v0.6.0 — TTY and console

- [ ] TTY allocation for services that require a terminal (`tty: true`)
- [ ] `getty`-style respawn for console login services
- [ ] VT switching awareness

## v1.0.0 — stable release

- [ ] All v0.x.0 features stable and documented
- [ ] Comprehensive integration test suite (real PID 1 in a VM or namespace)
- [ ] Man pages for `neoinit(8)` and `servctl(1)`
- [ ] Stable control socket protocol with versioning
- [ ] Security hardening: socket permissions, capability dropping

# Signals

| Signal    | Effect                                      |
|-----------|---------------------------------------------|
| `SIGCHLD` | Reap zombie processes                       |
| `SIGTERM` | Sync, unmount filesystems, power off        |
| `SIGINT`  | Sync, unmount filesystems, reboot           |
| `SIGHUP`  | Ignored                                     |
| `SIGPIPE` | Ignored                                     |

The kernel sends `SIGINT` to PID 1 on Ctrl+Alt+Del.

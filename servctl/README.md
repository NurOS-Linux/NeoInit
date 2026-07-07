# servctl

![License](https://img.shields.io/badge/license-GPL--3.0-blue)
![Language](https://img.shields.io/badge/language-C11-informational)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![Build](https://img.shields.io/badge/build-meson-success)

Control utility for **raesir** init system.

## Usage

```sh
servctl status
servctl list
servctl start <service>
servctl stop <service>
servctl restart <service>
```

## Protocol

`servctl` communicates with `raesir` via a Unix Domain Socket at `/run/raesir.sock`. It uses a simple text-based protocol:

1. Client connects to the socket.
2. Client sends a command string (e.g., `status`).
3. `raesir` processes the command and writes the response back to the socket.
4. Client reads the response until EOF and prints it.

## Build

```sh
meson setup builddir && cd builddir && ninja
ninja install
```

## Authors

See [CONTRIBUTORS.md](../CONTRIBUTORS.md).

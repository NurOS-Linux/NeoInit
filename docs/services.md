# Service definitions

neoinit reads service definitions from `/etc/neoinit/services/` at boot.
Files are processed in alphabetical order.

## YAML format (`.yaml`, `.yml`)

```yaml
name: myservice
description: My service
exec: /usr/bin/myservice --flag "quoted arg"
working_dir: /var/lib/myservice
restart: true
type: simple
env:
  - FOO=bar
  - BAZ=qux
```

| Field         | Required | Values               | Default  |
|---------------|----------|----------------------|----------|
| `exec`        | yes      | command + arguments  |          |
| `name`        | no       | string               | filename |
| `description` | no       | string               |          |
| `working_dir` | no       | path                 |          |
| `restart`     | no       | `true` / `false`     | `false`  |
| `type`        | no       | `simple` / `oneshot` | `simple` |
| `env`         | no       | list of `KEY=VALUE`  |          |

`exec` supports quoted arguments and backslash-escaped spaces.

`oneshot` — neoinit waits for the process to exit before launching the next service.
`simple` — process is launched and left running.

## systemd compatibility (`.service`)

Available when built with `-Dsystemd_compat=true` (default).

Recognised keys:

| Key                | Section    | Maps to          |
|--------------------|------------|------------------|
| `Description`      | `[Unit]`   | `description`    |
| `ExecStart`        | `[Service]`| `exec`           |
| `WorkingDirectory` | `[Service]`| `working_dir`    |
| `Environment`      | `[Service]`| `env` entry      |
| `Restart`          | `[Service]`| `restart`        |
| `Type`             | `[Service]`| `type`           |

All other keys and sections are ignored.

```ini
[Unit]
Description=My service

[Service]
ExecStart=/usr/bin/myservice --flag
WorkingDirectory=/var/lib/myservice
Environment=FOO=bar
Restart=on-failure
Type=simple
```

`Restart=no` disables restart. Any other value enables it.

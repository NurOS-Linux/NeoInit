# Service definitions

raesir reads service definitions from `/etc/raesir/services/` at boot.
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
after:
  - network
requires:
  - database
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
| `after`       | no       | list of service names |         |
| `requires`    | no       | list of service names |         |

`exec` supports quoted arguments and backslash-escaped spaces.

`after` and `requires` name other services by their `name` field (or filename
without extension, if `name` is not set). `after` is ordering only. `requires`
is a hard dependency. Both are parsed and stored on the service definition;
raesir does not yet act on them — startup order still follows alphabetical
file order until topological sort lands (see [ROADMAP.md](../ROADMAP.md)).

`oneshot` — raesir waits for the process to exit before launching the next service.
`simple` — process is launched and left running.

## systemd compatibility (`.service`)

Available when built with `-Dsystemd_compat=true` (default).

Recognised keys:

| Key                | Section    | Maps to          |
|--------------------|------------|------------------|
| `Description`      | `[Unit]`   | `description`    |
| `After`            | `[Unit]`   | `after` entries  |
| `Requires`         | `[Unit]`   | `requires` entries |
| `Wants`            | `[Unit]`   | `wants` entries  |
| `ExecStart`        | `[Service]`| `exec`           |
| `WorkingDirectory` | `[Service]`| `working_dir`    |
| `Environment`      | `[Service]`| `env` entry      |
| `Restart`          | `[Service]`| `restart`        |
| `Type`             | `[Service]`| `type`           |

All other keys and sections are ignored.

```ini
[Unit]
Description=My service
After=network
Requires=database
Wants=cache

[Service]
ExecStart=/usr/bin/myservice --flag
WorkingDirectory=/var/lib/myservice
Environment=FOO=bar
Restart=on-failure
Type=simple
```

`Restart=no` disables restart. Any other value enables it.

`After`, `Requires` and `Wants` each accept a space-separated list of service
names and may be repeated; repeated occurrences accumulate rather than
overwrite. As with the YAML fields, these are parsed and stored but not yet
enforced at startup.

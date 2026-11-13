# resourced

`resourced` is a Qt6/C++ implementation of the Maemo/Nemo resource manager daemon.
It replaces the older OHM-based resource manager with a smaller daemon built on
Qt6 and D-Bus, without Prolog or heavy external dependencies.

## Features

- Client registration and lifecycle management
- Resource granting, release, and re-evaluation
- Higher-priority preemption
- Shared resource handling via share/mask
- Security policy for allowed senders
- Configuration file parsing
- D-Bus protocol for register/acquire/release/unregister/update/audio/video
- Client cleanup on disconnect

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Tests

```bash
ctest --test-dir build
```

## Install

```bash
sudo cmake --install build
```

This installs:

- `/usr/bin/resourced`
- `/etc/resourced.conf`
- `/etc/dbus-1/system.d/org.maemo.resource.manager.conf`
- `/usr/lib/systemd/user/resourced.service`

## Run

```bash
sudo systemctl restart dbus
sudo resourced
```

For debug output:

```bash
sudo QT_LOGGING_RULES="org.glacier.resourced.debug=true" resourced
```

## Configuration

Example configuration is provided in `config/resourced.conf`:

```ini
[General]
DaemonName=resourced
DBusService=org.maemo.resource.manager
DBusPath=/org/maemo/resource/manager
LogFile=/var/log/resourced.log
LogLevel=Info
DefaultClientPriority=10
Bus=System

[Security]
AllowedSenders=org.nemomobile.*

[Preemption]
EnablePreemption=true
```

## D-Bus service

- Service: `org.maemo.resource.manager`
- Path: `/org/maemo/resource/manager`
- Interface: `org.maemo.resource.manager`

## License

See `LICENSE`.
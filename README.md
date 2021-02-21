# eg25-manager - Quectel EG25 management daemon

`eg25-manager` is a daemon for managing the Quectel EG25 modem found on the
Pine64 PinePhone.

It implements the following features:
  * cleanly power on/off the modem
  * configure/check essential parameters (such as the audio format) on startup
  * monitor the modem state through ModemManager
  * put the modem in low-power mode when suspending the system, and restore it
    back to normal behavior when resuming
  * monitor the modem state on resume and recover it if needed

## Dependencies

`eg25-manager` requires the following development libraries:
- libglib2.0-dev
- libgpiod-dev
- libmm-glib-dev

## Building

`eg25-manager` uses meson as its build system. Building and installing
`eg25-manager` is as simple as running the following commands:

```
$ meson ../eg25-build
$ ninja -C ../eg25-build
# ninja -C ../eg25-build install
```

## Configuration

`eg25-manager` uses device-specific configuration files, named after the
device-tree `compatible` field. These files are installed to
`/usr/share/eg25-manager`. They can be copied to `/etc/eg25-manager` then
modified, that way they won't be overwritten during an upgrade.

## Running

`eg25-manager` is usually run as a systemd service, but can also be
manually started from the command-line (requires root privileges):

```
# eg25manager
```

## License

`eg25-manager` is licensed under the GPLv3+.

ios-sim
=======

Supports Xcode 6 only since version 3.x.

The ios-sim tool is a command-line utility that launches an iOS application on
the iOS Simulator. This allows for niceties such as automated testing without
having to open Xcode.

Features
--------

* Choose the device family to simulate, i.e. iPhone or iPad. Run using "showdevicetypes" option to see available device types, and pass it in as the "devicetypeid" parameter.
* Setup environment variables.
* Pass arguments to the application.
* See the stdout and stderr, or redirect them to files.

See the `--help` option for more info.

Usage
-----

```
Usage: ios-sim <command> <options> [--args ...]

Commands:
  showsdks                        List the available iOS SDK versions
  showdevicetypes                 List the available device types (Xcode6+)
  launch <application path>       Launch the application at the specified path on the iOS Simulator
  start                           Launch iOS Simulator without an app

Options:
  --version                       Print the version of ios-sim
  --help                          Show this help text
  --verbose                       Set the output level to verbose
  --exit                          Exit after startup
  --debug                         Attach LLDB to the application on startup
  --use-gdb                       Use GDB instead of LLDB. (Requires --debug)
  --uuid <uuid>                   A UUID identifying the session (is that correct?)
  --env <environment file path>   A plist file containing environment key-value pairs that should be set
  --setenv NAME=VALUE             Set an environment variable
  --stdout <stdout file path>     The path where stdout of the simulator will be redirected to (defaults to stdout of ios-sim)
  --stderr <stderr file path>     The path where stderr of the simulator will be redirected to (defaults to stderr of ios-sim)
  --timeout <seconds>             The timeout time to wait for a response from the Simulator. Default value: 30 seconds
  --args <...>                    All following arguments will be passed on to the application
  --devicetypeid <device type>    The id of the device type that should be simulated (Xcode6+). Use 'showdevicetypes' to list devices.
                                  e.g "com.apple.CoreSimulator.SimDeviceType.Resizable-iPhone6, 8.0"
DEPRECATED in 3.x, use devicetypeid instead:
  --sdk <sdkversion>              The iOS SDK version to run the application on (defaults to the latest)
  --family <device family>        The device type that should be simulated (defaults to `iphone')
  --retina                        Start a retina device
  --tall                          In combination with --retina flag, start the tall version of the retina device (e.g. iPhone 5 (4-inch))
  --64bit                         In combination with --retina flag and the --tall flag, start the 64bit version of the tall retina device (e.g. iPhone 5S (4-inch 64bit))
```

Installation
------------

With node.js (at least 0.10.20):

    $ npm install ios-sim -g

Download an archive:

    $ curl -L https://github.com/phonegap/ios-sim/zipball/{{VERSION}} -o ios-sim-{{VERSION}}.zip
    $ unzip ios-sim-{{VERSION}}.zip

Or from a git clone:

    $ git clone git://github.com/phonegap/ios-sim.git

Then build and install from the source root:

    $ rake install prefix=/usr/local/

Troubleshooting
------------

Make sure you enable Developer Mode on your machine:
    
    $ DevToolsSecurity -enable

Make sure multiple instances of launchd_sim are not running:
    
    $ killall launchd_sim
    
Development
-----------

When you want to release a version, do:

    $ rake version:bump v=NEW_VERSION
    $ rake release
    $ npm version NEW_VERSION -m "Updated to npm version %s"
    
tmux
-----

To get ios-sim to launch correctly within tmux use the reattach-to-user-namespace wrapper.

```
reattach-to-user-namespace ios-sim launch ./build/MyTestApp.app
```
*source:* https://github.com/ChrisJohnsen/tmux-MacOSX-pasteboard

*brew:*  ```brew install reattach-to-user-namespace```

License
-------

Original author: Landon Fuller <landonf@plausiblelabs.com>
Copyright (c) 2008-2011 Plausible Labs Cooperative, Inc.
All rights reserved.

This project is available under the MIT license. See [LICENSE][license].

[license]: https://github.com/phonegap/ios-sim/blob/master/LICENSE

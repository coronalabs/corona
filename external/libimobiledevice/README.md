Corona Labs fork of libimobiledevice


```
git clone git@github.com:coronalabs/submodule-ideviceinstaller.git ideviceinstaller
git clone git@github.com:coronalabs/submodule-libimobiledevice.git libimobiledevice
git clone git@github.com:coronalabs/submodule-libplist.git libplist
git clone git@github.com:coronalabs/submodule-libusbmuxd.git libusbmuxd
./build.sh
```

The build.sh script configures and makes each repo and copies the needed build products to the device-support directory and then optionally to a final install location.  It is used by the Simulator Xcode project.

This script also edits the generated object files so that they look for their support libraries in a subdirectory called "lib" next to the executables.  This allows us to ensure that we always use our fixed versions of the libimobiledevice libraries rather than whatever happens to be installed on the end user's machine.

TODO:
migrate build.sh to use pkg-config and copy brew's lib[openssl/zip/plist]

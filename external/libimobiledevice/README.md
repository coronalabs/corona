Corona Labs fork of libimobiledevice

The four subrepos here are clones of Git repos on Github which are forks of the upstream originals.  Various bugs are fixed and minor features added.  To fetch / merge with the upstream install hggit on a developer machine (the original Git repos are not used to avoid Git/hggit dependencies on the build machines).

Git repos:

* git://github.com/coronalabs/ideviceinstaller.git
* git://github.com/coronalabs/libimobiledevice.git
* git://github.com/coronalabs/libplist.git
* git://github.com/coronalabs/libusbmuxd.git

The build.sh script configures and makes each repo and copies the needed build products to the device-support directory and then optionally to a final install location.  It is used by the Simulator Xcode project.

This script also edits the generated object files so that they look for their support libraries in a subdirectory called "lib" next to the executables.  This allows us to ensure that we always use our fixed versions of the libimobiledevice libraries rather than whatever happens to be installed on the end user's machine.

TODO:
migrate build.sh to use pkg-config and copy brew's lib[openssl/zip/plist]

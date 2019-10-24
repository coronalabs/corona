# How to Build Corona's Android Software

___Note:___ gradle builds are still work in progress. Check out `master` branch for current info

## Setup

To run and debug Corona for Android create directory `platform/test/assets2` and put your project there (`platform/test/assets2/main.lua` must exist).

To check out repository on proper branch use this command:
```bash
git clone --recurse-submodules -j8 --branch=gradle https://github.com/coronalabs/corona.git
```

If you already have [repo](https://github.com/coronalabs/corona) checked out, you can switch to `gradle` branch:
```bash
git checkout gradle
git submodule update --init -j8
```

Option `-j8` does work in parallel, but may be not supported on older versions of Git. It can be deleted safely.

## Android Studio

Currently Corona for Android is built with Gradle build system. It is well integrated with Android Studio or IntelliJ IDEA (pro or community) which can be used as IDE. To do this open, start the IDE, in welcome Window click "Open existing Android Studio project" and select `platform/android` directory.

## Command line builds

To build from command line, you have to have Android SDK adn NDK set up.
```bash
export ANDROID_SDK=~/Library/Android/sdk
export ANDROID_NDK=~/Library/Android/sdk/ndk-bundle
./gradlew assemble
```


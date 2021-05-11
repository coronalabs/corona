# How to Build Solar2D's Android Software

## Setup

To check out repository on proper branch use this command:
```bash
git clone --recurse-submodules -j8 https://github.com/coronalabs/corona.git
```

To run and debug Solar2D for Android create directory `platform/test/assets2` and put your project there (`platform/test/assets2/main.lua` must exist).

## Android Studio

Currently Solar2D for Android is built with Gradle build system. It is well integrated with Android Studio or IntelliJ IDEA (pro or community) which can be used as IDE. To do this open, start the IDE, in welcome Window click "Open existing Android Studio project" and select `platform/android` directory.

__WARNING:__ Do not update Android Plugin if requested. It would break the build.

## Command line builds

To build from command line, you have to have Android SDK adn NDK set up.
```bash
export ANDROID_SDK=~/Library/Android/sdk
export ANDROID_NDK=~/Library/Android/sdk/ndk-bundle
./gradlew assemble
```


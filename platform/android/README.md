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

### Setting up plugins

To set up all plugins in `build.settings` run `setUpCoronaAppAndPlugins` Gradle task. This will download all plugins and rewrite the `AndroidManifest.xml` with values required provided by plugins. To run Gradle task from Android Studio press "Execute Gradle Task" button, which has icon of an elephant, in Gradle tool window (View -> Tool Windows -> Gradle).

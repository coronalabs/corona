---
name: App is not building
about: Issues while building an app
title: ''
labels: ''
assignees: ''

---

Self help steps before reporting the issue:
[ ] Check [Solar2D forums](https://forums.solar2d.com/) for a solution
[ ] If Android: make sure Java8 is installed and reported when running `java -version` and `javac -version` if not, download and install [Java 8](https://www.oracle.com/java/technologies/javase/javase-jdk8-downloads.html)

**Before reporting**
A clear and concise description of what the bug is.

**Platforms involved**
Building on: macOS 10-15-16/Windows 7-8-10/
Building for: tvOS/iOS/macOS/Windows/HTML5/Linux/Android

**Attach your build.settings**

**Full build log**
Enable full build log:
On macOS:
```
defaults write com.coronalabs.Corona_Simulator debugBuildProcess  -int 5
```
On Windows:
```
reg ADD "HKEY_CURRENT_USER\Software\Ansca Corona\Corona Simulator\Preferences" /f /v debugBuildProcess /d 5
```

Before the build clean console and then copy log between backticks below
```
```

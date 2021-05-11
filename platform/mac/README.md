# macOS build Guide

To run and test Simulator open `ratatouille.xcodeproj` and select `rttplayer` target in Xcode. Press play.
Sometimes Xcode doesn't follow dependency order for tools, so you may have to build `Lua` and `Luac` targets first.

# Building your app with simulator

Note, that updated code would only work in Simulator. Device builds (like iOS, Android or HTML5) require special templates to be built first.

## Building iOS/tvOS apps

First one would need to build templates. To see how, check out the [README](../iphone/README.md) in `iphone` directory.
When templates are built and in place, open `CoronaBuilder.xcodeproj`. Then select `CoronaBuilder` target and build it (⌘B). When it is built you can (in the same project) select `rttplayer` target and run it. Now you should be able to build iOS applications from newly built Simulator.

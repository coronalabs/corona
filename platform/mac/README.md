# macOS build Guide

To run Corona Simulator open `ratatouille.xcodeproj` and select `rttplayer` target in Xcode. Press play.
Sometimes Xcode doesn't follow dependency order for tools, so you may have to build `Lua` and `Luac` targets first.

Note, that updated code would only work in Simulator. If you want to build iOS or Android app from this Simulator,
you would have to have uploaded patched templates to the server.

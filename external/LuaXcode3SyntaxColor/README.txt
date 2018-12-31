Syntax highlighting for Lua in Xcode 3

Place Lua.pbfilespec and Lua.xclangspec in:
~/Library/Application Support/Developer/Shared/Xcode/Specifications/
and reboot Xcode. May need to initially make a manual association of 
existing project script files using View-SyntaxColoring-Lua. For new 
projects the association is automatic.

See files for history and prior credits

The Lua.xclangspec code can definitely be improved, but as it is, 
it works reasonably well. It can get confused on chunk scope 
when encountering lines with multiple statements that include "end".

Graham Henstridge
graham@capgo.com
4 Dec 2007 

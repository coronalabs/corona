#!/bin/bash

# The first argument to this script should be the directory, e.g.
# The second arg is the name of the app
# ./build_mac.sh fishies fishies

path=`dirname $0`


$path/lua $path/PackageApp.lua -a $path -d com.anscamobile.$2 $1/assets $1

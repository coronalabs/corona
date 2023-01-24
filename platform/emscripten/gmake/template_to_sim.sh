#!/bin/bash -ex

./build_template.sh -nc "$@"

APP_PATH=$(osascript -e 'tell application "System Events" to POSIX path of (file of process "Corona Simulator" as alias)' 2>/dev/null || true)

if [ -z "$APP_PATH" ]
then
	APP_PATH="$(readlink ~/Library/Application\ Support/Corona/Native)../Corona Simulator.app"
fi

cp ../webtemplate.zip "$APP_PATH/Contents/Resources/webtemplate.zip"


#!/bin/bash
mkdir -p "${HOME}/Library/Application Support/Corona/Android Build/sdk/licenses"
cp ../sdk/licenses/android-sdk-license "${HOME}/Library/Application Support/Corona/Android Build/sdk/licenses/"
echo "sdk.dir=${HOME}/Library/Application Support/Corona/Android Build/sdk" > local.properties

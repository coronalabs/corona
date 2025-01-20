#!/bin/bash
if [ "$ANDROID_HOME" ]
then
    echo "sdk.dir=${ANDROID_HOME}" > local.properties
    mkdir -p "${ANDROID_HOME}/licenses"
    (echo '' ; cat ../sdk/licenses/android-sdk-license ; echo '') >> "${ANDROID_HOME}/licenses/android-sdk-license"
    cp "${ANDROID_HOME}/licenses/android-sdk-license" "${ANDROID_HOME}/licenses/android-sdk-license.bak"
    sort < "${ANDROID_HOME}/licenses/android-sdk-license.bak" | uniq > "${ANDROID_HOME}/licenses/android-sdk-license"
    rm "${ANDROID_HOME}/licenses/android-sdk-license.bak"
else
    mkdir -p "${HOME}/Library/Application Support/Corona/Android Build/sdk/licenses"
    cp ../sdk/licenses/android-sdk-license "${HOME}/Library/Application Support/Corona/Android Build/sdk/licenses/"
    echo "sdk.dir=${HOME}/Library/Application Support/Corona/Android Build/sdk" > local.properties
fi

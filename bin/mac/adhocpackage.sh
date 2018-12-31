#!/bin/bash


APP_NAME="Redline"
CONFIG="AdHoc"
BUILD_DIR="${CONFIG}-iphoneos"

# del old app
rm -rf "${APP_NAME}.ipa"

# make tmp directory
mkdir -p tmp/Payload
cd tmp

# copy artwork for iTunes
cp ../iTunesArtwork.png iTunesArtwork

# copy app
cp -rp "../build/${BUILD_DIR}/${APP_NAME}.app" Payload/

# zip
zip -r ${APP_NAME}.zip iTunesArtwork Payload

# rename so iTunes recognises this bundle
mv ${APP_NAME}.zip ../${APP_NAME}.ipa

# back and del tmp files
cd ..
rm -rf tmp
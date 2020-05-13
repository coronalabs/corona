#!/bin/bash

# builds output.zip for iPhone

# $1 = app name
# $2 = path to template.app
# $3 = path to tmp build dir
# $4 = path to certificate file
# $5 = path to input zip file // main.lu file (or src file used to create message digest)
# $6 = "little" or "big" (endianness of file)

# assign tmp dir to $dst
dst="$3"

path=`dirname $0`

#uuid=`uuidgen`
#dst="$3/$uuid"

#mkdir $dst

appName="$1"
appNameWithoutSpaces=`echo -n "$1" | sed "s/ //g"`

appDir="$appNameWithoutSpaces.app"
appPath="$dst/$appDir"

#echo "Using app dir: $appPath"

# copy template.app into $dst, renaming it to $appDir
cp -pR $2 "$appPath"
exePath="$appPath/$appNameWithoutSpaces"
echo "$appPath/template $exePath"
mv "$appPath/template" "$exePath"

# Unpack input zip file into the app bundle
unzip -o $5 -d $appPath
$path/car "$appPath/resource.car" "$appPath"/*.lu

rm -f "$appPath"/*.lu

# create output.zip in $dst
cd $dst
zip -rym output.zip "$appDir"
cd -


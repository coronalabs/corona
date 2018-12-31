#!/bin/bash

path=`dirname $0`

IDENTITY=$1
APP=$2
APP_ID=$3
PROVISION_PROFILE=$4

if [[ -z "$1" || -z "$2" || -z "$3" || -z "$4" ]]
then
        echo "USAGE: $0 signing_identity path_to_app app_id path_to_provision_profile"
		echo "Re codesign an existing app"
        exit 0
fi

echo "$0 Identity( $IDENTITY )   App( $APP )   AppId( $APP_ID )   Provision( $PROVISION_PROFILE )"

# Create temporary entitlements file
ENTITLEMENTS="/tmp/recodesign_entitlements.xcent"
if [[ -f "$ENTITLEMENTS" ]]
then
	rm -f $ENTITLEMENTS
fi

#sed -E "s/ARG0/$APP_ID/" "$path/template.xcent" > $ENTITLEMENTS

GET_TASK_ALLOW=0

if [[ $IDENTITY =~ "iPhone Developer:".* ]]
then
	GET_TASK_ALLOW=1
fi

defaults write "$APP/Info" CFBundleIdentifier $APP_ID

"$path/lua" "$path/generateXcent.lua" "$ENTITLEMENTS" "$IDENTITY" $APP_ID

cp -v "$PROVISION_PROFILE" "$APP/embedded.mobileprovision"

# Codesign the file
"$path/codesign.sh" "$ENTITLEMENTS" "$APP" "$IDENTITY"


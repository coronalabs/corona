#!/bin/bash
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

# xcode-sim.sh
#
# Start an app in a specified Xcode iOS/tvOS Simulator 
#
# The durations of the various "sleep"s are empirically determined and will probably change
# or become unnecessary with future versions of Xcode
#

DEBUG_BUILD_PROCESS=$(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null)

exec 2>&1

if [ "${DEBUG_BUILD_PROCESS:=0}" -gt 1 ]
then
	set -x
fi

trap 'kill -TERM 0' EXIT

# Our scheme relies on there being just one booted Xcode Simulator, this
# function shuts down any running simulators (ones in the "Booted" state)
function shutdownAll()
{
	BOOTED=$(xcrun simctl list devices | sed -ne '/Booted/s/.*(\([^)]*\).*(.*)/\1/p')

	for B in $BOOTED
	do
		echo "Shutting down $B ..."
		xcrun simctl shutdown "$B"
	done
}

APP="$1"
UDID="$2"

if [ "$UDID" == "" ] || [ ! -r "$APP" ]
then
	echo "ERROR: Usage: $(basename "$0") application.app xcode-sim-udid"
	exit 1
fi

SIMULATOR_APP="$(xcode-select -p)/Applications/Simulator.app"

BUNDLE_ID=$(plutil  -convert json "$APP"/Info.plist -o - -r |grep CFBundleIdentifier | sed -e 's/.* : "\([^"]*\)",/\1/')

# If the Simulator we want is already running and is the only one running, we'll try to use it
ISRUNNING=$(2>&1 xcrun simctl list devices | grep -c "($UDID) (Booted)")
NUMBOOTED=$(2>&1 xcrun simctl list devices | grep -c "(Booted)")

if [ "$ISRUNNING" != "1" ] || [ "$NUMBOOTED" != "1" ]
then
	echo "Restarting Xcode Simulator ..."
	shutdownAll

	killall 'Simulator'
	sleep 2

	# there doesn't seem to be a way to start the iOS Simulator UI from simctl,
	# all it does is start the device with no window so we need to run this
	# command to get a device booted with a graphical interface
	open -a "$SIMULATOR_APP" --args -CurrentDeviceUDID "$UDID"
fi


# Pause until our device boots to the point it can uninstall an app
# This is done by trying to uninstall a fake app until we get a response
# that indicates the simulated device is responding to commands (rather 
# the request just failing)
loopcount=1
count=$(2>&1 xcrun simctl uninstall "$UDID" fake.app.id | grep -E -c 'Unable to lookup|Invalid')
while [ "$count" -gt 0 ]
do
	# Put a cap on how long we'll wait
	if [[ $loopcount -gt 100 ]]
	then
		echo "ERROR: Can't start Xcode iOS/tvOS Simulator. Restarting your computer may fix this"
		exit 1
	fi

	# If we've been waiting too long, start over
	if [[ $((loopcount % 20)) == 0 ]]
	then
		echo "Restarting Xcode Simulator ..."
		killall 'Simulator'
		sleep 2
		open -a "$SIMULATOR_APP" --args -CurrentDeviceUDID "$UDID"
	fi
	loopcount=$((loopcount + 1))

    sleep 1
	count=$(2>&1 xcrun simctl uninstall "$UDID" fake.app.id | grep -E -c 'Unable to lookup|Invalid')
done

# Remove the app and reinstall it
xcrun simctl uninstall "$UDID" "$BUNDLE_ID"
xcrun simctl install "$UDID" "$APP"

# Reset the Simulator's syslog
echo > ~/"Library/Logs/CoreSimulator/$UDID/system.log"

# Necessary for some Simulators to avoid "FBSOpenApplicationErrorDomain" errors
sleep 2

# Launch the app
xcrun simctl launch "$UDID" "$BUNDLE_ID"

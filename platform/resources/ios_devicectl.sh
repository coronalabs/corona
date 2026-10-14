#!/bin/bash
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

# Support for sending apps to iOS and tvOS devices with Xcode's devicectl (Xcode 15 and later), sourced by
# ios_sendapp.sh and ios_syslog.sh.  Unlike the libimobiledevice tools in device-support, which are Intel only,
# devicectl runs natively on Apple silicon and reaches devices over the network as well as USB.  It doesn't
# support devices running iOS 16 or earlier.

DEVICECTL=$(xcrun --find devicectl 2>/dev/null)

# devicectl's output isn't just ASCII (device names like "Scott’s iPad" for one)
export LANG=en_US.UTF-8

# Reads the JSON written by "devicectl list devices" and prints "<identifier><tab><name>" for each device that
# can run apps for the given platform: USB connected ones first, then the most recently connected
DEVICECTL_SELECT_DEVICES_JS='
function run(argv) {
	const json = $.NSString.stringWithContentsOfFileEncodingError(argv[0], $.NSUTF8StringEncoding, null)
	const devices = JSON.parse(json.js).result.devices.filter(device => {
		const hardware = device.hardwareProperties || {}, connection = device.connectionProperties || {}
		return hardware.reality === "physical" && hardware.platform === argv[1] &&
			connection.transportType !== undefined && connection.tunnelState !== "unavailable" &&
			(connection.pairingState === "paired" || connection.transportType === "wired") &&
			! connection.isMobileDeviceOnly
	})
	const wired = device => device.connectionProperties.transportType === "wired" ? 0 : 1
	const lastConnected = device => device.connectionProperties.lastConnectionDate || ""
	devices.sort((a, b) => wired(a) - wired(b) || lastConnected(b).localeCompare(lastConnected(a)))
	return devices.map(device => device.identifier + "\t" + device.deviceProperties.name).join("\n")
}
'

# Sets DEVICECTL_DEVICES to the devices ("<identifier><tab><name>") to send an app to for the target OS ("iPhone OS"
# or "Apple TVOS"): the first one found unless the Simulator is set to send apps to all devices.  Fails if devicectl
# isn't available or finds no devices.
devicectl_find_devices()
{
	local PLATFORM=iOS SEND_TO_ALL JSON DEVICE

	DEVICECTL_DEVICES=()

	if [ ! -x "$DEVICECTL" ]
	then
		return 1
	fi

	if [ "$1" == "Apple TVOS" ]
	then
		PLATFORM=tvOS
	fi

	JSON=$(mktemp -t corona-devicectl) || return 1

	"$DEVICECTL" list devices --quiet --timeout 60 --json-output "$JSON" >/dev/null 2>&1

	while IFS= read -r DEVICE
	do
		if [ -n "$DEVICE" ]
		then
			DEVICECTL_DEVICES[${#DEVICECTL_DEVICES[@]}]="$DEVICE"
		fi
	done < <(osascript -l JavaScript -e "$DEVICECTL_SELECT_DEVICES_JS" "$JSON" "$PLATFORM" 2>/dev/null)

	rm -f "$JSON"

	SEND_TO_ALL=$(defaults read com.coronalabs.Corona_Simulator sendToAllDevices 2>/dev/null)

	if [ "${#DEVICECTL_DEVICES[@]}" -gt 1 ] && [ "$SEND_TO_ALL" != 1 ] && [ "$SEND_TO_ALL" != "YES" ]
	then
		DEVICECTL_DEVICES=( "${DEVICECTL_DEVICES[0]}" )
	fi

	[ "${#DEVICECTL_DEVICES[@]}" -gt 0 ]
}

# The identifier and name parts of an entry in DEVICECTL_DEVICES
devicectl_device_id()
{
	echo "${1%%	*}"
}

devicectl_device_name()
{
	echo "${1#*	}"
}

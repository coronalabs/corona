#!/bin/bash
#
# This script copies Corona ticket information from ~/Library/Preferences to /Library/Preferences.
# You must run Corona Simulator once and successfully log in before running this script.
# The purpose is for a sysadmin to enable use of Corona by any non-privileged user on that machine.
#

USERNAME=$(defaults read $HOME/Library/Preferences/com.coronalabs.Corona_Simulator Username)
CREDENTIALS=$(defaults read $HOME/Library/Preferences/com.coronalabs.Corona_Simulator $USERNAME)
TICKET=$(defaults read $HOME/Library/Preferences/com.coronalabs.Corona_Simulator Resource)

if [[ "$USERNAME" == "" || "$CREDENTIALS" == "" || "$TICKET" == "" ]]
then
	echo "Authentication information is not set in preferences. Run Corona Simulator and log in."
	exit -1
fi

defaults write /Library/Preferences/com.coronalabs.Corona_Simulator Username "$USERNAME"
defaults write /Library/Preferences/com.coronalabs.Corona_Simulator "$USERNAME" $CREDENTIALS
defaults write /Library/Preferences/com.coronalabs.Corona_Simulator Resource $TICKET

chmod 644 /Library/Preferences/com.coronalabs.Corona_Simulator.plist

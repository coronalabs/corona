#!/bin/bash
set -e

# Xcode Build Phase

# Script: codesign embedded framework
# -----------------------------------------------------------------------------

# Codesign
codesign_framework() {
	FRAMEWORK_BINARY=$1

	if [ -z "${DEVELOPER_BASE}" ]
	then
		# "/Applications/Xcode.app/Contents/Developer"
		DEVELOPER_BASE=$(xcode-select -p)
	fi

	# Exports per Xcode 7.1
	export CODESIGN_ALLOCATE="${DEVELOPER_BASE}/Toolchains/XcodeDefault.xctoolchain/usr/bin/codesign_allocate"
	export PATH="${DEVELOPER_BASE}/Platforms/AppleTVOS.platform/Developer/usr/bin:/Applications/Xcode.app/Contents/Developer/usr/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"

	# --preserve flags per Xcode 7.1
	echo /usr/bin/codesign --force --sign ${EXPANDED_CODE_SIGN_IDENTITY} --preserve-metadata=identifier,entitlements --timestamp=none "${FRAMEWORK_BINARY}"
	/usr/bin/codesign --force --sign ${EXPANDED_CODE_SIGN_IDENTITY} --preserve-metadata=identifier,entitlements --timestamp=none "${FRAMEWORK_BINARY}"
}


# Process args
BINARY_PATH=$1


# Device vs Simulator
if [ "${CODE_SIGNING_REQUIRED}" = "YES" ]
then
	if [ -z "${EXPANDED_CODE_SIGN_IDENTITY}" ]
	then
		echo "ERROR: You need to choose a signing identity in your project"
		exit -1
	fi

	if [ -z "${BINARY_PATH}" ]
	then
		# Default is to sign *all* embedded frameworks in .app bundle
		BINARY_FOLDER=$BUILT_PRODUCTS_DIR/$FRAMEWORKS_FOLDER_PATH

		echo "Codesign embedded frameworks in folder ($BINARY_FOLDER):"

		for f in "${BINARY_FOLDER}"/*.framework
		do
			if [ -e "$f" ]
			then
				codesign_framework "$f"
			fi
		done
	else
		# Only sign 1 framework
		codesign_framework "${BINARY_PATH}"
	fi
fi


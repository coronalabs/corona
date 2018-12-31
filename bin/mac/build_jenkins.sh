#!/bin/bash -x

path=`dirname $0`

#
# Prints usage
# 
# usage() {
# 	echo ""
# 	echo "USAGE: $0 lua_srcfile dst_dir [DEBUG|RELEASE]"
# 	exit -1
# }


########################################################################################
# Checks exit value for error
########################################################################################
checkError() {
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}

# 
# Canonicalize relative paths to absolute paths
# 
# pushd $SCRIPT_ROOT > /dev/null
# dir=`pwd`
# SCRIPT_ROOT=$dir
# popd > /dev/null
# 
# pushd $DST_DIR > /dev/null
# dir=`pwd`
# DST_DIR=$dir
# popd > /dev/null


# -----------------------------------------------------------------------------

# Walk all of the subrepos and run purge on them since the purge run by jenkins at job start doesn't clean out sub repos. (http://mercurial.808500.n3.nabble.com/PATCH-stable-purge-add-option-to-recurse-subrepos-td3989418.html)
awk '{print $2}' .hgsubstate | xargs -n 1 hg --config extensions.purge= clean --all -R 

# Restore artifacts after syncing/purging
mv ../docs.zip docs.zip

#make sure corona is unmounted
umount /Volumes/CoronaSDK*

#unlock the keychain for signing
security unlock-keychain -p 'zxcvbnm,./' ~/Library/Keychains/login.keychain


env

cd "${WORKSPACE}"

rm -rf docs
mkdir docs
unzip -a docs.zip -d docs
checkError

# -----------------------------------------------------------------------------

#set the build number 
sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_REVISION.*$/#define Rtt_BUILD_REVISION $BUILD_NUMBER/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"
sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_YEAR[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_YEAR $YEAR/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"
sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_MONTH[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_MONTH $MONTH/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"
sed -i .bak -E "s/^#define[[:space:]]*Rtt_BUILD_DAY[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_DAY $DAY/" "${WORKSPACE}/librtt/Core/Rtt_Version.h"

#change the version in the Info.plist for mac simulator and convert it back to xml
defaults write ${WORKSPACE}/platform/mac/Info CFBundleVersion $YEAR.$BUILD_NUMBER
defaults write ${WORKSPACE}/platform/mac/Info CFBundleShortVersionString $YEAR.$BUILD_NUMBER
plutil -convert xml1 ${WORKSPACE}/platform/mac/Info.plist


if [ -e "${WORKSPACE}/../webtemplate.zip" ]
then
    echo "Replacing webtemplate.zip"
    cp "${WORKSPACE}/../webtemplate.zip" "${WORKSPACE}/platform/resources/webtemplate.zip"
else
    echo "WARNING: No webtemplate.zip found!"
fi

if [ -e "${WORKSPACE}/../linuxtemplate.tar.gz" ]
then
    echo "Replacing linuxtemplate.tar.gz"
    cp "${WORKSPACE}/../linuxtemplate.tar.gz" "${WORKSPACE}/platform/resources/linuxtemplate.tar.gz"
else
    echo "WARNING: No linuxtemplate.tar.gz found!"
fi

if [ -e "${WORKSPACE}/../raspbiantemplate.tar.gz" ]
then
    echo "Replacing raspbiantemplate.tar.gz"
    cp "${WORKSPACE}/../raspbiantemplate.tar.gz" "${WORKSPACE}/platform/resources/raspbiantemplate.tar.gz"
else
    echo "WARNING: No raspbiantemplate.tar.gz found!"
fi

echo Building: $Release

if [ "$Release" = "release" ]
then
    # build production release
    bin/mac/build_dmg.sh -b "$YEAR.$BUILD_NUMBER" -e "${WORKSPACE}/../CoronaEnterprise-${YEAR}.${BUILD_NUMBER}.tgz" "${WORKSPACE}" "${WORKSPACE}/docs"
    checkError
    mkdir release
    mv CoronaSDK-$YEAR.$BUILD_NUMBER-release.dmg release/

# -----------------------------------------------------------------------------
else
    # build daily build
    bin/mac/build_dmg.sh -d -b "$YEAR.$BUILD_NUMBER" -c "$CUSTOM_ID" -s "$S3_BUCKET"  -e "${WORKSPACE}/../CoronaEnterprise-${YEAR}.${BUILD_NUMBER}.tgz" "${WORKSPACE}" "${WORKSPACE}/docs"
    checkError
fi

# -----------------------------------------------------------------------------

#hg id -i
zip -v -y -r CoronaSDK-dsym-$YEAR.$BUILD_NUMBER.zip platform/mac/build/Release/*.dSYM 


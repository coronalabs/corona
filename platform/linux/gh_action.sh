#!/bin/bash
set -ex

sudo snap install snapcraft --classic
sudo lxd init --minimal

rm -rf docs/SampleCode/.git docs/SampleCode/.gitignore

cp -v Webtemplate/webtemplate.zip platform/resources/webtemplate.zip

tar -xvzf Native/CoronaNative.tar.gz CoronaEnterprise/Corona/android/resource/android-template.zip CoronaEnterprise/Corona/android/lib/gradle/Corona.aar



SNAPDIR=platform/linux/snapcraft/snap
sed -i "s|2100|${YEAR:=2100}|" $SNAPDIR/snapcraft.yaml
if [[ "${BUILD_NUMBER:=9999}" == "9999" ]]
then
    sed -i "s|stable|devel|"  $SNAPDIR/snapcraft.yaml
    if [ "$GITHUB_SHA" ]
    then
         sed -i "s|9999|9999.${GITHUB_SHA::7}|" $SNAPDIR/snapcraft.yaml
    fi
else
    sed -i "s|9999|$BUILD_NUMBER|" $SNAPDIR/snapcraft.yaml
fi


cp -Rv $SNAPDIR ./
snapcraft --use-lxd
cp -v ./*.snap "${WORKSPACE}"/output

FS2D="$(mktemp -d)"
sudo mount -t squashfs -o ro ./*.snap "$FS2D"
cp -v "$FS2D/usr/local/bin/Solar2D/Resources/linuxtemplate_x64.tgz" "${WORKSPACE}"/output
sudo umount "$FS2D"

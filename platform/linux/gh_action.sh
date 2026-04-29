#!/bin/bash
(
set -ex

cd "$(dirname "$0")/../.."

sudo snap install snapcraft --classic
sudo lxd init --minimal


rm -rf docs/SampleCode/.git docs/SampleCode/.gitignore

if [ -e Webtemplate/webtemplate.zip ]
then
    cp -v Webtemplate/webtemplate.zip platform/resources/webtemplate.zip
fi

if [ -e Native/CoronaNative.tar.gz ]
then
    tar -xvzf Native/CoronaNative.tar.gz CoronaEnterprise/Corona/android/resource/android-template.zip CoronaEnterprise/Corona/android/lib/gradle/Corona.aar
fi


rm -rf ./*.snap ./snap
cp -Rv platform/linux/snapcraft/snap ./

sed -i "s|2100|${YEAR:=2100}|" snap/snapcraft.yaml
if [[ "${BUILD_NUMBER:=9999}" == "9999" ]]
then
    sed -i "s|stable|devel|"  snap/snapcraft.yaml
    if [ "$GITHUB_SHA" ]
    then
         sed -i "s|9999|9999.${GITHUB_SHA::7}|" snap/snapcraft.yaml
    fi
else
    sed -i "s|9999|$BUILD_NUMBER|" snap/snapcraft.yaml
fi


sudo snapcraft --use-lxd

mkdir -p output
cp -v ./*.snap output/

FS2D="$(mktemp -d)"
sudo mount -t squashfs -o ro ./*.snap "$FS2D"
cp -v "$FS2D/usr/local/bin/Solar2D/Resources/linuxtemplate_x64.tgz" output/
sudo umount "$FS2D"
)
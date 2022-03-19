#!/bin/bash
(
set -ex

cd "$(dirname "$0")/../.."

sudo apt install flatpak

# Add the Flathub repository
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

flatpak install flathub org.flatpak.Builder
flatpak install flathub org.freedesktop.Platform//21.08 org.freedesktop.Sdk//21.08

rm -rf docs/SampleCode/.git docs/SampleCode/.gitignore

if [ -e Webtemplate/webtemplate.zip ]
then
    cp -v Webtemplate/webtemplate.zip platform/resources/webtemplate.zip
fi

if [ -e Native/CoronaNative.tar.gz ]
then
    tar -xvzf Native/CoronaNative.tar.gz CoronaEnterprise/Corona/android/resource/android-template.zip CoronaEnterprise/Corona/android/lib/gradle/Corona.aar
fi


rm -rf ./*.flatpak
mkdir -p .flatpak
cp -Rv platform/linux/flatpak/* ./.flatpak
cd .flatpak

YML="com.solar2d.simulator.yml" 

sed -i "s|2100|${YEAR:=2100}|" "${YML}"
if [[ "${BUILD_NUMBER:=9999}" == "9999" ]]
then
    sed -i "s|stable|devel|"  "${YML}"
    if [ "$GITHUB_SHA" ]
    then
         sed -i "s|9999|9999.${GITHUB_SHA::7}|" "${YML}"
    fi
else
    sed -i "s|9999|$BUILD_NUMBER|" "${YML}"
fi


flatpak-builder --force-clean flat-build "${YML}"

# put in a repo
#flatpak-builder --repo=repo --force-clean build "${YML}"

mkdir -p output
#cp -v ./*.snap output/

#FS2D="$(mktemp -d)"
#sudo mount -t squashfs -o ro ./*.snap "$FS2D"
#cp -v "$FS2D/usr/local/bin/Solar2D/Resources/linuxtemplate_x64.tgz" output/
#sudo umount "$FS2D"

)

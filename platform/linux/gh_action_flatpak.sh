#!/bin/bash
(
set -ex

cd "$(dirname "$0")/../.."

sudo apt install flatpak -y

# Add the Flathub repository
sudo flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

sudo flatpak install flathub org.flatpak.Builder -y
sudo flatpak install flathub org.freedesktop.Platform//21.08 -y
sudo flatpak install org.freedesktop.Sdk//21.08 -y

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

# build flat app and put it in a repo
flatpak run org.flatpak.Builder --force-clean --repo repo build-dir "${YML}"

# Create a single-file bundle from a local repository
flatpak build-bundle ./repo solar2d.flatpak com.solar2d.simulator 

cp -v solar2d.flatpak ../

)

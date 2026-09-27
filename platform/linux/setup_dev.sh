#!/bin/bash 
set -e


if [ -x "$(command -v apk)" ]; then
#  sudo apk add --no-cache $packagesNeeded
  echo "You must manually install dependencies"
elif [ -x "$(command -v apt-get)" ]; then

  sudo apt-get update
#  Core build tools and modern Java
  sudo apt-get install -y cmake ninja-build build-essential libtinfo6 openjdk-17-jdk-headless openjdk-17-jre-headless
#  Graphics, audio, and media dependencies
  sudo apt-get install -y zlib1g-dev libgl1-mesa-dev libglu1-mesa-dev libopenal-dev libfreetype6-dev libpng-dev
  sudo apt-get install -y libcurl4-openssl-dev libjpeg-dev libssl-dev libvorbis-dev libogg-dev uuid-dev
#  GTK, WebKit, Readline, and SDL2
  sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libwebkit2gtk-4.1-dev libgtk-3-dev libreadline-dev libsdl2-dev
#  wxWidgets
  sudo apt-get install -y libwxgtk3.2-dev

elif [ -x "$(command -v dnf)" ]; then
#  sudo dnf install $packagesNeeded
  echo "You must manually install dependencies"
elif [ -x "$(command -v zypper)" ]; then 
#  sudo zypper install $packagesNeeded
  echo "You must manually install dependencies"
else
  echo "FAILED TO INSTALL PACKAGE: Package manager not found. You must manually install: $packagesNeeded">&2; 
fi


echo "In order to build for Android, you need to install Android Studio, install Android Api level 28 via the SDK manager and accept the license agreements."
echo "Then you can build via Solar2D for Android."

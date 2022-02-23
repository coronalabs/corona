#!/bin/bash 


if [ -x "$(command -v apk)" ]; then
#  sudo apk add --no-cache $packagesNeeded
  echo "You must manually install dependencies"
elif [ -x "$(command -v apt-get)" ]; then

  sudo apt-get update
  sudo apt-get install -y cmake ninja-build dos2unix libtinfo5 openjdk-8-jdk-headless openjdk-8-jre-headless
  sudo apt-get install -y build-essential libwxgtk3.0-gtk3-dev zlib1g-dev libgl1-mesa-dev libglu1-mesa-dev libopenal-dev libfreetype6-dev libpng-dev libcrypto++-dev
  sudo apt-get install -y libcurl4-openssl-dev libpng-dev libjpeg-dev libssl-dev libvorbis-dev libogg-dev uuid-dev zlib1g-dev 
  sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libwebkit2gtk-4.0-dev libgtk-3-dev
  sudo apt-get install -y libsdl2-dev

  # install libreadline7_7
  TEMP_DEB="$(mktemp)" &&
  wget -O "$TEMP_DEB" 'http://mirrors.kernel.org/ubuntu/pool/main/r/readline/libreadline7_7.0-3_amd64.deb' &&
  sudo dpkg -i "$TEMP_DEB"
  rm -f "$TEMP_DEB"

  # install wxWidgets-3.1.4
  wxurl=https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.4/wxWidgets-3.1.4.tar.bz2
  wxtar=~/Downloads/wxWidgets-3.1.4.tar.bz2
  if [ -f "$wxtar" ]; then
    echo "Using existing wxWidgets"
  else
    cd ~/Downloads
    wget "$wxurl"
    tar -xf "$wxtar" -C ~

    cd ~/wxWidgets-3.1.4
    mkdir buildgtk
    cd buildgtk
    ../configure --with-gtk
    make -j4
    sudo make install
  fi

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

#!/bin/bash
set -ex

echo Install dependencies
# The Linux and macOS virtual machines both run using passwordless sudo
sudo apt-get update
sudo snap install snapcraft --classic
# sudo snap install lxd
sudo lxd init --minimal
sudo apt-get install -y cmake build-essential dos2unix libwxgtk3.0-gtk3-dev zlib1g-dev libgl1-mesa-dev libglu1-mesa-dev \
                                libopenal-dev libfreetype6-dev libpng-dev libcrypto++-dev libcurl4-openssl-dev  \
                                libpng-dev libjpeg-dev libssl-dev libvorbis-dev libogg-dev uuid-dev zlib1g-dev \
                                libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libwebkit2gtk-4.0-dev libgtk-3-dev

TEMP_DEB="$(mktemp)" &&
wget -O "$TEMP_DEB" 'http://mirrors.kernel.org/ubuntu/pool/main/r/readline/libreadline7_7.0-3_amd64.deb' &&
sudo dpkg -i "$TEMP_DEB"
rm -f "$TEMP_DEB"

# install wxWidgets-3.1.4
if [ -f /usr/local/include/wx-3.1/wx/wx.h ]; then
    echo "Using existing wxWidgets"
else
    cd "$WORKSPACE"
    wget "https://github.com/coronalabs/binary-data/releases/download/1.0/wxWidgets-3.1.4.tar.bz2" -O "wxWidgets-3.1.4.tar.bz2"
    tar -xf "wxWidgets-3.1.4.tar.bz2" -C ~/
    cd ~/wxWidgets-3.1.4
    mkdir buildgtk
    cd buildgtk
    ../configure --with-gtk --with-opengl
    make -j4
    sudo make install
fi

echo Build binaries
cd "$WORKSPACE"
rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/tmp/Solar2D ..
make -j4
make install

echo Clean-up docs
cd "$WORKSPACE"
rm -rf docs/SampleCode/.git docs/SampleCode/.gitignore

echo Copy results to output
mkdir -p /tmp/Solar2D/bin/Solar2D/Resources/SampleCode
cp -Rv docs/SampleCode/* /tmp/Solar2D/bin/Solar2D/Resources/SampleCode
mkdir -p output
cd /tmp/Solar2D/bin
tar -czvf "${WORKSPACE}"/output/CoronaSimulator-x86-64.tgz ./
cd Solar2D/Resources
cp -v /tmp/Solar2D/bin/Solar2D/Resources/linuxtemplate_x64.tgz "${WORKSPACE}"/output

echo Build snap
cd "$WORKSPACE"
mkdir -p platform/linux/snapcraft/bin
cp -Rv /tmp/Solar2D/bin/Solar2D/* platform/linux/snapcraft/bin
chmod -R a+r platform/linux/snapcraft/bin/Resources

# Put webtemplate in snap
cp -v Webtemplate/webtemplate.zip platform/linux/snapcraft/bin/Resources/

# Put Android resources in snap
tar -xvzf Native/CoronaNative.tar.gz CoronaEnterprise/Corona/android/resource/android-template.zip          
mkdir -p platform/linux/snapcraft/bin/Resources/Native/Corona/android/resource
cp -v CoronaEnterprise/Corona/android/resource/android-template.zip platform/linux/snapcraft/bin/Resources/Native/Corona/android/resource/
tar -xvzf Native/CoronaNative.tar.gz CoronaEnterprise/Corona/android/lib/gradle/Corona.aar
mkdir -p platform/linux/snapcraft/bin/Resources/Native/Corona/android/lib/gradle
cp -v CoronaEnterprise/Corona/android/lib/gradle/Corona.aar platform/linux/snapcraft/bin/Resources/Native/Corona/android/lib/gradle/

mkdir -p platform/linux/snapcraft/usr/local/lib
mkdir -p platform/linux/snapcraft/usr/local/lib/wx
cp -Rv /usr/local/lib/libwx* platform/linux/snapcraft/usr/local/lib
cp -Rv /usr/local/lib/wx/* platform/linux/snapcraft/usr/local/lib/wx
mkdir -p platform/linux/snapcraft/usr/local/share/aclocal
mkdir -p platform/linux/snapcraft/usr/local/share/locale
cp -Rv /usr/local/share/aclocal/wx* platform/linux/snapcraft/usr/local/share/aclocal
cp --parents `find /usr/local/share/locale -name 'wx*'` platform/linux/snapcraft

mkdir -p platform/linux/snapcraft/usr/local/bin
cp -Rv /usr/local/bin/wx* platform/linux/snapcraft/usr/local/bin
cd platform/linux/snapcraft/usr/local/bin
ln -sf ../lib/wx/config/gtk3-unicode-3.1 wx-config
cd ../../../

# set version
sed -i "s|YEAR|$YEAR|" snap/snapcraft.yaml 
if [[ "$BUILD_NUMBER" == "9999" ]]
then
    sed -i "s|BUILD_NUMBER|$BUILD_NUMBER.${GITHUB_SHA::7}|" snap/snapcraft.yaml
    sed -i "s|stable|devel|"  snap/snapcraft.yaml
else
    sed -i "s|BUILD_NUMBER|$BUILD_NUMBER|" snap/snapcraft.yaml
fi

# build snap
sudo snapcraft --use-lxd
cp -v ./*.snap "${WORKSPACE}"/output

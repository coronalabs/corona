#!/bin/bash -e

# this file is run inside ubundu environment
if [[ "$OSTYPE" != "linux-gnu" ]]
then
    echo "This script is meant to run on Linux"
    exit 1
fi

if [ -d /tmpfs ]
then
    echo "Using tmpfs."
	rm -rf Release
	rm -rf Debug
	rm -rf ARM
	rm -rf Simulator
	mkdir -p /tmpfs/Release /tmpfs/Debug /tmpfs/ARM /tmpfs/Simulator
	ln -s /tmpfs/Release Release
	ln -s /tmpfs/Debug Debug
	ln -s /tmpfs/ARM ARM
	ln -s /tmpfs/Simulator Simulator
else
    echo "tmpfs is not used!"
fi

echo "Lua Scripts Build started..."
echo "=========================="
make -j4 -f lua2cpp.mk

# echo "Simulator Build started..."
# echo "=========================="
# make -j4 -f linux_rtt_simulator.mk

# cp -rf Simulator/linux_rtt deb_simulator/CONTENTS/usr/bin/CoronaSimulator
# cp -rf Simulator/linux_rtt CoronaSimulator/CoronaSimulator

# if [ -d /tmpfs ]
# then
# 	rm Simulator
# fi

# tar -czvf CoronaSimulator-x86-64.tar.gz ./CoronaSimulator


echo "x86/64 Build started..."
echo "======================="
make -j4 -f linux_rtt.mk

#cp Release/linux_rtt deb/CONTENTS/usr/bin/
cp Debug/linux_rtt deb/CONTENTS/usr/bin/

if [ -d /tmpfs ]
then
	rm Release
	rm Debug
fi

cd deb
mkdir -p ./CONTENTS/usr/share/corona/res_widget
cp ../../../subrepos/widget/widget_theme_*.png ./CONTENTS/usr/share/corona/res_widget/
tar -czvf ../linuxtemplate.tar.gz .
cd ../


echo "ARM Build started..."
echo "===================="
make -j4 -f linux_rtt_arm.mk

#cp ARM/linux_rtt deb_arm/CONTENTS/usr/bin/

if [ -d /tmpfs ]
then
	rm ARM
fi

cd deb_arm
mkdir -p ./CONTENTS/usr/share/corona/res_widget
cp ../../../subrepos/widget/widget_theme_*.png ./CONTENTS/usr/share/corona/res_widget/
tar -czvf ../raspbiantemplate.tar.gz .
cd ../

echo "All done!"


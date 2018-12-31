#!/bin/bash
#
# Build the libimobiledevice (http://www.libimobiledevice.org/) suite for the Corona Simulator
#

echo "Building libimobiledevice"

set -e
set -x

if [ "$1" != "" ]
then
	INSTALL_DIR="$1"
fi

BUILD_DIR="$(pwd)/device-support"

rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
mkdir "$BUILD_DIR"/lib

# Configure and build the four components of libimobiledevice that we use

# The processing of the output of autogen.sh is necessary because Xcode thinks
# some normal output is error messages and flags them during a build.

(
	cd libplist/ || exit

	if [ ! -f Makefile ]
	then
		./autogen.sh 2>&1 | grep -F -v 'configure.ac:'
		make clean
	fi

	make

	cp src/.libs/libplist.3.dylib "$BUILD_DIR"/lib
)

(
	cd libusbmuxd/ || exit

	if [ ! -f Makefile ]
	then
		libplist_CFLAGS='-I../../libplist/include' libplist_LIBS='-L../../libplist/src/.libs -lplist' ./autogen.sh  2>&1 | grep -F -v 'configure.ac:'
		make clean
	fi

	make

	cp src/.libs/libusbmuxd.4.dylib "$BUILD_DIR"/lib
)

(
	cd libimobiledevice/ || exit

	# brew install openssl

	if [ ! -f Makefile ]
	then
		# This assumes a brew install of openssl
		openssl_CFLAGS="-I/usr/local/opt/openssl/include" openssl_LIBS="-L/usr/local/opt/openssl/lib -lssl -lcrypto" libusbmuxd_CFLAGS='-I../../libusbmuxd/include' libusbmuxd_LIBS='-L../../libusbmuxd/src/.libs -lusbmuxd' libplist_CFLAGS='-I../../libplist/include' libplist_LIBS='-L../../libplist/src/.libs -lplist' libplistmm_CFLAGS='-I../../libplist/include' libplistmm_LIBS='-L../../libplist/src/.libs -lplist++' ./autogen.sh  2>&1 | grep -F -v 'configure.ac:'
		make clean
	fi
	
	make

	cp src/.libs/libimobiledevice.6.dylib "$BUILD_DIR"/lib
	cp tools/.libs/* "$BUILD_DIR"
)

(
	cd ideviceinstaller/ || exit

	if [ ! -f Makefile ]
	then
		libimobiledevice_CFLAGS='-I../../libimobiledevice/include' libimobiledevice_LIBS='-L../../libimobiledevice/src/.libs -limobiledevice' libplist_CFLAGS='-I../../libplist/include' libplist_LIBS='-L../../libplist/src/.libs -lplist' ./autogen.sh  2>&1 | grep -F -v 'configure.ac:'
		make clean
	fi

	make

	# shellcheck disable=SC2046
	cp $(ls src/.libs/ideviceinstaller) "$BUILD_DIR"
)

cp -a additional_libs/* "$BUILD_DIR"/lib

pushd /usr/local/opt/openssl
OPENSSL_DIR=$(pwd -P)
popd

pushd /usr/local/opt/libzip
LIBZIP_DIR=$(pwd -P)
popd


# This replaces the system library paths with ones relative to the executables
# so that we can make sure we use our own copies of these libraries rather 
# than whatever version happens to be installed on an user's machine
(
	cd "$BUILD_DIR" || exit

	for E in idevice*
	do
		echo "Updating $E..."
		for L in lib/*.dylib
		do
			install_name_tool -change "/usr/local/$L" "@executable_path/$L" "$E"
			install_name_tool -change "/usr/local/opt/openssl/$L" "@executable_path/$L" "$E"
			install_name_tool -change "/usr/local/opt/libzip/$L" "@executable_path/$L" "$E"
			install_name_tool -change "$OPENSSL_DIR/$L" "@executable_path/$L" "$E"
			install_name_tool -change "$LIBZIP_DIR/$L" "@executable_path/$L" "$E"
		done
	done

	for L in lib/*.dylib
	do
		echo "Updating $L..."
		for LL in lib/*.dylib
		do
			install_name_tool -change "/usr/local/$LL" "@executable_path/$LL" "$L"
			install_name_tool -change "/usr/local/opt/openssl/$LL" "@executable_path/$LL" "$L"
			install_name_tool -change "/usr/local/opt/libzip/$LL" "@executable_path/$LL" "$L"
			install_name_tool -change "$OPENSSL_DIR/$L" "@executable_path/$L" "$E"
			install_name_tool -change "$LIBZIP_DIR/$L" "@executable_path/$L" "$E"
		done
	done

	cd lib || exit

	for L in *
	do
		install_name_tool -id "@executable_path/lib/$L" "$L"
	done
)

cp -a additional_utils/* "$BUILD_DIR"

# If we were given a directory to install to, do so
if [ "$INSTALL_DIR" != "" ]
then
	cp -va "$BUILD_DIR"/* "$INSTALL_DIR"/
fi

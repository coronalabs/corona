/*
This, combined with `CLANG_ENABLE_MODULES` in libplayer_core
will add `LC_LINKER_OPTION` auto linker option helper to library
To check, run:

otool -l libplayer.a | grep -A 4 LC_LINKER_OPTION | grep string | grep -v '\-framework' | awk '{ print $3 }' | sort | uniq

*/

@import AssetsLibrary;
@import AVKit;
@import CoreLocation;
@import CoreMotion;
@import Foundation;
@import GameController;
#ifndef Rtt_MetalANGLE
@import GLKit;
#else
@import MetalANGLE;
#endif
@import MapKit;
@import MessageUI;
@import MobileCoreServices;
@import OpenAL;
@import Photos;
@import SQLite3;
@import StoreKit;
@import SystemConfiguration;
@import WebKit;

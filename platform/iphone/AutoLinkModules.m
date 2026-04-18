/*
This, combined with `CLANG_ENABLE_MODULES` in libplayer_core
will add `LC_LINKER_OPTION` auto linker option helper to library
To check, run:

otool -l libplayer.a | grep -A 4 LC_LINKER_OPTION | grep string | grep -v '\-framework' | awk '{ print $3 }' | sort | uniq

*/

@import AssetsLibrary;
// AVKit's iOSSupport module on macOS 26 SDK references macOS-only headers
// (AVCaptureView.h) that aren't in the Mac Catalyst overlay — skip on Catalyst.
#if !TARGET_OS_MACCATALYST
@import AVKit;
#endif
@import CoreLocation;
@import CoreMotion;
@import Foundation;
@import GameController;
// GLKit is deprecated and MetalANGLE.framework is iOS-only; skip on Mac Catalyst
// (auto-link is handled at the app level for Catalyst builds).
#if !TARGET_OS_MACCATALYST
#ifndef Rtt_MetalANGLE
@import GLKit;
#else
@import MetalANGLE;
#endif
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

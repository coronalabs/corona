//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacDevice.h"

#import "Rtt_AppleCallback.h"

#ifdef Rtt_AUTHORING_SIMULATOR
	#include "Rtt_PlatformSimulator.h"
#endif

#include "Rtt_MacDeviceIdentifier.h"
#ifndef Rtt_NO_GUI
#include "CoronaViewPrivate.h"
#endif // ! Rtt_NO_GUI

#import <Foundation/NSNotification.h>
#import "NSString+Extensions.h"
#import <Foundation/NSThread.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSRunningApplication.h>
#import <Foundation/NSBundle.h>
#ifdef Rtt_AUTHORING_SIMULATOR
#import "AppDelegate.h"
#endif

#include <CoreServices/CoreServices.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <objc/message.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
	#import <AudioToolbox/AudioServices.h>
#endif

#include <CommonCrypto/CommonCrypto.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static char kNull[] = "(null)";

MacConsoleDevice::MacConsoleDevice( Rtt_Allocator &allocator, CoronaView *view )
:	fAllocator( allocator ),
	fView( view ),
	fDeviceManufacturer( nil ),
	fDeviceModel( nil ),
	fSystemVersion( nil ),
	fDeviceIdentifier( NULL ),
	fInputDeviceManager( &allocator )
{
}

MacConsoleDevice::~MacConsoleDevice()
{
	delete [] fDeviceIdentifier;
	[fSystemVersion release];
	[fDeviceModel release];
	[fDeviceManufacturer release];
}
	
void
MacConsoleDevice::SetManufacturer( NSString *newValue )
{
	if ( fDeviceManufacturer != newValue )
	{
		[fDeviceManufacturer release];
		fDeviceManufacturer = [newValue retain];
	}
}
	
void
MacConsoleDevice::SetModel( NSString *newValue )
{
	if ( fDeviceModel != newValue )
	{
		[fDeviceModel release];
		fDeviceModel = [newValue retain];
	}
}

const char*
MacConsoleDevice::GetName() const
{
    // Note that it's important to pick an API that doesn't block when getting the hostname (as the NSHost ones do)
    return [[(NSString *)SCDynamicStoreCopyComputerName(NULL, NULL) autorelease] UTF8String];
}

const char*
MacConsoleDevice::GetManufacturer() const
{
	return fDeviceManufacturer ? [fDeviceManufacturer UTF8String] : kNull;
}

const char*
MacConsoleDevice::GetModel() const
{
	return fDeviceModel ? [fDeviceModel UTF8String] : kNull;
}

MPlatformDevice::EnvironmentType
MacConsoleDevice::GetEnvironment() const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	return kSimulatorEnvironment;
#else
	return kDeviceEnvironment;
#endif
}

const char*
MacConsoleDevice::GetPlatformName() const
{
	return "Mac OS X";  // the OS is called "macOS" so this is incorrect but backwards compatible
}

const char*
MacConsoleDevice::GetPlatform() const
{
	return "macos";  // modern OS name
}

const char*
MacConsoleDevice::GetPlatformVersion() const
{
	if ( fSystemVersion == nil )
	{
		// OS version determination (ironically this is very OS version dependent)
		typedef struct {
			NSInteger majorVersion;
			NSInteger minorVersion;
			NSInteger patchVersion;
		} OperatingSystemVersion;
        OperatingSystemVersion osVersion = {0};
		SEL operatingSystemVersionSelector = NSSelectorFromString(@"operatingSystemVersion");

		if ([[NSProcessInfo processInfo] respondsToSelector:operatingSystemVersionSelector])
		{
			// this works on 10.10 and above (and, apparently, 10.9)
            NSMethodSignature *signature = [NSProcessInfo instanceMethodSignatureForSelector:operatingSystemVersionSelector];
            if(signature)
            {
                NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
                [invocation setTarget:[NSProcessInfo processInfo]];
                [invocation setSelector:operatingSystemVersionSelector];
                [invocation invoke];
                [invocation getReturnValue:&osVersion];
            }
        }
		else
		{
			// works on 10.8 and below but is now deprecated (actually it doesn't work correctly with "minorVersion" > 9)
			SInt32 versMaj, versMin, versPatch;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
			Gestalt(gestaltSystemVersionMajor, &versMaj);
			Gestalt(gestaltSystemVersionMinor, &versMin);
			Gestalt(gestaltSystemVersionBugFix, &versPatch);
#pragma GCC diagnostic pop

			osVersion.majorVersion = versMaj;
			osVersion.minorVersion = versMin;
			osVersion.patchVersion = versPatch;
		}

		fSystemVersion = [[NSString stringWithFormat:@"%d.%d.%d", (int) osVersion.majorVersion, (int) osVersion.minorVersion,  (int) osVersion.patchVersion] retain];
	}

	return [fSystemVersion UTF8String];
}

const char*
MacConsoleDevice::GetArchitectureInfo() const
{
	NSInteger arch = [[NSRunningApplication currentApplication] executableArchitecture];
	switch (arch)
	{
		case NSBundleExecutableArchitectureI386:
			return "i386";
		case NSBundleExecutableArchitectureX86_64:
			return "x86_64";
		case NSBundleExecutableArchitecturePPC:
			return "ppc";
		case NSBundleExecutableArchitecturePPC64:
			return "ppc64";
		  default:
			return "unknown";
	}
}

static NSString *
GetMacAddress()
{
	NSString *result = nil;

	U8 addr[Rtt_EthernetAddressSize];
	if ( Rtt_GetDeviceIdentifier( addr ) )
	{
		// Convert addr into a hex string
		U8 addrHex[Rtt_EthernetAddressSize*2 + 1];
#ifdef Rtt_DEBUG
		int numBytes =
#endif
		sprintf( (char*)addrHex, "%02x%02x%02x%02x%02x%02x",
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5] );

		Rtt_ASSERT( (sizeof(addrHex) - 1) == numBytes );

		result = [NSString stringWithExternalString:(char*)addrHex];
	}

	return result;
}

const char*
MacConsoleDevice::GetDeviceIdentifier() const
{
	if ( ! fDeviceIdentifier )
	{
		U8 addr[Rtt_EthernetAddressSize];
		if ( Rtt_GetDeviceIdentifier( addr ) )
		{
			// Convert addr into a hex string
			U8 addrHex[Rtt_EthernetAddressSize*2 + 1];
			int numBytes = sprintf( (char*)addrHex, "%02x%02x%02x%02x%02x%02x",
				addr[0], addr[1], addr[2], addr[3], addr[4], addr[5] );
			Rtt_ASSERT( (sizeof(addrHex) - 1) == numBytes );

			U8 addrMD5[CC_MD5_DIGEST_LENGTH];
			CC_MD5( addrHex, numBytes, addrMD5 );

			fDeviceIdentifier = new char[CC_MD5_DIGEST_LENGTH*2 + 1];

			char *p = fDeviceIdentifier;
			for ( int i = 0; i < CC_MD5_DIGEST_LENGTH; i++ )
			{
				p += sprintf( p, "%02x", addrMD5[i] );
			}

			Rtt_ASSERT( strlen( fDeviceIdentifier ) == CC_MD5_DIGEST_LENGTH*2 );

		}

	}

	return fDeviceIdentifier;
}

const char*
MacConsoleDevice::GetUniqueIdentifier( IdentifierType t ) const
{
	const char *result = NULL;

	switch ( t )
	{
		case MPlatformDevice::kDeviceIdentifier:
			result = GetDeviceIdentifier();
			break;
		case MPlatformDevice::kMacIdentifier:
			result = [GetMacAddress() UTF8String];
			break;
		case MPlatformDevice::kUdidIdentifier:
			// TODO
			break;
		default:
			break;
	}

	return result;
}

PlatformInputDeviceManager&
MacConsoleDevice::GetInputDeviceManager()
{
	return fInputDeviceManager;
}

void
MacConsoleDevice::Vibrate() const
{
}

bool
MacConsoleDevice::HasEventSource( EventType type ) const
{
	bool hasEventSource = false;
	
	switch (type)
	{
		case MPlatformDevice::kAccelerometerEvent:
		case MPlatformDevice::kGyroscopeEvent:
		case MPlatformDevice::kMultitouchEvent:
		case MPlatformDevice::kHeadingEvent:
		case MPlatformDevice::kOrientationEvent:
			hasEventSource = false;
			break;
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kKeyEvent:
		case MPlatformDevice::kInputDeviceStatusEvent:
		case MPlatformDevice::kMouseEvent:
			hasEventSource = true;
			break;
		default:
			// Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return hasEventSource;
}

void
MacConsoleDevice::BeginNotifications( EventType type ) const
{
	fTracker.BeginNotifications( type );

#ifndef Rtt_NO_GUI
	if (type == kLocationEvent)
	{
		[fView startLocationUpdating];
	}
	else if (kMouseEvent == type )
	{
		[fView.glView setSendAllMouseEvents:YES];
	}
#endif // ! Rtt_NO_GUI
}

void
MacConsoleDevice::EndNotifications( EventType type ) const
{
	fTracker.EndNotifications( type );
	
#ifndef Rtt_NO_GUI
	if (type == kLocationEvent)
	{
		[fView endLocationUpdating];
	}
	else if (kMouseEvent == type )
	{
		[fView.glView setSendAllMouseEvents:NO];
	}
#endif // ! Rtt_NO_GUI
}

bool
MacConsoleDevice::DoesNotify( EventType type ) const
{
	return fTracker.DoesNotify( type );
}

void
MacConsoleDevice::SetAccelerometerInterval( U32 frequency ) const
{
}

void
MacConsoleDevice::SetGyroscopeInterval( U32 frequency ) const
{
}

void
MacConsoleDevice::SetLocationAccuracy( Real meters ) const
{
}

void
MacConsoleDevice::SetLocationThreshold( Real meters ) const
{
}

DeviceOrientation::Type
MacConsoleDevice::GetOrientation() const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	return DeviceOrientation::kUnknown;
#else
	return DeviceOrientation::kUpright;
#endif // Rtt_AUTHORING_SIMULATOR
}

// ----------------------------------------------------------------------------

// TODO: Move this to a separate file
#if !defined( Rtt_WEB_PLUGIN )

MacDevice::MacDevice( Rtt_Allocator &allocator, PlatformSimulator& simulator )
:	Super( allocator, NULL ),
	fSimulator( simulator ),
	fPreviousOrientation( DeviceOrientation::kUnknown )
{
}

void
MacDevice::Vibrate() const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
	AudioServicesPlaySystemSound( kUserPreferredAlert );
#endif
}

bool
MacDevice::HasEventSource( EventType type ) const
{
	bool hasEventSource = false;
	
	// FIXME: Most of these should be controlled by the skin in use
	switch (type)
	{
		case MPlatformDevice::kGyroscopeEvent:
		case MPlatformDevice::kMultitouchEvent:
		case MPlatformDevice::kHeadingEvent:
		case MPlatformDevice::kMouseEvent:
			hasEventSource = false;
			break;
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kKeyEvent:
		case MPlatformDevice::kInputDeviceStatusEvent:
			hasEventSource = true;
			break;
		case MPlatformDevice::kAccelerometerEvent:
		case MPlatformDevice::kOrientationEvent:
			hasEventSource = true;	// The Simulator can simulate these events
			break;
		default:
			// Rtt_ASSERT_NOT_REACHED();
			break;
	}
	
	return hasEventSource;
}

void
MacDevice::BeginNotifications( EventType type ) const
{
	Super::BeginNotifications( type );

#ifdef Rtt_AUTHORING_SIMULATOR
	fSimulator.BeginNotifications( type );

	if ( kLocationEvent == type )
	{
		// Tell the AppDelegate to start sending location updates
		AppDelegate *delegate = (AppDelegate *)[NSApp delegate];
		[delegate performSelectorOnMainThread:@selector(startLocationUpdating) withObject:nil waitUntilDone:NO];
	}
	else if (kMouseEvent == type )
	{
		AppDelegate *delegate = (AppDelegate *)[NSApp delegate];
		[delegate.layerHostView setSendAllMouseEvents:YES];
	}
#endif // Rtt_AUTHORING_SIMULATOR
}

void
MacDevice::EndNotifications( EventType type ) const
{
	Super::EndNotifications( type );

#ifdef Rtt_AUTHORING_SIMULATOR
	fSimulator.EndNotifications( type );
	
	if ( kLocationEvent == type )
	{
		// Tell the AppDelegate to stop sending location updates
		AppDelegate *delegate = (AppDelegate *)[NSApp delegate];
		[delegate performSelectorOnMainThread:@selector(endLocationUpdating) withObject:nil waitUntilDone:NO];
	}
	else if (kMouseEvent == type )
	{
		AppDelegate *delegate = (AppDelegate *)[NSApp delegate];
		[delegate.layerHostView setSendAllMouseEvents:NO];
	}
#endif // Rtt_AUTHORING_SIMULATOR
}

void
MacDevice::SetAccelerometerInterval( U32 frequency ) const
{
	Rtt_WARN_SIM( frequency >= 10 && frequency <= 100, ( "WARNING: Accelerometer frequency on iPhone must be in the range [10,100] Hz" ) );
}

void
MacDevice::SetGyroscopeInterval( U32 frequency ) const
{
	Rtt_WARN_SIM( frequency >= 10 && frequency <= 100, ( "WARNING: Gyroscope frequency on iPhone must be in the range [10,100] Hz" ) );
}

void
MacDevice::SetLocationAccuracy( Real meters ) const
{
}

void
MacDevice::SetLocationThreshold( Real meters ) const
{
}

DeviceOrientation::Type
MacDevice::GetOrientation() const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	return fSimulator.GetOrientation();
#else
	return DeviceOrientation::kUpright;
#endif // Rtt_AUTHORING_SIMULATOR
}

const char*
MacDevice::GetPlatform() const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	return fSimulator.GetOSName();
#else
	return "macos";
#endif // Rtt_AUTHORING_SIMULATOR
}

#endif // Rtt_WEB_PLUGIN

// ----------------------------------------------------------------------------

MacAppDevice::MacAppDevice( Rtt_Allocator &allocator, DeviceOrientation::Type orientation )
:	Super( allocator, NULL ),
	fDeviceOrientation( orientation )
{
}
	
DeviceOrientation::Type
MacAppDevice::GetOrientation() const
{
	return fDeviceOrientation;
}
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


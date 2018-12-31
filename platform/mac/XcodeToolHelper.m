//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Assert.h"

#import "XcodeToolHelper.h"

#import <AppKit/AppKit.h>


@implementation XcodeToolHelper

static const NSString* kXcodeToolHelperUserDefaultsPrefix = @"XcodeOverrideTool_";

+ (NSString*) toolLocationFromPreferences:(NSString*)toolbasename printWarning:(BOOL)should_print_warning
{
	// toolpath is the full path including the executable itself
	// Preference convention is to make all keys have the prefix: XcodeOverrideTool_ followed by the tool name
	NSString* key = [kXcodeToolHelperUserDefaultsPrefix stringByAppendingString:toolbasename];
	NSString* toolpath = [[NSUserDefaults standardUserDefaults] objectForKey:key];

	if ( nil != toolpath && should_print_warning )
	{
		NSLog(@"Note: '%@' location has been overridden in Preferences:\n\t%@ = %@", toolbasename, key, toolpath);
	}
	
	return toolpath;
}

+ (void) printNotFoundWarningForTool:(NSString*)toolbasename
{
	NSLog(@"Warning: Could not find Xcode build tool: %@\n - perhaps Xcode isn't installed", toolbasename);
}

//
// Return the path for codesign_allocate
//
+ (NSString*) pathForCodesignAllocateUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"codesign_allocate" printWarning:should_print_warning];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( nil != toolpath )
	{
		return toolpath;
	}

	// No override in effect. Do the normal thing.
	toolpath = [XcodeToolHelper findXcodePathFor:@"codesign_allocate"];

	if ([toolpath length] == 0)
	{
		// If xcrun can't find "codesign_allocate", this is the best bet
		toolpath = [developerbase stringByAppendingPathComponent:@"Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate"];
	}

	if (should_print_warning && ! [[NSFileManager defaultManager] fileExistsAtPath:toolpath])
	{
		toolpath = nil;

		[XcodeToolHelper printNotFoundWarningForTool:@"codesign_allocate"];
	}

	return toolpath;
}

//
// Return the path for productbuild
//
+ (NSString*) pathForProductBuildUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"productbuild" printWarning:should_print_warning];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( nil != toolpath )
	{
		return toolpath;
	}

	// No override in effect. Do the normal thing.
	toolpath = [XcodeToolHelper findXcodePathFor:@"productbuild"];

	if (should_print_warning && ! [[NSFileManager defaultManager] fileExistsAtPath:toolpath])
	{
		toolpath = nil;

		[XcodeToolHelper printNotFoundWarningForTool:@"productbuild"];
	}

	return toolpath;
}

//
// Return the path for copypng
//
+ (NSString*) pathForCopyPngUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"copypng" printWarning:should_print_warning];
	
	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( nil != toolpath )
	{
		return toolpath;
	}
	
	// No override in effect. Don't use [XcodeToolHelper findXcodePathFor:] because we want to suppress the warning
	toolpath = [self launchTaskAndReturnOutput:@"/usr/bin/xcrun" arguments:@[@"--find", @"copypng"] printWarning:NO];
	
	if ([toolpath length] == 0)
	{
		// Xcode prior to 7.0 doesn't configure "copypng" as an xcrun tool but all the versions we care about have it here
		toolpath = [developerbase stringByAppendingPathComponent:@"Platforms/iPhoneOS.platform/Developer/usr/bin/copypng"];
	}
	
	if (should_print_warning && ! [[NSFileManager defaultManager] fileExistsAtPath:toolpath])
	{
		toolpath = nil;
		
		[XcodeToolHelper printNotFoundWarningForTool:@"copypng"];
	}
	
	return toolpath;
}

//
// Return the path for codesign
//
+ (NSString*) pathForCodesignUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"codesign" printWarning:should_print_warning];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( nil != toolpath )
	{
		return toolpath;
	}

	// No override in effect. Do the normal thing.
	toolpath = @"/usr/bin/codesign";

	if ( NO == [[NSFileManager defaultManager] fileExistsAtPath:toolpath] )
	{
		toolpath = nil;
		if ( should_print_warning )
		{
			[XcodeToolHelper printNotFoundWarningForTool:@"codesign"];
		}
	}

	return toolpath;
}

//
// Return the path for Application Loader
//
+ (NSString*) pathForApplicationLoaderUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"applicationloader" printWarning:should_print_warning];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( nil != toolpath )
	{
		return toolpath;
	}

	// No override in effect. Do the normal thing.
	toolpath = [self getXcodePath];
	toolpath = [toolpath stringByAppendingPathComponent:@"../Applications/Application Loader.app"];

	if ( NO == [[NSFileManager defaultManager] fileExistsAtPath:toolpath] )
	{
		toolpath = nil;
		if ( should_print_warning )
		{
			[XcodeToolHelper printNotFoundWarningForTool:@"Application Loader"];
		}
	}

	return toolpath;
}

+ (NSString*) pathForIOSDeviceInstallationUtility
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"iosDeviceInstallationUtility" printWarning:NO];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( toolpath != nil )
	{
		return toolpath;
	}

	// No override in effect. Do the normal thing.
	toolpath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"ios_sendapp.sh"];

	if ( [[NSFileManager defaultManager] fileExistsAtPath:toolpath] == NO )
	{
		toolpath = nil;
	}

	return toolpath;
}

+ (NSString*) pathForIOSDeviceSyslogUtility
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"iosDeviceSyslogUtility" printWarning:NO];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( toolpath != nil )
	{
		return toolpath;
	}

	toolpath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"ios_syslog.sh"];

	if ( [[NSFileManager defaultManager] fileExistsAtPath:toolpath] == NO )
	{
		toolpath = nil;
	}

	return toolpath;
}

+ (NSString*) pathForXcodeSimulatorDeviceInstallationUtility
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"xcodeSimulatorDeviceInstallationUtility" printWarning:NO];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( toolpath != nil )
	{
		return toolpath;
	}

	// No override in effect. Do the normal thing.
	toolpath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"xcodesim_sendapp.sh"];

	if ( [[NSFileManager defaultManager] fileExistsAtPath:toolpath] == NO )
	{
		toolpath = nil;
	}

	return toolpath;
}

+ (NSString*) pathForXcodeSimulatorDeviceSyslogUtility
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"xcodeSimulatorDeviceSyslogUtility" printWarning:NO];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( toolpath != nil )
	{
		return toolpath;
	}

	toolpath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"xcodesim_syslog.sh"];

	if ( [[NSFileManager defaultManager] fileExistsAtPath:toolpath] == NO )
	{
		toolpath = nil;
	}

	return toolpath;
}

+ (NSString*) pathForAndroidDeviceInstallationUtility
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"androidDeviceInstallationUtility" printWarning:NO];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( toolpath != nil )
	{
		return toolpath;
	}

	toolpath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"android_sendapp.sh"];

	if ( [[NSFileManager defaultManager] fileExistsAtPath:toolpath] == NO )
	{
		toolpath = nil;
	}

	return toolpath;
}

+ (NSString*) pathForAndroidDeviceSyslogUtility
{
	// User preferences always overrides.
	NSString* toolpath = [XcodeToolHelper toolLocationFromPreferences:@"androidDeviceSyslogUtility" printWarning:NO];

	//  Don't do any validation for overrides because the only reason overrides are in effect is to force work around problems.
	if ( toolpath != nil )
	{
		return toolpath;
	}

	toolpath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"android_syslog.sh"];

	if ( [[NSFileManager defaultManager] fileExistsAtPath:toolpath] == NO )
	{
		toolpath = nil;
	}

	return toolpath;
}

//
// Find the Xcode "developer root" using xcode-select
//
+ (NSString*) getXcodePath
{
	return [self launchTaskAndReturnOutput:@"/usr/bin/xcode-select" arguments:@[@"-print-path"] printWarning:YES];
}

//
// Find the Xcode path (if any) for a utility using xcrun
//
+ (NSString *) findXcodePathFor:(NSString *)cmd
{
	return [self launchTaskAndReturnOutput:@"/usr/bin/xcrun" arguments:@[@"--find", cmd] printWarning:YES];
}

//
// Get the version number of the current Xcode
//
+ (double) getXcodeVersion
{
    return [[self launchTaskAndReturnOutput:@"/bin/sh" arguments:@[@"-c", @"/usr/bin/xcodebuild -version | sed -n '1s/Xcode //'p"] printWarning:NO] doubleValue];
}

//
// Launch a command and capture its output and return it as a string
//
+ (NSString *) launchTaskAndReturnOutput:(NSString *)cmd arguments:(NSArray *)args printWarning:(BOOL)printWarning
{
	NSString *result = nil;
	NSMutableData *resultData = [[[NSMutableData alloc] init] autorelease];
	NSTask *task = [[NSTask alloc] init];
	NSPipe *stdoutPipe = [NSPipe pipe];
	NSPipe *stderrPipe = [NSPipe pipe];

	[task setLaunchPath:cmd];
	[task setArguments:args];

	[task setStandardOutput:stdoutPipe];
	[task setStandardError:stderrPipe];

	NSFileHandle *stderrFileHandle = [stderrPipe fileHandleForReading];

	// Using a readability handler allows us to get more than 4096 bytes without blocking the pipe
	[[task.standardOutput fileHandleForReading] setReadabilityHandler:^(NSFileHandle *file) {
		NSData *data = [file availableData]; // read to current EOF

		[resultData appendData:data];
	}];

	@try
	{
		[task launch];
		[task waitUntilExit];

		if (! [task isRunning] && [task terminationStatus] != 0 && printWarning)
		{
			// Command failed, emit any stderr to the log
			NSData *stderrData = [stderrFileHandle readDataToEndOfFile];
			NSLog(@"Error running %@ %@: %s", cmd, args, (const char *)[stderrData bytes]);
		}

		result = [[[NSMutableString alloc] initWithData:resultData encoding:NSUTF8StringEncoding] autorelease];
	}
	@catch( NSException* exception )
	{
		NSLog( @"launchTaskAndReturnOutput: exception %@ (%@ %@)", exception, cmd, args );
	}
	@finally
	{
		[[task.standardOutput fileHandleForReading] setReadabilityHandler:nil];
		[task release];
	}

	result = [result stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

	return result;
}

+ (NSDictionary *) loadSupportedIOSSimulatorDevices:(NSString *) osName
{
	NSString *iosSimDevices = [XcodeToolHelper launchTaskAndReturnOutput:@"/usr/bin/xcrun" arguments:@[@"simctl", @"list", @"--json", @"devices"] printWarning:NO];

	// NSLog(@"iosSimDevices: %@", iosSimDevices);
	NSError *error = nil;

	NSDictionary *devicesDict = [NSJSONSerialization JSONObjectWithData:[iosSimDevices dataUsingEncoding:NSUTF8StringEncoding] options:NSJSONReadingAllowFragments error:&error];

	// NSLog(@"deviceDict: %@", devicesDict);

	NSMutableDictionary *availableDevices = [[NSMutableDictionary alloc] initWithCapacity:10];

	devicesDict = [devicesDict objectForKey:@"devices"];

	if (devicesDict != nil)
	{
		[devicesDict enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
			if (! [key isKindOfClass:[NSString class]] || ! [obj isKindOfClass:[NSArray class]])
			{
				*stop = YES;
			}
			else
			{
				// Keys are NSString, obj are NSArray of NSDictionary of NSString
				for (NSDictionary *dict in obj)
				{
					if ([key hasPrefix:osName])
					{
						if ([[dict objectForKey:@"availability"] isEqualToString:@"(available)"])
						{
							NSMutableDictionary *osDict = nil;

							if ((osDict = [availableDevices objectForKey:key]) == nil)
							{
								osDict = [[NSMutableDictionary alloc] initWithCapacity:10];
								[availableDevices setObject:osDict forKey:key];
							}

							[osDict setObject:[dict objectForKey:@"udid"] forKey:[dict objectForKey:@"name"]];
							// NSLog(@"%@ / %@ : %@", key, [dict objectForKey:@"name"], [dict objectForKey:@"udid"]);
						}
					}
				}
			}
		}];
	}

	if ([availableDevices count] == 0)
	{
		Rtt_LogException("ERROR: unexpected output from 'simctl'.  Cannot enumerate Xcode %s Simulator types: %s",
						 [osName UTF8String], [iosSimDevices UTF8String]);
	}

	return availableDevices;
}

@end

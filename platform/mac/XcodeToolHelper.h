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

#import <Foundation/Foundation.h>
#import "NSString+Extensions.h"

// Helper functions to help deal with Xcode command lines tools.
@interface XcodeToolHelper : NSObject

+ (NSString*) toolLocationFromPreferences:(NSString*)toolbasename printWarning:(BOOL)should_print_warning;

+ (void) printNotFoundWarningForTool:(NSString*)toolbasename;

+ (NSString*) pathForCodesignAllocateUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning;

+ (NSString*) pathForCopyPngUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning;

+ (NSString*) pathForCodesignUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning;

+ (NSString*) pathForProductBuildUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning;

+ (NSString*) pathForIOSDeviceInstallationUtility;
+ (NSString*) pathForIOSDeviceSyslogUtility;
+ (NSString*) pathForXcodeSimulatorDeviceInstallationUtility;
+ (NSString*) pathForXcodeSimulatorDeviceSyslogUtility;
+ (NSString*) pathForAndroidDeviceInstallationUtility;
+ (NSString*) pathForAndroidDeviceSyslogUtility;

// Note: This returns Application Loader.app, not the full Application Loader.app/Contents/MacOS/Application Loader
+ (NSString*) pathForApplicationLoaderUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning;

+ (NSString*) getXcodePath;

+ (double) getXcodeVersion;

+ (NSString *) findXcodePathFor:(NSString *)cmd;

+ (NSString *) launchTaskAndReturnOutput:(NSString *)cmd arguments:(NSArray *)args printWarning:(BOOL)printWarning;

+ (NSDictionary *) loadSupportedIOSSimulatorDevices:(NSString *) osName;

@end

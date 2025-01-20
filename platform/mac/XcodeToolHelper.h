//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

+ (NSString*) pathForCodesignFramework;
+ (NSString*) pathForResources;

// Note: This returns Application Loader.app, not the full Application Loader.app/Contents/MacOS/Application Loader
+ (NSString*) pathForApplicationLoaderUsingDeveloperBase:(NSString*)developerbase printWarning:(BOOL)should_print_warning;

+ (NSString*) getXcodePath;

+ (double) getXcodeVersion;

+ (NSString *) findXcodePathFor:(NSString *)cmd;

+ (NSString *) launchTaskAndReturnOutput:(NSString *)cmd arguments:(NSArray *)args printWarning:(BOOL)printWarning;

+ (NSDictionary *) loadSupportedIOSSimulatorDevices:(NSString *) osName;

@end

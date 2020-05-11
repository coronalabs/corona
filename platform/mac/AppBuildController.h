//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

#include "Rtt_TargetDevice.h"
#include "Rtt_MacSimulatorServices.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	class AppPackagerParams;
	class MPlatformServices;
	class PlatformAppPackager;
	class SimulatorAnalytics;
}

// ----------------------------------------------------------------------------

@interface AppBuildController : NSWindowController< NSAlertDelegate, NSTextViewDelegate >
{
    @protected
        AppDelegate *appDelegate;

	@protected
        IBOutlet NSMatrix *postBuildRadioGroup;
		IBOutlet NSPopUpButton* fSigningIdentities;
		IBOutlet NSButton *fEnableLiveBuild;

		Rtt::SimulatorAnalytics *fAnalytics;
		NSString *platformName;
		NSString *platformTitle;
        Rtt::MacSimulatorServices *simulatorServices;
		NSTask *tailSyslogTask;
		NSTask *launchedTask;
		NSString *appName;

	@private
		NSString *appVersion;
		NSString *dstPath;
		NSString *projectPath;
		NSString *progressSheetMessage;
        NSWindow *progressBannerParent;
        NSTimer *progressBannerTimer;

	@private
		IBOutlet NSWindow *progressSheetBanner;
		IBOutlet NSImageView *buildProgressBanner;
		IBOutlet NSProgressIndicator *buildProgressIndicator;
		IBOutlet NSButton *stopButton;
		IBOutlet NSProgressIndicator *buildDownloadProgress;
}

@property (nonatomic, readwrite, copy) NSString *appName;
@property (nonatomic, readwrite, copy) NSString *appVersion;
@property (nonatomic, readwrite, copy) NSString *dstPath;
@property (nonatomic, readwrite, copy) NSString *projectPath;
@property (nonatomic, readwrite, copy) NSString *progressSheetMessage;
@property (nonatomic, readwrite, copy) NSString *platformName;
@property (nonatomic, readwrite, copy) NSString *platformTitle;

+ (NSString*)defaultDstDir;

- (id)initWithWindowNibName:(NSString*)nibFile projectPath:(NSString *)projPath;

- (const char *)getAppVersion;
- (const char *)dstDir;
- (NSString *)appPackagePath;
- (NSString *)appBundleFile;

- (void)setAnalytics:(Rtt::SimulatorAnalytics *)analytics;
- (void)logEvent:(NSString *)eventName;
- (void)logEvent:(NSString *)eventName key:(NSString *)eventDataKey value:(NSString *)eventDataValue;
- (void)logEvent:(NSString *)eventName keyValues:(NSDictionary *)keyValues;

// Build methods to override
- (BOOL)validateProject;
- (BOOL)isStoreBuild;
- (BOOL)isDeveloperBuild;
- (Rtt::PlatformAppPackager*)createAppPackager:(Rtt::MPlatformServices*)services;
- (Rtt::AppPackagerParams*)createAppPackagerParams:(Rtt::PlatformAppPackager*)packager;
- (NSString*)alertMessage;
- (void)willShowAlert:(NSAlert*)alert;
- (BOOL)verifyBuildTools:(id)sender;
- (BOOL)shouldRemoveWhitespaceFromPackageName;
- (NSString*)appExtension;
- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void  *)contextInfo;
- (void)closeBuild:(id)sender;

// Actions
- (IBAction)build:(id)sender;
- (IBAction)cancelBuild:(id)sender;
- (IBAction)browseDstPath:(id)sender;

- (BOOL)buildFormComplete;
- (NSMenu*)signingIdentitiesMenu;
- (void)didSelectIdentity:(NSNotification*)notification;

- (Rtt::TargetDevice::Platform)targetPlatform;
- (void)beginProgressSheetBanner:(NSWindow*)parent withDelay:(NSTimeInterval)delay;
- (void)endProgressSheetBanner:(NSWindow *)parent;
- (BOOL)shouldInitiateBuild:(id)sender;
- (void) setProgressBarLabel:(NSString *) newLabel;
- (void)setBuildMessage:(NSString *)message;
- (void) runLengthyOperationForWindow:(NSWindow *)window delayProgressWindowBy:(NSTimeInterval)delay allowStop:(BOOL)allowStop withBlock:(void (^)(void))block;

- (NSString *) launchTaskAndReturnOutput:(NSString *)cmd arguments:(NSArray *)args;
- (void) stopLaunchedTask;
- (BOOL)textView:(NSTextView *)aTextView clickedOnLink:(id)link atIndex:(NSUInteger)charIndex;
- (NSString *) limitString:(NSString *)inString toLength:(size_t)inLength;
- (NSAttributedString *)autoLinkURLs:(NSString *)string;
- (NSAttributedString *)formatBasicMarkdown:(NSString *)string;
- (void) showMessage:(NSString *)title message:(NSString *)message helpURL:(NSString *)helpURL parentWindow:(NSWindow *)parentWindow;
- (void) showError:(NSString *)title message:(NSString *)message helpURL:(NSString *)helpURL parentWindow:(NSWindow *)parentWindow;
- (NSModalResponse) showModalSheet:(NSString *)title message:(NSString *)message buttonLabels:(NSArray *)buttonLabels alertStyle:(NSAlertStyle)alertStyle helpURL:(NSString *)helpURL parentWindow:(NSWindow *)parentWindow completionHandler:(void (^)(NSModalResponse returnCode))completionHandler;
- (void) saveBuildPreferences;
- (void) restoreBuildPreferences;

- (void) startTailDeviceSyslog:(NSString *)deviceSyslogUtility appBundlePath:(NSString *)appBundlePath deviceID:(NSString *)deviceID;
- (void) stopTailDeviceSyslog;

@end

// ----------------------------------------------------------------------------

//
// Category to allow NSAlert instances to be run synchronously as sheets.
//
@interface NSAlert (SynchronousSheet)

-(NSInteger) runModalSheetForWindow:(NSWindow *)window completionHandler:(void (^)(NSModalResponse returnCode))completionHandler;
-(NSInteger) runModalSheetForWindow:(NSWindow *)aWindow;
-(NSInteger) runModalSheet;

@end

// ----------------------------------------------------------------------------


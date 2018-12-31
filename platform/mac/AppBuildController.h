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

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_WebServicesSession.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_MacSimulatorServices.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	class Authorization;
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
		const Rtt::Authorization *fAuthorizer;
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
@property (nonatomic, readonly, getter=authorizer, assign) const Rtt::Authorization *fAuthorizer;

+ (NSString*)defaultDstDir;

- (id)initWithWindowNibName:(NSString*)nibFile projectPath:(NSString *)projPath authorizer:(const Rtt::Authorization *)authorizer;

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
- (BOOL) loginSession:(Rtt::WebServicesSession *)session services:(Rtt::MacPlatformServices *)services ticket:(const Rtt::AuthorizationTicket *)ticket message:(NSString **)message;

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


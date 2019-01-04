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

//#import <AppKit/AppKit.h>
//#import <Foundation/NSObject.h>
//#import <Foundation/NSGeometry.h>
//#import <AppKit/NSNibDeclarations.h>
#import "GLView.h"
#import <AppKit/AppKit.h>
#import <CoreLocation/CoreLocation.h>
#import "NSString+Extensions.h"

#include "Rtt_PlatformSimulator.h"
#include "Rtt_TargetDevice.h"

//#import <Foundation/Foundation.h>

// ----------------------------------------------------------------------------

@class DialogController;
@class GLView;
@class CoronaWindowController;
@class AndroidAppBuildController;
@class IOSAppBuildController;
@class OSXAppBuildController;
@class TVOSAppBuildController;
@class WebAppBuildController;
@class LinuxAppBuildController;
@class NSError;
@class NSMatrix;
@class NSNotification;
@class NSPopUpButton;
@class NSProgressIndicator;
@class NSString;
@class NSTextField;
@class NSView;
@class NSWindow;
@class BuildSessionState;
@class SDKItem;

namespace Rtt
{
	class Authorization;
	class AuthorizationTicket;
	class MacAuthorizationDelegate;
	class MacConsolePlatform;
	class MacPlatformServices;
	class MacSimulator;
	class SimulatorAnalytics;
}

@interface AppDelegate : NSObject <NSMenuDelegate,NSAlertDelegate,GLViewDelegate, CLLocationManagerDelegate
#if !defined( Rtt_WEB_PLUGIN )
	,NSUserNotificationCenterDelegate
#endif
	>
{
	Rtt::MacSimulator* fSimulator;
	NSString* fAppPath;
	Rtt::SimulatorOptions fOptions;

	Rtt::MacConsolePlatform *fConsolePlatform;
	Rtt::SimulatorAnalytics *fAnalytics;

	NSString *fSdkRoot;

	IBOutlet NSView *fOpenAccessoryView;
	IBOutlet NSPopUpButton *fDeviceSkins;
	int fSkin;

	IBOutlet NSView *fBuildAccessoryView;
	IBOutlet NSMatrix *fBuildPlatform;

	NSString* appName;
	NSNumber* appVersionCode;
	NSString* appVersion;
	NSString* dstPath;
	NSString* projectPath;

	// TODO: Move to separate controller
	// BuildProgress.xib
	IBOutlet NSWindow* progressSheet;
	IBOutlet NSProgressIndicator* progressBar;

	// TODO: Move to separate controller
	// BuildProgressBanner.xib
	IBOutlet NSWindow *progressSheetBanner;
	IBOutlet NSProgressIndicator* progressBarBanner;
	int progressBarLabelWaitTime;
	IBOutlet NSTextField *progressBarLabel;
	IBOutlet NSImageView *buildProgressBanner;
	IBOutlet NSButton *buildProgressLearnMore;
	BOOL inCountDown;
	// This is a temporary variable to hold the webservicessession object to survive the weird build: code flow.
	// This ulimately should go away, but the object wrapper can be reused for Blocks.
	BuildSessionState* temporaryBuildSessionState;

	DialogController *fPasswordController;

	id fPreferencesControllerDelegate;
	IBOutlet NSWindow* fPreferencesWindow;
	IBOutlet NSWindow* fCustomDeviceWindow;

    
    NSString *customDeviceName;
    int customDeviceWidth;
    int customDeviceHeight;
    BOOL customDeviceIsRotatable;
    BOOL customDevicePortraitOrientation;
	int customDevicePlatformTag;

	BOOL fIsRemote;

	NSString* cachedSampleDirectoryPath;
	
	__block CoronaWindowController *fHomeScreen;
	BOOL applicationIsTerminating; // used to try to avoid occasional shutdown crash (home screen related?)
	
	// Android
	BOOL fBuildProblemNotified;
	
	FSEventStreamRef luaResourceFolderMonitor;
	IBOutlet NSButton* rememberMyPreferenceAccessoryCheckboxView;
	BOOL relaunchPromptIsOpen;
	BOOL applicationHasBeenInitialized;
	BOOL authorizedToLaunch;
	BOOL launchedWithFile;
	BOOL allowLuaExit;
	BOOL fSimulatorWasSuspended;
	BOOL fOpenLastProject;
	time_t fNextUpsellTime;


    AndroidAppBuildController *fAndroidAppBuildController;
    IOSAppBuildController *fIOSAppBuildController;
	OSXAppBuildController *fOSXAppBuildController;
	TVOSAppBuildController *fTVOSAppBuildController;
	WebAppBuildController *fWebAppBuildController;
	LinuxAppBuildController *fLinuxAppBuildController;

	Rtt::MacPlatformServices *fServices;
	Rtt::MacAuthorizationDelegate *fAuthorizerDelegate;
	Rtt::Authorization *fAuthorizer;	

	// Used to synchronize the Open Project accessory view to pick the start skin
	IBOutlet NSPopUpButton* popupButtonOpenAccessorySkinSelection;

	NSTask *consoleTask;
	long fRelaunchCount;
}

@property (nonatomic, readonly, getter=simulator) Rtt::MacSimulator *fSimulator;
@property (nonatomic, readwrite, copy) NSString *appName;
@property (nonatomic, readwrite, copy) NSNumber *appVersionCode;
@property (nonatomic, readwrite, copy) NSString *appVersion;
@property (nonatomic, readwrite, copy) NSString *dstPath;
@property (nonatomic, readonly) NSString *projectPath;
@property (nonatomic, readwrite, copy) NSString *androidAppPackage;
@property (nonatomic, readwrite, copy) NSString *androidKeyStore;
@property (nonatomic, readwrite, assign) int fSkin;
//@property (nonatomic, readwrite, assign) NSPopUpButton* signingIdentities;
@property (nonatomic, readwrite, copy) NSString* cachedSampleDirectoryPath;
@property (nonatomic, readonly, copy) NSString* fAppPath;
@property (nonatomic, readwrite, assign) BOOL applicationHasBeenInitialized;
@property (nonatomic, readwrite, assign) BOOL launchedWithFile;
@property (nonatomic, readwrite, assign) BOOL allowLuaExit;
@property (nonatomic, readonly, getter=authorizer) Rtt::Authorization *fAuthorizer;
@property (nonatomic, readonly, getter=homeScreen) CoronaWindowController *fHomeScreen;
@property (nonatomic, readonly, retain) GLView* layerHostView;
@property (nonatomic, readwrite) BOOL stopRequested;
@property (nonatomic, readwrite) float buildDownloadProgess;
@property (nonatomic, readwrite) BOOL respondsToBackKey;
@property (nonatomic, readonly, getter=analytics) Rtt::SimulatorAnalytics *fAnalytics;

-(BOOL)isRunning;
-(IBAction)showHelp:(id)sender;
-(void)signin;

-(IBAction)orderFrontStandardAboutPanel:(id)sender;

-(BOOL)isRelaunchable;

-(const Rtt::AuthorizationTicket*)ticket;

#if !defined( Rtt_PROJECTOR )
-(BOOL)isRunnable;
-(BOOL)isBuildAvailable;
-(BOOL)isAndroidBuildAvailable;
-(BOOL)isHTML5BuildHidden;
@property (assign) IBOutlet NSMenuItem *ccc;
-(BOOL)isLinuxBuildHidden;
-(BOOL)isTVOSBuildHidden;
-(BOOL)isNookStoreBuildAvailable;

-(IBAction)showPreferences:(id)sender;
-(void)deauthorize:(id)sender;
-(IBAction)deauthorizeConfirm:(id)sender;
-(IBAction)deauthorizeHelp:(id)sender;

-(void) showOpenPanel:(NSString*)title withAccessoryView:(NSView*)accessoryView startDirectory:(NSString*)start_directory completionHandler:(void(^)(NSString* path))completionhandler;

-(void)openWithPath:(NSString*)path;
-(IBAction)open:(id)sender;
#endif // Rtt_PROJECTOR

-(BOOL)runApp:(NSString*)appPath;
-(NSView*)openAccessoryView;

- (void) closeSimulator:(id)sender;
-(IBAction)close:(id)sender;
-(IBAction)showProjectSandbox:(id)sender;
- (IBAction) showProjectFiles:(id)sender;
- (IBAction) clearProjectSandbox:(id)sender;
- (BOOL) setClearProjectSandboxTitle;

-(BOOL)setSkinForTitle:(NSString*)title;

-(IBAction)launchSimulator:(id)sender;
-(IBAction)rotateLeft:(id)sender;
-(IBAction)rotateRight:(id)sender;
-(IBAction)shake:(id)sender;
-(IBAction)toggleSuspendResume:(id)sender;

-(NSWindow*)currentWindow;

-(void)sendLocationEvent;

-(void)beginProgressSheet:(NSWindow*)parent;
-(void)endProgressSheet;

-(void)beginPasswordSheetWithUser:(NSString*)usr
				   modalForWindow:(NSWindow*)parent
					modalDelegate:(id)delegate
						  message:(NSString*)msg
					  contextInfo:(void*)contextInfo;

-(IBAction)openForBuildiOS:(id)sender;
-(IBAction)openForBuildAndroid:(id)sender;
-(IBAction)openForBuildHTML5:(id)sender;
-(IBAction)openForBuildLinux:(id)sender;
-(IBAction)openForBuildOSX:(id)sender;
-(IBAction)openForBuildTVOS:(id)sender;
-(void)notifyRuntimeError:(NSString *)message;
- (NSString *) getOSVersion;

- (BOOL) isTrial;
- (BOOL) isDailyBuild;

- (IBAction) presentWelcomeWindow:(id)sender;
- (void) closeWelcomeWindow;

//-(void)setRuntimeWithView:(GLView*)view;
-(Rtt::Runtime*)runtime;
//-(Rtt::MacPlatform*)platform;

-(NSArray*)GetRecentDocuments;

//-(void)applicationDidBecomeActive:(NSNotification*)aNotification;
- (void) startDebugAndOpenPanel; // second half of applicationDidFinishLaunching

- (IBAction) openMainLuaInEditor:(id)sender;
- (BOOL) promptAndRelaunchSimulatorIfUserApproves;

-(IBAction)changedPreference:(id)sender;

-(void) saveAppSpecificPreference:(NSString *)prefName value:(NSString *)prefValue;
-(NSString *) restoreAppSpecificPreference:(NSString *)prefName defaultValue:(NSString *)defaultValue;
-(NSString *) getAppSpecificPreferenceKeyName:(NSString *)prefName;
-(NSString *) getAppSpecificPreferenceKeyName:(NSString *)prefName withProjectPath:(NSString *)projectDirectoryPath;

- (void) runExtension:(NSString *) extName;

// Used by Rtt_MacAuthorizationDelegate.mm
-(NSString*)getAndReleaseResultFromPasswordSheet;
- (void)didPresentError:(BOOL)didRecover contextInfo:(void*)contextInfo;

-(IBAction)customDeviceOK:(id)sender;
-(IBAction)customDeviceCancel:(id)sender;

- (BOOL) alertShowHelp:(NSAlert *) alert;
- (void) startLocationUpdating;
- (void) endLocationUpdating;

-(void)notifyWithTitle:(NSString*)title description:(NSString*)description iconData:(NSImage*)iconData;
@end

@interface CoronaSimulatorApplication : NSApplication
{
	NSInteger fAttentionRequestID;
}
@property (nonatomic, readwrite) BOOL suppressAttentionRequests;

- (NSInteger)requestUserAttention:(NSRequestUserAttentionType)requestType;

@end

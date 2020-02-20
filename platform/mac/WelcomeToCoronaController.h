//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>
@class LicenseViewController;
@class PasswordViewController;
@class DimView;
struct lua_State;

@interface WelcomeToCoronaController : NSWindowController
{
	BOOL spotlightQueryIsRunning;
	BOOL didFindSampleCodeDirectory;
	NSString* sampleCodeDirectory;
	NSMetadataQuery* metadataQuery;

	IBOutlet NSTextField* welcomeToCoronaLabel;
	IBOutlet NSTextField* welcomeToCoronaLabel2;
	IBOutlet NSTextField* welcomeToCoronaLabelTM;
	IBOutlet NSTextField* examplesLabel;
	
	IBOutlet NSButton* dontShowThisAgainButton;

	LicenseViewController* licenseViewController;
	NSView* licenseView;
	PasswordViewController* passwordViewController;
	NSView* passwordView;

	// A simple, empty view to host other layer backed views
	NSView* layerContainerView;
	// A view to be used to dim the main view and prevent clicking
	DimView* dimView;
	
	BOOL windowGoingAway;
	CFTimeInterval windowFadeAnimationStartTime;
	
	struct lua_State* welcomeLuaConfiguration;
}

@property(nonatomic, assign) BOOL spotlightQueryIsRunning;
@property(nonatomic, assign) BOOL didFindSampleCodeDirectory;
@property(nonatomic, copy) NSString* sampleCodeDirectory;

@property(nonatomic, assign) NSTextField* welcomeToCoronaLabel;
@property(nonatomic, assign) NSTextField* welcomeToCoronaLabel2;
@property(nonatomic, assign) NSTextField* welcomeToCoronaLabelTM;
@property(nonatomic, assign) NSTextField* examplesLabel;
@property(nonatomic, assign) NSButton* dontShowThisAgainButton;
@property(nonatomic, assign) NSView* licenseView;
@property(nonatomic, assign) NSView* passwordView;
@property(nonatomic, assign) NSView* layerContainerView;
@property(nonatomic, retain) DimView* dimView;

@property(nonatomic, assign, getter=isWindowGoingAway) BOOL windowGoingAway;

// designated initializer
- (id) initWithWindowNibName:(NSString*)window_nib_name;


- (IBAction) openProject:(id)the_sender;
- (IBAction) buildProject:(id)the_sender;
- (IBAction) launchGettingStarted:(id)the_sender;
- (IBAction) launchProgrammingGuide:(id)the_sender;

- (IBAction) openCoronaExample:(id)the_sender;
- (IBAction) openMoreCoronaExamples:(id)the_sender;

- (IBAction) closeWindow:(id)the_sender;

- (IBAction) acceptLicense:(id)the_sender;
- (IBAction) rejectLicense:(id)the_sender;

// For when window is fading out and is about to close, call this to bring it back.
- (void) resurrectWindow;

@end

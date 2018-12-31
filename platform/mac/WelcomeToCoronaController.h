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

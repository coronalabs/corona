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

// Important Note: Currently, the Spotlight implementation uses About_Corona_Sample_Code.txt as a magnet to find the Sample Code directory.
// /Volumes/CoronaSDK/Corona2.0-Beta8/SampleCode/About_Corona_Sample_Code.txt
// If this file is removed, we won't find anything. Or if there are multiple locations this file is found, the code may get confused.
// (Though there are a few safety precautions taken to avoid getting confused in this case.)
// The more unique the filename is, the better.

#include "Core/Rtt_Build.h"
#include "Rtt_Lua.h"
#import "WelcomeToCoronaController.h"
#import "AppDelegate.h"
#import "Rtt_MacPlatform.h"

// #include "FontHelper.h" // Code currently disabled because we don't use it anymore
#import "LicenseViewController.h"
#import "PasswordViewController.h"
#import "DimView.h"
#import "PassiveHitView.h"
#import "WelcomeWindow.h"

#import <QuartzCore/QuartzCore.h>

#define ENABLE_LICENSE_AGREEMENT_VIEW 0
#define ENABLE_WINDOW_FADE_ANIMATIONS 1
#define WELCOME_LICENSE_ANIMATION_IN_DURATION 0.70
#define WELCOME_LICENSE_ANIMATION_OUT_DURATION 0.70
#define WELCOME_PASSWORD_ANIMATION_IN_DURATION 0.70
#define WELCOME_FADE_OUT_DURATION 0.70
#define WELCOME_FADE_IN_DURATION 0.70

// We now have a Lua configuration file to help change some of the properties of the Welcome window without recompile.
static NSString* const kWelcomeLuaConfigurationFile = @"WelcomeLuaConfiguration";
static const char* kSpotlightAnchorFile = "spotlightAnchorFile";
static const char* kPathToCoronaExampleForQuickTestOfSampleDirectory = "pathToCoronaExampleForQuickTestOfSampleDirectory";
static const char* kPathToCoronaExamples = "pathToCoronaExamples";
static const char* kGettingStartedURL = "gettingStartedURL";
static const char* kProgrammingGuideURL = "programmingGuideURL";

#ifdef Rtt_GAMEEDITION
static NSString* const kDefaultPathToCoronaExampleForQuickTestOfSampleDirectory = @"Bridge/main.lua";
#else
static NSString* const kDefaultPathToCoronaExampleForQuickTestOfSampleDirectory = @"GettingStarted/HelloWorld/main.lua";
#endif

@class AppDelegate;

@interface WelcomeToCoronaController ()
- (void) runExampleWithTag:(NSInteger)tag_id;
- (NSString*) findSampleCodePath;
- (void) setupLicenseAnimation;
- (void) startDimViewFadeIn:(id)user_data;
- (void) startWindowFadeInAnimation:(id)user_data;
- (void) startWindowFadeOutAnimation:(id)user_data;
- (void) spotlightForExampleDirectory;
- (BOOL) quickTestForExistanceWithSampleCodePath:(NSString*)sample_code_path;
- (void) queryHandler:(NSNotification*)the_notification;
- (NSString*) examplePathFromFragment:(NSString*)fragment_string;
- (void) setMouseOverOnMainWindow:(BOOL)is_enabled forView:(NSView*)the_view;
- (void) setMouseOverOnMainWindow:(BOOL)is_enabled;
- (NSAttributedString*) dropShadowAttributedStringFromAttributedString:(NSAttributedString*)source_string;
@end


@implementation WelcomeToCoronaController

@synthesize spotlightQueryIsRunning;
@synthesize didFindSampleCodeDirectory;
@synthesize sampleCodeDirectory;

@synthesize welcomeToCoronaLabel;
@synthesize welcomeToCoronaLabel2;
@synthesize welcomeToCoronaLabelTM;
@synthesize examplesLabel;
@synthesize dontShowThisAgainButton;
@synthesize licenseView;
@synthesize passwordView;

@synthesize windowGoingAway;


#pragma mark Initialization and Deallocation

// Gilbert wants to use custom fonts. We don't want to require the user to install them, so we run special code
// to load fonts from the bundle. We only need to do this once, and can do it early.
// Update: We don't have permission to use these fonts.
// This code block is disabled, but if we ever need to load custom fonts, this is how it is done.
/*
+ (void) initialize
{
	NSError* err=nil;
	if (!FontHelper_LoadLocalFonts(&err, [NSArray arrayWithObjects:@"FrutigerLTStd-Bold", @"FrutigerLTStd-Roman", nil]))
	{
		NSAlert *alert = [NSAlert alertWithError:err];
		[alert runModal];
		exit(0);
	}
}
*/

- (id) initWithWindowNibName:(NSString*)window_nib_name
{
	self = [super initWithWindowNibName:window_nib_name];
	if(nil != self)
	{
		didFindSampleCodeDirectory = NO;
		sampleCodeDirectory = nil;
		
		
		// Two different future ideas:
		// 1. Store Lua state in main application controller so it doesn't need to be reopened with the window.
		// 2. Use FSEvents to reload the Lua file.
		
		NSString* the_path;
		the_path = [[NSBundle mainBundle] pathForResource:kWelcomeLuaConfigurationFile ofType:@"lua"];
		if( nil != the_path )
		{
			welcomeLuaConfiguration = lua_open();
			
			if( NULL != welcomeLuaConfiguration )
			{
				int the_error;
				
				the_error = luaL_loadfile(welcomeLuaConfiguration, [the_path fileSystemRepresentation]) || lua_pcall(welcomeLuaConfiguration, 0, 0, 0);
				if(the_error)
				{
					//			NSLog(@"error");
					NSLog(@"Welcome Window Lua config load failed: %s", lua_tostring(welcomeLuaConfiguration, -1));
					lua_pop(welcomeLuaConfiguration, 1); /* pop error message from stack */
					lua_close(welcomeLuaConfiguration);
					welcomeLuaConfiguration = NULL;
				}
			}
		}
		// A little tricky: If it is easy to find the path,
		// we get the string here.
		// But if we can't find it, an asynchronous spotlight search starts
		// and we must set the path in a callback later.
#if 1
		NSString* sample_code_path = [self findSampleCodePath];
		if(nil != sample_code_path)
		{
			sampleCodeDirectory = [sample_code_path retain];
			didFindSampleCodeDirectory = YES;
		}
#endif

		
	}
	return self;
}

- (void) awakeFromNib
{
	// Update: We don't have permission to use these fonts.
	// This code block is disabled, but if we ever need to load custom fonts, this is how it is done.
/*
	FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Bold", welcomeToCoronaLabel);
	FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Bold", welcomeToCoronaLabel2);
	FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Bold", welcomeToCoronaLabelTM);
	FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Roman", examplesLabel);
*/
	
	// set back pointer on the window to the controller for convenience in the AppDelegate which lacks a direct reference to the controller.
	[(WelcomeWindow*)[self window] setWindowControllerBackPointer:self];
	
	
#if ENABLE_WINDOW_FADE_ANIMATIONS
	[[self window] setAlphaValue:0.0];
	[self performSelector:@selector(startWindowFadeInAnimation:) withObject:nil afterDelay:0.0];
#elif ENABLE_LICENSE_AGREEMENT_VIEW // WindowFade handles the license, but if WindowFade is disabled, we need this case
	BOOL did_agree_to_license = [[NSUserDefaults standardUserDefaults] boolForKey:kDidAgreeToLicense];
	if(NO == did_agree_to_license)
	{
		[self setupLicenseAnimation];
	}
#endif

	// Make text look embossed
	[[dontShowThisAgainButton cell] setBackgroundStyle:NSBackgroundStyleRaised];
	
	NSAttributedString* shadow_string = nil;
	
	shadow_string =  [self dropShadowAttributedStringFromAttributedString:[welcomeToCoronaLabel attributedStringValue]];
	[welcomeToCoronaLabel setAttributedStringValue:shadow_string];

	shadow_string = [self dropShadowAttributedStringFromAttributedString:[welcomeToCoronaLabel2 attributedStringValue]];
	[welcomeToCoronaLabel2 setAttributedStringValue:shadow_string];

	shadow_string = [self dropShadowAttributedStringFromAttributedString:[welcomeToCoronaLabelTM attributedStringValue]];
	[welcomeToCoronaLabelTM setAttributedStringValue:shadow_string];
}

- (void) dealloc
{
//	NSLog(@"dealloc WindowController");
	[(WelcomeWindow*)[self window] setWindowControllerBackPointer:nil];

	[[NSNotificationCenter defaultCenter] removeObserver:self];

	self.welcomeToCoronaLabel = nil;
	self.welcomeToCoronaLabel2 = nil;
	self.welcomeToCoronaLabelTM = nil;
	self.examplesLabel = nil;
	self.dontShowThisAgainButton = nil;
	self.licenseView = nil;
	self.passwordView = nil;
	self.layerContainerView = nil;
	self.dimView = nil;
	
	[licenseViewController release];
	
	[metadataQuery release];
	[sampleCodeDirectory release];

	lua_close(welcomeLuaConfiguration);

	[super dealloc];
}

// Just in case we ever need it
- (void) finalize
{
	lua_close(welcomeLuaConfiguration);
	[super finalize];
}

- (IBAction) closeWindow:(id)the_sender
{
	// Call performClose on the window so we get the option to override with windowShouldClose
	[[self window] performClose:the_sender];
	//	[self close];
}


#pragma mark Layer Backed Views
- (NSView*) layerContainerView
{
	if(nil == layerContainerView)
	{
		// Make the frame the same size as the window's content view
		NSRect the_frame = [[[self window] contentView] bounds];
		layerContainerView = [[NSView alloc] initWithFrame:the_frame];
		[layerContainerView setWantsLayer:YES];
	}
	return layerContainerView;
}
- (void) setLayerContainerView:(NSView*)layer_container_view
{
	if(layer_container_view != layerContainerView)
	{
		[layerContainerView release];
		layerContainerView = layer_container_view;
		[layerContainerView retain];
	}
}


- (DimView*) dimView
{
	if(nil == dimView)
	{
		// Make the frame the same size as the window's content view
		NSRect the_frame = [[[self window] contentView] bounds];
		dimView = [[DimView alloc] initWithFrame:the_frame];
		[dimView setWantsLayer:YES];
	}
	return dimView;
}
- (void) setDimView:(DimView*)dim_view
{
	if(dim_view != dimView)
	{
		[dimView release];
		dimView = dim_view;
		[dimView retain];
	}
}

#pragma mark Core Animation support methods
- (void) setupLicenseAnimation
{
	[self setMouseOverOnMainWindow:YES];

	licenseViewController = [[LicenseViewController alloc] initWithNibName:@"LicenseView" bundle:nil];
	licenseViewController.forwardMessageObject = self;
	licenseView = [licenseViewController view];
	
	
	NSRect license_frame = [licenseView frame];
	// FIXME: These coordinates are eyeballed
	NSPoint license_position = NSMakePoint(200, 600);
	license_frame.origin = license_position;
	
	
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:0.0f];
	
	//	[dimView setAlphaValue:0.1];
	[[[self dimView] animator] setAlphaValue:0.0];
	
	// Add the layer backed container view to the window's content view.
	// To avoid making the main view layer backed (because of weird glitches we are seeing with NSBox),
	// this should be a peer of the main view.
	[[[self window] contentView] addSubview:self.layerContainerView];
	
	[self.layerContainerView addSubview:[self dimView]];
	[self.layerContainerView addSubview:licenseView];
	[licenseView setFrame:license_frame];
	
	[NSAnimationContext endGrouping];
	
	
	// Ugh. Setting the alpha animation now seems to apply immediately, presumably related 
	// to the fact that the view was initially added this loop.
	// Schedule the fade animation to start in the next loop to workaround.
	[self performSelector:@selector(startDimViewFadeIn:) withObject:nil afterDelay:0.0];
	[self performSelector:@selector(startLicenseAnimateIn:) withObject:nil afterDelay:0.0];
	
}

- (void) startDimViewFadeIn:(id)user_data
{
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:WELCOME_LICENSE_ANIMATION_IN_DURATION];
	
	[[dimView animator] setAlphaValue:0.9];
	[NSAnimationContext endGrouping];
}

- (void) startLicenseAnimateIn:(id)user_data
{
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:WELCOME_LICENSE_ANIMATION_IN_DURATION];
	
	NSPoint license_position = NSMakePoint(200, 0);
	//	license_frame.origin = license_position;
	
	
	
	[[licenseView animator] setFrameOrigin:license_position];
	[NSAnimationContext endGrouping];
	
}

- (void) startWindowFadeInAnimation:(id)user_data
{
	CABasicAnimation* fade_animation = [CABasicAnimation animation];
	[fade_animation setValue:@"windowFadeIn" forKey:@"name"];
	if([user_data isKindOfClass:[NSNumber class]])
	{
		// Assuming elapsed time was from fade out that got interrupted part way through.
		// We want to start at the alpha value that we think is currently on screen
		NSNumber* elapsed_number = (NSNumber*)user_data;
		CFTimeInterval elapsed_time = [elapsed_number doubleValue];
		CFTimeInterval delta_time = elapsed_time / WELCOME_FADE_OUT_DURATION;
		// Linear interpolation formula
		// X = x0 + (x1-x0)t 
		CFTimeInterval current_alpha = 1.0 + (0.0 - 1.0) * delta_time;
		if(current_alpha < 0.0)
		{
			current_alpha = 0;
		}
		else if(current_alpha > 1.0)
		{
			current_alpha = 1.0;
		}
//		NSLog(@"current alpha=%f", current_alpha);
		fade_animation.fromValue = [NSNumber numberWithDouble:current_alpha];	
	}
	else
	{
		fade_animation.fromValue = [NSNumber numberWithFloat:0.0f];
	}

	fade_animation.toValue = [NSNumber numberWithFloat:1.0f];		
	fade_animation.delegate = self;
	fade_animation.duration = WELCOME_FADE_IN_DURATION;
	fade_animation.removedOnCompletion = YES;
	[[self window] setAnimations:[NSDictionary dictionaryWithObject:fade_animation forKey:@"alphaValue"]];
	[[[self window] animator] setAlphaValue:1.0];
	windowFadeAnimationStartTime = CACurrentMediaTime();
	
}

- (void)animationDidStop:(CAAnimation*)the_animation finished:(BOOL)finished_naturally
{
	NSString* animation_name = [the_animation valueForKey:@"name"];
	if([animation_name isEqualToString:@"windowFadeOut"])
	{
		// Animation might be interrupted by resurrection so check if finished naturally
		if(YES == finished_naturally)
		{
			// There seems to be a retain cycle here. 
			// Clear the animations list to break the cycle so the WindowController can be released.
			[[self window] setAnimations:nil];
			// expect that windowWillClose: will get called, but windowShouldClose will be bypassed
			[self close];
		}
	}
	else if([animation_name isEqualToString:@"windowFadeIn"])
	{
		// There seems to be a retain cycle here. 
		// Clear the animations list to break the cycle so the WindowController can be released.
		[[self window] setAnimations:nil];	
#if ENABLE_LICENSE_AGREEMENT_VIEW
		BOOL did_agree_to_license = [[NSUserDefaults standardUserDefaults] boolForKey:kDidAgreeToLicense];
		if(NO == did_agree_to_license)
		{
			[self setupLicenseAnimation];
		}
#endif	
	}
}

- (void) startWindowFadeOutAnimation:(id)user_data
{
	CABasicAnimation* fade_animation = [CABasicAnimation animation];
	[fade_animation setValue:@"windowFadeOut" forKey:@"name"];
	fade_animation.toValue = [NSNumber numberWithFloat:0.0f];
	fade_animation.delegate = self;
	fade_animation.duration = WELCOME_FADE_OUT_DURATION;
	fade_animation.removedOnCompletion = YES;
	[[self window] setAnimations:[NSDictionary dictionaryWithObject:fade_animation forKey:@"alphaValue"]];
	[[[self window] animator] setAlphaValue:0.0];
	windowFadeAnimationStartTime = CACurrentMediaTime();
}

- (void) setupPasswordAnimation
{
	passwordViewController = [[PasswordViewController alloc] initWithNibName:@"PasswordView" bundle:nil];
	passwordViewController.forwardMessageObject = self;
	passwordView = [passwordViewController view];
	NSLog(@"passwordView: %@", passwordView);
	NSRect password_frame = [passwordView frame];
	// FIXME: These coordinates are eyeballed
	NSPoint password_position = NSMakePoint(200, 200);
	password_frame.origin = password_position;
	[passwordView setFrame:password_frame];
/*	

	
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:0.0f];
	
	//	[dimView setAlphaValue:0.1];
	[[[self dimView] animator] setAlphaValue:0.0];
	
	// Add the layer backed container view to the window's content view.
	// To avoid making the main view layer backed (because of weird glitches we are seeing with NSBox),
	// this should be a peer of the main view.
	[[[self window] contentView] addSubview:self.layerContainerView];
	
	[self.layerContainerView addSubview:[self dimView]];
	[self.layerContainerView addSubview:passwordView];
	[passwordView setFrame:password_frame];
	
	[NSAnimationContext endGrouping];
	
	// Ugh. Setting the alpha animation now seems to apply immediately, presumably related 
	// to the fact that the view was initially added this loop.
	// Schedule the fade animation to start in the next loop to workaround.
	[self performSelector:@selector(startDimViewFadeIn:) withObject:nil afterDelay:0.0];
	[self performSelector:@selector(startPasswordAnimateIn:) withObject:nil afterDelay:0.0];
*/

    CATransition* ca_transition = [CATransition animation];
    [ca_transition setType:kCATransitionPush];
    [ca_transition setSubtype:kCATransitionFromLeft];
	ca_transition.duration = WELCOME_PASSWORD_ANIMATION_IN_DURATION;
    NSDictionary* transition_animation = [NSDictionary dictionaryWithObject:ca_transition forKey:@"subviews"];
    [self.layerContainerView setAnimations:transition_animation];
	
	
    [[self.layerContainerView animator] replaceSubview:licenseView with:passwordView];
	[licenseViewController release];
	licenseViewController = nil;
	licenseView = nil;	
}


- (void) startPasswordAnimateIn:(id)user_data
{
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:WELCOME_LICENSE_ANIMATION_IN_DURATION];
	
	NSPoint password_position = NSMakePoint(200, 0);
	//	license_frame.origin = license_position;
	
	
	
	[[passwordView animator] setFrameOrigin:password_position];
	[NSAnimationContext endGrouping];
	
}


#pragma mark Window delegate methods

#if ENABLE_WINDOW_FADE_ANIMATIONS
// Override to prevent window from immediately closing so we can animate its departure.
- (BOOL) windowShouldClose:(id)the_sender
{
//	NSLog(@"windowShouldClose");
	self.windowGoingAway = YES;
	[self startWindowFadeOutAnimation:nil];
	return NO;
}
#endif

// Warning: Watch out for the control flow due to overriding windowShouldClose for fadeout
// NSWindow performClose will invoke windowShouldClose which will avoid calling this method if it returns NO.
// But NSWindowController close or NSWindow close will bypass windowShouldClose and come here directly.
- (void) windowWillClose:(NSNotification*)the_notification
{
//	NSLog(@"windowWillClose");

	// This will allow this window controller to be released automatically
	// when the user closes the window.
	// Assumes that the window has set this controller as its delegate for this method to be called.
	// (This can be done in IB.)
	[self autorelease];
}

- (void) resurrectWindow
{
	if(NO == self.windowGoingAway)
	{
		return;
	}
/*
	if(windowNeedsResurrection == needs_resurrection)
	{
		return;
	}
	windowNeedsResurrection = needs_resurrection;
*/	
		
	
	NSNumber* elapsed_time = [NSNumber numberWithDouble:CACurrentMediaTime() - windowFadeAnimationStartTime];
	[[self window] setAnimations:nil];
	NSLog(@"elapsed_time: %@", elapsed_time);
	
	[self startWindowFadeInAnimation:elapsed_time];

//		windowNeedsResurrection = NO;
	self.windowGoingAway = NO;



	
}

#pragma mark Main Window helper methods

// Helper method to help create an NSAttributedString with a drop shadow. Intended for the main "Welcome to Corona" text.
- (NSAttributedString*) dropShadowAttributedStringFromAttributedString:(NSAttributedString*)source_string
{
	if(nil == source_string)
	{
		return nil;
	}
	// Create the shadow that sits behind the text
	NSShadow* drop_shadow = [[[NSShadow alloc] init] autorelease];
	[drop_shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.5]];
	[drop_shadow setShadowOffset:NSMakeSize(4.0, -4.0)];
	[drop_shadow setShadowBlurRadius:5.0];
	
	// Grab the attributes from the original string. If there are more than one attributes set, this will only grab the one at the first character.
	NSMutableDictionary* shadow_attributes = [[[source_string attributesAtIndex:0 effectiveRange:NULL] mutableCopy] autorelease];
	// Set the shadow attribute
	[shadow_attributes setObject:drop_shadow forKey:NSShadowAttributeName];
	
	// Create a new attributed string with your attributes dictionary attached
	NSAttributedString* ret_string = [[NSAttributedString alloc] initWithString:[source_string string] attributes:shadow_attributes];
	[ret_string autorelease];
	
	return ret_string;
}


#pragma mark IBAction methods for left-side pane

- (IBAction) openProject:(id)the_sender
{
	AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	// Pass self because the AppDelegate has special hooks to close this window
	[app_delegate open:self];
}

- (IBAction) buildProject:(id)the_sender
{
	AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	[app_delegate openForBuild:self];
}

- (IBAction) launchGettingStarted:(id)the_sender
{
	NSString* url_string;

	url_string = @"http://developer.coronalabs.com/content/getting-started-guide/";

	if( NULL != welcomeLuaConfiguration )
	{
		lua_getglobal( welcomeLuaConfiguration, kGettingStartedURL );
		if( lua_isstring( welcomeLuaConfiguration, -1 ) )
		{
			url_string = [NSString stringWithUTF8String:lua_tostring( welcomeLuaConfiguration, -1 )];
		}
		lua_pop( welcomeLuaConfiguration, 1 );
	}

	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url_string]];
}

- (IBAction) launchProgrammingGuide:(id)the_sender
{
	NSString* url_string = @"http://developer.coronalabs.com/content/application-programming-guide-projects/";
	if( NULL != welcomeLuaConfiguration )
	{
		lua_getglobal( welcomeLuaConfiguration, kProgrammingGuideURL );
		if( lua_isstring( welcomeLuaConfiguration, -1 ) )
		{
			url_string = [NSString stringWithUTF8String:lua_tostring( welcomeLuaConfiguration, -1 )];
		}
		lua_pop( welcomeLuaConfiguration, 1 );
	}
	
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url_string]];
}

#pragma mark Spotlight support methods
- (void) spotlightForExampleDirectory
{
    NSMetadataQuery* metadata_query = [[NSMetadataQuery alloc] init];
	metadataQuery = metadata_query;
	// We use this file as the anchor or magnet file for Spotlight.
#ifdef Rtt_GAMEEDITION
	NSString* predicate_string = @"kMDItemDisplayName == 'About_Corona_Game_Edition_Sample_Code.txt'";
#else
	NSString* predicate_string = @"kMDItemDisplayName == 'About_Corona_Sample_Code.txt'";
#endif
	
	if( NULL != welcomeLuaConfiguration )
	{
		lua_getglobal( welcomeLuaConfiguration, kSpotlightAnchorFile );
		if( lua_isstring( welcomeLuaConfiguration, -1 ) )
		{
			predicate_string = [NSString stringWithFormat:@"kMDItemDisplayName == '%s'", lua_tostring( welcomeLuaConfiguration, -1 )];
		}
		lua_pop( welcomeLuaConfiguration, 1 );
	}
	
	NSPredicate* search_predicate = [NSPredicate predicateWithFormat:predicate_string]; 
    [metadata_query setPredicate:search_predicate];
    
	[metadata_query setSortDescriptors:[NSArray arrayWithObject:[[[NSSortDescriptor alloc] initWithKey:(id)kMDItemFSContentChangeDate ascending:NO] autorelease]]];
		
		
    [[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(queryHandler:)
		name:NSMetadataQueryDidFinishGatheringNotification
		object:metadata_query];

	self.spotlightQueryIsRunning = YES;
    [metadata_query startQuery];
    // whatever comes now happens immediately:
}

// Runs a quick check to see if we can find a sample lua program from the sample code base path
// example expected string: /Volumes/CoronaSDK/Corona2.0-Beta5/SampleCode
- (BOOL) quickTestForExistanceWithSampleCodePath:(NSString*)sample_code_path
{
	NSString* relative_path_to_quick_test = kDefaultPathToCoronaExampleForQuickTestOfSampleDirectory;
	
	if( NULL != welcomeLuaConfiguration )
	{
		lua_getglobal( welcomeLuaConfiguration, kPathToCoronaExampleForQuickTestOfSampleDirectory );
		if( lua_isstring( welcomeLuaConfiguration, -1 ) )
		{
			relative_path_to_quick_test = [NSString stringWithUTF8String:lua_tostring( welcomeLuaConfiguration, -1 )];
		}
		lua_pop( welcomeLuaConfiguration, 1 );
	}
	
	NSString* test_path = [sample_code_path stringByAppendingPathComponent:relative_path_to_quick_test];


	if([[NSFileManager defaultManager] fileExistsAtPath:test_path])
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

- (void) queryHandler:(NSNotification*)the_notification
{
	// Assertion: metadataQuery == metadata_query
	NSMetadataQuery* metadata_query = [the_notification object];
	[metadata_query stopQuery];

	for(NSMetadataItem* metadata_item in [metadata_query results])
	{
/*
		NSLog(@"results: %@", [metadata_item attributes]);
		for(id an_attribute in [metadata_item attributes])
		{
			NSLog(@"an_attribute: %@, %@", an_attribute, [metadata_item valueForAttribute:an_attribute]);
		}
*/
		// Strange: kMDItemPath is not listed in the list of attributes, but we can ask for it directly
		// example expected string: /Volumes/CoronaSDK/Corona2.0-Beta8/SampleCode/About_Corona_Sample_Code.txt
		NSString* store_path = [[metadata_item valueForAttribute:(NSString*)kMDItemPath] stringByResolvingSymlinksInPath];
//		NSLog(@"store_path: %@", store_path);
		// example expected string: /Volumes/CoronaSDK/Corona2.0-Beta5/SampleCode
		NSString* spotlight_sample_code_dir = [store_path stringByDeletingLastPathComponent];
//		NSLog(@"spotlight sample_code dir: %@", spotlight_sample_code_dir);
		
		BOOL passed_test = [self quickTestForExistanceWithSampleCodePath:spotlight_sample_code_dir];
		if(YES == passed_test)
		{
//			NSLog(@"Odds are pretty good this is what we are looking for");
			didFindSampleCodeDirectory = YES;
			self.sampleCodeDirectory = spotlight_sample_code_dir;
			// Save to the cache
			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
			[app_delegate setCachedSampleDirectoryPath:spotlight_sample_code_dir];

			break;
		}
		else
		{
			continue;
		}
						
	}

	[[NSNotificationCenter defaultCenter]
		removeObserver:self
		name:NSMetadataQueryDidFinishGatheringNotification
		object:metadata_query];

	[metadataQuery release];
	metadataQuery = nil;
	metadata_query = nil;
	
	self.spotlightQueryIsRunning = NO;
	// The spotlight query is asynchronous, but seems to be running stuff on the main thread.
	// So I can't sleep or busy-wait/spin to let the query finish because it will block the runloop
	// and prevent the query from finishing.
	// I found this article that said starting a new runloop seems to solve the problem.
	// http://confuseddevelopment.blogspot.com/2006/10/waiting-for-spotlight-query-to-finish.html
	// This part will stop the CFRunloop if it is running and allow the other section that is waiting
	// on it to resume.	
	CFRunLoopStop(CFRunLoopGetCurrent());


}

// Uses Spotlight in the bad case, so we want to call early to minimize user waiting
// If nil is returned, a spotlight query is running and you have to get the value later.
- (NSString*) findSampleCodePath
{
	// Check if we already found the path
	AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	NSString* cached_sample_dir_path = [app_delegate cachedSampleDirectoryPath];
	if(nil != cached_sample_dir_path)
	{
		return cached_sample_dir_path;
	}
	
	
	NSString* path_to_sample_code = nil;
	NSString* corona_simulator_path = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
	NSFileManager* file_manager = [NSFileManager defaultManager];
	path_to_sample_code = [corona_simulator_path stringByAppendingPathComponent:@"SampleCode"];
	BOOL is_directory = NO;


	if([file_manager fileExistsAtPath:path_to_sample_code isDirectory:&is_directory] && YES == is_directory)
	{
	
		BOOL passed_test = [self quickTestForExistanceWithSampleCodePath:path_to_sample_code];
		if(YES == passed_test)
		{
//			NSLog(@"We found the sample code directory: %@", path_to_sample_code);
			// save the path to the cache
			[app_delegate setCachedSampleDirectoryPath:path_to_sample_code];
			return path_to_sample_code;
		}
		else
		{
//			NSLog(@"We did not find the sample code directory, try something else");
			[self spotlightForExampleDirectory];
		}
	}
	else
	{
//		NSLog(@"We did not find the sample code directory, try something else");
		[self spotlightForExampleDirectory];
		
	}
	return nil;
}

- (NSString*) examplePathFromFragment:(NSString*)fragment_string
{
	return [self.sampleCodeDirectory stringByAppendingPathComponent:fragment_string];
}

#pragma mark IBAction methods for right-side pane
// Assumes tags are set correctly in IB
- (IBAction) openCoronaExample:(id)the_sender
{
	NSInteger tag_id = [the_sender tag];
//	NSLog(@"%@, tag:%d", NSStringFromSelector(_cmd), tag_id);

	// The spotlight query is asynchronous, but seems to be running stuff on the main thread.
	// So I can't sleep or busy-wait/spin to let the query finish because it will block the runloop
	// and prevent the query from finishing.
	// I found this article that said starting a new runloop seems to solve the problem.
	// http://confuseddevelopment.blogspot.com/2006/10/waiting-for-spotlight-query-to-finish.html
	if(YES == self.spotlightQueryIsRunning)
	{
		CFRunLoopRun();
	}

	if(YES == didFindSampleCodeDirectory)
	{
		[self runExampleWithTag:tag_id];		
	}
	else
	{
		// Just open a panel and let the user deal with it?
		AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
		NSString* app_path = [app_delegate showOpenPanel:nil withAccessoryView:[app_delegate openAccessoryView] startDirectory:nil];
		if ( nil != app_path )
		{
			[app_delegate close:nil];
			[self closeWindow:nil];
			[app_delegate updateSkin];
			[app_delegate runApp:app_path];
		}
		else
		{
			// user cancelled
		}
	}

}

- (void) runExampleWithTag:(NSInteger)tag_id
{
	NSString* example_path = nil;
	NSString* path_fragment = nil;
	
	if( NULL != welcomeLuaConfiguration )
	{
		lua_getglobal( welcomeLuaConfiguration, kPathToCoronaExamples );
		if( lua_istable( welcomeLuaConfiguration, -1 ) )
		{
			lua_pushinteger( welcomeLuaConfiguration, tag_id ); // pushes the array index
			lua_gettable( welcomeLuaConfiguration, -2 ); // gets the path_fragment = pathToCoronaExamples[index]
			if( lua_isstring( welcomeLuaConfiguration, -1 ) )
			{
				path_fragment = [NSString stringWithUTF8String:lua_tostring( welcomeLuaConfiguration, -1 )];
			}
			lua_pop( welcomeLuaConfiguration, 1 ); // pops the result
		}
		lua_pop( welcomeLuaConfiguration, 1 ); // pops getglobal
	}
	
	if( nil != path_fragment )
	{
		example_path = [self examplePathFromFragment:path_fragment];
	}
	
	if([[NSFileManager defaultManager] fileExistsAtPath:example_path])
	{
		AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
		[app_delegate close:nil];
		[self closeWindow:nil];
		[app_delegate runApp:example_path];
	}
	else
	{
		// Just open a panel and let the user deal with it?
		AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
		NSString* app_path = [app_delegate showOpenPanel:nil withAccessoryView:[app_delegate openAccessoryView] startDirectory:nil];
		if ( nil != app_path )
		{
			[app_delegate close:nil];
			[self closeWindow:nil];
			[app_delegate updateSkin];
			[app_delegate runApp:app_path];
		}
		else
		{
			// user cancelled
		}
	}
}

- (IBAction) openMoreCoronaExamples:(id)the_sender
{

	// The spotlight query is asynchronous, but seems to be running stuff on the main thread.
	// So I can't sleep or busy-wait/spin to let the query finish because it will block the runloop
	// and prevent the query from finishing.
	// I found this article that said starting a new runloop seems to solve the problem.
	// http://confuseddevelopment.blogspot.com/2006/10/waiting-for-spotlight-query-to-finish.html
	if(YES == self.spotlightQueryIsRunning)
	{
		CFRunLoopRun();
	}
	
	AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	if(YES == didFindSampleCodeDirectory)
	{
		NSString* app_path = [app_delegate showOpenPanel:nil withAccessoryView:[app_delegate openAccessoryView] startDirectory:self.sampleCodeDirectory];
		if ( nil != app_path )
		{
			[app_delegate close:nil];
			[self closeWindow:nil];
			[app_delegate updateSkin];
			[app_delegate runApp:app_path];
		}
		else
		{
			// user cancelled
		}
	}
	else
	{
		// Just open a panel and let the user deal with it?
		NSString* app_path = [app_delegate showOpenPanel:nil withAccessoryView:[app_delegate openAccessoryView] startDirectory:nil];
		if ( nil != app_path )
		{
			[app_delegate close:nil];
			[self closeWindow:nil];
			[app_delegate updateSkin];
			[app_delegate runApp:app_path];
		}
		else
		{
			// user cancelled
		}
	}
}


#pragma mark License agreement actions

- (IBAction) acceptLicense:(id)the_sender
{

	[[NSUserDefaults standardUserDefaults] setBool:YES forKey:kDidAgreeToLicense];

#if 0

	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:WELCOME_LICENSE_ANIMATION_OUT_DURATION];

	[[licenseView animator] removeFromSuperview];
	[[[self dimView] animator] removeFromSuperview];

	[NSAnimationContext endGrouping];

	[licenseViewController release];
	licenseViewController = nil;
	licenseView = nil;
	self.dimView = nil;
#else
	
	[self setupPasswordAnimation];
	


#endif

}


- (IBAction) rejectLicense:(id)the_sender
{
	
	[[NSUserDefaults standardUserDefaults] setBool:NO forKey:kDidAgreeToLicense];
	[NSApp terminate:the_sender];
	
}
- (IBAction) cancelPasswordLogin:(id)the_sender
{
    [self.layerContainerView setAnimations:nil];

	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:WELCOME_LICENSE_ANIMATION_OUT_DURATION];
	
	[[passwordView animator] removeFromSuperview];
	[[[self dimView] animator] removeFromSuperview];
	
	[NSAnimationContext endGrouping];

	[licenseViewController release];
	licenseViewController = nil;
	licenseView = nil;
	
	[passwordView release];
	passwordView = nil;
	passwordView = nil;
	self.dimView = nil;
	
	[self setMouseOverOnMainWindow:NO];

}

- (void) setMouseOverOnMainWindow:(BOOL)is_enabled forView:(NSView*)the_view
{
	if([the_view respondsToSelector:@selector(setMouseOverDisabled:)])
	{
		[(PassiveHitView*)the_view setMouseOverDisabled:is_enabled];
	}
	
	for(NSView* a_view in [the_view subviews])
	{
		[self setMouseOverOnMainWindow:is_enabled forView:a_view];
	}
}

- (void) setMouseOverOnMainWindow:(BOOL)is_enabled
{
	NSView* the_view = [[self window] contentView];
	[self setMouseOverOnMainWindow:is_enabled forView:the_view];
}

@end

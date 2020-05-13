//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacDialogController.h"
#include "Rtt_MPlatform.h"
#import "AppDelegate.h"

#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>
#import <AppKit/NSAlert.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSPanel.h>

static NSUInteger kMaxUsernameLength = 255;
static NSUInteger kMaxPasswordLength = 255;

// ----------------------------------------------------------------------------

static const char kDoNotStopModal[] = "";

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------

@interface TrialDelegate : NSObject< DialogControllerDelegate >
{
	const Rtt::MPlatform *fPlatform;
	const char *fUrl; // NEW TO GE
}

-(id)initWithPlatform:(const Rtt::MPlatform*)platform;

-(void)setUrl:(const char*)url; // NEW TO GE

// For alert view delegate
-(void)alertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;

@end


@implementation TrialDelegate

-(id)initWithPlatform:(const Rtt::MPlatform*)platform
{
	self = [super init];
	if ( self )
	{
		Rtt_ASSERT( platform );
		fPlatform = platform;
		fUrl = "https://solar2d.com/"; // NEW TO GE
	}
	return self;
}

-(void)setUrl:(const char*)url // NEW TO GE
{
	Rtt_ASSERT( url );
	fUrl = url;
}

-(BOOL)shouldStopModal:(DialogController*)sender withCode:(NSInteger)code
{
	BOOL result = YES;

	switch( code )
	{
		case kActionDefault:
			fPlatform->OpenURL( fUrl );
			result = NO;
			break;
		case kActionAlternate:
			result = NO;
			break;
		case kActionOther1:
			result = YES;
			break;
	}

	return result;
}

-(void)alertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	if ( NSAlertFirstButtonReturn == returnCode )
	{
		if ( kDoNotStopModal == contextInfo )
		{
			return;
		}

		if ( contextInfo )
		{
			fPlatform->OpenURL( (const char*)contextInfo );
		}
		else
		{
			[[NSApplication sharedApplication] stopModalWithCode:kActionAlternate];
		}
	}
	else
	{
		// App flow no longer means cancelling login should exit
		// [[NSApplication sharedApplication] terminate:self];
	}
}

-(void)sheetDidEnd:(NSWindow*)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	DialogController *controller = (DialogController*)contextInfo;
	if ( returnCode == kActionDefault )
	{
		[sheet close];
	}
}

@end

// ----------------------------------------------------------------------------

@implementation DialogController

@synthesize inputs;
@synthesize userdata;
@synthesize fWindow;

-(id)initWithNibNamed:(NSString*)name
{
	return [self initWithNibNamed:name delegate:nil];
}

-(id)initWithNibNamed:(NSString*)name delegate:(id<DialogControllerDelegate>)delegate
{
	self = [super init];
	if ( self )
	{
		fDelegate = delegate;
		fSheetDelegate = nil;
		inputs = [[NSMutableDictionary alloc] init];
		userdata = NULL;
		[NSBundle loadNibNamed:name owner:self];
		[fWindow setReleasedWhenClosed:NO];
		fState = kDialogControllerNormal;
	}
	return self;
}

-(void)dealloc
{
	[fWindow release];
	[inputs release];
	[super dealloc];
}

-(NSInteger)runModal
{
	return [self runModalWithMessage:nil];
}

-(NSInteger)runModalWithMessage:(NSString *)message
{
	int response = kActionUnknown;
	if ( Rtt_VERIFY( kDialogControllerNormal == fState ) )
	{
		[fWindow center];

		if (message != nil)
		{
			[fWindow makeKeyAndOrderFront:self];
			
			NSAlert* alert = [[[NSAlert alloc] init] autorelease];
			[alert addButtonWithTitle:@"OK"];
			[alert setMessageText:message];
			[alert setAlertStyle:NSInformationalAlertStyle];
			[alert beginSheetModalForWindow:fWindow
							  modalDelegate:nil
							 didEndSelector:nil
								contextInfo:nil];
		}
		
		fState = kDialogControllerModal;
		response = (int) [[NSApplication sharedApplication] runModalForWindow:fWindow];

		// Dismiss window
		[fWindow close];

		fState = kDialogControllerNormal;
	}

	return response;
}

-(void)stopModalWithCode:(NSInteger)code
{
	[[NSApplication sharedApplication] stopModalWithCode:code];
}

-(IBAction)close:(id)sender
{
	[self stopModalWithCode:kActionClose];
}

-(void)beginSheet:(NSWindow*)parent modalDelegate:(id)delegate contextInfo:(void*)contextInfo
{
	if ( Rtt_VERIFY( kDialogControllerNormal == fState ) )
	{
		fState = kDialogControllerSheet;
		fSheetDelegate = delegate;

		// When the sheet ends, we call self's didEndSelector which calls fSheetDelegate
		// and then resets the state of the dialog controller
		SEL didEndSelector = @selector(sheetDidEnd:returnCode:contextInfo:);
		[NSApp beginSheet:fWindow modalForWindow:parent modalDelegate:self didEndSelector:didEndSelector contextInfo:contextInfo];
        [NSApp runModalForWindow:fWindow];
	}
}

-(void)sheetDidEnd:(NSWindow*)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	if ( fSheetDelegate && [fSheetDelegate respondsToSelector:@selector(sheetDidEnd:returnCode:contextInfo:)] )
	{
		[fSheetDelegate sheetDidEnd:sheet returnCode:returnCode contextInfo:contextInfo];
	}

    [self stopModalWithCode:kActionDefault];
	fSheetDelegate = nil;
	fState = kDialogControllerNormal;
}

-(BOOL)isActionEnabled:(NSInteger)code
{
	return ( ! fDelegate || ! [fDelegate respondsToSelector:@selector(isActionEnabled:forCode:)] ? YES : [fDelegate isActionEnabled:self forCode:code] );
}

-(BOOL)enabledDefault
{
	return [self isActionEnabled:kActionDefault];
}

-(BOOL)enabledAlternate
{
	return [self isActionEnabled:kActionAlternate];
}

-(BOOL)enabledOther1
{
	return [self isActionEnabled:kActionOther1];
}

-(void)handleAction:(id)sender withCode:(NSInteger)code
{
	if ( ! fDelegate || [fDelegate shouldStopModal:self withCode:code] )
	{
		if ( kDialogControllerModal == fState )
		{
			[[NSApplication sharedApplication] stopModalWithCode:code];
		}
		else
		{
			Rtt_ASSERT( kDialogControllerSheet == fState );
			[NSApp endSheet:fWindow returnCode:code];
			[fWindow orderOut:self];

			fState = kDialogControllerNormal;
		}		
	}
}

-(IBAction)actionDefault:(id)sender
{
	[self handleAction:sender withCode:kActionDefault];
}

-(IBAction)actionAlternate:(id)sender
{
	[self handleAction:sender withCode:kActionAlternate];
}

-(IBAction)actionOther1:(id)sender
{
	[self handleAction:sender withCode:kActionOther1];
}

-(IBAction)actionOther2:(id)sender
{
	[self handleAction:sender withCode:kActionOther2];
}

-(BOOL)validateValue:(id *)ioValue forKey:(NSString *)key error:(NSError **)outError
{
	if ( ! fDelegate || ! [fDelegate respondsToSelector:@selector(validateValue:forKey:error:)] )
	{
		return [super validateValue:ioValue forKey:key error:outError];
	}
	else
	{
		return [fDelegate validateValue:ioValue forKey:key error:outError];
	}
}

-(BOOL)validateValue:(id *)ioValue forKeyPath:(NSString *)inKeyPath error:(NSError **)outError
{
	if ( ! fDelegate || ! [fDelegate respondsToSelector:@selector(validateValue:forKeyPath:error:)] )
	{
		return [super validateValue:ioValue forKeyPath:inKeyPath error:outError];
	}
	else
	{
		return [fDelegate validateValue:ioValue forKeyPath:inKeyPath error:outError];
	}
}

@end


// ----------------------------------------------------------------------------


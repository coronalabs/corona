//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "AppDelegate.h"
#import "ConsoleWindowController.h"


static void NotificationCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo)
{
	AppDelegate *appDelegate = (__bridge AppDelegate*)observer;
	if(!appDelegate || !name)
	{
		return;
	}
	else if(CFStringCompare(name, CFSTR("CoronaConsole.bringToFront"), 0) == kCFCompareEqualTo)
	{
		[appDelegate bringToFront];
		CFNotificationCenterPostNotification( CFNotificationCenterGetDistributedCenter(), CFSTR("CoronaSimulator.bringToFront"), NULL, NULL, YES);

	}
	else if(CFStringCompare(name, CFSTR("CoronaConsole.clearConsole"), 0) == kCFCompareEqualTo)
	{
		[appDelegate clearConsole];
	}
}

@implementation AppDelegate

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app
{
	return NO;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(), (__bridge const void *)(self), NotificationCallback, CFSTR("CoronaConsole.clearConsole"), NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(), (__bridge const void*)(self), NotificationCallback, CFSTR("CoronaConsole.bringToFront"), NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{

	// (doing this programmatically doesn't work as the window doesn't appear so it's done in the Info.plist with LSUIElement)
	// Hide the menubar and Dock icon
	//[NSApp setActivationPolicy: NSApplicationActivationPolicyAccessory];
	// This is needed to make the window visible
	//[[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];

	setlinebuf(stdin);

	NSFileHandle *stdinHandle = [NSFileHandle fileHandleWithStandardInput];

	// Set this up to be notified when new data arrives (see [ConsoleWindowController handleStdoutNotification:])
	[[NSNotificationCenter defaultCenter] addObserver:[ConsoleWindowController sharedConsole]
											 selector:@selector(handleStdoutNotification:)
												 name:NSFileHandleReadCompletionNotification
											   object:stdinHandle];

	[stdinHandle readInBackgroundAndNotify];

	simulator = [NSRunningApplication runningApplicationWithProcessIdentifier:getppid()];

	if (! [[simulator bundleIdentifier] isEqualToString:@"com.coronalabs.Corona_Simulator"])
	{
		NSLog(@"CoronaConsole should only be run by Corona Simulator");
	}
	else
	{
		// If the parent Simulator terminates, we notice and terminate ourselves
		[simulator addObserver:self
					forKeyPath:@"isTerminated"
					   options:NSKeyValueObservingOptionNew
					   context:NULL];
	}
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	CFNotificationCenterRemoveObserver(CFNotificationCenterGetDistributedCenter(), (__bridge const void *)(self), NULL, NULL);
}

- (IBAction)performFindAction:(id)action
{
	[[ConsoleWindowController sharedConsole] performFindAction:action];
}

- (IBAction)clearConsole:(id)sender
{
	[[ConsoleWindowController sharedConsole] clearLog:sender];
}

-(void) clearConsole
{
	[self clearConsole:nil];
}

- (IBAction)gotoEndOfConsole:(id)sender
{
	[[ConsoleWindowController sharedConsole] endOfLog:sender];
}

- (IBAction)copy:(id)sender
{
	[[ConsoleWindowController sharedConsole] copy:sender];
}

- (void) bringToFront
{
	NSArray *windows = [NSApp windows];

	if ([windows count] > 0)
	{
		[[windows objectAtIndex:0] orderFrontRegardless];
	}
}

- (IBAction)nextWindow:(id)sender
{
	// If the user hits ⌘` (or ⌘~), activate the Simulator
	[simulator activateWithOptions:NSApplicationActivateAllWindows];
}

- (IBAction)quit:(id)sender
{
	// If we are asked to quit, ask the parent Simulator to quit
	// (we'll quit when we notice that the Simulator has gone)
	[simulator terminate];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object
						change:(NSDictionary *)change
					   context:(void *)context
{
	if ([keyPath isEqual:@"isTerminated"])
	{
		[[NSUserDefaults standardUserDefaults] synchronize];
		[NSApp terminate:nil];
	}
}

@end

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

#import "AppDelegate.h"
#import "ConsoleWindowController.h"

static void SignalHandler(int signal)
{
	// When we receive a SIGHUP, move ourselves to the top of the window order
	// (we need this because the interfaces Cocoa supplies (like [NSRunningApplication activateWithOptions:])
	// all affect the activation state of the app and we don't want to activate just to show the window)
	// Needs to be dispatched on the main thread because it's doing UI stuff
	dispatch_async(dispatch_get_main_queue(), ^{
		AppDelegate *appDelegate = (AppDelegate*)[NSApp delegate];

		[appDelegate bringToFront];
	});
}


@implementation AppDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
	// Catch SIGHUP (see handler above)
	struct sigaction action = { 0 };
	action.sa_handler = SignalHandler;
	sigaction(SIGHUP, &action, NULL);
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
	// Insert code here to tear down your application
}

- (IBAction)performFindAction:(id)action
{
	[[ConsoleWindowController sharedConsole] performFindAction:action];
}

- (IBAction)clearConsole:(id)sender
{
	[[ConsoleWindowController sharedConsole] clearLog:sender];
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

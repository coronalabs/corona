//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "ConsoleWindowController.h"
#import "AppDelegate.h"
#import "Theme.h"
#import "ThemeConsole.h"

#include <syslog.h>
// NSLog() is problematic use syslog: 	syslog(LOG_NOTICE, "%ld lines", [lines count]);

#ifdef Rtt_DEBUG
#define NSDEBUG(...) NSLog(__VA_ARGS__)
#else
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#endif

NSString *kUserPreferenceCurrentTheme = @"currentTheme";

static size_t ConsoleMaxChars = 10 * (1024 * 1024);
static size_t ConsoleChunkSize = 102400;
static size_t ConsoleRepeatLimit = 50;

@implementation ConsoleWindowController

+ (id)sharedConsole
{
	static ConsoleWindowController *sharedConsole = nil;

	@synchronized(self)
	{
		if (sharedConsole == nil)
		{
			sharedConsole = [[self alloc] init];
		}
	}

	return sharedConsole;
}

- (id)init
{
	self = [super initWithWindowNibName:@"ConsoleWindow"];

	if ( self )
	{
		projectPath = nil;
		popoverSizeAdjusted = NO;
		numErrors = 0;
		numWarnings = 0;
	}

	return self;
}

- (void)windowDidLoad
{
	if (floor(NSAppKitVersionNumber) < NSAppKitVersionNumber10_10)
	{
		// Too many things about the options popover don't work on OS X < 10.10 so disable it
		[_optionsButton setHidden:YES];
	}

	NSDEBUG(@"window did load ConsoleWindow");

	// These serve as "ultimate" backup values if all else fails
	fontManager = [NSFontManager sharedFontManager];
	font = [NSFont userFixedPitchFontOfSize:12];
	boldFont = [fontManager convertFont:font toHaveTrait:NSBoldFontMask];
	textFgColor = [NSColor blackColor];
	textBgColor = [NSColor whiteColor];
	timestampFgColor = [NSColor colorWithCalibratedRed:91.0/255.0 green:129.0/255.0 blue:255.0/255.0 alpha:1.0];
	timestampBgColor = textBgColor;
	errorFgColor = [NSColor whiteColor];
	errorBgColor = [NSColor colorWithCalibratedRed:207.0/255.0 green:63.0/255.0 blue:44.0/255.0 alpha:1.0];
	warningFgColor = [NSColor whiteColor];
	warningBgColor = [NSColor colorWithCalibratedRed:255.0/255.0 green:162.0/255.0 blue:0/255.0 alpha:1.0];

	// Allow window position to be saved
	[self setShouldCascadeWindows:NO];

	NSString *currentTheme = [[NSUserDefaults standardUserDefaults] stringForKey:kUserPreferenceCurrentTheme];

	if (currentTheme == nil)
	{
		currentTheme = @"Default";
	}
	[self loadTheme:currentTheme];

	size_t userSetConsoleMaxChars = [[NSUserDefaults standardUserDefaults] integerForKey:@"consoleMaxChars"];
	if (userSetConsoleMaxChars > ConsoleMaxChars)
	{
		ConsoleMaxChars = userSetConsoleMaxChars;
	}

	size_t repeatLimit = [[NSUserDefaults standardUserDefaults] integerForKey:@"consoleRepeatLimit"] - 1;
	if (repeatLimit != -1)
	{
		ConsoleRepeatLimit = repeatLimit;
	}

	NSMenu *textViewMenu = [self.textView menu];

	[textViewMenu removeItemAtIndex:[textViewMenu indexOfItemWithTitle:@"Spelling and Grammar"]];
	[textViewMenu removeItemAtIndex:[textViewMenu indexOfItemWithTitle:@"Substitutions"]];

	NSMenuItem *clearMenuItem = [[NSMenuItem alloc] initWithTitle:@"Clear Log" action:@selector(clearLog:) keyEquivalent:@""];
	[clearMenuItem setTarget:self];
	[clearMenuItem setEnabled:YES];
	[textViewMenu insertItem:clearMenuItem atIndex:0];

	NSMenuItem *sendMenuItem = [[NSMenuItem alloc] initWithTitle:@"Send Log to Corona Labs Support" action:@selector(sendLogViaEmail:) keyEquivalent:@""];
	[sendMenuItem setTarget:self];
	[sendMenuItem setEnabled:YES];
	// Only show this item if Shift and Alt are held down
	[sendMenuItem setAlternate:YES];
	[sendMenuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSShiftKeyMask)];
	[textViewMenu insertItem:sendMenuItem atIndex:1];

	[super windowDidLoad];

	
    
    self.textFinder = [[NSTextFinder alloc] init];
    [self.textFinder setClient:(id<NSTextFinderClient>)self.textView];
    [self.textFinder setFindBarContainer:[self.textView enclosingScrollView]];
    [self.textView setUsesFindBar:YES];
    [self.textFinder setIncrementalSearchingEnabled:YES];  // Disable incremental search
    [self.textFinder setIncrementalSearchingShouldDimContentView:NO];
    //[self.textFinder performAction:NSTextFinderActionShowFindInterface];

	[self setupTextAttributes];

}

- (void) setupTextAttributes
{
	[self.textView setBackgroundColor:textBgColor];

	textAttr = @{ NSFontAttributeName: font, NSForegroundColorAttributeName: textFgColor, NSBackgroundColorAttributeName: textBgColor };
	boldFontAttr = @{ NSFontAttributeName: boldFont };
	timestampAttr = @{ NSForegroundColorAttributeName: timestampFgColor, NSBackgroundColorAttributeName: timestampBgColor };
	errorAttr = @{ NSForegroundColorAttributeName: errorFgColor, NSBackgroundColorAttributeName: errorBgColor };
	warningAttr = @{ NSForegroundColorAttributeName: warningFgColor, NSBackgroundColorAttributeName: warningBgColor };

	CGFloat indent = floor([font advancementForGlyph:[font glyphWithName:@"0"]].width * 20);  // 20 is number of chars in timestamp
	paraStyle = [[NSMutableParagraphStyle alloc] init];
	[paraStyle setFirstLineHeadIndent:0];
	[paraStyle setHeadIndent:indent];

	// Setup sensible tab stops.  Unfortunately [NSMutableParagraphStyle setDefaultTabInterval:] appears
	// to have a bug which makes the first default tabstop too close to the last one defined so we
	// manually define the first 20 tabs and then set a default for the remainder.

	[paraStyle setTabStops:[NSArray array]]; // zero existing tabstops
	[paraStyle addTabStop:[[NSTextTab alloc] initWithType:NSLeftTabStopType location:indent]];
	CGFloat tabInterval = 72;
	for (int i = 1; i < 20; i++)
	{
		[paraStyle addTabStop:[[NSTextTab alloc] initWithType:NSLeftTabStopType location:indent+(i * tabInterval)]];
	}
	[paraStyle setDefaultTabInterval:tabInterval];

	paraAttr = @{ NSParagraphStyleAttributeName: paraStyle };

#ifdef Rtt_DEBUG
	// Ruler can be useful for debugging layout issues
	[self.textView setRulerVisible:YES];
	[self.textView setUsesRuler:YES];
#endif
}

- (IBAction) clearLog:(id)sender
{
	[self.textFinder noteClientStringWillChange];
	[[_textView textStorage] deleteCharactersInRange:NSMakeRange(0, [[_textView textStorage] length])];

	// reset indicators
	numWarnings = 0;
	[_warningDisplay setStringValue:@""];
	[_warningDisplay setHidden:YES];

	numErrors = 0;
	[_errorDisplay setStringValue:@""];
	[_errorDisplay setHidden:YES];

	[_jumpMenu setHidden:YES];
	[[_jumpMenu menu] removeAllItems];

	[_optionsPopover close];
}

- (IBAction) endOfLog:(id)sender
{
	[_textView scrollRangeToVisible:NSMakeRange([[_textView string] length], 0)];
}

- (IBAction) jumpToItem:(id)sender
{
	NSInteger itemIndex = [sender tag];

	[_textView scrollRangeToVisible:NSMakeRange(itemIndex, 0)];
}

- (IBAction) copy:(id)sender
{
	NSRange selectedRange = [self.textView selectedRange];

	// If nothing is selected, copy everything
	if (selectedRange.length == 0)
	{
		selectedRange = NSMakeRange(0, [[self.textView string] length]);
	}

	NSString *selectedText = [[self.textView string] substringWithRange:selectedRange];

	selectedText = [selectedText stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", (unichar)NSLineSeparatorCharacter] withString:@"\n                    "];
	selectedText = [selectedText stringByReplacingOccurrencesOfString:@"\r" withString:@"\n"];

	NSPasteboard *pasteBoard = [NSPasteboard generalPasteboard];
	[pasteBoard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
	[pasteBoard setString:selectedText forType:NSStringPboardType];
}

- (IBAction)options:(id)sender
{
	if (([[NSApp currentEvent] modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
	{
		[_sendLogButton setHidden:NO];

		if (popoverSizeAdjusted)
		{
			NSSize viewSize = [_optionsView frame].size;
			viewSize.height += 30;
			[_optionsView setFrameSize:viewSize];
			if ([_optionsPopover respondsToSelector:@selector(setContentSize:)])
			{
				[_optionsPopover setContentSize:viewSize];
			}
			popoverSizeAdjusted = NO;
		}
	}
	else
	{
		[_sendLogButton setHidden:YES];

		if (! popoverSizeAdjusted)
		{
			NSSize viewSize = [_optionsView frame].size;
			viewSize.height -= 30;
			[_optionsView setFrameSize:viewSize];
			if ([_optionsPopover respondsToSelector:@selector(setContentSize:)])
			{
				[_optionsPopover setContentSize:viewSize];
			}
			popoverSizeAdjusted = YES;
		}
	}

	[self populateThemesMenu];
	
	[_optionsPopover showRelativeToRect:[sender bounds] ofView:sender preferredEdge:NSMaxXEdge];
}

- (void) handleStdoutNotification:(NSNotification *)notification
{
	@autoreleasepool
	{
		NSData *data = [[notification userInfo] objectForKey:NSFileHandleNotificationDataItem];

		// NSDEBUG(@"handleStdoutNotification: got %ld bytes", [data length]);

		// This condition occurs when EOF is encountered
		if ([data length] == 0)
		{
			[NSApp terminate:nil];

			return;
		}

		// The vagaries of stdout buffering mean we can get multiple logs in one read so break them
		// up at newlines to make the formatting better (actual newlines in the log entries will be
		// replaced with carriage returns and handled in [log:])
		NSString *dataStr = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];
		NSString *lastLine = nil;
		int count = 0;

		// If we get multiple lines, minimize output so we can keep up by accumulating many lines in a buffer
		// separated by carriage returns so that they are displayed efficiently with just one timestamp.
		// Also summarize lines which are repeated multiple times to make the log more readable
		// (note that in real world use most repeated logs will get sent in different notifications so this
		// process wont come into play but when an app is spamming the log it will improve efficiency a lot
		// because the pipe will get backlogged)
		NSString *buf = @"";
		for (NSString *line in [dataStr componentsSeparatedByString:@"\n"])
		{
			// does this line differ from the previous one
			if (! [line isEqualToString:lastLine] || count > 1000)
			{
				//syslog(LOG_NOTICE, "line '%s', lastLine '%s'", [line UTF8String], [lastLine UTF8String]);
				if (count == 1)
				{
					// if line is repeated twice, just show both instances
					buf = [buf stringByAppendingString:lastLine];
					buf = [buf stringByAppendingString:@"\r"];
				}
				else if (count > ConsoleRepeatLimit)
				{
					// previous line was repeated 'count' times
					buf = [buf stringByAppendingFormat:@"### repeats %d times ###\r", count];
				}
				else
				{
					for (int i = 0; i < count; i++)
					{
						buf = [buf stringByAppendingString:lastLine];
						buf = [buf stringByAppendingString:@"\r"];
					}
				}

				lastLine = [NSString stringWithString:line];
				count = 0;

				// Send any logs we parse specially as separate lines so that the parsing works out
				if ([line contains:@"ERROR:"] || [line contains:@"WARNING:"] ||
					[line hasPrefix:@"Loading project from:"] || [line contains:@"Runtime error"])
				{
					// Send any accumulated buffer
					if ([buf length] > 0)
					{
						[self performSelectorOnMainThread:@selector(log:) withObject:buf waitUntilDone:NO];
						buf = @"";
					}

					[self performSelectorOnMainThread:@selector(log:) withObject:line waitUntilDone:NO];
				}
				else
				{
					buf = [buf stringByAppendingString:line];
					buf = [buf stringByAppendingString:@"\r"];
				}
				
			}
			else
			{
				++count;
			}
		}

		if (count > 0)
		{
			if (count > ConsoleRepeatLimit)
			{
				buf = [buf stringByAppendingFormat:@"### repeats %d time%s ###", count, (count == 1 ? "" : "s")];

			}
			else
			{
				for (int i = 0; i < count; i++)
				{
					buf = [buf stringByAppendingString:lastLine];
					buf = [buf stringByAppendingString:@"\r"];
				}
			}
		}

		[self performSelectorOnMainThread:@selector(log:) withObject:buf waitUntilDone:NO];

		dataStr = nil;

		[[notification object] readInBackgroundAndNotifyForModes:@[NSDefaultRunLoopMode, NSModalPanelRunLoopMode, NSEventTrackingRunLoopMode]];
	}
}

- (void) log:(NSString *)msg
{
	[self showWindow:self];

	// Remember the project path in case we need it to file a report
	if ([msg hasPrefix:@"Loading project from:"])
	{
		// msg looks like: Loading project from:	~/path/to/project
		//             or: Loading project from:	/tmp/path/to/project

		NSRange start = [msg rangeOfString:@"~"];

		if (start.location == NSNotFound)
		{
			start = [msg rangeOfString:@"/"];
		}

		if (start.location != NSNotFound)
		{
			NSRange pathRange = NSMakeRange(start.location, [msg length] - start.location);

			projectPath = [[[msg substringWithRange:pathRange] stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]] stringByExpandingTildeInPath];

			NSDEBUG(@"projectPath: |%@|", projectPath);

			[_projectPathDisplay setStringValue:[projectPath stringByAbbreviatingWithTildeInPath]];
		}
	}

	// NSDEBUG(@"log: got %ld bytes", [msg length]);

	// NSDEBUG(@"log: textStorage %ld bytes", [[_textView textStorage] length]);

	// Output a timestamp
	NSDateFormatter *dateFormat = [[NSDateFormatter alloc] init];
	[dateFormat setDateFormat:@"MMM dd HH:mm:ss.SSS "];
	NSString *timestamp = [dateFormat stringFromDate:[NSDate date]];

	dateFormat = nil;

	msg = [msg stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];

	BOOL addToJumpMenu = NO;
	if ([msg contains:@"ERROR:"] || [msg contains:@"Runtime error"])
	{
		++numErrors;
		[_errorDisplay setStringValue:[NSString stringWithFormat:@"%d error%s", numErrors, (numErrors == 1 ? "" : "s")]];
		[_errorDisplay setHidden:NO];
		addToJumpMenu = YES;
	}
	else if ([msg contains:@"WARNING:"])
	{
		++numWarnings;
		[_warningDisplay setStringValue:[NSString stringWithFormat:@"%d warning%s", numWarnings, (numWarnings == 1 ? "" : "s")]];
		[_warningDisplay setHidden:NO];
		addToJumpMenu = YES;
	}

	if (addToJumpMenu && [projectPath length] > 0)
	{
		NSString *tmpPath = [projectPath stringByAppendingString:@"/"];
		NSString *itemTitle = [msg stringByReplacingOccurrencesOfString:@"\n" withString:@" "];
		NSRange stackRange = [itemTitle rangeOfString:@"stack traceback:"];

		if (stackRange.location != NSNotFound)
		{
			itemTitle = [itemTitle substringWithRange:NSMakeRange(0, stackRange.location)];
		}

		// The UI already displays the project path so remove it from the menu
		itemTitle = [itemTitle stringByReplacingOccurrencesOfString:tmpPath withString:@""];

		NSMenuItem *jumpItem = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(jumpToItem:) keyEquivalent:@""];
		[jumpItem setTag:(NSInteger) [[_textView string] length]];
		[jumpItem setTarget:self];
		[[_jumpMenu menu] addItem:jumpItem];
		[_jumpMenu setHidden:NO];
	}

	if ([projectPath length] > 0)
	{
		// Elide the project path in the console as it's repetitious
		msg = [msg stringByReplacingOccurrencesOfString:[projectPath stringByAppendingString:@"/"] withString:@""];
	}

	// Replacing carriage returns with line separators makes the indentation work out in the textview
	msg = [msg stringByReplacingOccurrencesOfString:@"\r" withString:[NSString stringWithFormat:@"%C", (unichar)NSLineSeparatorCharacter]];

	[self appendToTextView:[timestamp stringByAppendingString:[msg stringByAppendingString:@"\n"]]];
}

- (void)appendToTextView:(NSString*)text
{
	dispatch_async(dispatch_get_main_queue(), ^{
		NSMutableAttributedString* attrString = [[NSMutableAttributedString alloc] initWithString:text];

		[attrString addAttributes:self->boldFontAttr range:NSMakeRange(0, 20)];
		[attrString addAttributes:self->timestampAttr range:NSMakeRange(0, 20)];
		[attrString addAttributes:self->textAttr range:NSMakeRange(20, [text length]-20)];
		[attrString addAttributes:self->paraAttr range:NSMakeRange(0, [text length])];

		if ([text contains:@"ERROR:"] || [text contains:@"Runtime error"]  )
		{
			[attrString addAttributes:self->errorAttr range:NSMakeRange(20, [text length]-20)];
		}
		else if ([text contains:@"WARNING:"])
		{
			[attrString addAttributes:self->warningAttr range:NSMakeRange(20, [text length]-20)];
		}
		NSRect visibleRect = [self->_textView visibleRect];
		NSRect documentRect = [[self->_scrollView contentView] documentRect];

		[self.textFinder noteClientStringWillChange];

		[[self->_textView textStorage] appendAttributedString:attrString];
		attrString = nil;
		[self->_textFinder setFindIndicatorNeedsUpdate:YES];

		// Scroll to new end if document is scrolled to end (but not if the scrollbar has been adjusted
		// manually to a point other than the end)
		if ((visibleRect.size.height + visibleRect.origin.y) == (documentRect.size.height + documentRect.origin.y))
		{
			[self->_textView scrollToEndOfDocument:nil];
		}

		NSString *string = [[self->_textView textStorage] string];
		NSUInteger numberOfLines, index, stringLength = [string length];

		if (stringLength > ConsoleMaxChars)
		{
			// Remove enough whole lines to reduce the overall length by at least ConsoleChunkSize

			for (index = 0, numberOfLines = 0; index < ConsoleChunkSize; numberOfLines++)
			{
				index = NSMaxRange( [string lineRangeForRange:NSMakeRange(index, 0)]);
			}

			NSDEBUG(@"log: textStorage %ld bytes ... trimming %ld chars", [[_textView textStorage] length], index);

			[self.textFinder noteClientStringWillChange];

			[[self->_textView textStorage] deleteCharactersInRange:NSMakeRange(0, index)];
		}
	});
}

- (IBAction)search:(id)sender
{
	NSDEBUG(@"- (void)search:%@", sender);
}

- (IBAction)performFindAction:(id)action
{
    NSTextFinderAction finderAction = (NSTextFinderAction) [action tag];

    NSDEBUG(@"- (void)performAction:%@ (%ld)", action, finderAction);

    [self.textFinder performAction:finderAction];
    
    // Disable the dimming/blur effects on content (but not the find bar itself)
    [self removeFindBarDimming:[self.textView enclosingScrollView]];
}




- (void)removeFindBarDimming:(NSView *)view
{
    // Recursively find and disable dimming views
    for (NSView *subview in [view subviews]) {
        NSString *className = [subview className];
        
        // Skip the actual find bar and its children - we want those to look normal
        if ([className containsString:@"NSTextFinderBarView"] ||
            [className containsString:@"NSBannerView"]) {
            continue;  // Don't process the find bar itself
        }
        
        // Only disable dimming views that are NOT inside the find bar
        if ([className containsString:@"PocketBlur"] ||
            [className containsString:@"AdditionalDimmingView"] ||
            [className containsString:@"LuminanceAdjustment"]) {
            [subview setHidden:YES];
            [subview setAlphaValue:0.0];
        }
        
        // Disable backdrop layers only outside the find bar
        if ([subview layer] && ![self isInsideFindBar:subview]) {
            [self removeBackdropLayers:[subview layer]];
        }
        
        // Recurse into subviews
        [self removeFindBarDimming:subview];
    }
}

- (BOOL)isInsideFindBar:(NSView *)view
{
    NSView *parent = [view superview];
    while (parent != nil) {
        NSString *className = [parent className];
        if ([className containsString:@"NSTextFinderBarView"] ||
            [className containsString:@"NSBannerView"]) {
            return YES;
        }
        parent = [parent superview];
    }
    return NO;
}

- (void)removeBackdropLayers:(CALayer *)layer
{
    if (!layer) return;
    
    NSString *className = [layer className];
    
    if ([className isEqualToString:@"CABackdropLayer"]) {
        layer.hidden = YES;
        layer.opacity = 0.0;
    }
    
    // Recurse into sublayers
    for (CALayer *sublayer in [layer sublayers]) {
        [self removeBackdropLayers:sublayer];
    }
}



- (IBAction) sendLogViaEmail:(id) sender
{
	NSString *configLuaFile = [projectPath stringByAppendingPathComponent:@"config.lua"];
	NSString *buildSettingsFile = [projectPath stringByAppendingPathComponent:@"build.settings"];

	__block NSString *tmpLogFile = @"/tmp/CoronaSDK-console.log";
	__block NSString *systemProfileFile = @"/tmp/CoronaSDK-systemprofile.spx";

	void (^gatherInfoBlock)() = ^()
	{
		NSError *error = nil;
		[[self.textView string] writeToFile:tmpLogFile atomically:NO encoding:NSUTF8StringEncoding error:&error];

		NSString *systemProfileCmd = [NSString stringWithFormat:@"/usr/sbin/system_profiler -detailLevel mini -timeout 30 -xml 2>&1 > %@", systemProfileFile];

		[self launchTaskAndReturnOutput:@"/bin/sh" arguments:@[@"-c", systemProfileCmd]];
	};

	// Make this call async
	NSOperationQueue *backgroundQueue = [[NSOperationQueue alloc] init];
	[backgroundQueue addOperationWithBlock:gatherInfoBlock];

	// Discard any events that got queued while waiting (stops buttons being clicked on dialog)
	[NSApp discardEventsMatchingMask:NSAnyEventMask beforeEvent:nil];
	
	// NSString *helpURL = @"https://coronalabs.com/links/simulator/xcode-required";
	NSString *title = @"Submit Console Log to Corona Labs Support";
	NSString *msg = @"You can opt to send the log and associated information (system profile and the current app's config.lua and build.settings) using Mail.app or just create a ZIP file you can send to Corona Labs using your preferred method.";
	NSAlert *alert = [[NSAlert alloc] init];

	[alert setAlertStyle:NSWarningAlertStyle];

	[alert setMessageText:title];
	[alert setInformativeText:msg];

	[alert addButtonWithTitle:@"Send via Mail.app"];
	[alert addButtonWithTitle:@"Generate ZIP File"];
	[alert addButtonWithTitle:@"Cancel"];

	// Show the sheet
	NSInteger alertResult = [alert runModal];

	// Wait for the info gathering to complete while pumping the run loop
	while ([backgroundQueue operationCount] > 0)
	{
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	}

	if (alertResult == NSAlertFirstButtonReturn)
	{
		NSString *toAddress = @"support+simulator@coronalabs.com";
		NSString *subject = @"Corona Simulator Problem Report";
		NSString *bodyText =
		@"Sending this email will deliver your current Corona Simulator console log and your system profile information to Corona Labs Support so we can try to assist you with the issue you are seeing.  The more detail you give the more likely it is we'll be able to help.  Remember we can't see what you're seeing so add any screenshots or projects you think will help get you a faster response.  If you don't use Mail.app then copy this message to your regular email client.\n\n"
		@"Describe your issue in one sentence\n\n\n"
		@"Is the issue reproducible (does it happen every time)?\n\n\n"
		@"Have you found a way to work around the issue?\n\n\n"
		@"Describe the issue in more detail (explain what happens and how that differs from what you expect)\n\n\n"
		@"What are we looking for in the enclosed log file (if you know)?\n\n\n";

		NSString *emailScriptStr = [NSString stringWithFormat:@"\
									tell application \"Mail\"\n\
									set newMessage to make new outgoing message with properties {subject:\"%@\", content:\"%@\" & return} \n\
									tell newMessage\n\
									set visible to true\n\
									set sender to \"%@\"\n\
									make new to recipient at end of to recipients with properties {name:\"%@\", address:\"%@\"}\n\
									tell content\n",
									subject, bodyText, @"Corona Labs Support", @"Corona Labs Support", toAddress ];

		// Make sure the version info is in the log even if it's been truncated or cleared
		// (due to runloop limitations we need to inject the text directly)
#if 0
		[[_textView textStorage] replaceCharactersInRange:NSMakeRange([[_textView textStorage] length], 0) withString:[NSString stringWithFormat:@"CoronaSDK Version: %s\n", Rtt_STRING_VERSION]];
		[[_textView textStorage] replaceCharactersInRange:NSMakeRange([[_textView textStorage] length], 0) withString:[NSString stringWithFormat:@"CoronaSDK Build: %s\n", Rtt_STRING_BUILD]];
#endif

		// add the log file as an attachment
		emailScriptStr = [emailScriptStr stringByAppendingFormat:@"make new attachment with properties {file name:\"%@\"} at after the last paragraph\n", tmpLogFile];

		// add the system profile as an attachment
		emailScriptStr = [emailScriptStr stringByAppendingFormat:@"make new attachment with properties {file name:\"%@\"} at after the last paragraph\n", systemProfileFile];

		if ([[NSFileManager defaultManager] fileExistsAtPath:configLuaFile])
		{
			emailScriptStr = [emailScriptStr stringByAppendingFormat:@"make new attachment with properties {file name:\"%@\"} at after the last paragraph\n", configLuaFile];
		}

		if ([[NSFileManager defaultManager] fileExistsAtPath:buildSettingsFile])
		{
			emailScriptStr = [emailScriptStr stringByAppendingFormat:@"make new attachment with properties {file name:\"%@\"} at after the last paragraph\n", buildSettingsFile];
		}

		//finish script
		emailScriptStr = [emailScriptStr stringByAppendingFormat:@"\
						  end tell\n\
						  end tell\n\
						  activate\n\
						  end tell"];

		//NSDEBUG(@"email applescript: %@",emailScriptStr);
		NSAppleScript *emailScript = [[NSAppleScript alloc] initWithSource:emailScriptStr];
		[emailScript executeAndReturnError:nil];

		/* send the message */
		[self log:@"Complete the submission in Mail"];
	}
	else if (alertResult == NSAlertSecondButtonReturn)
	{
		NSString *tmpZipFile = @"/tmp/CoronaSDK-support.zip";

		NSString *zipCmd = [NSString stringWithFormat:@"/usr/bin/zip -rqj '%@' '%@' '%@' '%@' '%@'", tmpZipFile, tmpLogFile, systemProfileFile, configLuaFile, buildSettingsFile];

		[self launchTaskAndReturnOutput:@"/bin/sh" arguments:@[@"-c", zipCmd]];

		// Show the .zip we built in the Finder
		[[NSWorkspace sharedWorkspace] selectFile:tmpZipFile inFileViewerRootedAtPath:@""];
	}
}

- (void) chooseTheme:(id) sender
{
	NSString *themeFile = [sender representedObject];
	[self loadTheme:themeFile];
	[_optionsPopover close];
}

- (void) loadTheme:(NSString *) themeName
{
	NSError *error = nil;
	Theme *theme = nil;
	NSString *themePath = themeName;
	static BOOL firstTime = YES;

	if (! [[NSFileManager defaultManager] isReadableFileAtPath:themePath])
	{
		// If the themePath isn't a readable file, assume it's a theme name in our bundle
		themePath = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Themes"] stringByAppendingPathComponent:themeName];
		themePath = [themePath stringByAppendingPathExtension:@"json"];
	}

	NSString *jsonStr = [NSString stringWithContentsOfFile:themePath encoding:NSUTF8StringEncoding error:&error];

	if (jsonStr != nil)
	{
		theme = [Theme ThemeWithJSONString:jsonStr usingEncoding:NSUTF8StringEncoding error:&error];

		if (theme != nil)
		{
			NSDEBUG(@"loadTheme: theme %@", theme);

			//NSDEBUG(@"fontWithName:%@ size:%g", theme.console.fontName , [theme.console.fontSize floatValue]);
			if ((font = [NSFont fontWithName:theme.console.fontName size:[theme.console.fontSize floatValue]]) == nil)
			{
				NSLog(@"Error in theme: font '%@' not found", theme.console.fontName);
				font = [NSFont userFixedPitchFontOfSize:12];
			}

			boldFont = [fontManager convertFont:font toHaveTrait:NSBoldFontMask];

			try {
				textFgColor = [NSColor colorWithHexString:theme.console.textFgColor];
				textBgColor = [NSColor colorWithHexString:theme.console.textBgColor];
				timestampFgColor = [NSColor colorWithHexString:theme.console.timestampFgColor];
				timestampBgColor = [NSColor colorWithHexString:theme.console.timestampBgColor];
				errorFgColor = [NSColor colorWithHexString:theme.console.errorFgColor];
				errorBgColor = [NSColor colorWithHexString:theme.console.errorBgColor];
				warningFgColor = [NSColor colorWithHexString:theme.console.warningFgColor];
				warningBgColor = [NSColor colorWithHexString:theme.console.warningBgColor];
			}
			catch (NSException *e) {
				NSLog(@"Error in theme: %@", e);
			}

			[_errorDisplay setBackgroundColor:errorBgColor];
			[_errorDisplay setTextColor:errorFgColor];
			[_warningDisplay setBackgroundColor:warningBgColor];
			[_warningDisplay setTextColor:warningFgColor];

			[self setupTextAttributes];

			[[NSUserDefaults standardUserDefaults] setObject:themeName forKey:kUserPreferenceCurrentTheme];

			if (! firstTime)
			{
				[self log:[NSString stringWithFormat:@"Theme changed to %@", themeName]];
			}

			firstTime = NO;
		}
	}

	if (jsonStr == nil || theme == nil)
	{
		[self log:[NSString stringWithFormat:@"Cannot load theme called %@ (%@)", themeName, error]];
	}
}

- (void) populateThemesMenu
{
	NSString *systemThemesDir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Themes"];
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *userThemesDir = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Corona/Simulator/Themes"];
	NSArray *systemThemeFiles = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:[NSURL fileURLWithPath:systemThemesDir isDirectory:YES]
															  includingPropertiesForKeys:[NSArray arrayWithObject:NSURLNameKey]
																				 options: NSDirectoryEnumerationSkipsHiddenFiles error:nil];
	NSArray *userThemeFiles = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:[NSURL fileURLWithPath:userThemesDir isDirectory:YES]
															includingPropertiesForKeys:[NSArray arrayWithObject:NSURLNameKey]
																			   options: NSDirectoryEnumerationSkipsHiddenFiles error:nil];
	NSMutableArray *themeFiles = [[NSMutableArray alloc] initWithArray:systemThemeFiles];
	[themeFiles addObjectsFromArray:userThemeFiles];

	// Sort themes alphabetically
	[themeFiles sortWithOptions:NSSortStable usingComparator:^NSComparisonResult(NSURL *url1, NSURL *url2) {
		return [[[url2 path] lastPathComponent] caseInsensitiveCompare:[[url1 path] lastPathComponent]];
	}];

	if ([themeFiles count] > 0)
	{
		NSDEBUG(@"themeFiles: %@", themeFiles);

		[[_themePopup menu] removeAllItems];

		for (NSURL *extURL in themeFiles)
		{
			NSString *themePath = [extURL path];

			if ([[themePath pathExtension] isEqualToString:@"json"])
			{
				NSString *themeTitle = [NSString stringWithString:[[themePath lastPathComponent] stringByDeletingPathExtension]];
				NSMenuItem *themeMenuItem = [[NSMenuItem alloc] initWithTitle:themeTitle action:@selector(chooseTheme:) keyEquivalent:@""];
				[themeMenuItem setTarget:self];
				[themeMenuItem setEnabled:YES];

				if ([themePath contains:@"Corona/Simulator/Themes"])
				{
					// Use the full path of user themes
					[themeMenuItem setRepresentedObject:themePath];
				}
				else
				{
					// Only use the title of internal themes so that the user's choice persists over Daily Builds
					[themeMenuItem setRepresentedObject:themeTitle];
				}
				[[_themePopup menu] insertItem:themeMenuItem atIndex:0];

				NSString *currentTheme = [[NSUserDefaults standardUserDefaults] stringForKey:kUserPreferenceCurrentTheme];

				if ([themeTitle isEqualToString:currentTheme])
				{
					// [[_themePopup cell] setMenuItem:themeMenuItem];
					[_themePopup selectItem:themeMenuItem];
				}
			}
		}
	}
}

- (NSString *) launchTaskAndReturnOutput:(NSString *)cmd arguments:(NSArray *)args
{
	NSString *result = nil;
	NSMutableData *resultData = [[NSMutableData alloc] init];
	NSTask *task = [[NSTask alloc] init];
	NSPipe *stdoutPipe = [NSPipe pipe];
	NSPipe *stderrPipe = [NSPipe pipe];

	[task setLaunchPath:cmd];
	[task setArguments:args];

	[task setStandardOutput:stdoutPipe];
	[task setStandardError:stderrPipe];

	NSFileHandle *stderrFileHandle = [stderrPipe fileHandleForReading];

	// Using a readability handler allows us to get more than 4096 bytes without blocking the pipe
	[[task.standardOutput fileHandleForReading] setReadabilityHandler:^(NSFileHandle *file) {
		NSData *data = [file availableData]; // read to current EOF

		[resultData appendData:data];
	}];

	@try
	{
		[task launch];
		[task waitUntilExit];

		if (! [task isRunning] && [task terminationStatus] != 0)
		{
			// Command failed, emit any stderr to the log
			NSData *stderrData = [stderrFileHandle readDataToEndOfFile];
			NSLog(@"Error running %@ %@: %s", cmd, args, (const char *)[stderrData bytes]);
		}

		result = [[NSMutableString alloc] initWithData:resultData encoding:NSUTF8StringEncoding];
	}
	@catch( NSException* exception )
	{
		NSLog( @"launchTaskAndReturnOutput: exception %@ (%@ %@)", exception, cmd, args );
	}
	@finally
	{
		[[task.standardOutput fileHandleForReading] setReadabilityHandler:nil];
	}

	result = [result stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

	return result;
}

@end


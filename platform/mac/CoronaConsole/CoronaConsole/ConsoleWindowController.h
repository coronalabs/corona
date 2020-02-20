//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>
#import "NSColor+HexString.h"

@interface ConsoleWindowController : NSWindowController< NSWindowDelegate >
{
	NSString *projectPath;
	int numWarnings;
	int numErrors;
	BOOL popoverSizeAdjusted;

	NSFontManager *fontManager;
	NSFont *font;
	NSFont *boldFont;
	NSColor *timestampFgColor;
	NSColor *timestampBgColor;
	NSColor *errorFgColor;
	NSColor *errorBgColor;
	NSColor *warningFgColor;
	NSColor *warningBgColor;
	NSColor *textFgColor;
	NSColor *textBgColor;
	NSDictionary *textAttr;
	NSDictionary *boldFontAttr;
	NSDictionary *timestampAttr;
	NSDictionary *errorAttr;
	NSDictionary *warningAttr;
	NSMutableParagraphStyle *paraStyle;
	NSDictionary *paraAttr;
}

@property (nonatomic, assign) IBOutlet NSScrollView *scrollView;
@property (nonatomic, assign) IBOutlet NSTextView *textView;
@property (nonatomic, retain) NSTextFinder *textFinder;
@property (weak) IBOutlet NSTextField *warningDisplay;
@property (weak) IBOutlet NSTextField *errorDisplay;
@property (weak) IBOutlet NSPopUpButton *jumpMenu;
@property (weak) IBOutlet NSTextField *projectPathDisplay;
@property (strong) IBOutlet NSPopover *optionsPopover;
@property (weak) IBOutlet NSButton *sendLogButton;
@property (weak) IBOutlet NSPopUpButton *themePopup;
@property (strong) IBOutlet NSView *optionsView;
@property (weak) IBOutlet NSButton *optionsButton;

- (IBAction) clearLog:(id)sender;
- (IBAction) endOfLog:(id)sender;

+ (id)sharedConsole;
- (void) log:(NSString *)msg;
- (void)performFindAction:(id)action;
- (void) handleStdoutNotification:(NSNotification *)notification;

@end

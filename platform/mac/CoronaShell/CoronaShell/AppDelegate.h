//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate, CoronaViewDelegate>
{
	id fGLView;
}
@property int titleBarHeight;
@property NSRect nonFullscreenWindowFrame;

- (IBAction) performPause:(id) sender;
- (IBAction) showHelp:(id) sender;
- (void) notifyRuntimeError:(NSString *) mesg;
- (NSWindow *) currentWindow;
@end


// Copyright 1997-2009 Omni Development, Inc.  All rights reserved.
//
// This software may only be used and reproduced according to the
// terms in the file OmniSourceLicense.html, which should be
// distributed with this project and can also be found at
// <http://www.omnigroup.com/developer/sourcecode/sourcelicense/>.

#import "NSAlert-OAExtensions.h"
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>



@interface _OAAlertSheetCompletionHandlerRunner : NSObject
{
    NSAlert *_alert;
    OAAlertSheetCompletionHandler _completionHandler;
}
@end
@implementation _OAAlertSheetCompletionHandlerRunner
- initWithAlert:(NSAlert *)alert completionHandler:(OAAlertSheetCompletionHandler)completionHandler;
{
    if (!(self = [super init]))
        return nil;
    
    _alert = [alert retain];
    _completionHandler = [completionHandler copy];
    return self;
}
- (void)dealloc;
{
    [_alert release];
    [_completionHandler release];
    [super dealloc];
}

- (void)startOnWindow:(NSWindow *)parentWindow;
{
    // We have to live until the callback, but a -retain will annoy clang-sa.
	[self performSelector:@selector(retain)];
    [_alert beginSheetModalForWindow:parentWindow modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;
{
    NSAssert(alert == _alert,@"Got a alert different from what I expected -- This should never happen");
    
    // Clean up the hidden -retain from -startOnWindow:, first and with -autorelease in case the block asplodes.
    [self performSelector:@selector(autorelease)];
    
    if (_completionHandler)
        _completionHandler(_alert, returnCode);
}

@end

@implementation NSAlert (OAExtensions)

- (void)beginSheetModalForWindow2:(NSWindow *)window completionHandler:(OAAlertSheetCompletionHandler)completionHandler;
{
    _OAAlertSheetCompletionHandlerRunner *runner = [[_OAAlertSheetCompletionHandlerRunner alloc] initWithAlert:self completionHandler:completionHandler];
    [runner startOnWindow:window];
    [runner release];
}

@end



void OABeginAlertSheet(NSString *title, NSString *defaultButton, NSString *alternateButton, NSString *otherButton, NSWindow *docWindow, OAAlertSheetCompletionHandler completionHandler, NSString *msgFormat, ...)
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:title];
    
    if (msgFormat) {
        va_list args;
        va_start(args, msgFormat);
        NSString *informationalText = [[NSString alloc] initWithFormat:msgFormat arguments:args];
        va_end(args);
        
        [alert setInformativeText:informationalText];
        [informationalText release];
    }
    
    if (defaultButton)
        [alert addButtonWithTitle:defaultButton];
    if (alternateButton)
        [alert addButtonWithTitle:alternateButton];
    if (otherButton)
        [alert addButtonWithTitle:otherButton];
    
    [alert beginSheetModalForWindow2:docWindow completionHandler:completionHandler];
    [alert release]; // retained by the runner while the sheet is up
}


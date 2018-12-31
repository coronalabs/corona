// Copyright 1997-2009 Omni Development, Inc.  All rights reserved.
//
// This software may only be used and reproduced according to the
// terms in the file OmniSourceLicense.html, which should be
// distributed with this project and can also be found at
// <http://www.omnigroup.com/developer/sourcecode/sourcelicense/>.
//
// $Id$

#import <AppKit/NSAlert.h>

#ifndef NSALERT_OAEXTENSIONS_H
#define NSALERT_OAEXTENSIONS_H

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif
	
typedef void (^OAAlertSheetCompletionHandler)(NSAlert *alert, NSInteger returnCode);

@interface NSAlert (OAExtensions)
- (void)beginSheetModalForWindow2:(NSWindow *)window completionHandler:(OAAlertSheetCompletionHandler)completionHandler;
@end

// Runs an NSAlert with an optional completion block. The returnCode passed to the block is based on the NSAlertFirstButtonReturn+N convention.
extern void OABeginAlertSheet(NSString *title, NSString *defaultButton, NSString *alternateButton, NSString *otherButton, NSWindow *docWindow, OAAlertSheetCompletionHandler completionHandler, NSString *msgFormat, ...);

	
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // NSALERT_OAEXTENSIONS_H


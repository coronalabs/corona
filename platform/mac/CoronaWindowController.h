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

#import <QuartzCore/QuartzCore.h>

#import <Cocoa/Cocoa.h>

#import "CoronaViewPrivate.h"

// ----------------------------------------------------------------------------

@class CoronaView;
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
@protocol CAAnimationDelegate;
#endif

namespace Rtt
{
	class RuntimeDelegate;
	class RuntimeDelegateWrapper;
}

//typedef void (^windowCloseCompletionBlock)(void);

@interface CoronaWindowController : NSWindowController< CoronaViewControllerDelegate
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
, CAAnimationDelegate
#endif
>
{
	@private
		CoronaView *fView;
		Rtt::RuntimeDelegateWrapper *fRuntimeDelegateWrapper;
		BOOL fIsInitialized;
	
		BOOL windowGoingAway;
		CFTimeInterval windowFadeAnimationStartTime;

        BOOL (^windowShouldCloseBlock)(void);
        void (^windowCloseCompletionBlock)(void);
        BOOL (^windowWillResizeBlock)(int oldWidth, int oldHeight, int newWidth, int newHeight);
        void (^fColorPanelCallbackBlock)(double r, double g, double b, double a);
}

@property (nonatomic, readonly, getter=view) CoronaView *fView;
@property(nonatomic, assign, getter=isWindowGoingAway) BOOL windowGoingAway;

- (id)initWithPath:(NSString*)path;
- (id)initWithPath:(NSString*)path width:(int)width height:(int)height title:(NSString *)windowTitle resizable:(bool) resizable showWindowTitle:(bool) showWindowTitle;

// NOTE: Receiver takes ownership of delegate
- (void)setRuntimeDelegate:(Rtt::RuntimeDelegate *)delegate;

- (void)didPrepare;


- (void)show;
- (void)hide;

- (void) newProject;


// For when window is fading out and is about to close, call this to bring it back.
- (void) resurrectWindow;

- (void) setWindowDidCloseCompletionBlock:(void (^)(void))block;
- (void) setWindowShouldCloseBlock:(BOOL (^)(void))block;
- (void) setWindowWillResizeBlock:(BOOL (^)(int oldWidth, int oldHeight, int newWidth, int newHeight))block;

- (void) projectLoaded;

- (void) colorPanelAction:(id)sender;
- (void) setColorPanelCallbackBlock:(void (^)(double r, double g, double b, double a))block;
- (void) hideColorPanel;
@end

// ----------------------------------------------------------------------------

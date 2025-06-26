//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

@interface CoronaWindowController : NSWindowController< NSDraggingDestination,CoronaViewControllerDelegate
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

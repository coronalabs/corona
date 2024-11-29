//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <GLKit/GLKit.h>
#import <CoreLocation/CoreLocation.h>

typedef enum {
    kNormal,
    kMinimized,
    kMaximized,
    kFullscreen,
} CoronaViewWindowMode;

@protocol CoronaViewDelegate;
@protocol CLLocationManagerDelegate;

// ----------------------------------------------------------------------------

@interface CoronaView : NSView < CLLocationManagerDelegate >

@property (nonatomic, assign) id <CoronaViewDelegate> coronaViewDelegate;

- (NSInteger)run;
- (NSInteger)runWithPath:(NSString*)path parameters:(NSDictionary *)params;
- (void)suspend;
- (void)resume;
- (void)terminate;
- (void) handleOpenURL:(NSString *)urlStr;
- (void) setScaleFactor:(CGFloat)scaleFactor;
- (void) restoreWindowProperties;

- (BOOL) settingsIsWindowResizable;
- (BOOL) settingsIsWindowCloseButtonEnabled;
- (BOOL) settingsIsWindowMinimizeButtonEnabled;
- (BOOL) settingsIsWindowMaximizeButtonEnabled;
- (int) settingsContentWidth;
- (int) settingsContentHeight;
- (NSString *) settingsWindowTitle;
- (CoronaViewWindowMode) settingsDefaultWindowMode;
- (int) settingsMinWindowViewWidth;
- (int) settingsMinWindowViewHeight;
- (int) settingsDefaultWindowViewWidth;
- (int) settingsDefaultWindowViewHeight;
- (int) settingsImageSuffixScaleCount;
- (double) settingsImageSuffixScaleByIndex:(int) index;
- (BOOL) settingsSuspendWhenMinimized;
- (BOOL) settingsIsWindowTitleShown;
- (BOOL) settingsIsTransparent;

- (id)sendEvent:(NSDictionary *)event;

@end

// ----------------------------------------------------------------------------

@protocol CoronaViewDelegate <NSObject>

@optional
- (id)coronaView:(CoronaView *)view receiveEvent:(NSDictionary *)event;

@optional
- (void)coronaViewWillSuspend:(CoronaView *)view;
- (void)coronaViewDidSuspend:(CoronaView *)view;
- (void)coronaViewWillResume:(CoronaView *)view;
- (void)coronaViewDidResume:(CoronaView *)view;
- (void)notifyRuntimeError:(NSString *)message;
- (void)didPrepareOpenGLContext:(id)sender;
@end

// ----------------------------------------------------------------------------


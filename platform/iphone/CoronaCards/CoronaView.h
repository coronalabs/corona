//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <GLKit/GLKit.h>

@protocol CoronaViewDelegate;

// ----------------------------------------------------------------------------

@interface CoronaView : GLKView

@property (nonatomic, assign) id <CoronaViewDelegate> coronaViewDelegate;

- (NSInteger)run;
- (NSInteger)runWithPath:(NSString*)path parameters:(NSDictionary *)params;
- (void)suspend;
- (void)resume;

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

@end

// ----------------------------------------------------------------------------


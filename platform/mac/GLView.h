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

#import <AppKit/AppKit.h>
#import "GLViewDelegate.h"

//#import <AppKit/NSView.h>

#include "Rtt_DeviceOrientation.h"

namespace Rtt
{
	class ApplePlatform;
	class Runtime;
} 

@class NSImage;
@class SPILDTopLayerView;

@interface GLView : NSOpenGLView
{
	Rtt::Runtime* fRuntime;
	NSPoint fStartPosition;
	float fFirstClickTime;
	NSTimeInterval fTapDelay;

	id< GLViewDelegate > fDelegate;
	U8 fNumTaps;
	Rtt::DeviceOrientation::Type fOrientation;

	NSRect nativeFrameRect; // currently only settable via initWithFrame:
	CGFloat zoomLevel; // only for bookkeeping purposes
	SPILDTopLayerView* suspendedOverlay;
    BOOL isReady;
    BOOL swapped;
    BOOL isSimulatorView;
	BOOL shouldInvalidate;
    NSMutableArray *fCursorRects;
	NSTrackingRectTag trackingRectTag;
	int numCursorHides;
#if Rtt_AUTHORING_SIMULATOR
	float lastTouchPressure;
	NSPoint lastPressurePoint;
#endif
}

@property (nonatomic, readwrite, getter=runtime, setter=setRuntime:) Rtt::Runtime *fRuntime;
@property (nonatomic, readwrite, getter=tapDelay, setter=setTapDelay:) NSTimeInterval fTapDelay;
@property (nonatomic, assign) CGFloat zoomLevel;
@property (nonatomic, assign) CGFloat scaleFactor;
@property (nonatomic, assign) BOOL isReady;
@property (nonatomic, assign) BOOL swapped;
@property (nonatomic, assign) BOOL isSimulatorView;
@property (nonatomic, assign) BOOL sendAllMouseEvents;
@property (nonatomic, assign) BOOL inFullScreenTransition;
@property (nonatomic, assign) BOOL isResizable;
@property (nonatomic, assign) BOOL allowOverlay;
@property (nonatomic, assign) BOOL cursorHidden;
@property (nonatomic, assign) NSPoint initialLocation;

+ (NSOpenGLPixelFormat*) basicPixelFormat;

- (void)setDelegate:(id< GLViewDelegate >)delegate;

// This method will force the Corona OpenGL renderer to redraw everything.
- (void) invalidate;
- (void) restoreWindowProperties;

- (Rtt::DeviceOrientation::Type)orientation;
- (void)setOrientation:(Rtt::DeviceOrientation::Type)newValue;
- (void)adjustPoint:(NSPoint*)p;
- (NSPoint)pointForEvent:(NSEvent*)event;

- (CGFloat) viewportWidth;
- (CGFloat) viewportHeight;

- (CGFloat)deviceWidth;
- (CGFloat)deviceHeight;


//- (CGFloat)uprightWidth;
//- (CGFloat)uprightHeight;

- (void) suspendNativeDisplayObjects:(bool) showOverlay;
- (void) resumeNativeDisplayObjects;

- (void) setCursor:(const char *) cursorName forRect:(NSRect) bounds;
- (void) hideCursor;
- (void) showCursor;
@end

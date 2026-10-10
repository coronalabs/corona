//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <AppKit/NSWindow.h>

#include "Core/Rtt_Build.h"
#include "Rtt_DeviceOrientation.h"

@class GLView;

#ifdef WE_MERGE_SIMULATOR_WINDOW_FUNCTIONALITY

@class NSImageView;
@class SkinView;
#endif // WE_MERGE_SIMULATOR_WINDOW_FUNCTIONALITY

// ----------------------------------------------------------------------------

@interface SimulatorDeviceWindow : NSWindow<NSWindowDelegate>
{
    BOOL fScaleDidChange;
    Rtt::DeviceOrientation::Type fCurrentSkinOrientation;
    Rtt::DeviceOrientation::Type fOriginalSkinOrientation;

	GLView* fScreenView;
	void (^performCloseBlock)(id);
	int fExponent;
	NSRect fScreenRect; // Original, upright rect
	Rtt::DeviceOrientation::Type fSkinNaturalOrientation; // The app orientation the art is drawn for; kUpright for phones, a landscape for a foldable's wide inner screen
}

// The orientation the art is drawn for. fCurrentSkinOrientation always tracks the app's orientation
// (the GL view gets it, so the content runs the right way); the art is shown turned relative to this.
@property(nonatomic, assign) Rtt::DeviceOrientation::Type skinNaturalOrientation;

// How the art and screen are currently turned on screen: the app's orientation relative to the natural one.
- (Rtt::DeviceOrientation::Type) skinDisplayOrientation;

@property(nonatomic, readwrite, copy) NSString *saveFrameName;

@property(nonatomic,readonly,getter=screenView) GLView *fScreenView;

- (NSSize) computeSkinFrameSize;
- (NSSize) nativeSize;
- (float)scale;
- (void)setScale:(float)newValue;
- (NSSize) updateGLViewFrameSize;
- (BOOL)canZoomIn;
- (BOOL)canZoomOut;
- (void)zoomIn:(id)sender;
- (void)zoomOut:(id)sender;
- (NSPoint) updateGLViewFrameOrigin;
- (void)constrainFrameRectToScreen;
- (void) setPerformCloseBlock:(void (^)(id sender))block;

@end

// ----------------------------------------------------------------------------


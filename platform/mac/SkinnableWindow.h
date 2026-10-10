//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <AppKit/NSWindow.h>

#import "SimulatorDeviceWindow.h"

#include "Rtt_DeviceOrientation.h"

@class NSImageView;
@class GLView;
@class SkinView;

// ----------------------------------------------------------------------------

@interface SkinnableWindow : SimulatorDeviceWindow
{
	NSPoint fMouseDownLocation;

	SkinView* fSkinView;
	BOOL fIsTransparent;
}

- (id)initWithScreenView:(GLView*)screenView
				viewRect:(NSRect)screenRect
                   title:(NSString*)title
               skinImage:(NSString*)path
             orientation:(Rtt::DeviceOrientation::Type)orientation
                   scale:(float)scale
		   isTransparent:(BOOL)isTransparent;

- (void)setOrientation:(Rtt::DeviceOrientation::Type)orientation;
- (void)rotate:(BOOL)clockwise;

// Replaces the device art and screen area, keeping the current orientation and zoom
// (e.g. when a foldable device folds or unfolds). naturalOrientation is the app orientation the
// new art is drawn for: kUpright for phone art, a landscape for a foldable's wide inner screen.
- (void)setSkinImage:(NSString*)path screenRect:(NSRect)screenRect naturalOrientation:(Rtt::DeviceOrientation::Type)naturalOrientation;

@end

// ----------------------------------------------------------------------------


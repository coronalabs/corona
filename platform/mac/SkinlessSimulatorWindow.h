//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <AppKit/AppKit.h>
#import "SimulatorDeviceWindow.h"

#include "Rtt_DeviceOrientation.h"

@class NSImageView;
@class GLView;
@class SkinView;

@interface SkinlessSimulatorWindow : SimulatorDeviceWindow
{
	S32 fRotationAngle;
}

@property(nonatomic, readwrite, copy) NSString *windowTitle;

- (id)initWithScreenView:(GLView*)screenView
				viewRect:(NSRect)screenRect
				   title:(NSString*)title
			 orientation:(Rtt::DeviceOrientation::Type)orientation
                   scale:(float)scale
		   isTransparent:(BOOL)isTransparent;

- (void)setOrientation:(Rtt::DeviceOrientation::Type)orientation;
- (void)rotate:(BOOL)clockwise;

@end

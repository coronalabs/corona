//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <AppKit/AppKit.h>

#include "Rtt_DeviceOrientation.h"

@class NSAffineTransform;

// ----------------------------------------------------------------------------

@interface SkinView : NSView
{
	NSImage* fSkinImage;
	NSAffineTransform* fTransform;
	S32 fAngle; // absolute angle, relative to vertical (upright portrait)
	
	NSSize nativeSize;
	NSImage* nativeSkinImage;
	CGFloat fScale;
}

@property(nonatomic,readonly,getter=angle) S32 fAngle;
@property(nonatomic) NSSize nativeSize;
@property(nonatomic, retain) NSImage* nativeSkinImage;


- (id)initWithFrame:(NSRect)frameRect
	  isTransparent:(BOOL)isTransparent;

- (NSSize) imageSize;

- (NSImage*)image;
- (void)setImage:(NSImage*)image;
- (BOOL)setImageWithURL:(NSURL*)urlpath;

- (S32)rotate:(bool)clockwise;
- (void)setOrientation:(Rtt::DeviceOrientation::Type)orientation;


@end

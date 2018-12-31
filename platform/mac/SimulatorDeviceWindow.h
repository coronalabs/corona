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
}

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


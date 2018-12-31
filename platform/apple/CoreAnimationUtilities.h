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

#ifndef COREANIMATION_UTILITIES_H
#define COREANIMATION_UTILITIES_H

#import <QuartzCore/QuartzCore.h>
#import <Foundation/Foundation.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif
	
// Shakes a view or window (or something that has an animator and responds to frame or frameOrigin properties.
void CoreAnimationUtilities_ShakeViewWithAnimation(id view, NSRect view_frame, CAAnimation* shake_animation);

// Returns autoreleased CAKeyframeAnimation
	
// Approximation of password rejection shake which is an x-axis only shake. Rect is the view frame which uses the size to compute how much to move.
CAKeyframeAnimation* CoreAnimationUtilities_PasswordShakeAnimationWithRect(CGRect view_frame);

// Starting with password shake, adding partial randomization along both x and y
CAKeyframeAnimation* CoreAnimationUtilities_RandomizedShakeAnimationWithRect(CGRect view_frame);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // COREANIMATION_UTILITIES_H
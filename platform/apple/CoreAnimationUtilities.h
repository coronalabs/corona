//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
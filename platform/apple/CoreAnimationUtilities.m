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

#import "CoreAnimationUtilities.h"

// Need CoreGraphics for CGPath. Different place on different OS's.
#include <TargetConditionals.h>
#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
	#import <UIKit/UIKit.h>
#else
//	#import <ApplicationServices/ApplicationServices.h>
	#import <Cocoa/Cocoa.h>
	#import <Quartz/Quartz.h>

#endif

#define COREANIMATIONUTILITIES_ANIMATION_NUMBER_OF_SHAKES 4
#define COREANIMATIONUTILITIES_ANIMATION_DURATION_OF_SHAKE 0.5f
#define COREANIMATIONUTILITIES_ANIMATION_VIGOUR_OF_SHAKE 0.08f

void CoreAnimationUtilities_ShakeViewWithAnimation(id view, NSRect view_frame, CAAnimation* shake_animation)
{
	static bool animating = false;
	
	if (animating)
		return;
	
	animating = true;
	
	NSRect original_frame = view_frame;
	NSRect frame_copy = original_frame;
	
	// modify the copy to force Core Animation to recognize a position change so it doesn't short circuit.
	frame_copy.origin.x = frame_copy.origin.x+1.0;
			
	// People are reporting that frameOrigin doesn't work in 64-bit but frame does. But so far, I can't get frame to work in 32-bit.
#if 0
	[view setAnimations:[NSDictionary dictionaryWithObject:shake_animation forKey:@"frame"]];

	[[view animator] setFrame:frame_copy display:NO];
	[[view animator] setFrame:original_frame display:NO];
#else
	[view setAnimations:[NSDictionary dictionaryWithObject:shake_animation forKey:@"frameOrigin"]];
	
	// If we're already animating, just return (otherwise the window wanders; fixes casenum
	[NSAnimationContext beginGrouping];
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context){
		// Start the animations.
		[[view animator] setFrameOrigin:frame_copy.origin];
		[[view animator] setFrameOrigin:original_frame.origin];
	} completionHandler:^{
		// This block will be invoked when all of the animations
		// started above have completed or been cancelled.
		animating = false;
	}];
	[NSAnimationContext endGrouping];

#endif
	
}

// Implementation inspired from:
// http://www.cimgf.com/2008/02/27/core-animation-tutorial-window-shake-effect/
CAKeyframeAnimation* CoreAnimationUtilities_RandomizedShakeAnimationWithRect(CGRect view_frame)
{
	CAKeyframeAnimation* shake_animation = [CAKeyframeAnimation animation];
	//  CAKeyframeAnimation* shake_animation = [CAKeyframeAnimation animationWithKeyPath:@"frameOrigin"];
	
	
    CGMutablePathRef shake_path = CGPathCreateMutable();
    CGPathMoveToPoint(shake_path, NULL, CGRectGetMinX(view_frame), CGRectGetMinY(view_frame));
    NSUInteger the_index;
	
	// Take the min of x and y so elongated displays don't look overly weird when shaking.
	CGFloat min_of_x_and_y = view_frame.size.width;
	if(view_frame.size.height < view_frame.size.width)
	{
		min_of_x_and_y = view_frame.size.height;
	}
	
    for(the_index = 0; the_index < COREANIMATIONUTILITIES_ANIMATION_NUMBER_OF_SHAKES; ++the_index)
    {
		// back and forth on x-axis with partial randomization on x and y
		
		// using arc4random because I'm trying to avoid contaminating anybody using seeds with rand()
		int32_t randx =  (int32_t)arc4random() % (unsigned)min_of_x_and_y/16;
		int32_t randy =  (int32_t)arc4random() % (unsigned)min_of_x_and_y/16;
		uint32_t pos_or_neg =  arc4random() % (unsigned)2;
		
		if(pos_or_neg)
		{
			randx = -randx;
		}
		
		pos_or_neg =  arc4random() % (unsigned)2;
		
		if(pos_or_neg)
		{
			randy = -randy;
		}
		
		pos_or_neg =  arc4random() % (unsigned)2;
		
		CGFloat yval = min_of_x_and_y/2.0 * COREANIMATIONUTILITIES_ANIMATION_VIGOUR_OF_SHAKE + (CGFloat)randy;
		if(pos_or_neg)
		{
			yval = -yval;
		}
		// divide by 2 restricts movement to half window size. 
		CGFloat xval = min_of_x_and_y/2 * COREANIMATIONUTILITIES_ANIMATION_VIGOUR_OF_SHAKE + randx;
        CGPathAddLineToPoint(shake_path, NULL, CGRectGetMinX(view_frame) - xval, CGRectGetMinY(view_frame) + yval);
		
		
		randx =  arc4random() % (unsigned)min_of_x_and_y/16;
		// randomize y again
		randy =  arc4random() % (unsigned)min_of_x_and_y/16;
		pos_or_neg =  arc4random() % (unsigned)2;
		
		
		if(pos_or_neg)
		{
			randx = -randx;
		}
		
		pos_or_neg =  arc4random() % (unsigned)2;
		
		if(pos_or_neg)
		{
			randy = -randy;
		}
		pos_or_neg =  arc4random() % (unsigned)2;
		
		yval = min_of_x_and_y/2.0 * COREANIMATIONUTILITIES_ANIMATION_VIGOUR_OF_SHAKE + (CGFloat)randy;
		if(pos_or_neg)
		{
			yval = -yval;
		}
		// divide by 2 restricts movement to half window size. 
		xval = min_of_x_and_y/2 * COREANIMATIONUTILITIES_ANIMATION_VIGOUR_OF_SHAKE + randx;
        CGPathAddLineToPoint(shake_path, NULL, CGRectGetMinX(view_frame) + xval, CGRectGetMinY(view_frame) + yval);
	}
    CGPathCloseSubpath(shake_path);
    shake_animation.path = shake_path;
	
	shake_animation.duration = COREANIMATIONUTILITIES_ANIMATION_DURATION_OF_SHAKE;
	
    return shake_animation;
	
}


// Implementation inspired from:
// http://www.cimgf.com/2008/02/27/core-animation-tutorial-window-shake-effect/
CAKeyframeAnimation* CoreAnimationUtilities_PasswordShakeAnimationWithRect(CGRect view_frame)
{

	
	CAKeyframeAnimation* shake_animation = [CAKeyframeAnimation animation];
	//  CAKeyframeAnimation* shake_animation = [CAKeyframeAnimation animationWithKeyPath:@"frameOrigin"];
	
	
    CGMutablePathRef shake_path = CGPathCreateMutable();
    CGPathMoveToPoint(shake_path, NULL, CGRectGetMinX(view_frame), CGRectGetMinY(view_frame));
    NSUInteger the_index;
	
	// Take the min of x and y so elongated displays don't look overly weird when shaking.
	CGFloat min_of_x_and_y = view_frame.size.width;
	if(view_frame.size.height < view_frame.size.width)
	{
		min_of_x_and_y = view_frame.size.height;
	}
	
    for(the_index = 0; the_index < COREANIMATIONUTILITIES_ANIMATION_NUMBER_OF_SHAKES; ++the_index)
    {
		// hard-coded back and forth on x-axis
		// divide by 4 restricts movement to 1/4 window size. For large windows, 1/2 seemed a bit much.
		CGPathAddLineToPoint(shake_path, NULL, CGRectGetMinX(view_frame) - min_of_x_and_y/4 * COREANIMATIONUTILITIES_ANIMATION_VIGOUR_OF_SHAKE, CGRectGetMinY(view_frame) );
		CGPathAddLineToPoint(shake_path, NULL, CGRectGetMinX(view_frame) + min_of_x_and_y/4 * COREANIMATIONUTILITIES_ANIMATION_VIGOUR_OF_SHAKE , CGRectGetMinY(view_frame) );
		
	}
    CGPathCloseSubpath(shake_path);
    shake_animation.path = shake_path;
	
	shake_animation.duration = COREANIMATIONUTILITIES_ANIMATION_DURATION_OF_SHAKE;
	
    return shake_animation;
	
}

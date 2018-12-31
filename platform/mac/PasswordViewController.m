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

#import "PasswordViewController.h"
#import <QuartzCore/QuartzCore.h>

@interface PasswordViewController ()

- (CAKeyframeAnimation*) shakeAnimation:(NSRect)view_frame;

@end

@implementation PasswordViewController

@synthesize forwardMessageObject;

- (void) awakeFromNib
{
	[[self view] setWantsLayer:YES];
}

- (IBAction) cancelPasswordLogin:(id)the_sender
{
	[forwardMessageObject cancelPasswordLogin:the_sender];
}

- (IBAction) submitPasswordLogin:(id)the_sender
{
//	[forwardMessageObject submitPasswordLogin:the_sender];
	NSLog(@"submitPasswordLogin, %@", [self view]);

	[[self view] setAnimations:[NSDictionary dictionaryWithObject:[self shakeAnimation:[[self view] frame]] forKey:@"frameOrigin"]];
//	[[self view] setAnimations:[NSDictionary dictionaryWithObject:[self shakeAnimation:[[self view] frame]] forKey:@"frame"]];
//	[[[self view] animator] setFrameOrigin:[[self view] frame].origin];
	[[[self view] animator] setFrameOrigin:NSMakePoint(201, 201)];
	[[[self view] animator] setFrameOrigin:NSMakePoint(200, 200)];
//	[[self view] startAnimation];

//	[[[self view] animator] setFrame:[[self view] frame]];
	
}


#define PASSWORD_ANIMATION_NUMBER_OF_SHAKES 4
#define PASSWORD_ANIMATION_DURATION_OF_SHAKE 0.5f
#define PASSWORD_ANIMATION_VIGOUR_OF_SHAKE 0.5f
#if 1
- (CAKeyframeAnimation*) shakeAnimation:(NSRect)view_frame
{
	CAKeyframeAnimation* shake_animation = [CAKeyframeAnimation animation];
  //  CAKeyframeAnimation* shake_animation = [CAKeyframeAnimation animationWithKeyPath:@"frameOrigin"];

#if 1
    CGMutablePathRef shake_path = CGPathCreateMutable();
    CGPathMoveToPoint(shake_path, NULL, NSMinX(view_frame), NSMinY(view_frame));
    NSUInteger the_index;
    for(the_index = 0; the_index < PASSWORD_ANIMATION_NUMBER_OF_SHAKES; ++the_index)
    {
        CGPathAddLineToPoint(shake_path, NULL, NSMinX(view_frame) - view_frame.size.width/2 * PASSWORD_ANIMATION_VIGOUR_OF_SHAKE, NSMinY(view_frame));
        CGPathAddLineToPoint(shake_path, NULL, NSMinX(view_frame) + view_frame.size.width/2 * PASSWORD_ANIMATION_VIGOUR_OF_SHAKE, NSMinY(view_frame));
    }
    CGPathCloseSubpath(shake_path);
    shake_animation.path = shake_path;
#else
	
	NSRect key_frame_1 = NSMakeRect(NSMinX(view_frame) - view_frame.size.width * PASSWORD_ANIMATION_VIGOUR_OF_SHAKE, NSMinY(view_frame), view_frame.size.width, view_frame.size.height);
	NSRect key_frame_2 = NSMakeRect(NSMinX(view_frame) + view_frame.size.width * PASSWORD_ANIMATION_VIGOUR_OF_SHAKE, NSMinY(view_frame), view_frame.size.width, view_frame.size.height);
	NSArray* key_frame_values = [NSArray arrayWithObjects:[NSValue valueWithRect:key_frame_1], [NSValue valueWithRect:key_frame_2], nil];
	[shake_animation setValues:key_frame_values];
	
	[shake_animation setDuration:PASSWORD_ANIMATION_DURATION_OF_SHAKE];
	[shake_animation setRepeatCount:PASSWORD_ANIMATION_NUMBER_OF_SHAKES];
#endif
	shake_animation.duration = PASSWORD_ANIMATION_DURATION_OF_SHAKE;

    return shake_animation;

}
#else
static int numberOfShakes = 8;
static float durationOfShake = 0.5f;
static float vigourOfShake = 0.05f;

- (CAKeyframeAnimation *)shakeAnimation:(NSRect)frame
{
NSLog(@"the_frame=%@", NSStringFromRect(frame));
    CAKeyframeAnimation *shakeAnimation = [CAKeyframeAnimation animation];
	
    CGMutablePathRef shakePath = CGPathCreateMutable();
    CGPathMoveToPoint(shakePath, NULL, NSMinX(frame), NSMinY(frame));
	int index;
	for (index = 0; index < numberOfShakes; ++index)
	{
		CGPathAddLineToPoint(shakePath, NULL, NSMinX(frame) - frame.size.width * vigourOfShake, NSMinY(frame));
		CGPathAddLineToPoint(shakePath, NULL, NSMinX(frame) + frame.size.width * vigourOfShake, NSMinY(frame));
	}
    CGPathCloseSubpath(shakePath);
    shakeAnimation.path = shakePath;
    shakeAnimation.duration = durationOfShake;
    return shakeAnimation;
	
}

#endif


@end

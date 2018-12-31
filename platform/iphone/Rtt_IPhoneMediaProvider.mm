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

#include "Core/Rtt_Build.h"
#include "Rtt_Lua.h"

#include "Rtt_PlatformMediaProviderBase.h"
#include "Rtt_IPhoneMediaProvider.h"

#include "Rtt_AppleBitmap.h"
#include "Rtt_AppleData.h"
#include "Rtt_Runtime.h"
#include "Rtt_RenderingStream.h"

#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>

#import "AppDelegate.h"

#import <MobileCoreServices/MobileCoreServices.h>

static bool Internal_IsIOS6_0()
{
    /* [[UIDevice currentDevice] systemVersion] returns "4.0", "3.1.3" and so on. */
    NSString* ver = [[UIDevice currentDevice] systemVersion];
	
    /* I assume that the default iOS version will be 6.0, that is why I set this to 6.0 */
    double version = 6.0;
	
    if ([ver length]>=3)
    {
        /*
		 The version string has the format major.minor.revision (eg. "3.1.3").
		 I am not interested in the revision part of the version string, so I can do this.
		 It will return the float value for "3.1", because substringToIndex is called first.
		 */
        version = [[ver substringToIndex:3] doubleValue];
    }
    return (version == 6.0);
}
static bool Internal_IsIOS7_0()
{
    /* [[UIDevice currentDevice] systemVersion] returns "4.0", "3.1.3" and so on. */
    NSString* ver = [[UIDevice currentDevice] systemVersion];
	
    /* I assume that the default iOS version will be 6.0, that is why I set this to 6.0 */
    double version = 7.0;
	
    if ([ver length]>=3)
    {
        /*
		 The version string has the format major.minor.revision (eg. "3.1.3").
		 I am not interested in the revision part of the version string, so I can do this.
		 It will return the float value for "3.1", because substringToIndex is called first.
		 */
        version = [[ver substringToIndex:3] doubleValue];
    }
    return (version == 7.0);
}
static bool Internal_UseIOS_IPadPhotoPickerLandscapeWorkaround()
{

	// Only run workaround for iPhone/iPod touch, not iPad (or anything else like AppleTV
	if ( UI_USER_INTERFACE_IDIOM() != UIUserInterfaceIdiomPad )
	{
		return false;
	}

	// Only run workaround for iOS 6.0.x
	if( Internal_IsIOS6_0() )
	{
		// Look for a special Info.plist key of our choosing if the user wants to opt-in.
		// We make users opt-in to avoid accidentally breaking cases that work, such as supporting all orientations.
		NSBundle* bundle = [NSBundle mainBundle];
		id value = [bundle objectForInfoDictionaryKey:@"CoronaUseIOS6IPadPhotoPickerLandscapeOnlyWorkaround"];
		if ( value && [value isKindOfClass:[NSNumber class]])
		{
			BOOL usehack = [(NSNumber*)value boolValue];
			return ( usehack == YES );
		}
	}
	else if( Internal_IsIOS7_0() )
	{
		NSBundle* bundle = [NSBundle mainBundle];
		id value = [bundle objectForInfoDictionaryKey:@"CoronaUseIOS7IPadPhotoPickerLandscapeOnlyWorkaround"];
		if ( value && [value isKindOfClass:[NSNumber class]])
		{
			BOOL usehack = [(NSNumber*)value boolValue];
			return ( usehack == YES );
		}
	}
	
	return false;
}

// Callback/Notification glue code
@interface IPhoneMediaPickerControllerDelegate : NSObject< UINavigationControllerDelegate, UIImagePickerControllerDelegate, UIPopoverControllerDelegate >
{
	NSObject<UIImagePickerControllerDelegate>* callback;
	BOOL usingPopoverController;
	Rtt::IPhoneMediaProvider* owner;
}

@property (nonatomic, readwrite, assign) NSObject<UIImagePickerControllerDelegate>* callback;
@property (nonatomic, readwrite, assign) Rtt::IPhoneMediaProvider* owner;
@property (nonatomic, assign) BOOL usingPopoverController;

@end

@implementation IPhoneMediaPickerControllerDelegate

@synthesize callback;
@synthesize usingPopoverController;
@synthesize owner;

- (void)dismissPicker:(UIImagePickerController *)picker completion:(void (^)(void))completion
{
	if(YES == self.usingPopoverController)
	{
		if(UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
		{
			[owner->fPopoverController dismissPopoverAnimated:true];
			completion();
		}
	}
	else
	{
		// Pre iOS-5, [picker parentViewController] worked, but it broke in iOS5.
		// StackOverflow suggests just calling dimiss directly on the picker will work for both iOS 4 and 5.
		// Testing on an iPad 2 with iOS 5 and an iPhone 4 with 4.3 seem to be okay with this.
//		[[picker parentViewController] dismissModalViewControllerAnimated:YES];		
		[picker dismissViewControllerAnimated:YES completion:completion];
	}
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
	// Return the state to the lua side after its been dismissed incase the user tries to show another controller before this has finished dismissing
	[self dismissPicker:picker completion:^{
		[callback imagePickerController:nil didFinishPickingMediaWithInfo:nil];
		owner->ReleaseVariables();
	}];
	
}

// If we need to clean-up anything for the popover, use this.
- (void) popoverControllerDidDismissPopover:(UIPopoverController*)popover_controller
{
	// Don't need to call dismissPicker because the popover should already disappear in this case.
	[callback imagePickerController:nil didFinishPickingMediaWithInfo:nil];
	owner->ReleaseVariables();
}

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
	// Return the state to the lua side after its been dismissed incase the user tries to show another controller before this has finished dismissing
	[self dismissPicker:picker completion:^{
		[callback imagePickerController:picker didFinishPickingMediaWithInfo:info];
		owner->ReleaseVariables();
	}];
}

@end
/* For iOS 6.0 on iPad, we needed to override shouldAutorotate to avoid a landscape-only crash. 
 * This is the only reason we use a subclass of UIImagePickerController instead of using it directly.
 */
@interface IPhonePickerController : UIImagePickerController
@end

@implementation IPhonePickerController
- (BOOL) shouldAutorotate
{
	// Only iOS 6/7 iPad
	if ( Internal_UseIOS_IPadPhotoPickerLandscapeWorkaround() )
	{
		// Disable to avoid crashes. Still seems to rotate if you flip 180 degrees despite this setting.
		return NO;
	}
	else
	{
		return [super shouldAutorotate];		
	}
}

- (BOOL)prefersStatusBarHidden
{
    return YES;
}

-(UIViewController *)childViewControllerForStatusBarHidden
{
    return nil;
}
@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneMediaProvider::IPhoneMediaProvider()
:	fImagePicker( nil ),
	fPopoverController( nil ),
	fDstPath( nil ),
	iOS5statusBarHidden( false ),
	fDelegate( nil )
{
}

IPhoneMediaProvider::~IPhoneMediaProvider()
{
//	Cleanup();
	[fDstPath release];
	// Should be safe to release even if we never use it.
	[fPopoverController release];
	[fImagePicker release];
	[fDelegate release];
}

void
IPhoneMediaProvider::Initialize()
{
}

bool
IPhoneMediaProvider::Internal_IsOS5_0()
{
    /* [[UIDevice currentDevice] systemVersion] returns "4.0", "3.1.3" and so on. */
    NSString* ver = [[UIDevice currentDevice] systemVersion];
	
    /* I assume that the default iOS version will be 5.0, that is why I set this to 5.0 */
    double version = 5.0;
	
    if ([ver length]>=3)
    {
        /*
		 The version string has the format major.minor.revision (eg. "3.1.3").
		 I am not interested in the revision part of the version string, so I can do this.
		 It will return the float value for "3.1", because substringToIndex is called first.
		 */
        version = [[ver substringToIndex:3] doubleValue];
    }
    return (version == 5.0);
}

UIImagePickerControllerSourceType
IPhoneMediaProvider::MediaProviderTypeToImagePickerSourceType( int source )
{
	PlatformMediaProviderBase::Source s = (PlatformMediaProviderBase::Source)source;

	UIImagePickerControllerSourceType result = UIImagePickerControllerSourceTypePhotoLibrary;
	switch( s )
	{
		case PlatformMediaProviderBase::kPhotoLibrary:
			result = UIImagePickerControllerSourceTypePhotoLibrary;
			break;
		case PlatformMediaProviderBase::kCamera:
			result = UIImagePickerControllerSourceTypeCamera;
			break;
		case PlatformMediaProviderBase::kSavedPhotosAlbum:
			result = UIImagePickerControllerSourceTypeSavedPhotosAlbum;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

void
IPhoneMediaProvider::Show( UIImagePickerControllerSourceType source, NSString* mediaTypes, NSObject<UIImagePickerControllerDelegate>* delegate, lua_State* L, int tableIndex, NSTimeInterval maxTime, UIImagePickerControllerQualityType quality )
{
	// iOS 5.0 introduces a bug where the status bar comes back if it is hidden on dismiss.
	// Seems to be fixed in 5.1 beta (unless iPod touch 4th gen was not originally affected)
	if ( Internal_IsOS5_0() && (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) )
	{
		iOS5statusBarHidden = [[UIApplication sharedApplication] isStatusBarHidden];
	}
	
	if ( ! fDelegate )
	{
		fDelegate = [[IPhoneMediaPickerControllerDelegate alloc] init];
		fDelegate.owner = this;
	}
	
	fDelegate.callback = delegate;
	
	if ( ! fImagePicker )
	{
		
		/* For iOS 6.0 on iPad, we needed to override shouldAutorotate to avoid a landscape-only crash.
		 * This is the only reason we use a subclass of UIImagePickerController instead of using it directly.
		 */
		fImagePicker = [[IPhonePickerController alloc] init];
		fImagePicker.delegate = fDelegate;
		// Source of where to get the media, eg. photo library, camera roll, or the camera
		fImagePicker.sourceType = source;
		// which types of media to select from the source, eg. movies or images
		fImagePicker.mediaTypes = [[NSArray alloc] initWithObjects: mediaTypes, nil];
		
		if ( maxTime>0 )
		{
			fImagePicker.videoMaximumDuration = maxTime;
		}
		
		if ( quality )
		{
			fImagePicker.videoQuality = quality;
		}

		/* Problem: When iPad was introduced, our image picker code broke. It seemed that you must put a picker in a popover controller 
		 * and could not directly add it to the main view controller.
		 * But with iPad 2, there seems to be a new fullscreen picker which must not go in the image picker or you don't get a fullscreen picker.
		 * We don't have iPad 1's with the correct old iOS versions to do proper regression testing.
		 * Furthermore, there are no API clues about this behavior difference.
		 * So I am resorting to an evil thing of querying the iOS version number directly.
		 * Addendum: It appears that UIImagePickerControllerSourceTypePhotoLibrary throws an exception if not in a popover controller even in 4.3.
		 * Also, we are getting a weird black bar on the right-side of the picker with UIImagePickerControllerSourceTypeSavedPhotosAlbum. It also crashes frequently
		 * when doing elastic bounds scrolls in either fullscreen or popover controller. For now, I've decided to keep it in a popover so developers have less incentive
		 * to use it over PhotoLibrary which doesn't crash.
		 */
		if( (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) && ( UIImagePickerControllerSourceTypeCamera != fImagePicker.sourceType ) )
		{
			if( ! fPopoverController )
			{
				fPopoverController = [[UIPopoverController alloc] initWithContentViewController:fImagePicker];     
				[fPopoverController setDelegate:fDelegate];					
			}
		}
	}


	if( (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) && ( UIImagePickerControllerSourceTypeCamera != fImagePicker.sourceType) )
	{
		fDelegate.usingPopoverController = YES;
		// We need a view to attach the popover to.
		// I don't know what to use, but this is one I can get a pointer to.
		AppDelegate* app_delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
		UIView* view_to_attach_popover_to = app_delegate.viewController.view;
		
		CGFloat left = 0;
		CGFloat top = 0;
		CGFloat width = 1;
		CGFloat height = 1;
		UIPopoverArrowDirection direction = UIPopoverArrowDirectionAny;
		if( ( lua_type( L, tableIndex) == LUA_TTABLE ) ) // look for the keys that we are interested in
		{
			// Intended to make it easy to pass origin=button.contentBounds
			lua_getfield( L, tableIndex, "origin" );
			if ( lua_type( L, -1) == LUA_TTABLE ) 
			{
				S32 xmin = 0;
				S32 ymin = 0;
				S32 xmax = 1;
				S32 ymax = 1;
				
				lua_getfield( L, -1, "xMin" );
				if ( lua_type( L, -1) == LUA_TNUMBER ) 
				{
					xmin = (S32)lua_tointeger( L, -1 );
				}
				lua_pop( L, 1 );
				
				lua_getfield( L, -1, "yMin" );
				if ( lua_type( L, -1) == LUA_TNUMBER ) 
				{
					ymin = (S32)lua_tointeger( L, -1 );
				}
				lua_pop( L, 1 );
				
				lua_getfield( L, -1, "xMax" );
				if ( lua_type( L, -1) == LUA_TNUMBER ) 
				{
					xmax = (S32)lua_tointeger( L, -1 );
				}
				lua_pop( L, 1 );
				
				lua_getfield( L, -1, "yMax" );
				if ( lua_type( L, -1) == LUA_TNUMBER ) 
				{
					ymax = (S32)lua_tointeger( L, -1 );
				}
				lua_pop( L, 1 );
				
				// FIXME: Need to convert from Corona content coordinates to Cocoa coordinates.
				const Display& display = ((AppDelegate*)[[UIApplication sharedApplication] delegate]).runtime->GetDisplay();

				// Converts to pixels (but we need points)
				display.ContentToScreen(xmin, ymin);
				display.ContentToScreen(xmax, ymax);
				
				// Convert Pixels to Points => divide by the contentScaleFactor
				// UIView contentScaleFactor will be 1.0 on an OpenGL view even on a retina display.
				// The UIScreen scale method seems to be what we want. 
				// The caveat is maybe we want the current screen instead of the main screen if we start dealing with remote/attached displays, but I'm not sure.
				CGFloat scale_factor = [[UIScreen mainScreen] scale];
				left = xmin / scale_factor;
				top = ymin / scale_factor;
				width = (xmax-xmin) / scale_factor ;
				height = (ymax-ymin) / scale_factor;
				
			}
			lua_pop( L, 1 );

			lua_getfield( L, tableIndex, "permittedArrowDirections" );
			if ( lua_type( L, -1) == LUA_TNUMBER) 
			{
				// Support backdoor integer in case users need to specify the undocumented value '0' for no arrow
				direction = lua_tonumber( L, -1 );
			}
			else if ( lua_type( L, -1) == LUA_TSTRING )
			{
				if ( 0 == strcmp( "any", lua_tostring( L, -1 ) ) )
				{
					direction = UIPopoverArrowDirectionAny;
				}
				else if( 0 == strcmp( "up", lua_tostring( L, -1 ) ) )
				{
					direction = UIPopoverArrowDirectionUp;
				}
				else if( 0 == strcmp( "down", lua_tostring( L, -1 ) ) )
				{
					direction = UIPopoverArrowDirectionDown;
				}
				else if( 0 == strcmp( "left", lua_tostring( L, -1 ) ) )
				{
					direction = UIPopoverArrowDirectionLeft;
				}
				else if( 0 == strcmp( "right", lua_tostring( L, -1 ) ) )
				{
					direction = UIPopoverArrowDirectionRight;
				}
			}
			else if ( lua_type( L, -1) == LUA_TTABLE )
			{
				int max = (int)lua_objlen( L, -1 );
				// Make sure the table isn't empty.
				if ( max > 0 )
				{
					// We need to clear the the 'Any' direction set above.
					direction = 0;
					// Assumes an array of strings
					for ( int i = 1; i <= max; i++ )
					{
						lua_rawgeti( L, -1, i );
						
						if ( 0 == strcmp( "any", lua_tostring( L, -1 ) ) )
						{
							direction |= UIPopoverArrowDirectionAny;
						}
						else if( 0 == strcmp( "up", lua_tostring( L, -1 ) ) )
						{
							direction |= UIPopoverArrowDirectionUp;
						}
						else if( 0 == strcmp( "down", lua_tostring( L, -1 ) ) )
						{
							direction |= UIPopoverArrowDirectionDown;
						}
						else if( 0 == strcmp( "left", lua_tostring( L, -1 ) ) )
						{
							direction |= UIPopoverArrowDirectionLeft;
						}
						else if( 0 == strcmp( "right", lua_tostring( L, -1 ) ) )
						{
							direction |= UIPopoverArrowDirectionRight;
						}
						
						lua_pop( L, 1 );
					}
				}
			}
		}
		
		
		// resize to fit some designated size of the image picker.
		//				[fPopoverController setPopoverContentSize:CGSizeMake(480,480) animated:YES];
		// This seems to affect the location on screen.
		// iOS 5 is now presenting a ZeroRect warning:
		// -[UIPopoverController presentPopoverFromRect:inView:permittedArrowDirections:animated:]: the rect passed in to this method must have non-zero width and height. This will be an exception in a future release.
		// So we should avoid ZeroRect to avoid any possible problems with 0,0 size.
		// casenum:12439. Apple is rejecting a user for having an arrow pointing in the popover. 
		// casenum:12439 Ugh. Disabling the arrow direction seems to have adversely affected the size of the popover (at least in the Xcode simulator).
		// Instead of 1,1, I need to specify a real size. However, I can't seem to get the size to be as tall as with the arrow.
		// Experimentally, setting taller than 508 seems to do nothing with no arrow. But with an arrow, it seems to move the x,y position, so I conclude setting too large is bad.
		// Widths greater than 320 seemed to have no effect. I did not try smaller widths.
		// This might be locking us into behavior we don't want if Apple decides the defaults are different in the future.
		// On iOS 4.3 (Xcode simulator), if there is no photo, the box dynamically shrinks (animated). It looks funny. But making the rect shorter seems to have no effect.
//			CGRect selected_rect = CGRectMake(0,0,320,508);
		// casenum:12439 Update: User was rejected again. We need to expose the parameters.
		// Also, I realize I misunderstood what the rect represents. The rect seems to represent the button the popover is anchored to, not the popover itself.
		// The intent is that the view you attach to is likely the button, so the rect coordinate space is relative to that view in most examples.
		// Since we attach to the root view, we need to provide a rect that represents the button, relative to the main view.
		// But the end result is that the rect must be the rect of the theoretical button that the popover originates from.
		CGRect selected_rect = CGRectMake(left, top, width, height);

		

		// Officially, there is no API to disable the arrow as UIPopoverArrowDirection has no official flag to disable this.
		// We've been passing UIPopoverArrowDirectionAny. However, Stackoverflow reports that passing 0 will disable the arrow and somebody passed the review process.
		[fPopoverController presentPopoverFromRect:selected_rect inView:view_to_attach_popover_to permittedArrowDirections:direction animated:YES];
	}
	else
	{
		fDelegate.usingPopoverController = NO;
		AppDelegate* delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
		UIViewController* viewController = delegate.viewController;
		[viewController presentViewController:fImagePicker animated:YES completion:nil];
	}
}

void
IPhoneMediaProvider::ReleaseVariables()
{
	[fPopoverController release];
	fPopoverController = nil;

	[fImagePicker release];
	fImagePicker = nil;
}
	
/*
void
IPhoneMediaProvider::SetProperty( U32 mask, bool newValue )
{
	Super::SetProperty( mask, newValue );

}
*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


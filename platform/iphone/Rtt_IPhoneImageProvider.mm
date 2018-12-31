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

#include "Rtt_IPhoneImageProvider.h"
#include "Rtt_IPhoneMediaProvider.h"
#include "Rtt_AppleBitmap.h"
#include "Rtt_AppleData.h"
#include "Rtt_Runtime.h"
#include "Rtt_RenderingStream.h"

#import <AssetsLibrary/AssetsLibrary.h>
#import <AVFoundation/AVFoundation.h>
#import <Photos/Photos.h>
#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>

#import "AppDelegate.h"
#import <MobileCoreServices/MobileCoreServices.h>


// Callback/Notification glue code
@interface IPhoneImagePickerControllerDelegate : NSObject< UIImagePickerControllerDelegate >
{
	Rtt::IPhoneImageProvider* callback;
}

@property (nonatomic, readwrite, assign) Rtt::IPhoneImageProvider* callback;

@end

@implementation IPhoneImagePickerControllerDelegate

@synthesize callback;
- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
	if ( picker )
	{
		UIImage *pickedImage = [[info objectForKey:UIImagePickerControllerOriginalImage] retain];
		callback->DidDismiss(pickedImage, info);
		[pickedImage release];
	}
	else
	{
		callback->DidDismiss(nil, nil);
	}
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneImageProvider::IPhoneImageProvider( const ResourceHandle<lua_State> & handle )
:	PlatformImageProvider( handle ),
	fDstPath( nil ),
	iOS5statusBarHidden( false )
{
	fDelegate = [[IPhoneImagePickerControllerDelegate alloc] init];
	fDelegate.callback = this;
	fMediaProvider = Rtt_NEW( LuaContext::GetAllocator( handle.Dereference() ), IPhoneMediaProvider );
}

IPhoneImageProvider::~IPhoneImageProvider()
{
//	Cleanup();
	[fDstPath release];
	[fDelegate release];
	Rtt_DELETE( fMediaProvider );
}

void
IPhoneImageProvider::Initialize()
{
}
	
bool
IPhoneImageProvider::Supports( int source ) const
{
	return [UIImagePickerController isSourceTypeAvailable:fMediaProvider->MediaProviderTypeToImagePickerSourceType( source )];
}

bool
IPhoneImageProvider::HasAccessTo( int source ) const
{
	bool sourceTypeAvailable = Supports(source);
	
	// We need to check the privacy settings if we're on a device/OS version that supports it.
	if (sourceTypeAvailable)
	{
		AuthorizationStatus authStatus = getAuthorizationStatusForSourceType( source );
		switch ( authStatus )
		{
			case AuthorizationStatusNotDetermined:
				// we assume that by default user would allow usage on iOS
				return true;
				break;
			case AuthorizationStatusAuthorized:
				// The source is present and we have access to it!
				return true;
				break;
			default:
				// Access is denied for one reason or another.
				return false;
				break;
		}
	}
	
	return sourceTypeAvailable;
}

bool
IPhoneImageProvider::Show( int source, const char* filePath, lua_State* L )
{
	[fDstPath release];
	if ( NULL != filePath )
	{
		fDstPath = [[NSString alloc] initWithUTF8String:filePath];
	}
	else
	{
		fDstPath = nil;
	}
	
	bool result = Rtt_VERIFY( Supports( source ) );

	if ( result )
	{
		/*
		If this is called from media.show then the ipad only parameters would be in the second table parameter
		eg. media.show( media.PhotoLibrary, { listener [, origin] [, permittedArrowDirections] } )
		If this is called from media.selectVideo then the table would be in the first parameter
		eg. media.selectVideo({ listener [, origin] [, permittedArrowDirections]})
		*/
		int tableIndex = 1;
		if ( lua_type( L, tableIndex ) != LUA_TTABLE )
		{
			tableIndex++;
		}

		fMediaProvider->Show( fMediaProvider->MediaProviderTypeToImagePickerSourceType( source ), (NSString *) kUTTypeImage, fDelegate, L, tableIndex, 0, (UIImagePickerControllerQualityType)0 );
	}
	else
	{
		EndSession();
	}

	return result;
}

void
IPhoneImageProvider::DidDismiss( UIImage* image, NSDictionary* editingInfo )
{
	AppleFileBitmap* bitmap = NULL;
	if ( image )
	{
		if ( fDstPath )
		{
			NSData *data = nil;
			NSString *lowercase = [fDstPath lowercaseString];
			if ( [lowercase hasSuffix:@"png"] )
			{
				data = UIImagePNGRepresentation( image );
			}
			else if ( [lowercase hasSuffix:@"jpg"] || [lowercase hasSuffix:@"jpeg"] )
			{
				data = UIImageJPEGRepresentation( image, 1.0 );
			}
			[data writeToFile:fDstPath atomically:YES];

			[fDstPath release];
			fDstPath = nil;
		}
		else
		{
			Rtt_Allocator* allocator = ((AppDelegate*)[[UIApplication sharedApplication] delegate]).runtime->Allocator(); Rtt_UNUSED( allocator );
			bitmap = Rtt_NEW( allocator, IPhoneFileBitmap( image ) );
		}
	}

	PlatformImageProvider::Parameters params( bitmap, NULL );
	params.wasCompleted = (image != NULL);
	Super::DidDismiss( AddProperties, & params );
	
	// iOS 5.0 introduces a bug where the status bar comes back if it is hidden on dismiss.
	// Seems to be fixed in 5.1 beta (unless iPod touch 4th gen was not originally affected)
	if ( fMediaProvider->Internal_IsOS5_0() && (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) )
	{
		[[UIApplication sharedApplication] setStatusBarHidden:iOS5statusBarHidden withAnimation:UIStatusBarAnimationNone];
	}
}
	
/*
void
IPhoneImageProvider::SetProperty( U32 mask, bool newValue )
{
	Super::SetProperty( mask, newValue );

}
*/

AuthorizationStatus
IPhoneImageProvider::getAuthorizationStatusForSourceType( int source )
{
	switch ( source )
	{
		case PlatformMediaProviderBase::kCamera:
			if ( [AVCaptureDevice respondsToSelector:@selector(authorizationStatusForMediaType:)] )
			{
				// There's no privacy setting for the Camera prior to iOS 8.
				// Calling this on iOS 7 will always return AuthorizationStatusAuthorized!
				return (AuthorizationStatus)[AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
			}
			else return AuthorizationStatusAuthorized;
			break;
		case PlatformMediaProviderBase::kPhotoLibrary:
		case PlatformMediaProviderBase::kSavedPhotosAlbum:
			if ( [PHPhotoLibrary class] )
			{
				return (AuthorizationStatus)[PHPhotoLibrary authorizationStatus];
			}
			else
			{
				return (AuthorizationStatus)[ALAssetsLibrary authorizationStatus];
			}
			break;
		default:
			return AuthorizationStatusNotDetermined;
			break;
	}
	
	return AuthorizationStatusNotDetermined;
}


////TODO: move this code to native.showPopup("requestPermissions") and remove deadlock...
//bool
//IPhoneImageProvider::requestAuthorizationForSourceType( int source )
//{
//	// Request access to the source.
//	__block bool authorizationGranted = false;
//	dispatch_semaphore_t authorizationSemaphore = dispatch_semaphore_create(0);
//
//	switch ( source )
//	{
//		case PlatformMediaProviderBase::kCamera:
//			if ( [AVCaptureDevice respondsToSelector:@selector(requestAccessForMediaType:completionHandler:)] )
//			{
//				[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
//					authorizationGranted = granted;
//					dispatch_semaphore_signal(authorizationSemaphore);
//				}];
//			} // There's no privacy setting for the Camera prior to iOS 8.
//			break;
//		case PlatformMediaProviderBase::kPhotoLibrary:
//		case PlatformMediaProviderBase::kSavedPhotosAlbum:
//			if ( [PHPhotoLibrary class] )
//			{
//				// We get access through the photo library.
//				[PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
//					authorizationGranted = PHAuthorizationStatusAuthorized == status;
//					dispatch_semaphore_signal(authorizationSemaphore);
//				}];
//			}
//			else
//			{
//				// We have to get access to the assets library.
//				ALAssetsLibrary *assetsLibrary = [[ALAssetsLibrary alloc] init];
//				
//				// Manually put this on background thread since the callbacks run on the main thread by default!
//				dispatch_async( dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 ), ^{
//					[assetsLibrary enumerateGroupsWithTypes:ALAssetsGroupAll usingBlock:^(ALAssetsGroup *group, BOOL *stop) {
//						if (*stop) {
//							// User hit the OK button.
//							authorizationGranted = true;
//							dispatch_semaphore_signal(authorizationSemaphore);
//							return;
//						}
//						*stop = TRUE;
//					} failureBlock:^(NSError *error) {
//						// User hit the Don't Allow button.
//						authorizationGranted = false;
//						dispatch_semaphore_signal(authorizationSemaphore);
//					}];
//				});
//			}
//			break;
//		default:
//			return false;
//			break;
//	}
//	
//	// Wait for a result from the user before proceeding.
//	// Corona is suspended while the permission request
//	// dialog is up so this shouldn't be harmful to us.
//	dispatch_semaphore_wait(authorizationSemaphore, DISPATCH_TIME_FOREVER);
//	dispatch_release(authorizationSemaphore);
//	return authorizationGranted;
//}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_Lua.h"

#include "Rtt_IPhoneVideoProvider.h"
#include "Rtt_IPhoneMediaProvider.h"

#include "Rtt_AppleBitmap.h"
#include "Rtt_AppleData.h"
#include "Rtt_Runtime.h"
#include "Rtt_RenderingStream.h"

#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>

#import "AppDelegate.h"

#import <MobileCoreServices/MobileCoreServices.h>
#import <AVFoundation/AVFoundation.h>
#import <AVFoundation/AVAsset.h>

// Callback/Notification glue code
@interface IPhoneVideoPickerControllerDelegate : NSObject< UIImagePickerControllerDelegate >
{
	Rtt::IPhoneVideoProvider* callback;
}

@property (nonatomic, readwrite, assign) Rtt::IPhoneVideoProvider* callback;
// This is a hack, since new iOS security measures make file inaccessible
//  as soon as the original URL is released.
@property (nonatomic, readwrite, assign) NSMutableArray<NSURL*> *urls;

@end

@implementation IPhoneVideoPickerControllerDelegate

@synthesize callback;
@synthesize urls;

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
	if ( !callback )
	{
		return;
	}
	
	NSURL *pickedMovie = [info objectForKey:UIImagePickerControllerMediaURL];

	if ( pickedMovie )
	{
		[urls addObject:pickedMovie];
		NSDictionary *fileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:[pickedMovie path] error:nil];
		NSNumber *fileSizeNumber = [fileAttributes objectForKey:NSFileSize];
		long fileSize = [fileSizeNumber longValue];
		
		AVURLAsset *sourceAsset = [AVURLAsset URLAssetWithURL:pickedMovie options:nil];
		CMTime duration = sourceAsset.duration;
		callback->DidDismiss(pickedMovie, (int)(duration.value/duration.timescale), fileSize);
	}
	else
	{
		callback->DidDismiss(nil, -1, -1);
	}
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneVideoProvider::IPhoneVideoProvider( const ResourceHandle<lua_State> & handle )
:	PlatformVideoProvider( handle ),
	iOS5statusBarHidden( false )
{
	fDelegate = [[IPhoneVideoPickerControllerDelegate alloc] init];
	fDelegate.callback = this;
	fDelegate.urls = [[NSMutableArray alloc] init];
	fMediaProvider = Rtt_NEW( LuaContext::GetAllocator( handle.Dereference() ), IPhoneMediaProvider );
}

IPhoneVideoProvider::~IPhoneVideoProvider()
{
//	Cleanup();
	[fDelegate.urls release];
	[fDelegate release];
	Rtt_DELETE( fMediaProvider );
}

void
IPhoneVideoProvider::Initialize()
{
}

bool
IPhoneVideoProvider::Supports( int source ) const
{
	return [UIImagePickerController isSourceTypeAvailable:fMediaProvider->MediaProviderTypeToImagePickerSourceType( source )];
}

bool
IPhoneVideoProvider::Show( int source, lua_State* L, int maxTime, int quality )
{

	bool result = Rtt_VERIFY( Supports( source ) );

	if ( result )
	{
		/*
		Since this can only be called from media.selectVideo then the table would be in the first parameter
		eg. media.selectVideo({ listener [, origin] [, permittedArrowDirections]})
		*/
		NSTimeInterval interval = maxTime;
		UIImagePickerControllerQualityType videoQuality = UIImagePickerControllerQualityTypeLow;
		
		switch ( quality )
		{
			case PlatformMediaProviderBase::kHigh :
				videoQuality = UIImagePickerControllerQualityTypeHigh;
				break;
			case PlatformMediaProviderBase::kMedium :
				videoQuality = UIImagePickerControllerQualityTypeMedium;
				break;
			default:
				videoQuality = UIImagePickerControllerQualityTypeLow;
				break;
		}
		
		fMediaProvider->Show(fMediaProvider->MediaProviderTypeToImagePickerSourceType( source ), (NSString *) kUTTypeMovie, fDelegate, L, 1, interval, videoQuality);
	}
	else
	{
		EndSession();
	}

	return result;
}

void
IPhoneVideoProvider::DidDismiss( NSURL* movie, int duration, long fileSize )
{
	PlatformVideoProvider::VideoProviderResult videoProviderResult;
	if ( movie )
	{
		videoProviderResult.SelectedVideoFileName = [movie.absoluteString UTF8String];
		videoProviderResult.Size = fileSize;
		videoProviderResult.Duration = duration;
	}
	Super::DidDismiss( AddProperties, & videoProviderResult );

	// iOS 5.0 introduces a bug where the status bar comes back if it is hidden on dismiss.
	// Seems to be fixed in 5.1 beta (unless iPod touch 4th gen was not originally affected)
	if ( fMediaProvider->Internal_IsOS5_0() && (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) )
	{
		[[UIApplication sharedApplication] setStatusBarHidden:iOS5statusBarHidden withAnimation:UIStatusBarAnimationNone];
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


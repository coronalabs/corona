//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacImageProvider.h"

#include "Rtt_AppleBitmap.h"
#include "Rtt_AppleData.h"
#include "Rtt_MacSimulator.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_Runtime.h"
#include "CoronaLua.h"
#import "Rtt_MacPlatform.h"

#import <AppKit/NSApplication.h>
#import <AppKit/NSOpenPanel.h>
#import <Quartz/Quartz.h> // For ImageKit

#import "AppDelegate.h"

// Add the ability to save an NSImage to a particular file (see MacPickerControllerDelegate:pictureTakerDidEnd below)
@interface NSImage(saveImageToFile)
- (void) saveImageToFile:(NSString*) fileName imageType:(NSBitmapImageFileType)imageType;
@end

@implementation NSImage(saveImageToFile)

- (void) saveImageToFile:(NSString*) fileName imageType:(NSBitmapImageFileType)imageType
{
    // Cache the reduced image
    NSData *imageData = [self TIFFRepresentation];
    NSBitmapImageRep *imageRep = [NSBitmapImageRep imageRepWithData:imageData];
    NSDictionary *imageProps = [NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.0] forKey:NSImageCompressionFactor];
    imageData = [imageRep representationUsingType:imageType properties:imageProps];

	NSError *error;
	if (! [imageData writeToFile:fileName options:NSDataWritingAtomic error:&error])
	{
		Rtt_TRACE_SIM(("saveImageToFile: %s", [[error description] UTF8String]));
	}
}

@end

// ----------------------------------------------------------------------------

// Callback/Notification glue code
@interface MacPickerControllerDelegate : NSObject
{
	Rtt::MacImageProvider* imageProvider;
}
- (void) setImageProvider:(Rtt::MacImageProvider*)imageprovider;
- (void)pictureTakerDidEnd:(IKPictureTaker*)sheet returnCode:(NSInteger)returncode contextInfo:(void*)contextinfo;
@end

@implementation MacPickerControllerDelegate

- (void) setImageProvider:(Rtt::MacImageProvider*)imageprovider
{
	imageProvider = imageprovider;
}

- (void) pictureTakerDidEnd:(IKPictureTaker*)picturetaker returnCode:(NSInteger)returncode contextInfo:(void*)contextinfo
{
    if ( returncode == NSOKButton )
    {
        // Fix bug #11480: save media to file on Simulator analogously to devices
        
        Rtt::MacImageProvider *macImageProvider = (Rtt::MacImageProvider *) contextinfo;
        
		NSImage *outputImage = nil;
		
        if ( macImageProvider && macImageProvider->fDstPath )
        {
            NSString *filePath = [NSString stringWithCString:macImageProvider->fDstPath encoding:NSUTF8StringEncoding];
            NSString *lowercase = [filePath lowercaseString];
            if ( [lowercase hasSuffix:@"png"] )
            {
                [[picturetaker outputImage] saveImageToFile:filePath imageType:NSPNGFileType];
            }
            else if ( [lowercase hasSuffix:@"jpg"] || [lowercase hasSuffix:@"jpeg"] )
            {
                [[picturetaker outputImage] saveImageToFile:filePath imageType:NSJPEGFileType];
            }
        }
		else
		{
			outputImage = [picturetaker outputImage];
		}
		
        // To match device behavior, if a save path is provided then we don't return the display object
        imageProvider->DidDismiss(outputImage, TRUE);
    }
    else
    {
        // Fix bug #19525: always call DidDismiss() whether or not OK was pressed to match device behavior
        
        imageProvider->DidDismiss(nil, FALSE);
    }
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
	
MacImageProvider::MacImageProvider( const ResourceHandle<lua_State> & handle ) 
	: PlatformImageProvider( handle ),
	fPickerControllerDelegate( nil ),
	fInterfaceIsUp(false),
	fDstPath( NULL )
{
	fPickerControllerDelegate = [[MacPickerControllerDelegate alloc] init];
	[fPickerControllerDelegate setImageProvider:this];
}

MacImageProvider::~MacImageProvider()
{
	[fPickerControllerDelegate release];
    
    if (fDstPath != NULL )
    {
        free( fDstPath );
    }
}

	
bool
MacImageProvider::Supports( int source ) const
{
    // We use the IKPictureTaker now so this warning is no longer valid
	// Rtt_TRACE_SIM( ( "WARNING: Simulator does not support camera, photo lib or saved photos album.\n" ) );
	// Rtt_TRACE_SIM( ( "         Instead, you should create a JPEG image file and choose it in the open file dialog.\n" ) );

	return true;
}

bool
MacImageProvider::Show( int source, const char* filePath, lua_State* L )
{
	bool result = Supports( source );
    
	if (filePath != NULL )
	{
		fDstPath = strdup( filePath );
	}
	else
	{
		fDstPath = NULL;
	}
	
	if ( result )
	{		
		if ( fInterfaceIsUp )
		{
			Rtt_TRACE_SIM( ( "Can't show while already showing" ) );
			return false;
			
		}
		if ( PlatformImageProvider::kCamera == source )
		{
			AppDelegate* delegate = (AppDelegate*)[NSApp delegate];
			NSView* view = (NSView*)[delegate layerHostView];
			IKPictureTaker* picturetaker = [IKPictureTaker pictureTaker];
			[picturetaker setValue:[NSNumber numberWithBool:YES] forKey:IKPictureTakerShowEffectsKey];
//			[picturetaker setValue:[NSNumber numberWithBool:YES] forKey:IKPictureTakerAllowsVideoCaptureKey];
			[picturetaker setValue:[NSValue valueWithSize:NSMakeSize(640, 480)] forKey:IKPictureTakerOutputImageMaxSizeKey];
//			[picturetaker setValue:[NSValue valueWithSize:NSMakeSize(512, 256)] forKey:IKPictureTakerCropAreaSizeKey]; // deprecated
			
			
			[picturetaker beginPictureTakerSheetForWindow:[view window] withDelegate:fPickerControllerDelegate didEndSelector:@selector(pictureTakerDidEnd:returnCode:contextInfo:) contextInfo:this];
			fInterfaceIsUp = true;
		}
		else
		{
			   
			
			NSOpenPanel* panel = [NSOpenPanel openPanel];

			[panel setAllowsMultipleSelection:NO];
			[panel setCanChooseDirectories:NO];
			[panel setCanChooseFiles:YES];
			// Pull the user's last folder from user defaults for their convenience
			NSString* start_directory = [[NSUserDefaults standardUserDefaults] stringForKey:kImageFolderPath];
			if(nil != start_directory)
			{
				Rtt_ASSERT( [start_directory isAbsolutePath] );
				[panel setDirectoryURL:[NSURL fileURLWithPath:start_directory]];
			}
			AppleFileBitmap* bitmap = NULL;
			AppleData* data = NULL;
			
			[panel setAllowedFileTypes:[NSArray arrayWithObjects:@"jpeg", @"jpg", @"png", nil]];

			if ( NSOKButton == [panel runModal] )
			{
				NSArray* fileURLs = [panel URLs];
				[[NSUserDefaults standardUserDefaults] setObject:[[panel directoryURL] path] forKey:kImageFolderPath];

				if ( fDstPath )
				{
					// Copy picked file to destination path
					NSURL *srcURL = [fileURLs lastObject];
					NSURL *dstURL = [NSURL fileURLWithPath:[[NSFileManager defaultManager] stringWithFileSystemRepresentation:fDstPath length:strlen(fDstPath)]];

					// If the source and destination have the same image types (i.e. the same extension), just
					// copy the files otherwise we have to convert the image format
					NSString *srcExtn = [[[srcURL path] pathExtension] lowercaseString];
					NSString *destExtn = [[[dstURL path] pathExtension] lowercaseString];

					if ([destExtn isEqualToString:srcExtn])
					{
						// Same image types: just copy the files

						NSError *error = nil;

						// We only do this so copyItemAtURL: doesn't fail if the item already exists
						// so no need to report any error
						[[NSFileManager defaultManager] removeItemAtURL:dstURL error:&error];

						error = nil;
						[[NSFileManager defaultManager] copyItemAtURL:srcURL toURL:dstURL error:&error];

						if (error != nil)
						{
							CoronaLuaWarning(L, "media.selectPhoto cannot copy image from %s (%s)", [[srcURL path] UTF8String], [[error description] UTF8String]);
						}
					}
					else
					{
						// Different image types: convert the format

						NSImage *img = [[[NSImage alloc] initWithContentsOfURL:srcURL] autorelease];

						if (img != nil)
						{
							NSString *filePath = [NSString stringWithUTF8String:fDstPath];
							NSString *lowercase = [filePath lowercaseString];
							if ([lowercase hasSuffix:@"png"] )
							{
								[img saveImageToFile:filePath imageType:NSPNGFileType];
							}
							else if ( [lowercase hasSuffix:@"jpg"] || [lowercase hasSuffix:@"jpeg"] )
							{
								[img saveImageToFile:filePath imageType:NSJPEGFileType];
							}
						}
						else
						{
							CoronaLuaWarning(L, "media.selectPhoto cannot load an image from %s", [[srcURL path] UTF8String]);
						}
					}

					fDstPath = NULL;
					
					DidDismiss( nil, TRUE );
				}
				else
				{
					NSString *filePath = [[fileURLs lastObject] path];
					
					Rtt_Allocator* allocator __attribute__((unused)) = NULL;

					bitmap = Rtt_NEW( allocator, AppleFileBitmap( [filePath UTF8String] ) );
					
					data = Rtt_NEW( allocator, AppleData( [NSData dataWithContentsOfFile:filePath] ) );
					
					PlatformImageProvider::Parameters params( bitmap, data );
					Super::DidDismiss( AddProperties, & params );
				}
			}
			else
			{
				DidDismiss( nil, FALSE );
			}
		}
	}
	else
	{
		EndSession();
	}

	return result;
}
	
void
MacImageProvider::DidDismiss( NSImage* image, bool completed )
{
	AppleFileBitmap* bitmap = NULL;
	if ( image )
	{	
		// This doesn't work for MacApp (no simulator)
		// TODO: Need better way to get Allocator via Runtime.
//		Rtt_Allocator* allocator = ((AppDelegate*)[NSApp delegate]).simulator->GetPlayer()->GetRuntime().Allocator(); Rtt_UNUSED( allocator);
		Rtt_Allocator* allocator __attribute__((unused)) = NULL;
		bitmap = Rtt_NEW( allocator, MacFileBitmap( image ) );
	}

	PlatformImageProvider::Parameters params( bitmap, NULL );
	params.wasCompleted = completed;
	Super::DidDismiss( AddProperties, & params );
	fInterfaceIsUp = false;

}
/*
void
MacImageProvider::HandlePictureTakerDidEnd( NSImage* image )
{
	
	Rtt_Allocator* allocator = ((AppDelegate*)[NSApp delegate]).simulator->GetPlayer()->GetRuntime().Allocator(); Rtt_UNUSED( allocator);
	Rtt::AppleFileBitmap* bitmap = Rtt_NEW( allocator, MacFileBitmap( [picturetaker outputImage] ) );
	//		data = Rtt_NEW( allocator, AppleData( [NSData dataWithContentsOfFile:filePath] ) );
	
	PlatformImageProvider::Parameters params( bitmap, NULL );
	Super::DidDismiss( AddProperties, & params );

}

*/
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


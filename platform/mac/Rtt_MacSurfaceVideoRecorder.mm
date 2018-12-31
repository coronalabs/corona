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

#if 0

#include "Core/Rtt_Build.h"

#include "Rtt_MacSurfaceVideoRecorder.h"

#include "Rtt_MacPlatform.h"
#include "Rtt_PlatformBitmap.h"
#include "Rtt_BitmapPaint.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#define FOO 0

static const U32 kNumberFramesToSkip = 5;

MacSurfaceVideoRecorder::MacSurfaceVideoRecorder( Runtime& runtime )
:	fRuntime( runtime ),
	fMovie( nil ),
	fFrameCount( 0 ),
	fFrameInterval( QTMakeTime( kNumberFramesToSkip, runtime.GetFPS() ) ),
#if FOO
	fAttributes( [[NSDictionary alloc] initWithObjectsAndKeys:@"tga ", QTAddImageCodecType, nil] )
#else
	fAttributes( [[NSDictionary alloc] initWithObjectsAndKeys:@"png ", QTAddImageCodecType, nil] )
#endif
{
}

MacSurfaceVideoRecorder::~MacSurfaceVideoRecorder()
{
	[fAttributes release];
	StopRecording();
}

void
MacSurfaceVideoRecorder::WillFlush( const PlatformSurface& surface ) const
{
}

void
MacSurfaceVideoRecorder::DidFlush( const PlatformSurface& surface ) const
{
	BitmapPaint* frame = fRuntime.Capture( NULL );

	if ( frame && 0 == (fFrameCount % kNumberFramesToSkip) )
	{
#if FOO
		size_t numBytes = frame->Width() * frame->Height() * PlatformBitmap::BytesPerPixel( frame->GetFormat() );
		NSData* data = [[NSData alloc] initWithBytesNoCopy:const_cast< void* >( frame->Bits( fRuntime.Allocator() ) ) length:numBytes freeWhenDone:NO];
		NSImage* image = [[NSImage alloc] initWithData:data];
		[fMovie addImage:image forDuration:fFrameInterval withAttributes:fAttributes];
		[image release];
		[data release];
#else
		CGImageRef macImage = MacGUIPlatform::CreateMacImage( fRuntime.Allocator(), *frame->GetBitmap() );
		NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc] initWithCGImage:macImage];
		NSImage* image = [[NSImage alloc] init];

		[image addRepresentation:bitmapRep];
		[fMovie addImage:image forDuration:fFrameInterval withAttributes:fAttributes];
		[image release];
		[bitmapRep release];
#endif

		Rtt_DELETE( frame );
	}
}

bool
MacSurfaceVideoRecorder::StartRecording()
{
	Rtt_ASSERT( ! fMovie );

	NSSavePanel *savePanel;

	savePanel = [NSSavePanel savePanel];
	[savePanel setExtensionHidden:NO];
	[savePanel setCanSelectHiddenExtension:NO];
	[savePanel setTreatsFilePackagesAsDirectories:NO];
	// Pull the user's last folder from user defaults for their convenience
	NSString* start_directory = [[NSUserDefaults standardUserDefaults] stringForKey:kVideoFolderPath];
	if(nil != start_directory)
	{
		Rtt_ASSERT( [start_directory isAbsolutePath] );
		[savePanel setDirectory:start_directory];
	}
	bool result = ( [savePanel runModal] == NSOKButton );
	if ( result )
	{
		NSString* filename = [savePanel filename]; Rtt_ASSERT( filename );
		[[NSUserDefaults standardUserDefaults] setObject:[savePanel directory] forKey:kVideoFolderPath];

		fMovie = [[QTMovie alloc] initToWritableFile:filename error:nil];

		Rtt_ASSERT_NOT_IMPLEMENTED();
		// fRuntime.Surface().SetDelegate( this );
	}

	return result;
}

void
MacSurfaceVideoRecorder::StopRecording()
{
	if ( fMovie )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		// fRuntime.Surface().SetDelegate( NULL );

		// Flush contents to file.
		[fMovie updateMovieFile];

		[fMovie release];
		fMovie = nil;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif

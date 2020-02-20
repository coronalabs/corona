//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ImageSheetPaint.h"
#include "Renderer/Rtt_RenderData.h"
#include "Display/Rtt_ImageFrame.h"
#include "Display/Rtt_ImageSheet.h"
#include "Rtt_TextureResource.h"
#include "Display/Rtt_ImageSheetPaintAdapter.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ImageSheetPaint*
ImageSheetPaint::NewBitmap( Rtt_Allocator *allocator, const AutoPtr< ImageSheet >& sheet )
{
	ImageSheetPaint *result = Rtt_NEW( allocator, ImageSheetPaint( sheet ) );

	return result;
}

ImageSheetPaint*
ImageSheetPaint::NewBitmap( Rtt_Allocator *allocator, const AutoPtr< ImageSheet >& sheet, int frameIndex )
{
	if ( frameIndex >= sheet->GetNumFrames() )
	{
		Rtt_TRACE_SIM( ( "WARNING: Supplied frameIndex(%d) exceeds image sheet's maximum frameIndex (%d). Value will be clamped.\n", frameIndex + 1, sheet->GetNumFrames() ) ); // Lua is 1-based
		frameIndex = sheet->GetNumFrames() - 1;
	}

	if ( frameIndex < 0 )
	{
		Rtt_TRACE_SIM( ( "WARNING: Supplied frameIndex(%d) cannot be less than 1. Value will be clamped.\n", frameIndex + 1 ) ); // Lua is 1-based
		frameIndex = 0;
	}

	ImageSheetPaint *result = Rtt_NEW( allocator, ImageSheetPaint( sheet, frameIndex ) );

	return result;
}

// ----------------------------------------------------------------------------

ImageSheetPaint::ImageSheetPaint( const AutoPtr< ImageSheet >& sheet, int frameIndex )
:	Super( sheet->GetTextureResource() ),
	fSheet( sheet ),
	fFrameIndex( frameIndex )
{
	Initialize( kImageSheet );
}

ImageSheetPaint::~ImageSheetPaint()
{
}

const Paint*
ImageSheetPaint::AsPaint( Type type ) const
{
	return ( Super::kImageSheet == type ? this : Super::AsPaint( type ) );
}


const AutoPtr< ImageSheet >&
ImageSheetPaint::GetSheet() const
{
	return fSheet;
}

	
int
ImageSheetPaint::GetNumFrames() const
{
	return fSheet->GetNumFrames();
}
	
void
ImageSheetPaint::SetFrame( int frameIndex )
{
	fFrameIndex = frameIndex;
	Invalidate( kTextureTransformFlag );
}

int
ImageSheetPaint::GetFrame() const
{
	return fFrameIndex;
}

const ImageFrame *
ImageSheetPaint::GetImageFrame() const
{
	const ImageFrame *result = NULL;

	// setup frame
	if ( Rtt_VERIFY( fFrameIndex >= 0 ) )
	{
		result = fSheet->GetFrame( fFrameIndex );
	}

	return result;
}

void ImageSheetPaint::ApplyPaintUVTransformations( ArrayVertex2& vertices ) const
{
	const ImageFrame* imgFrame = GetImageFrame();
	if( Rtt_VERIFY( imgFrame ) )
	{
		Vertex2* verticesArray = vertices.WriteAccess();
		for (int i = 0, iLen = vertices.Length(); i < iLen; i++)
		{
			imgFrame->UpdateUVVertex(verticesArray[i]);
		}
	}
}

const MLuaUserdataAdapter&
ImageSheetPaint::GetAdapter() const
{
	return ImageSheetPaintAdapter::Constant();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


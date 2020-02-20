//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"
#include "librtt/Rtt_GPUStream.h"
#include "AndroidZipFileEntry.h"
#include "NativeToJavaBridge.h"
#include "Rtt_AndroidBitmap.h"
#include "Rtt_PlatformFont.h"
#include "Display/Rtt_Display.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidBitmap::AndroidBitmap( Rtt_Allocator & context )
:	fImageData(&context)
{
}

AndroidBitmap::~AndroidBitmap()
{
}

const void * 
AndroidBitmap::GetBits( Rtt_Allocator* context ) const
{
	return (void*)fImageData.GetImageByteBuffer();
}

void 
AndroidBitmap::FreeBits() const
{
	fImageData.DestroyImageByteBuffer();
}

U32 
AndroidBitmap::Width() const
{
	return fImageData.GetWidth();
}

U32 
AndroidBitmap::Height() const
{
	return fImageData.GetHeight();
}

Real
AndroidBitmap::GetScale() const
{
	return fImageData.GetScale();
}

PlatformBitmap::Orientation 
AndroidBitmap::GetOrientation() const
{
	return fImageData.GetOrientation();
}

PlatformBitmap::Format 
AndroidBitmap::GetFormat() const
{
	// TODO: This may not be right. Looks like it *might* be kABGR,
	// but the byte buffer may also be reversed so everything cancels out!
	// TODO: If we fix this to be correct, we should also remove the hard-coded
	// alpha channel index in GraphicsLibrary::newOutline()
	return kRGBA;
}

// ----------------------------------------------------------------------------

static U8
GetInitialPropertiesValue()
{
	return PlatformBitmap::kIsPremultiplied;
}

AndroidAssetBitmap::AndroidAssetBitmap( Rtt_Allocator &context, const char *filePath, NativeToJavaBridge *ntjb )
:	Super( context ),
	fProperties( GetInitialPropertiesValue() ),
	fPath( & context, filePath ),
	fImageDecoder( &context, ntjb )
{
	// Set up the image decoder.
	fImageDecoder.SetTarget(&fImageData);
	fImageDecoder.SetPixelFormatToRGBA();
	fImageDecoder.SetMaxWidth(Rtt::Display::GetMaxTextureSize());
	fImageDecoder.SetMaxHeight(Rtt::Display::GetMaxTextureSize());

	// Fetch the image's information such as pixel width, height, and orientation.
	// The decoder will write this information to the targeted "fImageData" member variable.
	fImageDecoder.SetToDecodeImageInfoOnly();
	fImageDecoder.DecodeFromFile(filePath);

	// Set up the decoder to read the image pixels for when GetBits() gets called.
	fImageDecoder.SetToDecodeAllImageData();
}

AndroidAssetBitmap::~AndroidAssetBitmap()
{
}

const void* 
AndroidAssetBitmap::GetBits( Rtt_Allocator *context ) const
{
	const void *bits = Super::GetBits(context);
	if (!bits)
	{
		fImageDecoder.DecodeFromFile(fPath.GetString());
		bits = Super::GetBits(context);
	}
	return bits;
}

AndroidImageDecoder&
AndroidAssetBitmap::ImageDecoder()
{
	return fImageDecoder;
}

U32
AndroidAssetBitmap::SourceWidth() const
{
	return Super::Width();
}

U32
AndroidAssetBitmap::SourceHeight() const
{
	return Super::Height();
}

U32
AndroidAssetBitmap::Width() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceWidth() : UprightWidth() );
}

U32
AndroidAssetBitmap::Height() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceHeight() : UprightHeight() );
}

U32
AndroidAssetBitmap::UprightWidth() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceWidth() : SourceHeight();
}

U32
AndroidAssetBitmap::UprightHeight() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceHeight() : SourceWidth();
}

bool
AndroidAssetBitmap::IsProperty( PropertyMask mask ) const
{
	return IsPropertyInternal( mask );
}

void
AndroidAssetBitmap::SetProperty( PropertyMask mask, bool newValue )
{
	if ( ! Super::IsPropertyReadOnly( mask ) )
	{
		const U8 p = fProperties;
		const U8 propertyMask = (U8)mask;
		fProperties = ( newValue ? p | propertyMask : p & ~propertyMask );
	}

	switch ( mask )
	{
		case kIsBitsFullResolution:
			break;
		case kIsBitsAutoRotated:
			break;
		default:
			break;
	}
}

// ----------------------------------------------------------------------------

AndroidMaskAssetBitmap::AndroidMaskAssetBitmap( Rtt_Allocator& context, const char *filePath, NativeToJavaBridge *ntjb )
:	Super( context, filePath, ntjb )
{
	ImageDecoder().SetPixelFormatToGrayscale();
}

PlatformBitmap::Format 
AndroidMaskAssetBitmap::GetFormat() const
{
	return PlatformBitmap::kMask;
}

// ----------------------------------------------------------------------------

AndroidTextBitmap::AndroidTextBitmap( Rtt_Allocator & context, NativeToJavaBridge *ntjb, const char str[], const Rtt::PlatformFont& inFont, int width, int height, const char alignment[], Real &baselineOffset )
:	Super( context ),
	fText( & context, str ),
	fFont( context, inFont.Name(), inFont.Size(), ((Rtt::AndroidFont&)inFont).IsBold() ),
	fWrapWidth( width ),
	fClipWidth( width ),
	fClipHeight( height ),
	fAlignment(& context, alignment), 
	fNativeToJavaBridge(ntjb)
{
	// Calculate a clipping width and height so that the bitmap will not exceed OpenGL's max texture size.
	int maxTextureSize = (int)Rtt::Display::GetMaxTextureSize();
	fClipWidth = maxTextureSize;
	if (fClipHeight > maxTextureSize) {
		fClipHeight = maxTextureSize;
	}
	// Render the given text to this object's bitmap.
	GetBits(&context);
	baselineOffset = fBaselineOffset;
}

AndroidTextBitmap::~AndroidTextBitmap()
{
}

const void * 
AndroidTextBitmap::GetBits( Rtt_Allocator * context ) const
{
	const void * bits = Super::GetBits( context );
	if (NULL == bits)
	{
		fNativeToJavaBridge->RenderText(
				fText.GetString(), fFont.Name(), fFont.Size(), fFont.IsBold(),
				fWrapWidth, fClipWidth, fClipHeight, fAlignment.GetString(), fImageData, fBaselineOffset );
		bits = Super::GetBits( context );
	}
	return bits;
}

PlatformBitmap::Format 
AndroidTextBitmap::GetFormat() const
{
	return PlatformBitmap::kMask;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_PlatformBitmapTexture.h"

#include "Display/Rtt_PlatformBitmap.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformBitmapTexture::PlatformBitmapTexture( Rtt_Allocator *allocator, PlatformBitmap& bitmap )
:	Super( allocator ),
	fBitmap( bitmap )
{
}

void 
PlatformBitmapTexture::Allocate()
{
}

void 
PlatformBitmapTexture::Deallocate()
{
}

U32
PlatformBitmapTexture::GetWidth() const
{
	return fBitmap.Width();
}

U32 
PlatformBitmapTexture::GetHeight() const
{
	return fBitmap.Height();
}

Texture::Format
PlatformBitmapTexture::ConvertFormat( PlatformBitmap::Format format )
{
	Texture::Format result = Texture::kRGBA;

	switch ( format )
	{
		case PlatformBitmap::kRGB:
			result = Texture::kRGB;
			break;
		case PlatformBitmap::kRGBA:
			result = Texture::kRGBA;
			break;
		case PlatformBitmap::kARGB:
			result = Texture::kARGB;
			break;
		case PlatformBitmap::kBGRA:
			result = Texture::kBGRA;
			break;
#ifdef Rtt_OPENGLES
		case PlatformBitmap::kABGR:
			result = Texture::kRGBA; // ???
			break;
#endif
		case PlatformBitmap::kMask:
			// NOTE: We use kLuminance instead of kAlpha, b/c we want repeated
			// the gray value across all 4 channels. Compare:
			// * Luminance: (a,a,a,a)
			// * Alpha:     (0,0,0,a)
#if defined(Rtt_NINTENDO_ENV) || defined(Rtt_LINUX_ENV)
			// NN SDK does not support Luminance & Alpha.. weird
			result = Texture::kRGBA;
#else
			result = Texture::kLuminance;
#endif
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

Texture::Format
PlatformBitmapTexture::GetFormat() const
{
	return ConvertFormat( fBitmap.GetFormat() );
}

Texture::Filter
PlatformBitmapTexture::GetFilter() const
{
	return RenderTypes::Convert( fBitmap.GetMagFilter() );
}

Texture::Wrap
PlatformBitmapTexture::GetWrapX() const
{
	return RenderTypes::Convert( fBitmap.GetWrapX() );
}

Texture::Wrap
PlatformBitmapTexture::GetWrapY() const
{
	return RenderTypes::Convert( fBitmap.GetWrapY() );
}

size_t 
PlatformBitmapTexture::GetSizeInBytes() const
{
	return fBitmap.NumBytes();
}

U8
PlatformBitmapTexture::GetByteAlignment() const
{
	return fBitmap.GetByteAlignment();
}

const U8*
PlatformBitmapTexture::GetData() const
{
	return (const U8 *)fBitmap.GetBits( GetAllocator() );
}

void
PlatformBitmapTexture::ReleaseData()
{
	fBitmap.FreeBits();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

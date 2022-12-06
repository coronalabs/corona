//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_PlatformBitmap.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformBitmap::PlatformBitmap()
:	fMagFilter( RenderTypes::kLinearTextureFilter ),
	fMinFilter( RenderTypes::kLinearTextureFilter ),
	fWrapX( RenderTypes::kClampToEdgeWrap ),
	fWrapY( RenderTypes::kClampToEdgeWrap ),
	fScaleX( Rtt_REAL_0 ),
	fScaleY( Rtt_REAL_0 )
{
}

PlatformBitmap::~PlatformBitmap()
{
}

bool
PlatformBitmap::HitTest( Rtt_Allocator *context, int i, int j, U8 threshold ) const
{
	bool result = false;

	if ( i >= 0 && j >= 0 && i < (int)Width() && j < (int)Height() )
	{
		PlatformBitAccess lockBits( * const_cast< PlatformBitmap * >( this ) );

		const void *data = GetBits( context );

		if ( Rtt_VERIFY( data ) )
		{
			Format format = GetFormat();
			size_t bytesPerPixel = PlatformBitmap::BytesPerPixel( format );

			int index = (int) bytesPerPixel * ( i + j*Width() );

			switch ( format )
			{
				case kMask:
					{
						const U8 *pixels = ((const U8*)data);
						U8 p = pixels[index];
						result = ( p > threshold );
					}
					break;
				
				default:
					// TODO: Use GetColorByteIndexesFor()
					Rtt_ASSERT_NOT_IMPLEMENTED();
					break;
			}
		}

		// If we're hit testing, it's likely we'll do this again, 
		// so don't bother freeing the bits, i.e. comment out the call to FreeBits()
		// FreeBits();
	}

	return result;
}

U32
PlatformBitmap::UprightWidth() const
{
	Rtt_ASSERT( 0 == DegreesToUpright() );
	return Width();
}

U32
PlatformBitmap::UprightHeight() const
{
	Rtt_ASSERT( 0 == DegreesToUpright() );
	return Height();
}

bool
PlatformBitmap::WasScaled() const
{
	Real epsilon = Rtt_FloatToReal(0.000001f);
	Real scale = GetScale();
	return ((scale < (Rtt_REAL_1 - epsilon)) || (scale > (Rtt_REAL_1 + epsilon)));
}

Real
PlatformBitmap::GetScale() const
{
	return Rtt_REAL_1;
}

bool
PlatformBitmap::IsProperty( PropertyMask mask ) const
{
	return false;
}

void
PlatformBitmap::SetProperty( PropertyMask mask, bool newValue )
{
	Rtt_ASSERT_NOT_REACHED();
}

PlatformBitmap::Orientation
PlatformBitmap::GetOrientation() const
{
	return kUp;
}

U8
PlatformBitmap::GetByteAlignment() const
{
	return 4;
}

bool
PlatformBitmap::HasAlphaChannel() const
{
	bool result = false;

	Format format = GetFormat();

	switch ( format )
	{
		case kRGBA:
		case kBGRA:
		case kABGR:
		case kARGB:
			result = true;
			break;
		default:
			break;
	}

	return result;
}

size_t
PlatformBitmap::NumBytes() const
{
	size_t bytesPerPixel = PlatformBitmap::BytesPerPixel( GetFormat() );
	size_t numPixels = Width() * Height();

	return bytesPerPixel * numPixels;
}

size_t
PlatformBitmap::NumTextureBytes( bool roundToNextPow2 ) const
{
	size_t bytesPerPixel = PlatformBitmap::BytesPerPixel( GetFormat() );
	U32 w = Width();
	U32 h = Height();
	if ( roundToNextPow2 )
	{
		w = NextPowerOf2( w );
		h = NextPowerOf2( h );
	}
	size_t numPixels = w * h;

	return bytesPerPixel * numPixels;
}

S32
PlatformBitmap::DegreesToUprightBits() const
{
	return ( ! IsProperty( kIsBitsAutoRotated ) ? DegreesToUpright() : 0 );
}

bool
PlatformBitmap::IsLandscape() const
{
	return UprightWidth() > UprightHeight();
}

size_t
PlatformBitmap::BytesPerPixel( Format format )
{
	switch( format )
	{
		case kMask:
			return 1;
		case kLUMINANCE_ALPHA:
			return 2;
		case kRGB:
			return 3;
		case kRGBA:
		case kBGRA:
		case kARGB:
		case kABGR:
			return 4;
		default:
			break;
	}

	Rtt_ASSERT_NOT_REACHED();
	return 0;
}

S32
PlatformBitmap::CalculateRotation( Orientation start, Orientation end )
{
	S32 result = 0;

	if ( start != end )
	{
		S8 delta = (S8)end - (S8)start;
		bool isNegative = delta < 0;
		U8 magnitude = ( isNegative ? -delta : delta );
		switch( magnitude )
		{
			case 1:
			case 2:
				result = magnitude * 90;
				if ( isNegative ) { result = -result; }
				break;
			case 3:
				result = ( isNegative ? 90 : -90 );
				break;
			default:
				break;
		}
	}

	return result;
}

// For external bitmap to fetch the correct bytes order.
bool
PlatformBitmap::GetColorByteIndexesExternal(int *alphaIndex, int *redIndex, int *greenIndex, int *blueIndex) const
{
	return false;
}

// Retrieves the color byte indexes for a single pixel for the given image format.
// This function takes endianness into account.
// Index arguments will be replaced with corresponding byte indexes.
// Index arguments will be assigned -1 with the image format does not provide that color channel.
// It is okay to pass in NULL for the index arguments if you're not interested in retrieving those indexes.
// Returns true if at least one argument has been assigned an index.
// Returns false if no arguments were assigned or if given a format that
//   that does not provide colors such as kMask (ie: grayscale) or kUndefined.
bool
PlatformBitmap::GetColorByteIndexesFor(
	Format format, int *alphaIndex, int *redIndex, int *greenIndex, int *blueIndex)
{
	bool hasSucceeded = false;

	// Initialize color indexes to invalid values.
	int alphaIndexOut = -1;
	int redIndexOut = -1;
	int greenIndexOut = -1;
	int blueIndexOut = -1;

	// Fetch the color byte index by format and endianness.
	switch (format)
	{
		case PlatformBitmap::kRGB:
			#ifdef Rtt_LITTLE_ENDIAN
				blueIndexOut = 0;
				greenIndexOut = 1;
				redIndexOut = 2;
			#else
				blueIndexOut = 2;
				greenIndexOut = 1;
				redIndexOut = 0;
			#endif
			break;
		case PlatformBitmap::kRGBA:
			#ifdef Rtt_LITTLE_ENDIAN
				alphaIndexOut = 0;
				blueIndexOut = 1;
				greenIndexOut = 2;
				redIndexOut = 3;
			#else
				alphaIndexOut = 3;
				blueIndexOut = 2;
				greenIndexOut = 1;
				redIndexOut = 0;
			#endif
			break;
		case PlatformBitmap::kBGRA:
			#ifdef Rtt_LITTLE_ENDIAN
				alphaIndexOut = 0;
				redIndexOut = 1;
				greenIndexOut = 2;
				blueIndexOut = 3;
			#else
				alphaIndexOut = 3;
				redIndexOut = 2;
				greenIndexOut = 1;
				blueIndexOut = 0;
			#endif
			break;
		case PlatformBitmap::kABGR:
			#ifdef Rtt_LITTLE_ENDIAN
				redIndexOut = 0;
				greenIndexOut = 1;
				blueIndexOut = 2;
				alphaIndexOut = 3;
			#else
				redIndexOut = 3;
				greenIndexOut = 2;
				blueIndexOut = 1;
				alphaIndexOut = 0;
			#endif
			break;
		case PlatformBitmap::kARGB:
			#ifdef Rtt_LITTLE_ENDIAN
				blueIndexOut = 0;
				greenIndexOut = 1;
				redIndexOut = 2;
				alphaIndexOut = 3;
			#else
				blueIndexOut = 3;
				greenIndexOut = 2;
				redIndexOut = 1;
				alphaIndexOut = 0;
			#endif
			break;
		default:
			// New formats need to have a case above
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	// Copy indexes to arguments, provided that they have valid pointers. (NULL pointers are okay.)
	if (alphaIndex != NULL)
	{
		*alphaIndex = alphaIndexOut;
		hasSucceeded = true;
	}
	if (redIndex != NULL)
	{
		*redIndex = redIndexOut;
		hasSucceeded = true;
	}
	if (greenIndex != NULL)
	{
		*greenIndex = greenIndexOut;
		hasSucceeded = true;
	}
	if (blueIndex != NULL)
	{
		*blueIndex = blueIndexOut;
		hasSucceeded = true;
	}

	// Return true if at least 1 argument was been assigned a color byte index.
	return hasSucceeded;
}

void
PlatformBitmap::Lock()
{
}

void
PlatformBitmap::Unlock()
{
}

#ifdef Rtt_ANDROID_ENV

void 
PlatformBitmap::SwapRGB()
{
	if ( GetFormat() != kRGBA )
		return;

	char * pixels = const_cast<char *>( (const char *) GetBits( NULL ) );
	
	SwapBitmapRGB( pixels, Width(), Height() );
}


void
PlatformBitmap::SwapBitmapRGB( char * pixels, int w, int h )
{
	// ??? this does byteswap but shouldn't this be handled in the format at submit time?
	if ( pixels != NULL ) 
	{
		U32 pw = w * 4;

		for ( U32 y = 0; y < h; y++ ) {
			for ( U32 x = 0; x < pw; x += 4 ) {
				char c0 = pixels[x];
				pixels[x] = pixels[x + 2];
				pixels[x + 2] = c0;
			}
			pixels += pw;
		}
	}
}
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


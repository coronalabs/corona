//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_Texture.h"

#include "Core/Rtt_Assert.h"
#include "Rtt_RenderTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

 Texture::Format::Format( FormatValue value )
:	fValue( value )
{
}

// ----------------------------------------------------------------------------

Texture::FormatValue
Texture::Format::GetValue() const
{
	return (FormatValue)GetStockFormatAndFlag( fValue );
}

bool
Texture::Format::IsNonCore() const
{
	return HasFormatFlag( fValue );
}

#define PACK_ASTC( WIDTH, HEIGHT ) ( ( WIDTH << 4 ) | ( HEIGHT ) )

static const U16 kASTCDims[] = {
	PACK_ASTC( 4, 4 ),
	PACK_ASTC( 5, 4 ),
	PACK_ASTC( 5, 5 ),
	PACK_ASTC( 6, 5 ),
	PACK_ASTC( 6, 6 ),
	PACK_ASTC( 8, 5 ),
	PACK_ASTC( 8, 6 ),
	PACK_ASTC( 8, 8 ),
	PACK_ASTC( 10, 5 ),
	PACK_ASTC( 10, 6 ),
	PACK_ASTC( 10, 8 ),
	PACK_ASTC( 10, 10 ),
	PACK_ASTC( 12, 10 ),
	PACK_ASTC( 12, 12 )
};

int
Texture::Format::BlockDimsID( U8 width, U8 height )
{
	Rtt_ASSERT( width < 16 );
	Rtt_ASSERT( height < 16 );

	U8 packed = PACK_ASTC( width, height );
	for ( int i = 0; i < sizeof( kASTCDims ) / sizeof( *kASTCDims ); i++ )
	{
		if ( kASTCDims[i] == packed )
		{
			return i;
		}
	}

	Rtt_ASSERT_NOT_REACHED();

	return -1;
}

void
Texture::Format::GetBlockDims( int blockDimsID, U8& width, U8& height )
{
	Rtt_ASSERT( blockDimsID >= 0 );
	Rtt_ASSERT( blockDimsID < sizeof( kASTCDims ) / sizeof( *kASTCDims ) );

	U8 packed = kASTCDims[blockDimsID];

	width = packed >> 4;
	height = packed & 0xF;
}

#undef PACK_ASTC

template<int N> inline int
RoundUp( U8 dim )
{
	return ( dim + N - 1 ) / N;
}

static int
RoundUpToMultiple( U16 dim, U8 size )
{
	switch (size)
	{
		case 4: // most cases
			return RoundUp<4>( dim );
		case 8: // ASTC...
			return RoundUp<8>( dim );
		case 3: // ...and ditto the rest, albeit not powers of 2
			return RoundUp<3>( dim );
		case 5:
			return RoundUp<5>( dim );
		case 6:
			return RoundUp<6>( dim );
		case 10:
			return RoundUp<10>( dim );
		case 12:
			return RoundUp<12>( dim );
		default:
			Rtt_ASSERT_NOT_REACHED();
		
			return 0;
	}
}

int
Texture::Format::GetCompressedSize( U16 w, U16 h, U8 blockWidth, U8 blockHeight, U8 blockSize )
{
	int blocksW = RoundUpToMultiple( w, blockWidth );
	int blocksH = RoundUpToMultiple( h, blockHeight );

	return blocksW * blocksH * blockSize;
}

// ----------------------------------------------------------------------------

Texture::Texture( Rtt_Allocator* allocator )
:	Super( allocator ),
	fIsRetina( false ),
	fIsTarget( false )
{
}

Texture::~Texture()
{
}

CPUResource::ResourceType 
Texture::GetType() const
{
	return CPUResource::kTexture;
}

void 
Texture::Allocate()
{
}

void 
Texture::Deallocate()
{
}

Texture::Wrap
Texture::GetWrapX() const
{
	return kClampToEdge;
}

Texture::Wrap
Texture::GetWrapY() const
{
	return kClampToEdge;
}

size_t 
Texture::GetSizeInBytes() const
{
	Format format = GetFormat();
	U32 w = GetWidth();
	U32 h = GetHeight();

	switch( format.GetValue() )
	{
		case kLuminance:	return w * h * 1;
		case kRGB:			return w * h * 3;
		case kRGBA:			return w * h * 4;
		case kBGRA:			return w * h * 4;
		case kABGR:			return w * h * 4;
		case kARGB:			return w * h * 4;
		default:
			if ( format.IsNonCore() )
			{
				FormatDetails details = FormatDetails::UnpackFromValue( format.GetBackingValue() );
				if ( 0 == details.fBlockSize ) // not compressed?
				{
					return w * h * details.fBytesPerComponent * details.fNumComponents;
				}
				else
				{
					return Format::GetCompressedSize( w, h, details.fBlockWidth, details.fBlockHeight, details.fBlockSize );
				}
			}
			else
			{
				return 0;
			}
	}
}

U8
Texture::GetByteAlignment() const
{
	return 4;
}

const U8*
Texture::GetData() const
{
	return NULL;
}

void
Texture::ReleaseData()
{
}

void
Texture::SetFilter( Filter newValue )
{
	// Must implement in derived class
	Rtt_ASSERT_NOT_REACHED();
}

void
Texture::SetWrapX( Wrap newValue )
{
	// Must implement in derived class
	Rtt_ASSERT_NOT_REACHED();
}

void
Texture::SetWrapY( Wrap newValue )
{
	// Must implement in derived class
	Rtt_ASSERT_NOT_REACHED();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

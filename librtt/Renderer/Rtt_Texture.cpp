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

#include "Display/Rtt_CompositePaint.h"

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
	if ( !IsNonCore() )
	{
		return (FormatValue)FormatDetails::GetStockFormat( fValue );
	}
	else
	{
		return kNumFormats; // arbitrary invalid value
	}
}

bool
Texture::Format::IsNonCore() const
{
	return !FormatDetails::IsCore( fValue );
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
RoundUp( U16 dim )
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

TextureFormatDescription
TextureFormatDescription::MakeStandard()
{
	TextureFormatDescription desc = {};
	
	Rtt_ASSERT( !desc.IsCompressed() );
	Rtt_ASSERT( !desc.IsWordPacked() );
//	Rtt_ASSERT( !desc.IsDepthStencil() );
	
	return desc;

}

TextureFormatDescription
TextureFormatDescription::MakeCompressed()
{
	TextureFormatDescription desc = {};
	
	// This is a nonsense provisional value, used to do some registration.
	desc.fBlockSize = 0xFF;
	
	Rtt_ASSERT( desc.IsCompressed() );
	Rtt_ASSERT( !desc.IsWordPacked() );
	Rtt_ASSERT( !desc.IsWordPacked() );
	
	return desc;
}

TextureFormatDescription
TextureFormatDescription::MakeWordPacked()
{
	TextureFormatDescription desc = {};

	desc.fFlags |= kIsWordPacked;
	
	Rtt_ASSERT( !desc.IsCompressed() );
//	Rtt_ASSERT( !desc.IsDepthStencil() );
	
	return desc;
}

TextureFormatDescription
TextureFormatDescription::MakeDepthStencil()
{
	TextureFormatDescription desc = {};
	
	// N.B. this flag is provisional, used to do some registration. The format in question
	// might even be strictly stencil (and then we might have the other extreme).
	desc.fFlags |= kIsDepthRelated;
	
	Rtt_ASSERT( !desc.IsCompressed() );
	Rtt_ASSERT( !desc.IsWordPacked() );
	
	return desc;
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
				return FormatDetails::GetSize( w, h, format.GetBackingValue() );
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
	
Texture*
TextureList::GetFill0() const
{
	if ( !IsArray() )
	{
		return fFill0;
	}
	else
	{
		U32 n = GetCount();
		return ( n >= 1 ) ? GetArray()[0] : NULL;
	}
}

Texture*
TextureList::GetFill1() const
{
	if ( !IsArray() )
	{
		return fFill1;
	}
	else
	{
		U32 n = GetCount();
		return ( n >= 2 ) ? GetArray()[1] : NULL;
	}
}

void
TextureList::PointToArray( Texture** texArray, U32 count )
{
	Rtt_STATIC_ASSERT( sizeof(texArray) <= sizeof(fFill0) );
	Rtt_STATIC_ASSERT( sizeof(uintptr_t) <= sizeof(fFill1) );

	fFill0 = (Texture*)texArray;
	fFill1 = (Texture*)(uintptr_t)( 0x1 | ( count << 1 ) ); // ensure low bit set
}

bool
TextureList::IsArray() const
{
	uintptr_t asUint = (uintptr_t)fFill1;
	return ( 0 != ( asUint & 0x1 ) );
}

// ^^^ TODO? we could add this same low bit trick to mark fill textures
// and dispense with much of the weirdness

Texture**
TextureList::GetPositionAfterFills() const
{
	return IsArray() ? GetArray() + 2 : NULL; // TODO relax + 2 :D
}
	
U32
TextureList::GetCountAfterFills() const
{
	return IsArray() ? GetCount() - 2 : 0; // TODO relax + 2 :D
}

const U8*
TextureList::GetNamesList() const
{
	return IsArray() ? CompositePaint::GetNameListGivenTextureList( GetArray(), GetCountAfterFills() ) : NULL;
}

U32
TextureList::GetCount() const
{
	Rtt_ASSERT( IsArray() );

	uintptr_t asUint = (uintptr_t)fFill1;
	return (U32)( asUint >> 1 );
}

Texture**
TextureList::GetArray() const
{
	Rtt_ASSERT( IsArray() );
	
	return (Texture**)fFill0;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

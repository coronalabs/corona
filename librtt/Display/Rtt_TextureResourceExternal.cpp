//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Display/Rtt_TextureResourceExternal.h"
#include "Rtt_TextureResourceExternalAdapter.h"
#include "Rtt_PlatformBitmapTexture.h"
#include "Rtt_Display.h"
#include "Rtt_TextureFactory.h"
#include "CoronaLua.h"
#include "CoronaGraphics.h"
#include "Rtt_DisplayDefaults.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

#pragma mark ==External Bitmap==

	
class ExternalBitmap : public PlatformBitmap
{
public:
	ExternalBitmap(const CoronaExternalTextureCallbacks* sourceCallbacks, void* context)
	: fSrc(*sourceCallbacks)
	, fContext(context)
	, fCustomFormat(0)
	, fCustomTarget(0)
	{
		
	}
	
	void Finalize()
	{
		if ( fSrc.onFinalize )
		{
			fSrc.onFinalize(GetUserData());
		}
		memset(&fSrc, 0, sizeof(fSrc));
	}
	
	~ExternalBitmap()
	{
		Finalize();
	}
	
	virtual const void* GetBits( Rtt_Allocator* context ) const override
	{
		const void *bits = NULL;
		if ( Rtt_VERIFY(fSrc.onRequestBitmap) )
		{
			bits = fSrc.onRequestBitmap(GetUserData());
			if (bits == NULL)
			{
				Rtt_LogException("ERROR: TextureResourceExternal - received null pointer for bitmap data");
			}
		}
		return bits;
	}
	
	virtual void FreeBits() const override
	{
		if( fSrc.onReleaseBitmap)
		{
			return fSrc.onReleaseBitmap(GetUserData());
		}
	}
	
	virtual U32 Width() const override
	{
		if(fSrc.getWidth)
		{
			return fSrc.getWidth(GetUserData());
		}
		return 0;
	}
	
	virtual U32 Height() const override
	{
		if(fSrc.getHeight)
		{
			return fSrc.getHeight(GetUserData());
		}
		return 0;
	}
	
	Format AuxGetFormat() const
	{
		CoronaExternalBitmapFormat fmt = kExternalBitmapFormat_Undefined;
		if( fSrc.getFormat )
		{
			fmt = fSrc.getFormat(GetUserData());
		}
		switch(fmt)
		{
			case kExternalBitmapFormat_Mask:
				return PlatformBitmap::kMask;
			case kExternalBitmapFormat_RGB:
				return PlatformBitmap::kRGB;
			case kExternalBitmapFormat_RGBA:
				return PlatformBitmap::kRGBA;
			case kExternalBitmapFormat_Undefined:
				return PlatformBitmap::kRGBA;
		}
		return PlatformBitmap::kRGBA;
	}
	
	virtual Format GetFormat() const override
	{
		Format format;
		
		if ( !fCustomFormat )
		{
			format = AuxGetFormat();
		}
		else
		{
			format.SetBackingValue( fCustomFormat );
		}
		
		if ( fCustomTarget )
		{
			format.SetBackingValue( format.GetBackingValue() | fCustomTarget );
		}
		
		return format;
	}
	
	int GetField(lua_State* L, const char* field)
	{
		if ( fSrc.onGetField )
		{
			return fSrc.onGetField(L, field, GetUserData());
		}
		return 0;
	}
	
	inline void* GetUserData() const
	{
		return fContext;
	}
	
	void SetCustomFormat( U32 format ) { fCustomFormat = format; }
	void SetCustomTarget( U32 target ) { fCustomTarget = target; }
	
private:
	CoronaExternalTextureCallbacks fSrc;
	void* fContext;

	// Current extension details.
	U32 fCustomFormat;
	U32 fCustomTarget;
};

	
#pragma mark == Texture Resource External ==

static void
ProcessFormat( TextureFactory& factory, const CoronaExternalTextureExtension_CustomFormat* format_ext, ExternalBitmap* bitmap, bool isMask )
{
	U32 index = format_ext->formatIndex;
	if ( index > 0 && index <= factory.GetCurrentFormatCount() )
	{
		const TextureFormatDescription& desc = factory.GetCurrentFormatList()[index - 1];
		U32 value = FormatDetails::BuildFromDescription( &desc, index );

		if ( !desc.IsWordPacked() && ( 1 == desc.fNumComponents ) && isMask )
		{
			FormatDetails::AddMaskBit( value );
		}

		bitmap->SetCustomFormat( value );
		// TODO: further need to validate? value != ~0
	}
}

static void
ProcessTarget( const CoronaExternalTextureExtension_TextureTarget* target_ext, ExternalBitmap* bitmap )
{
	int family = -1;
	switch ( target_ext->family )
	{
	case kTextureFamily_Float:
		family = Texture::kFloatingPoint;
		break;
	case kTextureFamily_Uint:
		family = Texture::kUnsignedInteger;
		break;
	case kTextureFamily_Sint:
		family = Texture::kSignedInteger;
		break;
	case kTextureFamily_Other:
		family = Texture::kOtherFamily;
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
	}
	
	int target = -1;
	switch ( target_ext->shape ) // TODO: assumed to validate first... might want these as separate extensions to add methods...
	{
	case kTextureShape_1D:
		target = Texture::k1D;
		break;
	case kTextureShape_2D:
		target = Texture::k2D;
		break;
	case kTextureShape_3D:
		target = Texture::k3D;
		break;
	case kTextureShape_Cube:
		target = Texture::kCube;
		break;
	case kTextureShape_Rectangle:
		target = Texture::kRectangle;
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
	}
	
	U32 value = FormatDetails::GatherFamilyInfo( family, target, target_ext->isArray );
	bitmap->SetCustomTarget( value );
}

static void
ProcessExtensions( TextureFactory& factory, const CoronaExternalTextureCallbacks2* callbacks2, ExternalBitmap* bitmap )
{
	for ( CoronaExternalTextureExtensionBase* ext = callbacks2->firstExtension; NULL != ext; ext = ext->next )
	{
		switch ( ext->type )
		{
		case kExternalTextureExtension_TextureTarget:
			ProcessTarget( (CoronaExternalTextureExtension_TextureTarget*)ext, bitmap );
			break;
		case kExternalTextureExtension_CustomFormat: {
			bool isMask = ( NULL != callbacks2->base.getFormat ) && kExternalBitmapFormat_Mask == callbacks2->base.getFormat( bitmap->GetUserData() );
			ProcessFormat( factory, (CoronaExternalTextureExtension_CustomFormat*)ext, bitmap, isMask );
		}	break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}
	}
}

TextureResourceExternal *
TextureResourceExternal::Create(TextureFactory& factory,
									const CoronaExternalTextureCallbacks *callbacks,
									void *callbacksContext,
									bool isRetina )
{	
	Display& display = factory.GetDisplay();
	
	ExternalBitmap *bitmap = Rtt_NEW(display.GetAllocator(),
									ExternalBitmap(callbacks, callbacksContext));
	
	if ( sizeof(CoronaExternalTextureCallbacks2) == callbacks->size )
	{
		ProcessExtensions( factory, (CoronaExternalTextureCallbacks2*)callbacks, bitmap );
	}
	
	bitmap->SetMagFilter( display.GetDefaults().GetMagTextureFilter() );
	bitmap->SetMinFilter( display.GetDefaults().GetMinTextureFilter() );
	bitmap->SetWrapX( display.GetDefaults().GetTextureWrapX() );
	bitmap->SetWrapY( display.GetDefaults().GetTextureWrapY() );
	
	Texture *texture = Rtt_NEW( display.GetAllocator(),
									PlatformBitmapTexture( display.GetAllocator(), *bitmap ) );
	
	TextureResourceExternal *result = Rtt_NEW( display.GetAllocator(),
									TextureResourceExternal( factory, texture, bitmap ) );
	
	texture->SetRetina( isRetina );
	
	return result;
}


TextureResourceExternal::TextureResourceExternal(
											 TextureFactory &factory,
											 Texture *texture,
											 PlatformBitmap *bitmap )
					   : TextureResource(factory, texture, bitmap, kTextureResourceExternal)
{

}

TextureResourceExternal::~TextureResourceExternal()
{
	GetTextureFactory().RemoveFromTeardownList(GetCacheKey());
}

const MLuaUserdataAdapter&
TextureResourceExternal::GetAdapter() const
{
	return TextureResourceExternalAdapter::Constant();
}

int TextureResourceExternal::GetField(lua_State *L, const char *field) const
{
	return ((ExternalBitmap*)GetBitmap())->GetField( L, field);
}

void* TextureResourceExternal::GetUserData() const
{
	return ((ExternalBitmap*)GetBitmap())->GetUserData();
}

	
void TextureResourceExternal::Teardown()
{
	((ExternalBitmap*)GetBitmap())->Finalize();
}
	
	
} // namespace Rtt





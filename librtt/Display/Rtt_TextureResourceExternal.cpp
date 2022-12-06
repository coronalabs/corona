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
	
	virtual Format GetFormat() const override
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

	// Override this method to return correct bytes order when format is PlatformBitmap::kRGBA
	// Temporary fixed for alpha channel index in GraphicsLibrary::newOutline()
	virtual bool GetColorByteIndexesExternal(int *alphaIndex, int *redIndex, int *greenIndex, int *blueIndex) const override
	{
		bool hasSucceeded = false;
		if (GetFormat() == PlatformBitmap::kRGBA)
		{
			if (alphaIndex != NULL)
			{
				*alphaIndex = 3;
				hasSucceeded = true;
			}
			if (blueIndex != NULL)
			{
				*blueIndex = 2;
				hasSucceeded = true;
			}
			if (greenIndex != NULL)
			{
				*greenIndex = 1;
				hasSucceeded = true;
			}
			if (redIndex != NULL)
			{
				*redIndex = 0;
				hasSucceeded = true;
			}
		}
		return hasSucceeded;
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
	
private:
	CoronaExternalTextureCallbacks fSrc;
	void* fContext;
};

	
#pragma mark == Texture Resource External ==

TextureResourceExternal *
TextureResourceExternal::Create(TextureFactory& factory,
									const CoronaExternalTextureCallbacks *callbacks,
									void *callbacksContext,
									bool isRetina )
{	
	Display& display = factory.GetDisplay();
	
	PlatformBitmap *bitmap = Rtt_NEW(display.GetAllocator(),
									ExternalBitmap(callbacks, callbacksContext));
	
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





//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TextureResource.h"
#include "Rtt_LuaUserdataProxy.h"
#include "Rtt_TextureFactory.h"
#include "Rtt_Display.h"
#include "Rtt_Scene.h"

// ----------------------------------------------------------------------------

namespace Rtt
{


TextureResource::TextureResource(TextureFactory &factory, Texture *texture,  PlatformBitmap *bitmap, TextureResourceType type)
	: fFactory(factory)
	, fTexture(texture)
	, fBitmap(bitmap)
	, fType(type)
	, fProxy(NULL)
{
	if (fTexture)
	{
		fFactory.DidAddTexture( (TextureResource&)* this );
	}
}
	
TextureResource::~TextureResource()
{
	if (fTexture)
	{
		fFactory.WillRemoveTexture( (TextureResource&)* this );
	}
	Rtt_DELETE( fBitmap );
	fFactory.QueueRelease( fTexture );

	if ( fProxy )
	{
		fFactory.GetDisplay().GetStage()->GetScene().QueueRelease( fProxy );
		fProxy->DetachUserdata(); // Notify proxy that object is invalid
	}	
}
	
// ----------------------------------------------------------------------------

PlatformBitmap::Format
TextureResource::ConvertFormat( Texture::Format format )
{
	PlatformBitmap::Format result = PlatformBitmap::kRGBA;

	switch ( format )
	{
		case Texture::kRGB:
			result = PlatformBitmap::kRGB;
			break;
		case Texture::kRGBA:
			result = PlatformBitmap::kRGBA;
			break;
		case Texture::kARGB:
			result = PlatformBitmap::kARGB;
			break;
		case Texture::kBGRA:
			result = PlatformBitmap::kBGRA;
			break;
#ifdef Rtt_OPENGLES
		case Texture::kABGR:
			result = PlatformBitmap::kRGBA; // ???
			break;
#endif
		case Texture::kLuminance:
		case Texture::kAlpha:
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

U32
TextureResource::GetWidth() const
{
	return ( GetBitmap() ? GetBitmap()->Width() : GetTexture().GetWidth() );
}

U32
TextureResource::GetHeight() const
{
	return ( GetBitmap() ? GetBitmap()->Width() : GetTexture().GetHeight() );
}

	
void
TextureResource::PushProxy( lua_State *L )
{
	if ( ! fProxy )
	{
		fProxy = LuaUserdataProxy::New( L, const_cast< Self * >( this ) );
		fProxy->SetAdapter( & GetAdapter() );
	}
	
	fProxy->Push( L );
}

void
TextureResource::DetachProxy( )
{
	fProxy = NULL;
}

void TextureResource::ReleaseSelfFromFactoryOwnership()
{
	if (fProxy)
	{
		fFactory.GetDisplay().GetStage()->GetScene().QueueRelease( fProxy );
		fProxy->DetachUserdata(); // Notify proxy that object is invalid
	}
	DetachProxy();
	
	fFactory.Release(GetCacheKey());
}
	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


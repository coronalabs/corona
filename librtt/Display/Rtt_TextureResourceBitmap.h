//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_TextureResourceBitmap__
#define __Rtt_TextureResourceBitmap__


#include "Display/Rtt_TextureResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformBitmap;
class TextureFactory;
class FrameBufferObject;

// ----------------------------------------------------------------------------

class TextureResourceBitmap : public TextureResource
{
	public:
		typedef TextureResourceBitmap Self;
	
	public:

		static Self *Create(
			TextureFactory& factory,
			int w, int h,
			Texture::Format format,
			Texture::Filter filter,
			Texture::Wrap wrap,
			bool save_to_file );

		static Self *Create(
			TextureFactory& factory,
			PlatformBitmap *bitmap,
			bool isRetina );
			
		static Self *CreateDefault(
			TextureFactory& factory,
			Texture::Format format,
			Texture::Filter filter);
			
		static Self *CreateVideo(
			TextureFactory& factory);
	
	protected:
		TextureResourceBitmap(
			TextureFactory &factory,
			Texture *texture );
		TextureResourceBitmap(
			TextureFactory &factory,
			Texture *texture,
			PlatformBitmap *bitmap );

	public:
		virtual ~TextureResourceBitmap();

		void Preload();
	
	
	//Lua interface
	virtual const MLuaUserdataAdapter& GetAdapter() const override;	
	
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_TextureResourceBitmap__

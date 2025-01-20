//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TextureBitmap_H__
#define _Rtt_TextureBitmap_H__

#include "Renderer/Rtt_Texture.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class TextureBitmap : public Texture
{
	public:
		typedef Texture Super;
		typedef TextureBitmap Self;

	public:
		TextureBitmap( Rtt_Allocator* allocator, U32 width, U32 height, Format format, Filter filter);
		virtual ~TextureBitmap();

		virtual void Allocate();
		virtual void Deallocate();

		virtual U32 GetWidth() const;
		virtual U32 GetHeight() const;
		virtual Format GetFormat() const;
		virtual Filter GetFilter() const;

		virtual const U8 *GetData() const;

		// To avoid excess copying, image data may be manipulated directly.
		// Invalidate() will result in the data being subloaded to the GPU.
		U8 *WriteAccess() { return fData; }

	private:
		U32 fWidth;
		U32 fHeight;
		Format fFormat;
		Filter fFilter;
		U8* fData;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TextureBitmap_H__

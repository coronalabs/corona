//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TextureVolatile_H__
#define _Rtt_TextureVolatile_H__

#include "Renderer/Rtt_Texture.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class TextureVolatile : public Texture
{
	public:
		typedef Texture Super;
		typedef TextureVolatile Self;

	public:
		TextureVolatile(
			Rtt_Allocator *allocator,
			U32 width, U32 height, Format format, Filter filter, Wrap wrapX, Wrap wrapY );

	public:
		virtual U32 GetWidth() const;
		virtual U32 GetHeight() const;
		virtual Format GetFormat() const;
		virtual Filter GetFilter() const;
		virtual Wrap GetWrapX() const;
		virtual Wrap GetWrapY() const;

		virtual void SetFilter( Filter newValue );
		virtual void SetWrapX( Wrap newValue );
		virtual void SetWrapY( Wrap newValue );

	private:
		U32 fWidth;
		U32 fHeight;
		Format fFormat;
		Filter fFilter;
		Wrap fWrapX;
		Wrap fWrapY;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TextureVolatile_H__

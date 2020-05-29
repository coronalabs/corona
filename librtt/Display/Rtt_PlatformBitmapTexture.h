//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformBitmapTexture_H__
#define _Rtt_PlatformBitmapTexture_H__

#include "Renderer/Rtt_Texture.h"

#include "Display/Rtt_PlatformBitmap.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformBitmap;

// ----------------------------------------------------------------------------

// Adapter for PlatformBitmap
class PlatformBitmapTexture : public Texture
{
	public:
		typedef Texture Super;

	public:
		static Texture::Format ConvertFormat( PlatformBitmap::Format format );

		PlatformBitmapTexture( Rtt_Allocator *allocator, PlatformBitmap& bitmap );

	public:
		virtual void Allocate();
		virtual void Deallocate();
		virtual U32 GetWidth() const;
		virtual U32 GetHeight() const;
		virtual Format GetFormat() const;
		virtual Filter GetFilter() const;
		virtual Wrap GetWrapX() const;
		virtual Wrap GetWrapY() const;
		virtual size_t GetSizeInBytes() const;
		virtual U8 GetByteAlignment() const;
		virtual const U8* GetData() const;
		virtual void ReleaseData();

	public:
		PlatformBitmap& GetBitmap() const { return fBitmap; }

	protected:
		PlatformBitmap& fBitmap;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformBitmapTexture_H__

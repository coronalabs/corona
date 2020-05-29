//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_BufferBitmap_H__
#define _Rtt_BufferBitmap_H__

#include "Core/Rtt_Types.h"

#include "Display/Rtt_PlatformBitmap.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Does TextureBitmap replace the need for this???
// 
class BufferBitmap : public PlatformBitmap
{
	public:
		typedef PlatformBitmap Super;

	public:
		BufferBitmap( Rtt_Allocator* allocator, size_t w, size_t h, Super::Format format, Orientation orientation = kUp );
		BufferBitmap( Rtt_Allocator* allocator, size_t w, size_t h, Super::Format format, Real angle );
		virtual ~BufferBitmap();

	protected:
		void ReleaseBits();

	public:
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual Super::Format GetFormat() const;
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		virtual Orientation GetOrientation() const;

	public:
		const void *ReadAccess() { return fData; }
		void* WriteAccess() { return fData; }
		void Flip( bool flipHorizontally, bool flipVertically );
		void UndoPremultipliedAlpha();

	private:
		mutable void* fData;
		U32 fWidth;
		U32 fHeight;
		U8 fProperties;
		U8 fFormat;
		S8 fOrientation;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_BufferBitmap_H__

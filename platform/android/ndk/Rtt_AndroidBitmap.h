//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidBitmap_H__
#define _Rtt_AndroidBitmap_H__

#include "librtt/Display/Rtt_PlatformBitmap.h"
#include "Core/Rtt_Array.h"
#include "Core/Rtt_String.h"
#include "Rtt_AndroidFont.h"
#include "AndroidImageData.h"
#include "AndroidImageDecoder.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{
class PlatformFont;

// ----------------------------------------------------------------------------

class AndroidBitmap : public PlatformBitmap
{
	public:
		AndroidBitmap( Rtt_Allocator & context );
		virtual ~AndroidBitmap();

		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual Real GetScale() const;
		virtual Format GetFormat() const;
		virtual PlatformBitmap::Orientation GetOrientation() const;
		
	protected:
		mutable AndroidImageData fImageData;
};

class AndroidAssetBitmap : public AndroidBitmap
{
	public:
		typedef AndroidBitmap Super;

		AndroidAssetBitmap( Rtt_Allocator& context, const char *filePath, NativeToJavaBridge *ntjb );
		virtual ~AndroidAssetBitmap();

		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;
		virtual const void * GetBits( Rtt_Allocator * context ) const;

	protected:
		AndroidImageDecoder& ImageDecoder();
		U32 SourceWidth() const;
		U32 SourceHeight() const;
		Rtt_INLINE bool IsPropertyInternal( PropertyMask mask ) const { return (fProperties & mask) ? true : false; }

	private:
		U8 fProperties;
		String fPath;
		mutable AndroidImageDecoder fImageDecoder;
};

class AndroidMaskAssetBitmap : public AndroidAssetBitmap
{
	public:
		typedef AndroidAssetBitmap Super;

		AndroidMaskAssetBitmap( Rtt_Allocator& context, const char *filePath, NativeToJavaBridge *ntjb );

		virtual Format GetFormat() const;
};

class AndroidTextBitmap : public AndroidBitmap
{
	public:
		typedef AndroidBitmap Super;

		AndroidTextBitmap( Rtt_Allocator & context, NativeToJavaBridge *ntjb, const char str[], const PlatformFont& font, int width, int height, const char alignment[], Real & baselineOffset );
		virtual ~AndroidTextBitmap();

		virtual const void * GetBits( Rtt_Allocator * context ) const;
		virtual PlatformBitmap::Format GetFormat() const;
		
	private:
		AndroidFont fFont;
		String fText;
		int fWrapWidth;
		int fClipWidth;
		int fClipHeight;
		mutable Real fBaselineOffset;
		String fAlignment;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidBitmap_H__

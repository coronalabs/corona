//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleBitmap_H__
#define _Rtt_AppleBitmap_H__

#include "Display/Rtt_PlatformBitmap.h"

#include "Rtt_AppleFont.h"

// ----------------------------------------------------------------------------

struct CGImage;
@class NSString;
#ifdef Rtt_IPHONE_ENV
	@class UIImage;

	// NSTextAlignment
	#import <UIKit/NSText.h>
#endif

namespace Rtt
{

class PlatformFont;

// ----------------------------------------------------------------------------

#define Rtt_TEST_BITMAP

class AppleBitmap : public PlatformBitmap
{
	public:
		typedef PlatformBitmap Super;
		typedef AppleBitmap Self;

#ifdef Rtt_TEST_BITMAP
	public:
		static void Test( class Runtime& runtime );
#endif

	protected:
		AppleBitmap();
		virtual ~AppleBitmap();

	public:
		virtual void FreeBits() const;

	protected:
		mutable void* fData;
};

class AppleFileBitmap : public AppleBitmap
{
	public:
		typedef AppleBitmap Super;

	public:
		AppleFileBitmap( const char* inPath, bool isMask = false );
		virtual ~AppleFileBitmap();

	protected:
		#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
			AppleFileBitmap( UIImage *image, bool isMask );
		#endif
		#ifdef Rtt_MAC_ENV
			AppleFileBitmap( NSImage *image, bool isMask );
		#endif

	
	protected:
		void Initialize();
		float CalculateScale() const;
		U32 SourceWidth() const;
		U32 SourceHeight() const;
	
	protected:
		Rtt_INLINE bool IsPropertyInternal( PropertyMask mask ) const { return (fProperties & mask) ? true : false; }

#ifdef Rtt_DEBUG
		void PrintChannel( const U8 *bytes, int channel, U32 bytesPerPixel ) const;
#endif
		CGRect TransformCGContext( CGContextRef context, size_t width, size_t height ) const;
		void* GetBitsGrayscale( Rtt_Allocator* context ) const;
		void* GetBitsColor( Rtt_Allocator* context ) const;

	public:
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual PlatformBitmap::Format GetFormat() const;
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;
		virtual bool WasScaled() const;
		virtual Real GetScale() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		virtual PlatformBitmap::Orientation GetOrientation() const;

	private:
		struct CGImage* fImage;
		float fScale;
		S8 fOrientation;
		U8 fProperties;
		U8 fIsMask;
//		S8 fAngle; // multiples of 90 degrees, range is [-1,2]
};

#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
// We use this in 2 situations:
// * Some API's only give access to UIImage, not CGImage.
// * For actual image files, we have to use this for iOS 3.x b/c it doesn't have ImageIO (which we weak link to)
class IPhoneFileBitmap : public AppleFileBitmap
{
	public:
		typedef AppleFileBitmap Super;

	public:
		IPhoneFileBitmap( UIImage *image, bool isMask = false );
		virtual ~IPhoneFileBitmap();

	private:
		UIImage *fUIImage;
};
#endif

#ifdef Rtt_MAC_ENV
class MacFileBitmap : public AppleFileBitmap
{
	public:
		typedef AppleFileBitmap Super;
		
	public:
		MacFileBitmap( NSImage *image, bool isMask = false );
		
	private:
//		NSImage *fNSImage;
	};
#endif
	
class AppleTextBitmap : public AppleBitmap
{

	public:
		typedef AppleBitmap Super;

	public:
        AppleTextBitmap( const char str[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset );
		virtual ~AppleTextBitmap();

	public:
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual PlatformBitmap::Format GetFormat() const;

	protected:
#ifdef Rtt_DEBUG
		void PrintChannel( const U8 *bytes ) const;
#endif

	private:
		NSString* fText;
		AppleFont* fFont;
		AppleSize fDimensions;
		bool fIsMultiline;
		NSTextAlignment fTextAlignment;
};


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleBitmap_H__

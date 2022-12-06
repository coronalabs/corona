//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformBitmap_H__
#define _Rtt_PlatformBitmap_H__

#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Real.h"
#include "Core/Rtt_Types.h"

#include "Renderer/Rtt_RenderTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformBitmap
{
	public:
		typedef enum Format
		{
			kUndefined = 0,
			kMask,
			kRGB,
			kRGBA, // Channels are (left to right) from MSB to LSB, so A is in the least-significant 8 bits
			kBGRA, // Channels are (left to right) from MSB to LSB, so A is in the least-significant 8 bits
			kABGR, // Channels are (left to right) from MSB to LSB, so A is in the most-significant 8 bits
			kARGB,
			kLUMINANCE_ALPHA,
			kNumFormats
		}
		Format;

	public:
		typedef enum _Orientation
		{
			kMinValue = 0,
			kRight = kMinValue,
			kUp,
			kLeft,
			kDown,

			kNumTypes,
			kMaxValue = kNumTypes - 1
		}
		Orientation;

	public:
		// Note: the default for these properties is *always* false,
		// since the default implementation of IsProperty() is to return false.
		typedef enum _PropertyMask
		{
			kIsPremultiplied = 0x1,

			// The properties kIsBitsFullResolution and kIsBitsAutoRotated were
			// added so that full-resolution images can be saved to the photo library
			// at the proper orientation.
			// 
			// When kIsBitsAutoRotated is true, then Bits() will return an "upright",
			// pre-rotated buffer.  In this case, BitsWidth/Height() and Width/Height()
			// should return the same number.
			kIsBitsFullResolution = 0x2,
			kIsBitsAutoRotated = 0x4,

			// Chooses the image among a family of image files (sharing the same prefix)
			// whose scale is <= the content scale
			kIsNearestAvailablePixelDensity = 0x8
		}
		PropertyMask;

		static Rtt_INLINE bool IsPropertyReadOnly( PropertyMask mask ) { return kIsPremultiplied == mask; }

	public:
		PlatformBitmap();
		virtual ~PlatformBitmap() = 0;

	public:
		// GetBits() returns an uncompressed buffer of the image pixel data.
		// There are two things to note about this pixel data:
		// 
		// (1) Resolution
		// By default, the pixel data is auto-scaled to screen resolution; this
		// is useful for preview purposes. When kIsBitsFullResolution is set, the
		// pixel data is the original resolution of the image; this is useful
		// for saving images to disk. 
		// 
		// (2) Orientation
		// By default, the pixel data is in the original orientation of the 
		// source buffer. Normally, images are stored in their upright position,
		// so this is exactly what you want. When the image comes from a camera,
		// there is EXIF metadata so the data may not be stored upright. Here,
		// DegreesToUpright() gives a multiple of 90 (up to +/-180) so you'll
		// know how much to rotate the image. During rendering (preview),
		// GPUStream hardware-accelerates this rotation.
		// 
		// When kIsBitsAutoRotated is set, the pixel data is automatically
		// transformed. This is useful for saving images back to disk since
		// often times, you cannot save EXIF metadata along with the image copy.
		virtual const void* GetBits( Rtt_Allocator* context ) const = 0;
		virtual void FreeBits() const = 0;

		// Returns true if the value (0-100%) of the pixel at row,col (i,j) is greater than threshold
		bool HitTest( Rtt_Allocator *context, int i, int j, U8 threshold = 0 ) const;

		// Returns width of buffer returned by Bits()
		virtual U32 Width() const = 0;

		// Returns height of buffer returned by Bits()
		virtual U32 Height() const = 0;
		virtual Format GetFormat() const = 0;

		// Returns width of image when upright
		virtual U32 UprightWidth() const;

		// Returns height of image when upright
		virtual U32 UprightHeight() const;

		// Indicates if the image was downscaled compared to the original image file/source.
		virtual bool WasScaled() const;

		// Gets the scaling factor applied to this image compared to the original image file/source.
		// For example, if the image was downsampled when loaded from file, then this scaler would be less than one.
		virtual Real GetScale() const;

		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );

		bool IsPremultiplied() const { return IsProperty( kIsPremultiplied ); }
		bool HasAlphaChannel() const;

		// The orientation of the source buffer relative to the upright position.
		// Essentially, the direction you'd rotate an image *before* writing out
		// the data to a buffer.
		virtual Orientation GetOrientation() const;

		virtual U8 GetByteAlignment() const;

		// For external bitmap to fetch the correct bytes order
		virtual bool GetColorByteIndexesExternal(int *alphaIndex, int *redIndex, int *greenIndex, int *blueIndex) const;

#ifdef Rtt_ANDROID_ENV
		void SwapRGB();
		static void SwapBitmapRGB( char * base, int w, int h );
#endif
		
	protected:
		Rtt_INLINE S32 DegreesToUpright() const { return CalculateRotation( GetOrientation(), kUp ); }

	public:
		size_t NumBytes() const;
		size_t NumTextureBytes( bool roundToNextPow2 ) const;

		// The angle to rotate the image buffer returned by Bits() to make it upright
		S32 DegreesToUprightBits() const;

		bool IsLandscape() const;

		// TODO: Remove as this is subsumed by GetFormat();
		Rtt_INLINE bool IsMask() const { return GetFormat() == kMask; }

	public:
		RenderTypes::TextureFilter GetMagFilter() const { return (RenderTypes::TextureFilter)fMagFilter; }
		void SetMagFilter( RenderTypes::TextureFilter newValue ) { fMagFilter = newValue; }

		RenderTypes::TextureFilter GetMinFilter() const { return (RenderTypes::TextureFilter)fMinFilter; }
		void SetMinFilter( RenderTypes::TextureFilter newValue ) { fMinFilter = newValue; }

	public:
		RenderTypes::TextureWrap GetWrapX() const { return (RenderTypes::TextureWrap)fWrapX; }
		void SetWrapX( RenderTypes::TextureWrap newValue ) { fWrapX = newValue; }

		RenderTypes::TextureWrap GetWrapY() const { return (RenderTypes::TextureWrap)fWrapY; }
		void SetWrapY( RenderTypes::TextureWrap newValue ) { fWrapY = newValue; }

		// Allows you to remap textures.
		// Primarily used by gradients
		Real GetNormalizationScaleX() const { return fScaleX; }
		Real GetNormalizationScaleY() const { return fScaleY; }
		void SetNormalizationScaleX( Real newValue ) { fScaleX = newValue; }
		void SetNormalizationScaleY( Real newValue ) { fScaleY = newValue; }

	public:
		static size_t BytesPerPixel( Format format );
		static S32 CalculateRotation( Orientation start, Orientation end );
		static bool GetColorByteIndexesFor(Format format, int *alphaIndex, int *redIndex, int *greenIndex, int *blueIndex);

	protected:
		friend class PlatformBitAccess;
		virtual void Lock();
		virtual void Unlock();

	private:
		U8 fMagFilter;
		U8 fMinFilter;
		U8 fWrapX;
		U8 fWrapY;
		Real fScaleX;
		Real fScaleY;

/*
	public:
		Rtt_FORCE_INLINE Matrix& Transform() { return fDstToPlatformBitmap; }
		Rtt_FORCE_INLINE Matrix& PlatformBitmapTransform() { return fPlatformBitmapTransform; }
		
	private:
		Matrix fDstToPlatformBitmap;
		Matrix fPlatformBitmapTransform;
*/
};

class PlatformBitAccess
{
	public:
		PlatformBitAccess( PlatformBitmap& bitmap )
		:	fBitmap( bitmap )
		{
			fBitmap.Lock();
		}

		~PlatformBitAccess()
		{
			fBitmap.Unlock();
		}

	private:
		PlatformBitmap& fBitmap;
};


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformBitmap_H__

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidImageData_H__
#define _AndroidImageData_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
#include "Display/Rtt_PlatformBitmap.h"


/// Stores an image's bytes and its information such as width, height, pixel format, etc.
/// <br>
/// This class is usd by the AndroidBitmap class to store image data.
class AndroidImageData
{
	public:
		AndroidImageData(Rtt_Allocator *allocatorPointer);
		virtual ~AndroidImageData();

		Rtt_Allocator* GetAllocator() const;
		U32 GetWidth() const;
		void SetWidth(U32 value);
		U32 GetHeight() const;
		void SetHeight(U32 value);
		Rtt_Real GetScale() const;
		void SetScale(Rtt_Real value);
		Rtt::PlatformBitmap::Orientation GetOrientation() const;
		void SetOrientation(Rtt::PlatformBitmap::Orientation value);
		void SetOrientationInDegrees(int value);
		void SetPixelFormatToRGBA();
		void SetPixelFormatToGrayscale();
		bool IsPixelFormatRGBA() const;
		bool IsPixelFormatGrayscale() const;
		U32 GetPixelSizeInBytes() const;
		bool CreateImageByteBuffer();
		void DestroyImageByteBuffer();
		U8* GetImageByteBuffer() const;

	private:
		Rtt_Allocator *fAllocatorPointer;
		U8 *fImageByteBufferPointer;
		U32 fWidth;
		U32 fHeight;
		Rtt_Real fScale;
		Rtt::PlatformBitmap::Orientation fOrientation;
		bool fIsPixelFormatGrayscale;
};

#endif // _AndroidImageData_H__

//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

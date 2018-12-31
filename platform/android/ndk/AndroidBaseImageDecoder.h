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


#ifndef _AndroidBaseImageDecoder_H__
#define _AndroidBaseImageDecoder_H__

#include "Core/Rtt_Types.h"
#include "AndroidOperationResult.h"


// Forward declarations.
class AndroidImageData;
struct Rtt_Allocator;


/// Abstract class from which all image decoders must derive from.
class AndroidBaseImageDecoder
{
	public:
		AndroidBaseImageDecoder(Rtt_Allocator *allocatorPointer);
		virtual ~AndroidBaseImageDecoder();

		Rtt_Allocator* GetAllocator() const;
		AndroidImageData* GetTarget() const;
		void SetTarget(AndroidImageData *imageDataPointer);
		bool IsPixelFormatRGBA() const;
		bool IsPixelFormatGrayscale() const;
		void SetPixelFormatToRGBA();
		void SetPixelFormatToGrayscale();
		bool IsDecodingImageInfoOnly() const;
		bool IsDecodingAllImageData() const;
		void SetToDecodeImageInfoOnly();
		void SetToDecodeAllImageData();
		U32 GetMaxWidth() const;
		void SetMaxWidth(U32 value);
		U32 GetMaxHeight() const;
		void SetMaxHeight(U32 value);
		AndroidOperationResult DecodeFromFile(const char *filePath);

	protected:
		virtual AndroidOperationResult OnDecodeFromFile(const char *filePath) = 0;

	private:
		Rtt_Allocator *fAllocatorPointer;
		AndroidImageData *fImageDataPointer;
		bool fIsPixelFormatGrayscale;
		bool fIsDecodingImageInfoOnly;
		U32 fMaxWidth;
		U32 fMaxHeight;
};

#endif // _AndroidBaseImageDecoder_H__

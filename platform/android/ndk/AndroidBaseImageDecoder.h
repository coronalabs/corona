//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidImageDecoder_H__
#define _AndroidImageDecoder_H__

#include "AndroidBaseImageDecoder.h"
#include "AndroidOperationResult.h"


// Forward declarations.
class AndroidImageData;
class NativeToJavaBridge;
struct Rtt_Allocator;


/// Decodes all image formats supported by Corona by its file extension.
class AndroidImageDecoder : public AndroidBaseImageDecoder
{
	public:
		AndroidImageDecoder(Rtt_Allocator *allocatorPointer, NativeToJavaBridge *ntjb);
		virtual ~AndroidImageDecoder();

	protected:
		AndroidOperationResult OnDecodeFromFile(const char *filePath);

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

#endif // _AndroidImageDecoder_H__

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidJavaImageDecoder_H__
#define _AndroidJavaImageDecoder_H__

#include "AndroidBaseImageDecoder.h"
#include "AndroidOperationResult.h"

// Forward declarations.
class NativeToJavaBridge;
struct Rtt_Allocator;


/// Decodes an image file via the Android "BitmapFactory" Java class.
class AndroidJavaImageDecoder : public AndroidBaseImageDecoder
{
	public:
		AndroidJavaImageDecoder(Rtt_Allocator *allocatorPointer, NativeToJavaBridge *ntjb);
		AndroidJavaImageDecoder(const AndroidBaseImageDecoder &decoder, NativeToJavaBridge *ntjb);
		virtual ~AndroidJavaImageDecoder();

	protected:
		virtual AndroidOperationResult OnDecodeFromFile(const char *filePath);

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

#endif // _AndroidJavaImageDecoder_H__

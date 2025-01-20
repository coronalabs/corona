//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidBaseNativeImageDecoder_H__
#define _AndroidBaseNativeImageDecoder_H__

#include "AndroidBaseImageDecoder.h"
#include "AndroidOperationResult.h"

// Forward declarations.
struct Rtt_Allocator;
class AndroidBinaryReader;
class NativeToJavaBridge;

/// Abstract class from which all native C/C++ image decoders must derive from.
/// <br>
/// Provides a means of decoding an image via a reader/stream object.
class AndroidBaseNativeImageDecoder : public AndroidBaseImageDecoder
{
	public:
		AndroidBaseNativeImageDecoder(Rtt_Allocator *allocatorPointer, NativeToJavaBridge *ntjb);
		AndroidBaseNativeImageDecoder(const AndroidBaseImageDecoder &decoder, NativeToJavaBridge *ntjb);
		virtual ~AndroidBaseNativeImageDecoder();

		AndroidOperationResult DecodeFrom(AndroidBinaryReader &reader);

	protected:
		AndroidOperationResult OnDecodeFromFile(const char *filePath);
		virtual AndroidOperationResult OnDecodeFrom(AndroidBinaryReader &reader) = 0;

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

#endif // _AndroidBaseNativeImageDecoder_H__

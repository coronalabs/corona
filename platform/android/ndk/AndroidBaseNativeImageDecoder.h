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

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidBinaryReadResult_H__
#define _AndroidBinaryReadResult_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
#include "AndroidOperationResult.h"


class AndroidBinaryReadResult : public AndroidOperationResult
{
	protected:
		AndroidBinaryReadResult(
				Rtt_Allocator *allocatorPointer, bool hasSucceeded,
				U32 bytesRead, bool hasBytesRemaining, const char *errorMessage);

	public:
		AndroidBinaryReadResult();
		virtual ~AndroidBinaryReadResult();

		bool HasBytesRemaining() const;
		U32 GetBytesRead() const;

		static AndroidBinaryReadResult SucceededWith(
					Rtt_Allocator *allocatorPointer, U32 bytesRead, bool hasBytesRemaining);
		static AndroidBinaryReadResult FailedWith(Rtt_Allocator *allocatorPointer, const char *errorMessage);

	private:
		U32 fBytesRead;
		bool fHasBytesRemaining;
};

#endif // _AndroidBinaryReadResult_H__

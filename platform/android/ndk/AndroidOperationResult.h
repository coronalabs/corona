//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidOperationResult_H__
#define _AndroidOperationResult_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_AutoPtr.h"


/// Provides a simple success/failure result for an operation.
/// <br>
/// Instances of this type are immutable.
class AndroidOperationResult
{
	protected:
		AndroidOperationResult(Rtt_Allocator *allocatorPointer, bool hasSucceeded, const char *errorMessage);

	public:
		AndroidOperationResult();
		virtual ~AndroidOperationResult();

		bool HasSucceeded() const;
		bool HasFailed() const;
		const char* GetErrorMessage() const;

		static AndroidOperationResult Succeeded(Rtt_Allocator *allocatorPointer);
		static AndroidOperationResult FailedWith(Rtt_Allocator *allocatorPointer, const char *errorMessage);

	private:
		bool fHasSucceeded;
		Rtt::AutoPtr<Rtt::String> fErrorMessageReference;
};

#endif // _AndroidOperationResult_H__

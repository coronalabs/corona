//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_OperationResult.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

const OperationResult OperationResult::kSucceeded(true, (const char*)NULL);


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

OperationResult::OperationResult(bool hasSucceeded, const char* utf8Message)
:	fHasSucceeded(hasSucceeded)
{
	if (utf8Message && (utf8Message[0] != '\0'))
	{
		fUtf8MessagePointer = Rtt_MakeSharedConstStdStringPtr(utf8Message);
	}
}

OperationResult::OperationResult(bool hasSucceeded, const Rtt::SharedConstStdStringPtr& utf8Message)
:	fHasSucceeded(hasSucceeded),
	fUtf8MessagePointer(utf8Message)
{
}

OperationResult::~OperationResult()
{
}


// ----------------------------------------------------------------------------
// Public Methods
// ----------------------------------------------------------------------------

bool OperationResult::HasSucceeded() const
{
	return fHasSucceeded;
}

bool OperationResult::HasFailed() const
{
	return !fHasSucceeded;
}

const char* OperationResult::GetUtf8Message() const
{
#if 1
	return GetUtf8MessageAsSharedPointer()->c_str();
#else
	if (fUtf8MessagePointer.IsNull())
	{
		return "";
	}
	return fUtf8MessagePointer->c_str();
#endif
}

Rtt::SharedConstStdStringPtr OperationResult::GetUtf8MessageAsSharedPointer() const
{
	static Rtt::SharedConstStdStringPtr kSharedEmptyStringPointer = Rtt_MakeSharedConstStdStringPtr("");
	return fUtf8MessagePointer.NotNull() ? fUtf8MessagePointer : kSharedEmptyStringPointer;
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

OperationResult OperationResult::SucceededWith(const char* utf8Message)
{
	return OperationResult(true, utf8Message);
}

OperationResult OperationResult::SucceededWith(const Rtt::SharedConstStdStringPtr& utf8Message)
{
	return OperationResult(true, utf8Message);
}

OperationResult OperationResult::FailedWith(const char* utf8Message)
{
	return OperationResult(false, utf8Message);
}

OperationResult OperationResult::FailedWith(const Rtt::SharedConstStdStringPtr& utf8Message)
{
	return OperationResult(false, utf8Message);
}

}	// namespace Rtt

// ----------------------------------------------------------------------------
// 
// EmptyEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "OperationResult.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
OperationResult::OperationResult(bool hasSucceeded, Platform::String^ message)
{
	fHasSucceeded = hasSucceeded;
	fMessage = message;
}

#pragma endregion


#pragma region Public Instance Methods/Properties
bool OperationResult::HasSucceeded::get()
{
	return fHasSucceeded;
}

bool OperationResult::HasFailed::get()
{
	return !fHasSucceeded;
}

Platform::String^ OperationResult::Message::get()
{
	return fMessage;
}

#pragma endregion


#pragma region Public Static Methods/Properties
OperationResult^ OperationResult::Succeeded::get()
{
	static OperationResult sSucceeded(true, nullptr);
	return %sSucceeded;
}

OperationResult^ OperationResult::SucceededWith(Platform::String^ message)
{
	if (!message)
	{
		return OperationResult::Succeeded;
	}
	return ref new OperationResult(true, message);
}

OperationResult^ OperationResult::FailedWith(Platform::String^ message)
{
	return ref new OperationResult(false, message);
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT

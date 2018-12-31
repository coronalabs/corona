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

#include "stdafx.h"
#include "OperationResult.h"
#include "WinString.h"


namespace Interop {

#pragma region Constants
static const std::shared_ptr<const std::wstring> kEmptySharedStringPointer(std::make_shared<const std::wstring>(L""));

const OperationResult OperationResult::kSucceeded(true, (const wchar_t*)nullptr);

#pragma endregion


#pragma region Constructors/Destructors
OperationResult::OperationResult(bool hasSucceeded, const char *message)
:	fHasSucceeded(hasSucceeded)
{
	if (message && (message[0] != '\0'))
	{
		WinString stringConverter;
		stringConverter.SetUTF8(message);
		fMessage = std::make_shared<const std::wstring>(stringConverter.GetUTF16());
	}
}

OperationResult::OperationResult(bool hasSucceeded, const wchar_t *message)
:	fHasSucceeded(hasSucceeded)
{
	if (message && (message[0] != L'\0'))
	{
		fMessage = std::make_shared<const std::wstring>(message);
	}
}

OperationResult::OperationResult(bool hasSucceeded, const std::shared_ptr<const std::wstring> &message)
:	fHasSucceeded(hasSucceeded),
	fMessage(message)
{
}

OperationResult::~OperationResult()
{
}

#pragma endregion


#pragma region Public Methods/Functions
bool OperationResult::HasSucceeded() const
{
	return fHasSucceeded;
}

bool OperationResult::HasFailed() const
{
	return !fHasSucceeded;
}

const wchar_t* OperationResult::GetMessage() const
{
	return GetMessageAsSharedPointer()->c_str();
}

std::shared_ptr<const std::wstring> OperationResult::GetMessageAsSharedPointer() const
{
	return fMessage.get() ? fMessage : kEmptySharedStringPointer;
}

bool OperationResult::operator==(const OperationResult &result) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (this == &result)
	{
		return true;
	}

	// Check if the success result matches.
	if (fHasSucceeded != result.fHasSucceeded)
	{
		return false;
	}

	// Check if the messages match.
	// Optimization: Check if shared pointers match first. If they do, then we don't need to do a string compare.
	if (GetMessageAsSharedPointer() != result.GetMessageAsSharedPointer())
	{
		if (wcscmp(GetMessage(), result.GetMessage()))
		{
			return false;
		}
	}

	// The result objects match!
	return true;
}

bool OperationResult::operator!=(const OperationResult &result) const
{
	return !(*this == result);
}

OperationResult OperationResult::FailedWith(const char *message)
{
	return OperationResult(false, message);
}

OperationResult OperationResult::FailedWith(const wchar_t *message)
{
	return OperationResult(false, message);
}

OperationResult OperationResult::FailedWith(const std::shared_ptr<const std::wstring> &message)
{
	return OperationResult(false, message);
}

#pragma endregion

}	// namespace Interop

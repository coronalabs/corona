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

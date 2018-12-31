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
#include "LogEntry.h"


#pragma region Constructors/Destructors
LogEntry::LogEntry(DWORD integerId)
:	fIntegerId(integerId),
	fType(LogEntry::Type::kNormal),
	fRtfCharacterCount(0)
{
}

LogEntry::~LogEntry()
{
}

#pragma endregion


#pragma region Public Methods
DWORD LogEntry::GetIntegerId() const
{
	return fIntegerId;
}

LogEntry::Type LogEntry::GetType() const
{
	return fType;
}

void LogEntry::SetType(const LogEntry::Type& value)
{
	fType = value;
}

Interop::DateTime LogEntry::GetDateTime() const
{
	return fDateTime;
}

void LogEntry::SetDateTime(const Interop::DateTime& value)
{
	fDateTime = value;
}

const char* LogEntry::GetText() const
{
	if (!fText.get())
	{
		return "";
	}
	return fText->c_str();
}

void LogEntry::SetText(std::shared_ptr<const std::string> text)
{
	fText = text;
}

unsigned int LogEntry::GetRtfCharacterCount() const
{
	return fRtfCharacterCount;
}

void LogEntry::SetRtfCharacterCount(unsigned int value)
{
	fRtfCharacterCount = value;
}

#pragma endregion

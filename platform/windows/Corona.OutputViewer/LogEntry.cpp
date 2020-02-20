//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

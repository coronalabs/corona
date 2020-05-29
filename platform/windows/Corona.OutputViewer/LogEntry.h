//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\DateTime.h"
#include <cstdint>
#include <memory>
#include <string>
#include <Windows.h>


class LogEntry
{
	public:
		enum class Type : int8_t
		{
			kNormal,
			kWarning,
			kError
		};

		LogEntry(DWORD integerId);
		virtual ~LogEntry();

		DWORD GetIntegerId() const;
		LogEntry::Type GetType() const;
		void SetType(const LogEntry::Type& value);
		Interop::DateTime GetDateTime() const;
		void SetDateTime(const Interop::DateTime& value);
		const char* GetText() const;
		void SetText(std::shared_ptr<const std::string> text);
		unsigned int GetRtfCharacterCount() const;
		void SetRtfCharacterCount(unsigned int value);

	private:
		const LogEntry& operator=(const LogEntry&) {}

		DWORD fIntegerId;
		LogEntry::Type fType;
		Interop::DateTime fDateTime;
		std::shared_ptr<const std::string> fText;
		unsigned int fRtfCharacterCount;
};

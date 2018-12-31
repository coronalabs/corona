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

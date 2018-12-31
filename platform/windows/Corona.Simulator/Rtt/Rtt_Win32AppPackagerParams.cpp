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
#include "Rtt_Win32AppPackagerParams.h"
#include "Core\Rtt_Build.h"
#include "Rtt_Runtime.h"


namespace Rtt
{

#pragma region Constructors/Destructors
Win32AppPackagerParams::Win32AppPackagerParams(const Win32AppPackagerParams::CoreSettings& settings)
:	AppPackagerParams
	(
		settings.AppName,
		settings.VersionString,
		"",
		"",
		settings.SourceDirectoryPath,
		settings.DestinationDirectoryPath,
		"",
		TargetDevice::kWin32Platform,
		"",
		TargetDevice::kUnknownVersion,
		0,
		nullptr,
		nullptr,
		nullptr,
		false
	),
	fRuntimePointer(nullptr)
{
	SetIncludeBuildSettings(true);
}

Win32AppPackagerParams::~Win32AppPackagerParams()
{
}

#pragma endregion


#pragma region Public Methods
const char* Win32AppPackagerParams::GetExeFileName() const
{
	return fExeFileName.GetString();
}

void Win32AppPackagerParams::SetExeFileName(const char* value)
{
	fExeFileName.Set(value);
}

const char* Win32AppPackagerParams::GetCompanyName() const
{
	return fCompanyName.GetString();
}

void Win32AppPackagerParams::SetCompanyName(const char* value)
{
	fCompanyName.Set(value);
}

const char* Win32AppPackagerParams::GetCopyrightString() const
{
	return fCopyrightString.GetString();
}

void Win32AppPackagerParams::SetCopyrightString(const char* value)
{
	fCopyrightString.Set(value);
}

const char* Win32AppPackagerParams::GetAppDescription() const
{
	return fAppDescription.GetString();
}

void Win32AppPackagerParams::SetAppDescription(const char* value)
{
	fAppDescription.Set(value);
}

Runtime* Win32AppPackagerParams::GetRuntime() const
{
	return fRuntimePointer;
}

void Win32AppPackagerParams::SetRuntime(Runtime* value)
{
	fRuntimePointer = value;
}

#pragma endregion

} // namespace Rtt

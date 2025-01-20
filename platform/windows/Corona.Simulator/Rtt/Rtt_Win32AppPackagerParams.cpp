//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

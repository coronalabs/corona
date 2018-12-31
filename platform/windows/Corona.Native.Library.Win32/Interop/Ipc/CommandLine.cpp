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
#include "CommandLine.h"
#include "CommandLineRunner.h"
#include "WinString.h"


namespace Interop { namespace Ipc {

#pragma region Static Member Variables
bool CommandLine::sIsHideWindowEnabled = true;

bool CommandLine::sIsLogOutputEnabled = false;

bool CommandLine::sIsOutputCaptureEnabled = false;

#pragma endregion


#pragma region Constructors/Destructors
CommandLine::CommandLine()
{
}

#pragma endregion


#pragma region Public Static Functions
bool CommandLine::IsHideWindowEnabled()
{
	return sIsHideWindowEnabled;
}

void CommandLine::SetHideWindowEnabled(bool value)
{
	sIsHideWindowEnabled = value;
}

bool CommandLine::IsLogOutputEnabled()
{
	return sIsLogOutputEnabled;
}

void CommandLine::SetLogOutputEnabled(bool value)
{
	sIsLogOutputEnabled = value;
}

bool CommandLine::IsOutputCaptureEnabled()
{
	return sIsOutputCaptureEnabled;
}

void CommandLine::SetOutputCaptureEnabled(bool value)
{
	sIsOutputCaptureEnabled = value;
}

CommandLineRunResult CommandLine::RunUntilExit(const char* commandLine)
{
	WinString stringTranscoder(commandLine);
	return CommandLine::RunUntilExit(stringTranscoder.GetUTF16());
}

CommandLineRunResult CommandLine::RunUntilExit(const wchar_t* commandLine)
{
	CommandLineRunner runner;
	runner.SetHideWindowEnabled(sIsHideWindowEnabled);
	runner.SetLogOutputEnabled(sIsLogOutputEnabled);
	runner.SetOutputCaptureEnabled(sIsOutputCaptureEnabled);
	runner.SetIsShellCommand(false);
	return runner.RunUntilExit(commandLine);
}

CommandLineRunResult CommandLine::RunShellCommandUntilExit(const char* commandLine)
{
	WinString stringTranscoder(commandLine);
	return CommandLine::RunShellCommandUntilExit(stringTranscoder.GetUTF16());
}

CommandLineRunResult CommandLine::RunShellCommandUntilExit(const wchar_t* commandLine)
{
	CommandLineRunner runner;
	runner.SetHideWindowEnabled(sIsHideWindowEnabled);
	runner.SetLogOutputEnabled(sIsLogOutputEnabled);
	runner.SetOutputCaptureEnabled(sIsOutputCaptureEnabled);
	runner.SetIsShellCommand(true);
	return runner.RunUntilExit(commandLine);
}

#pragma endregion

} }	// namespace Interop::Ipc

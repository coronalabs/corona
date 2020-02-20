//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

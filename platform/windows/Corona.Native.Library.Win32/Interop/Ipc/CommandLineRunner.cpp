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
#include "CommandLineRunner.h"
#include "Core\Rtt_Assert.h"
#include "AsyncPipeReader.h"
#include "IODataEventArgs.h"
#include "Process.h"
#include "WinString.h"
#include <Shlwapi.h>


namespace Interop { namespace Ipc {

#pragma region Constructors/Destructors
CommandLineRunner::CommandLineRunner()
:	fIsHideWindowEnabled(true),
	fIsLogOutputEnabled(true),
	fIsShellCommand(false),
	fLastRunResult(CommandLineRunResult::FailedWith(L"A command has not been run yet."))
{
}

CommandLineRunner::~CommandLineRunner()
{
}

#pragma endregion


#pragma region Public Methods
bool CommandLineRunner::IsHideWindowEnabled() const
{
	return fIsHideWindowEnabled;
}

void CommandLineRunner::SetHideWindowEnabled(bool value)
{
	fIsHideWindowEnabled = value;
}

bool CommandLineRunner::IsLogOutputEnabled() const
{
	return fIsLogOutputEnabled;
}

void CommandLineRunner::SetLogOutputEnabled(bool value)
{
	fIsLogOutputEnabled = value;
}

bool CommandLineRunner::IsOutputCaptureEnabled() const
{
	return fIsOutputCaptureEnabled;
}

void CommandLineRunner::SetOutputCaptureEnabled(bool value)
{
	fIsOutputCaptureEnabled = value;
}

bool CommandLineRunner::IsShellCommand() const
{
	return fIsShellCommand;
}

void CommandLineRunner::SetIsShellCommand(bool value)
{
	fIsShellCommand = value;
}

CommandLineRunResult CommandLineRunner::RunUntilExit(const char* commandLine)
{
	WinString stringTranscoder(commandLine);
	return RunUntilExit(stringTranscoder.GetUTF16());
}

bool CommandLineRunner::sStaticOutputCaptureEnabled;
std::string CommandLineRunner::sCapturedOutput;

/* static */ void CommandLineRunner::receivedDataHandler(AsyncPipeReader& sender, const IODataEventArgs& arguments)
{
#if !defined(	Rtt_NO_GUI)
	if (arguments.GetText())
	{
		if (sStaticOutputCaptureEnabled)
		{
			sCapturedOutput.append(arguments.GetText()->c_str());
		}
		else
		{
			Rtt_LogException(arguments.GetText()->c_str());
		}
	}
#endif
}

CommandLineRunResult CommandLineRunner::RunUntilExit(const wchar_t* commandLine)
{
#if defined(	Rtt_NO_GUI)
	Rtt_ASSERT_MSG( 0, "Code should NOT be reached" );
	fLastRunResult = CommandLineRunResult::FailedWith(L"Code should NOT be reached.");
	return fLastRunResult;
#else
	// Validate.
	if (!commandLine || (L'\0' == commandLine[0]))
	{
		fLastRunResult = CommandLineRunResult::FailedWith(L"Command line string was null or empty.");
		return fLastRunResult;
	}

	// Acquire the application file path string.
	std::wstring filePath;
	if (fIsShellCommand)
	{
		// For shell commands, we'll use the Microsoft "cmd.exe" command prompt app.
		filePath = L"cmd.exe";
	}
	else
	{
		// Extract the file path from the given command line string.
		int argumentCount = 0;
		auto argumentArray = ::CommandLineToArgvW(commandLine, &argumentCount);
		if (argumentArray)
		{
			if (argumentCount > 0)
			{
				filePath = argumentArray[0];
			}
			::LocalFree(argumentArray);
		}
	}

	// Set up the argument portion of the command line string.
	std::wstring commandLineArguments;
	if (fIsShellCommand)
	{
		commandLineArguments = L"/C ";
		commandLineArguments += commandLine;
	}
	else
	{
		auto stringPointer = ::PathGetArgsW(commandLine);
		if (stringPointer && (stringPointer != L'\0'))
		{
			commandLineArguments = stringPointer;
		}
	}

	// Set up the process launch settings.
	Interop::Ipc::Process::LaunchSettings launchSettings{};
	launchSettings.FileNamePath = filePath.c_str();
	launchSettings.CommandLineArguments = commandLineArguments.c_str();
	launchSettings.IsWindowHidden = fIsHideWindowEnabled;
	if (fIsLogOutputEnabled || fIsOutputCaptureEnabled)
	{
		launchSettings.IsStdOutRedirectionEnabled = true;
		launchSettings.IsStdErrRedirectionEnabled = true;
	}

	// Launch the requested process.
	auto launchResult = Interop::Ipc::Process::LaunchUsing(launchSettings);
	auto processPointer = launchResult.GetValue();
	if (!processPointer)
	{
		fLastRunResult = CommandLineRunResult::FailedWith(launchResult.GetMessage());
		return fLastRunResult;
	}

	// Set up a callback to log all stdout/stderr messages if redirected.
	sStaticOutputCaptureEnabled = fIsOutputCaptureEnabled;
	sCapturedOutput = "";
	AsyncPipeReader::ReceivedDataEvent::FunctionHandler receivedDataHandler(receivedDataHandler);

	// If stdout and stderr was redirected, then have the above callback log all received messages.
	auto pipeReaderPointer = processPointer->GetStdOutReader();
	if (pipeReaderPointer)
	{
		pipeReaderPointer->GetReceivedDataEventHandlers().Add(&receivedDataHandler);
		pipeReaderPointer->Start();
	}
	pipeReaderPointer = processPointer->GetStdErrReader();
	if (pipeReaderPointer)
	{
		pipeReaderPointer->GetReceivedDataEventHandlers().Add(&receivedDataHandler);
		pipeReaderPointer->Start();
	}

	// Block this thread until the process has terminated.
	processPointer->WaitForExit();

	// Returned the terminated process' exit code, if provided.
	auto fetchExitCodeResult = processPointer->GetExitCode();
	if (fetchExitCodeResult.HasSucceeded())
	{
		fLastRunResult = CommandLineRunResult::SucceededWith(fetchExitCodeResult.GetValue(), sCapturedOutput);
	}
	else
	{
		fLastRunResult = CommandLineRunResult::FailedWith(fetchExitCodeResult.GetMessage(), sCapturedOutput);
	}
	return fLastRunResult;

	#endif
}

CommandLineRunResult CommandLineRunner::GetLastRunResult()
{
	return fLastRunResult;
}

#pragma endregion

} }	// namespace Interop::Ipc

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandLineRunResult.h"


namespace Interop { namespace Ipc {

#pragma region Constructors/Destructors
CommandLineRunResult::CommandLineRunResult(bool hasSucceeded, const char *message, int32_t exitCode)
:	OperationResult(hasSucceeded, message),
	fExitCode(exitCode),
	fOutput("")
{
}

CommandLineRunResult::CommandLineRunResult(bool hasSucceeded, const wchar_t *message, int32_t exitCode)
	: OperationResult(hasSucceeded, message),
	fExitCode(exitCode),
	fOutput("")
{
}

CommandLineRunResult::CommandLineRunResult(bool hasSucceeded, const wchar_t *message, int32_t exitCode, std::string output)
	: OperationResult(hasSucceeded, message),
	fExitCode(exitCode),
	fOutput(output)
{
}

CommandLineRunResult::~CommandLineRunResult()
{
}

#pragma endregion


#pragma region Public Methods
int32_t CommandLineRunResult::GetExitCode() const
{
	return fExitCode;
}

std::string CommandLineRunResult::GetOutput() const
{
	return fOutput;
}

#pragma endregion


#pragma region Public Static Functions
CommandLineRunResult CommandLineRunResult::SucceededWith(int32_t exitCode)
{
	return CommandLineRunResult(true, (wchar_t*)nullptr, exitCode);
}

CommandLineRunResult CommandLineRunResult::SucceededWith(int32_t exitCode, std::string output)
{
	return CommandLineRunResult(true, (wchar_t*)nullptr, exitCode, output);
}

CommandLineRunResult CommandLineRunResult::FailedWith(const char *message)
{
	return CommandLineRunResult(false, message, -1);
}

CommandLineRunResult CommandLineRunResult::FailedWith(const wchar_t *message)
{
	return CommandLineRunResult(false, message, -1);
}

CommandLineRunResult CommandLineRunResult::FailedWith(const wchar_t *message, std::string output)
{
	return CommandLineRunResult(false, message, -1, output);
}

#pragma endregion

} }	// namespace Interop::Ipc

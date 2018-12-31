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

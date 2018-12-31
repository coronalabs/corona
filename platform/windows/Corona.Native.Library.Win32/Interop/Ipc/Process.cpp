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
#include "Process.h"
#include "Core\Rtt_Assert.h"
#include "Interop\UI\HandleMessageEventArgs.h"
#include "Interop\UI\MessageOnlyWindow.h"
#include "AsyncPipeReader.h"
#include "WinString.h"
#include <Shlwapi.h>
#include <thread>


namespace Interop { namespace Ipc {

#pragma region Constructors/Destructors
Process::Process()
:	fReceivedMessageEventHandler(this, &Process::OnReceivedMessage),
	fFilePath(L""),
	fCommandLineArguments(L""),
	fStdInHandle(nullptr),
	fStdOutReaderPointer(nullptr),
	fStdErrReaderPointer(nullptr),
	fHasExited(false),
	fExitCode(0),
	fRegisterWaitHandle(nullptr),
	fMessageOnlyWindowPointer(Interop::UI::MessageOnlyWindow::GetSharedInstanceForCurrentThread()),
	fCustomExitedMessageId(0)
{
	// Finish initialization.
	ZeroMemory(&fProcessInfo, sizeof(fProcessInfo));

	// Add event handlers.
	if (fMessageOnlyWindowPointer)
	{
		fMessageOnlyWindowPointer->GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
		fCustomExitedMessageId = fMessageOnlyWindowPointer->ReserveMessageId();
	}
}

Process::~Process()
{
	ReleaseResources();
}

#pragma endregion


#pragma region Public Methods
Process::ExitedEvent::HandlerManager& Process::GetExitedEventHandlers()
{
	return fExitedEvent.GetHandlerManager();
}

const wchar_t* Process::GetFilePath() const
{
	return fFilePath.c_str() ? fFilePath.c_str() : L"";
}

const wchar_t* Process::GetCommandLineArguments() const
{
	return fCommandLineArguments.c_str() ? fCommandLineArguments.c_str() : L"";
}

HANDLE Process::GetProcessHandle() const
{
	return fProcessInfo.hProcess;
}

DWORD Process::GetProcessId() const
{
	return fProcessInfo.dwProcessId;
}

HANDLE Process::GetStdInHandle() const
{
	return fStdInHandle;
}

AsyncPipeReader* Process::GetStdOutReader()
{
	return fStdOutReaderPointer;
}

AsyncPipeReader* Process::GetStdErrReader()
{
	return fStdErrReaderPointer;
}

bool Process::HasExited() const
{
	// Poll for the process' current exit status.
	// Note: This is a cheat. This call updates this object's member variables within a const method.
	const_cast<Process*>(this)->PollStatus();
	return fHasExited;
}

ValueResult<int32_t> Process::GetExitCode() const
{
	// Poll for the process' current exit status.
	// Note: This is a cheat. This call updates this object's member variables within a const method.
	const_cast<Process*>(this)->PollStatus();

	// Return a failure result if the process has not exited yet.
	if (HasExited() == false)
	{
		return ValueResult<int32_t>::FailedWith(L"Process has not exited yet.");
	}

	// Return the terminated process' exit code.
	return ValueResult<int32_t>::SucceededWith(fExitCode);
}

HWND Process::FetchMainWindowHandle()
{
	// Do not continue if the process is no longer running.
	if (HasExited())
	{
		return nullptr;
	}

	// Traverse all windows on the desktop to find the referenced process' main/root window.
	// Note: The below algorithm is how Microsoft's .NET "Process" class searches for it.
	struct EnumWindowContext
	{
		Process* ProcessPointer;
		HWND MainWindowHandle;
	};
	EnumWindowContext enumWindowContext{};
	enumWindowContext.ProcessPointer = this;
	::EnumWindows(
		[](HWND windowHandle, LPARAM lParam)->BOOL
		{
			// Validate.
			if (!windowHandle || !lParam)
			{
				return FALSE;
			}
			auto contextPointer = (EnumWindowContext*)lParam;
			if (!contextPointer->ProcessPointer)
			{
				return FALSE;
			}

			// Check if the given window belongs to the referenced process.
			DWORD windowProcessId = 0;
			::GetWindowThreadProcessId(windowHandle, &windowProcessId);
			if (windowProcessId == contextPointer->ProcessPointer->GetProcessId())
			{
				// Check if this window does not have any parent windows, making it the main/root window.
				if (!::GetWindow(windowHandle, GW_OWNER) && IsWindowVisible(windowHandle))
				{
					// Found it! Copy the main window's handle to the context.
					// Returning FALSE makes EnumWindow() stop enumerating through the rest of the desktop windows.
					contextPointer->MainWindowHandle = windowHandle;
					return FALSE;
				}
			}

			// This is not the window we're looking for.
			// Return TRUE to request the next window on the desktop.
			return TRUE;
		},
		(LPARAM)&enumWindowContext
	);

	// Return the result.
	return enumWindowContext.MainWindowHandle;
}

OperationResult Process::RequestCloseMainWindow()
{
	// Do not continue if the process is no longer running.
	if (HasExited())
	{
		return OperationResult::FailedWith(L"Process has already been terminated.");
	}

	// Fetch the process' main/root window handle.
	auto windowHandle = FetchMainWindowHandle();
	if (!windowHandle)
	{
		return OperationResult::FailedWith(L"Failed to find process' main window.");
	}

	// Attempt to close the window.
	BOOL wasClosePosted = ::PostMessageW(windowHandle, WM_CLOSE, 0, 0);
	if (!wasClosePosted)
	{
		return OperationResult::FailedWith(L"Failed to close the window.");
	}

	// We've succesfully posted a close message to the window.
	return OperationResult::kSucceeded;
}

OperationResult Process::RequestShowMainWindow()
{
	// Do not continue if the process is no longer running.
	if (HasExited())
	{
		return OperationResult::FailedWith(L"Process has already terminated.");
	}

	// Fetch the process' main/root window handle.
	auto windowHandle = FetchMainWindowHandle();
	if (!windowHandle)
	{
		return OperationResult::FailedWith(L"Failed to find process' main window.");
	}

	::ShowWindow(windowHandle, SW_SHOWNORMAL); // in case it's minimized
	::SetForegroundWindow(windowHandle);

	return OperationResult::kSucceeded;
}

OperationResult Process::RequestForceQuit(int32_t exitCode)
{
	// Poll for the process' current status.
	PollStatus();

	// Do not continue if the process has already terminated.
	if (fHasExited || !fProcessInfo.hProcess)
	{
		return OperationResult::FailedWith(L"Process has already been terminated.");
	}

	// Force quit the process.
	BOOL wasTerminated = ::TerminateProcess(fProcessInfo.hProcess, (UINT)exitCode);
	if (!wasTerminated)
	{
		return OperationResult::FailedWith(L"Failed to terminate the process.");
	}

	// The termination request has been successfully delivered.
	return OperationResult::kSucceeded;
}

void Process::WaitForExit()
{
	// Return immediately if reference process has already exited.
	if (fHasExited)
	{
		return;
	}

	// Block the calling thread until the referenced process has been terminated.
	while (true)
	{
		// Poll the process' stdout/stderr pipe buffers, if redirected.
		bool isWaitingForPipeData = false;
		if (fStdOutReaderPointer)
		{
			fStdOutReaderPointer->Poll();
			if (fStdOutReaderPointer->IsRunning() && !fStdOutReaderPointer->WasClosed())
			{
				isWaitingForPipeData = true;
			}
		}
		if (fStdErrReaderPointer)
		{
			fStdErrReaderPointer->Poll();
			if (fStdErrReaderPointer->IsRunning() && !fStdErrReaderPointer->WasClosed())
			{
				isWaitingForPipeData = true;
			}
		}

		// Check if the process has been terminated.
		// But only after all redirected stdout/stderr pipe data has been received, if applicable.
		if (!isWaitingForPipeData)
		{
			PollStatus();
			if (fHasExited)
			{
				break;
			}
		}

		// Put this thread to sleep for a short while before checking again.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

#pragma endregion


#pragma region Private Methods
void Process::OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments)
{
	// Do not continue if the received message was already handled.
	if (arguments.WasHandled())
	{
		return;
	}

	// Handle the received message.
	if (arguments.GetMessageId() == fCustomExitedMessageId)
	{
		// The application process has been terminated.
		// Wait for last received stdout/stderr data and then poll for its exit code.
		WaitForExit();

		// Flag that this message was handled.
		arguments.SetReturnResult(0);
		arguments.SetHandled();
	}
}

void Process::PollStatus()
{
	// Validate.
	if (!fProcessInfo.hProcess)
	{
		return;
	}

	// Do not continue if the last call to this method detected that the process has exited.
	if (fHasExited)
	{
		return;
	}

	// Check if the process has exited, and if so, fetch its exit code.
	DWORD exitCode = 0;
	BOOL hasSucceeded = ::GetExitCodeProcess(fProcessInfo.hProcess, &exitCode);
	if (hasSucceeded)
	{
		// Note that an exit code of STILL_ACTIVE (ie: 259) is ambiguous, which is a weakness in the above Win32 API.
		// It may mean that the app is still running or it has exited with a 259 exit code value.
		if (exitCode != STILL_ACTIVE)
		{
			// The process has been exited.
			fExitCode = (int32_t)exitCode;
			fHasExited = true;
		}
		else
		{
			DWORD waitResult = ::WaitForSingleObject(fProcessInfo.hProcess, 0);
			if (WAIT_OBJECT_0 == waitResult)
			{
				// The process has been exited with a 259 exit code.
				fExitCode = (int32_t)exitCode;
				fHasExited = true;
			}
		}
	}

	// If the process has just exited, then release all resources associated with it and raise an event.
	if (fHasExited)
	{
		ReleaseResources();
		fExitedEvent.Raise(*this, EventArgs::kEmpty);
	}
}

void Process::ReleaseResources()
{
	// Unregister our OnAsyncExited() callback from the system.
	// Note: The INVALID_HANDLE_VALUE value below blocks the function call until OnAsyncExite() exits.
	//       This guarrantees that the "this" pointer passed into that function is always valid.
	if (fRegisterWaitHandle)
	{
		::UnregisterWaitEx(fRegisterWaitHandle, INVALID_HANDLE_VALUE);
		fRegisterWaitHandle = nullptr;
	}

	// Close all active pipes.
	if (fStdInHandle)
	{
		::CloseHandle(fStdInHandle);
		fStdInHandle = nullptr;
	}
	if (fStdOutReaderPointer)
	{
		auto pipeHandle = fStdOutReaderPointer->GetHandle();
		delete fStdOutReaderPointer;
		fStdOutReaderPointer = nullptr;
		if (pipeHandle)
		{
			::CloseHandle(pipeHandle);
		}
	}
	if (fStdErrReaderPointer)
	{
		auto pipeHandle = fStdErrReaderPointer->GetHandle();
		delete fStdErrReaderPointer;
		fStdErrReaderPointer = nullptr;
		if (pipeHandle)
		{
			::CloseHandle(pipeHandle);
		}
	}

	// Release the process' Win32 handles.
	if (fProcessInfo.hProcess)
	{
		::CloseHandle(fProcessInfo.hProcess);
		fProcessInfo.hProcess = nullptr;
	}
	if (fProcessInfo.hThread)
	{
		::CloseHandle(fProcessInfo.hThread);
		fProcessInfo.hThread = nullptr;
	}

	// Remove event handlers.
	if (fMessageOnlyWindowPointer)
	{
		fMessageOnlyWindowPointer->GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);
		fMessageOnlyWindowPointer->UnreserveMessageId(fCustomExitedMessageId);
		fCustomExitedMessageId = 0;
	}
}

#pragma endregion


#pragma region Public Static Functions
Process::LaunchResult Process::LaunchUsing(const Process::LaunchSettings& settings)
{
	// Validate the given file path.
	if (!settings.FileNamePath || (L'\0' == settings.FileNamePath[0]))
	{
		return LaunchResult::FailedWith(L"File path cannot be set to an empty string.");
	}

	// Build a command line used to launch the given file and with the given arguments.
	// Note: Launching an EXE via a command line string allows us to support relative paths
	//       too which takes advantage of the standard Windows lookup path.
	WinString commandLineArguments;
	commandLineArguments.Append(L'\"');
	commandLineArguments.Append(settings.FileNamePath);
	commandLineArguments.Append(L'\"');
	if (settings.CommandLineArguments && (settings.CommandLineArguments[0] != L'\0'))
	{
		commandLineArguments.Append(L' ');
		commandLineArguments.Append(settings.CommandLineArguments);
	}

	// Set up a stdin pipe if enabled.
	HANDLE stdInReadPipeHandle = nullptr;
	HANDLE stdInWritePipeHandle = nullptr;
	if (settings.IsStdInRedirectionEnabled)
	{
		SECURITY_ATTRIBUTES securityAttributes{};
		securityAttributes.nLength = sizeof(securityAttributes);
		securityAttributes.bInheritHandle = TRUE;
		::CreatePipe(&stdInReadPipeHandle, &stdInWritePipeHandle, &securityAttributes, 0);
		if (stdInWritePipeHandle)
		{
			::SetHandleInformation(stdInWritePipeHandle, HANDLE_FLAG_INHERIT, 0);
		}
	}

	// Set up a stdout pipe if enabled.
	HANDLE stdOutReadPipeHandle = nullptr;
	HANDLE stdOutWritePipeHandle = nullptr;
	if (settings.IsStdOutRedirectionEnabled)
	{
		SECURITY_ATTRIBUTES securityAttributes{};
		securityAttributes.nLength = sizeof(securityAttributes);
		securityAttributes.bInheritHandle = TRUE;
		::CreatePipe(&stdOutReadPipeHandle, &stdOutWritePipeHandle, &securityAttributes, 0);
		if (stdOutReadPipeHandle)
		{
			::SetHandleInformation(stdOutReadPipeHandle, HANDLE_FLAG_INHERIT, 0);
		}
	}

	// Set up a stderr pipe if enabled.
	HANDLE stdErrReadPipeHandle = nullptr;
	HANDLE stdErrWritePipeHandle = nullptr;
	if (settings.IsStdErrRedirectionEnabled)
	{
		SECURITY_ATTRIBUTES securityAttributes{};
		securityAttributes.nLength = sizeof(securityAttributes);
		securityAttributes.bInheritHandle = TRUE;
		::CreatePipe(&stdErrReadPipeHandle, &stdErrWritePipeHandle, &securityAttributes, 0);
		if (stdErrReadPipeHandle)
		{
			::SetHandleInformation(stdErrReadPipeHandle, HANDLE_FLAG_INHERIT, 0);
		}
	}

	// Configure the Win32 startup information.
	DWORD creationFlags = 0;
	STARTUPINFO startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	{
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;
		startupInfo.hStdInput = stdInReadPipeHandle ? stdInReadPipeHandle : INVALID_HANDLE_VALUE;
		startupInfo.hStdOutput = stdOutWritePipeHandle ? stdOutWritePipeHandle : INVALID_HANDLE_VALUE;
		startupInfo.hStdError = stdErrWritePipeHandle ? stdErrWritePipeHandle : INVALID_HANDLE_VALUE;
	}
	if (settings.IsWindowHidden)
	{
		startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
		startupInfo.wShowWindow = SW_HIDE;
		creationFlags |= CREATE_NO_WINDOW;
	}

	// Attempt to launch the application.
	PROCESS_INFORMATION processInfo{};
	::SetLastError(0);
	BOOL wasProcessCreated = ::CreateProcessW(
			nullptr, commandLineArguments.GetBuffer(),
			nullptr, nullptr, TRUE, creationFlags, nullptr, nullptr, &startupInfo, &processInfo);

	// Do not continue if we've failed to launch the application above.
	if (!wasProcessCreated || !processInfo.hProcess)
	{
		// Fetch an error message indicating why the app failed to launch.
		std::wstring message;
		if (!wasProcessCreated)
		{
			auto errorCode = ::GetLastError();
			if (errorCode)
			{
				LPWSTR utf16Buffer;
				::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						nullptr, errorCode,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&utf16Buffer, 0, nullptr);
				if (utf16Buffer && (utf16Buffer[0] != L'\0'))
				{
					message = utf16Buffer;
				}
				::LocalFree(utf16Buffer);
			}
		}
		if (message.empty())
		{
			message = L"Unknown error.";
		}

		// Close all pipes, if created.
		if (stdInReadPipeHandle)
		{
			::CloseHandle(stdInReadPipeHandle);
		}
		if (stdInWritePipeHandle)
		{
			::CloseHandle(stdInWritePipeHandle);
		}
		if (stdOutReadPipeHandle)
		{
			::CloseHandle(stdOutReadPipeHandle);
		}
		if (stdOutWritePipeHandle)
		{
			::CloseHandle(stdOutWritePipeHandle);
		}
		if (stdErrReadPipeHandle)
		{
			::CloseHandle(stdErrReadPipeHandle);
		}
		if (stdErrWritePipeHandle)
		{
			::CloseHandle(stdErrWritePipeHandle);
		}

		// Return a failure result.
		return LaunchResult::FailedWith(message.c_str());
	}

	// *** The application process was successfully launched. ***

	// Close the pipe handles given to the launched app.
	// We only need the stdout and stderr "read" handles and the stdin "write" handle on this end.
	if (stdInReadPipeHandle)
	{
		::CloseHandle(stdInReadPipeHandle);
	}
	if (stdOutWritePipeHandle)
	{
		::CloseHandle(stdOutWritePipeHandle);
	}
	if (stdErrWritePipeHandle)
	{
		::CloseHandle(stdErrWritePipeHandle);
	}

	// Create a new Process instance and initialize it with the above info.
	std::shared_ptr<Process> processPointer(new Process());
	processPointer->fProcessInfo = processInfo;
	processPointer->fFilePath = settings.FileNamePath;
	processPointer->fCommandLineArguments = settings.CommandLineArguments ? settings.CommandLineArguments : L"";
	processPointer->fStdInHandle = stdInWritePipeHandle;
	if (stdOutReadPipeHandle)
	{
		processPointer->fStdOutReaderPointer = new AsyncPipeReader(stdOutReadPipeHandle);
	}
	if (stdErrReadPipeHandle)
	{
		processPointer->fStdErrReaderPointer = new AsyncPipeReader(stdErrReadPipeHandle);
	}

	// Request the system to notify us when the application process exits.
	// The system will call our OnAsynExited() function on another thread when this happens.
	::RegisterWaitForSingleObject(
			&(processPointer->fRegisterWaitHandle), processInfo.hProcess, &Process::OnAsyncExited,
			processPointer.get(), INFINITE, WT_EXECUTEONLYONCE);

	// Return a success result containing a shared pointer to the Process instance created above.
	return LaunchResult::SucceededWith(processPointer);
}

#pragma endregion


#pragma region Private Static Functions
void Process::OnAsyncExited(PVOID contextPointer, BOOLEAN hasTimedOut)
{
	// Validate.
	if (!contextPointer)
	{
		return;
	}

	// Notify the "Process" object on the thread it was created on that the application has exited.
	// We do this by posting a custom message ID to it via its message-only window.
	auto processPointer = (Process*)contextPointer;
	HWND windowHandle = nullptr;
	if (processPointer->fMessageOnlyWindowPointer)
	{
		windowHandle = processPointer->fMessageOnlyWindowPointer->GetWindowHandle();
	}
	if (windowHandle && processPointer->fCustomExitedMessageId)
	{
		::PostMessageW(windowHandle, processPointer->fCustomExitedMessageId, 0, 0);
	}
}

#pragma endregion

} }	// namespace Interop::Ipc

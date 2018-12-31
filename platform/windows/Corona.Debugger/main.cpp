// ----------------------------------------------------------------------------
// 
// main.cpp
// Copyright (c) 2011 Ansca, Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#include "stdafx.h"
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
	#include "..\..\..\external\luasocket\src\luasocket.h"
	#include "..\..\..\external\luasocket\src\mime.h"
}
#include "Core\Rtt_Version.h"
#include "Core\Rtt_Config.h"
#include "Core\Rtt_Macros.h"
#include "Core\Rtt_Types.h"
#include "Rtt_LuaLibSocket.h"
#include <Shlwapi.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <Windows.h>


namespace Rtt
{
	int luaload_controller(lua_State *L);
}

typedef struct _LaunchArgs
{
	int argc;
	const char **argv;
	lua_CFunction loader;
	lua_CFunction openlib;
}
LaunchArgs;

static int LuaOpenModule(lua_State *L, lua_CFunction loader)
{
	int arg = lua_gettop(L);
	(*loader)(L);
	lua_insert(L, 1);
	lua_call(L, arg, 1);
	return 1;
}

template<int(*F)(lua_State*)>
static int LuaOpen(lua_State *L)
{
	return LuaOpenModule(L, F);
}

static void OnAsyncLogStdOut(const bool* wasExitRequestedPointer, HANDLE pipeHandle)
{
	// Validate arguments.
	if (!wasExitRequestedPointer || *wasExitRequestedPointer || !pipeHandle)
	{
		return;
	}

	// Start our infinite loop which reads the given pipe.
	const DWORD kBufferSizeInBytes = 32767;
	char byteBuffer[kBufferSizeInBytes];
	DWORD bytesCopiedToBuffer = 0;
	bool wasPipeClosed = false;
	while (true)
	{
		// Poll the pipe's buffer until:
		// - All of the bytes in the buffer have been read.
		// - The pipe has been closed or errored out.
		while (true)
		{
			// Do a non-blocking check for bytes in the pipe's buffer.
			// Note: This *will* block if a ReadFile() was called on another thread. (You should never do that.)
			BOOL result;
			DWORD bytesAvailable = 0;
			result = ::PeekNamedPipe(pipeHandle, nullptr, 0, nullptr, &bytesAvailable, nullptr);
			if (!result)
			{
				if (::GetLastError() != ERROR_IO_PENDING)
				{
					wasPipeClosed = true;
				}
				break;
			}

			// Do not continue if the pipe is empty.
			if (bytesAvailable <= 0)
			{
				break;
			}

			// Fetch data from the pipe.
			DWORD bytesReceived = 0;
			DWORD bytesRequested = (std::min)(bytesAvailable, kBufferSizeInBytes - bytesCopiedToBuffer);
			result = ::ReadFile(pipeHandle, byteBuffer + bytesCopiedToBuffer, bytesRequested, &bytesReceived, nullptr);
			if (!result)
			{
				if (::GetLastError() != ERROR_IO_PENDING)
				{
					wasPipeClosed = true;
				}
				break;
			}
			if (bytesReceived <= 0)
			{
				break;
			}
			if (bytesReceived > bytesRequested)
			{
				bytesReceived = bytesRequested;
			}
			bytesCopiedToBuffer += bytesReceived;

			// Write all received data to this console app's stdout.
			// Note: We only write when a newline character has been found. This way we avoid mixing
			//       characters in the console between this app's stdout and the child app's stdout.
			while (bytesCopiedToBuffer > 0)
			{
				// Find a newline character, starting from the back of the buffer.
				int endIndex;
				for (endIndex = (int)bytesCopiedToBuffer - 1; endIndex >= 0; endIndex--)
				{
					if ('\n' == byteBuffer[endIndex])
					{
						break;
					}
				}
				if ((DWORD)endIndex < bytesCopiedToBuffer)
				{
					// Newline was found. Write that substring to this app's stdout.
					DWORD bytesToCopy = (DWORD)endIndex + 1;
					fwrite(byteBuffer, sizeof(char), bytesToCopy, stdout);

					// Remove the above substring from this thread's buffer by shifting its bytes.
					bytesCopiedToBuffer -= bytesToCopy;
					if (bytesCopiedToBuffer > 0)
					{
						memmove_s(byteBuffer, kBufferSizeInBytes, byteBuffer + bytesToCopy, bytesCopiedToBuffer);
					}
				}
				else
				{
					// A newline character was not found.
					// If the buffer is full, then write the entire buffer to this app's stdout.
					if (bytesCopiedToBuffer >= kBufferSizeInBytes)
					{
						fwrite(byteBuffer, sizeof(char), kBufferSizeInBytes, stdout);
						bytesCopiedToBuffer = 0;
					}

					// Wait for more data from the pipe. We're hoping to get a newline character later.
					break;
				}
			}
		}

		// Exit out of this thead if:
		// - The pipe was closed or errored out.
		// - This thread was requested to exit out.
		if (wasPipeClosed || *wasExitRequestedPointer)
		{
			break;
		}

		// Give this thread a short break.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

static int
pmain( lua_State *L )
{
	LaunchArgs* launchArgs = (LaunchArgs*)lua_touserdata(L,1);
	lua_gc(L,LUA_GCSTOP,0);
	luaL_openlibs(L);
	if ( launchArgs->openlib )
	{
		(*launchArgs->openlib)( L );
	}
	lua_gc(L,LUA_GCRESTART,0);

	(*launchArgs->loader)( L );

	const char **argv = launchArgs->argv;
	int argc = launchArgs->argc;
	int i;
	lua_createtable(L,argc-1,1);
	for (i=0; i < argc; i++)
	{
		lua_pushstring(L,argv[i]);
		lua_rawseti(L,-2,i);
	}
	lua_setglobal(L,"arg");
	luaL_checkstack(L,i-1,"too many arguments to script");
	for (i=1; i < argc; i++)
	{
		lua_pushstring(L,argv[i]);
	}
	lua_call(L,i-1,0);
	return 0;
}

static int
run( int argc, const wchar_t *argv[], lua_CFunction loader, lua_CFunction openlib )
{
	// Create a new Lua state.
	lua_State *L = lua_open();
	if ( ! L )
	{
		fwprintf(stderr, L"%s: %s\n", argv[0], L"Not enough memory for Lua state.");
		return EXIT_FAILURE;
	}

	// Transcode the UTF-16 command line arguments to UTF-8.
	char** utf8ArgumentArray = nullptr;
	if (argc > 0)
	{
		utf8ArgumentArray = new char*[argc];
		for (int index = 0; index < argc; index++)
		{
			utf8ArgumentArray[index] = lua_create_utf8_string_from(argv[index]);
		}
	}

	// Run the Lua debugger project.
	LaunchArgs launchArgs = { argc, (const char**)utf8ArgumentArray, loader, openlib };
	int result = lua_cpcall(L, pmain, &launchArgs);

	// Delete the UTF-8 strings created above.
	if (utf8ArgumentArray)
	{
		for (int index = 0; index < argc; index++)
		{
			lua_destroy_utf8_string(utf8ArgumentArray[index]);
		}
		delete[] utf8ArgumentArray;
		utf8ArgumentArray = nullptr;
	}

	// Destroy the Lua state.
	lua_close(L);

	// Log an error if we've failed to run the Lua debugger.
	if (result)
	{
		const char* utf8Message = lua_tostring(L, -1);
		if (!utf8Message || ('\0' == utf8Message[0]))
		{
			utf8Message = "Unknown Lua error occurred.";
		}
		wchar_t* utf16Message = lua_create_utf16_string_from(utf8Message);
		if (utf16Message)
		{
			fwprintf(stderr, L"%s: %s\n", argv[0], utf16Message);
			lua_destroy_utf16_string(utf16Message);
		}
		return EXIT_FAILURE;
	}

	// Return a success result.
	return 0;
}

static int
initialize_luasocket( lua_State *L )
{
	using namespace Rtt;

	// Static Modules
	const luaL_Reg luaStaticModules[] =
	{
		{ "socket.core", luaopen_socket_core },
		{ "socket", LuaOpen<luaload_luasocket_socket> },
		{ NULL, NULL }
	};

	// Preload modules to eliminate dependency
	lua_getfield( L, LUA_GLOBALSINDEX, "package" );
	lua_getfield( L, -1, "preload" );
	for ( const luaL_Reg *lib = luaStaticModules; lib->func; lib++ )
	{
		lua_pushcfunction( L, lib->func );
		lua_setfield( L, -2, lib->name );
	}
	lua_pop( L, 2 );

	return 0;
}

int wmain( int argc, const wchar_t *argv[], wchar_t *envp[] )
{
	const int MAX_BUFFER_LENGTH = 2048;
	wchar_t utf16Buffer[MAX_BUFFER_LENGTH];

	// Fetch an absolute path to this application.
	// We'll use this path to find the Corona Simulator executable.
	std::wstring applicationPath;
	auto result = ::GetModuleFileNameW(NULL, utf16Buffer, MAX_BUFFER_LENGTH);
	if (result > 0)
	{
		// Copy the path to a std::wstring.
		applicationPath = utf16Buffer;

		// Remove the file name from the path string. Leave the trailing slash '\' at the end.
		int index = applicationPath.rfind(L'\\');
		if ((index > 0) && ((index + 1) < (int)applicationPath.length()))
		{
			index++;
			applicationPath.erase(index, std::wstring::npos);
		}
		else
		{
			applicationPath.clear();
		}
	}
	if (applicationPath.empty())
	{
		fprintf(stderr, "Failed to acquire Corona Simulator path.\r\n");
		return EXIT_FAILURE;
	}

	// Fetch a Corona project's "main.lua" file if provided at the command line.
	std::wstring mainLuaFilePath;
	if ((argc >= 2) && (argv[1] != L'\0'))
	{
		// Fetch the path. Make sure it ends with a "main.lua".
		const wchar_t kMainLuaFileName[] = L"main.lua";
		const int kMainLuaFileNameLength = wcslen(kMainLuaFileName);
		int argumentStringLength = wcslen(argv[1]);
		if (argumentStringLength > kMainLuaFileNameLength)
		{
			auto fileStringPointer = argv[1] + (argumentStringLength - kMainLuaFileNameLength);
			if (!_wcsicmp(fileStringPointer, kMainLuaFileName))
			{
				mainLuaFilePath = argv[1];
			}
			else
			{
				mainLuaFilePath = argv[1];
				mainLuaFilePath += L"\\";
				mainLuaFilePath += kMainLuaFileName;
			}
		}

		// Do not continue if argument doesn't reference a file or directory.
		if (mainLuaFilePath.empty() || !::PathFileExistsW(mainLuaFilePath.c_str()))
		{
			_fwprintf_p(stderr, L"Command line argument must reference a Corona project directory or \"main.lua\" file.");
			return EXIT_FAILURE;
		}
	}

	// Build a command line string to the Corona Simulator.
	std::wstring commandLineString;
	commandLineString = L'\"';
	commandLineString += applicationPath.c_str();
	commandLineString += L"Corona Simulator.exe\" -debug -singleton -no-console";
	if (!mainLuaFilePath.empty())
	{
		commandLineString += L" \"";
		commandLineString += mainLuaFilePath;
		commandLineString += L"\"";
	}
	utf16Buffer[0] = L'\0';
	wcsncpy_s(utf16Buffer, MAX_BUFFER_LENGTH, commandLineString.c_str(), _TRUNCATE);

	// Set up a pipe to be used to redirect the Corona Simulator's stdout to this app's stdout.
	HANDLE stdOutReadPipeHandle = nullptr;
	HANDLE stdOutWritePipeHandle = nullptr;
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

	// Start up the simulator in debug mode.
	PROCESS_INFORMATION processInfo{};
	STARTUPINFOW startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	if (stdOutWritePipeHandle)
	{
		startupInfo.hStdOutput = stdOutWritePipeHandle;
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;
	}
	auto wasProcessStarted = ::CreateProcessW(
			nullptr, utf16Buffer, nullptr, nullptr, TRUE,
			0, nullptr, nullptr, &startupInfo, &processInfo);
	if (!wasProcessStarted)
	{
		fprintf(stderr, "Failed to startup Corona Simulator at location...\r\n   ");
		_fwprintf_p(stderr, applicationPath.c_str());
		return EXIT_FAILURE;
	}

	// Close the stdout "write" pipe handle given to the launched app.
	// We only need the stdout "read" handle on this end.
	if (stdOutWritePipeHandle)
	{
		::CloseHandle(stdOutWritePipeHandle);
		stdOutWritePipeHandle = nullptr;
	}

	// Start reading the Corona Simulator's stdout on another thread.
	// Note: We must do this because the Lua debugger blocks the main thread.
	bool wasThreadExitRequested = false;
	std::thread stdOutThread(OnAsyncLogStdOut, &wasThreadExitRequested, stdOutReadPipeHandle);

	// Run the Lua debugger.
	// Note: This blocks until the Lua debugger returns out.
	int exitCode = run(argc, argv, &Rtt::luaload_controller, &initialize_luasocket);

	// Notify the stdout thread to exit out.
	// This will block until it's done logging all of the "applicationExit" messages.
	wasThreadExitRequested = true;
	stdOutThread.join();

	// Return the Lua debugger process' exit code.
	return exitCode;
}

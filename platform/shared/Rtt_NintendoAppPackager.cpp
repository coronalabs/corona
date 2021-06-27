//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_NintendoAppPackager.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_LuaLibSocket.h"
#include "Rtt_Archive.h"
#include "Rtt_FileSystem.h"
#include "Rtt_HTTPClient.h"
#include "Rtt_Time.h"

#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#if defined(Rtt_WIN_ENV) && !defined(Rtt_LINUX_ENV)
#include "Interop/Ipc/CommandLine.h"
#endif

Rtt_EXPORT int luaopen_lfs(lua_State* L);

extern "C" {
	int luaopen_socket_core(lua_State* L);
	int luaopen_mime_core(lua_State* L);
#ifdef Rtt_LINUX_ENV
	int luaopen_socket_core(lua_State* L);
	int CoronaPluginLuaLoad_ftp(lua_State* L);
	int CoronaPluginLuaLoad_socket(lua_State* L);
	int CoronaPluginLuaLoad_headers(lua_State* L);
	int CoronaPluginLuaLoad_http(lua_State* L);
	int CoronaPluginLuaLoad_mbox(lua_State* L);
	int CoronaPluginLuaLoad_smtp(lua_State* L);
	int CoronaPluginLuaLoad_tp(lua_State* L);
	int CoronaPluginLuaLoad_url(lua_State* L);
	int CoronaPluginLuaLoad_mime(lua_State* L);
	int CoronaPluginLuaLoad_ltn12(lua_State* L);
#endif
}

namespace Rtt
{
#ifndef Rtt_LINUX_ENV
	extern int luaload_luasocket_socket(lua_State* L);
	extern int luaload_luasocket_ftp(lua_State* L);
	extern int luaload_luasocket_headers(lua_State* L);
	extern int luaload_luasocket_http(lua_State* L);
	extern int luaload_luasocket_url(lua_State* L);
	extern int luaload_luasocket_mime(lua_State* L);
	extern int luaload_luasocket_ltn12(lua_State* L);
#endif

	bool CompileScriptsInDirectory(lua_State* L, AppPackagerParams& params, const char* dstDir, const char* srcDir);
	bool FetchDirectoryTreeFilePaths(const char* directoryPath, std::vector<std::string>& filePathCollection);

	int prn(lua_State* L);
	int CompileScriptsAndMakeCAR(lua_State* L);
	int processExecute(lua_State* L);

	// TODO: Move webPackageApp.lua out of librtt and into rtt_player in XCode
	// Current issue with doing that is this lua file needs to be precompiled into C
	// via custom build step --- all .lua files in librtt already do that, so we're
	// taking a shortcut for now by putting it under librtt.

	// webPackageApp.lua is pre-compiled into bytecodes and then placed in a byte array
	// constant in a generated .cpp file. The file also contains the definition of the 
	// following function which loads the bytecodes via luaL_loadbuffer.
	int luaload_nintendoPackageApp(lua_State* L);


	// ----------------------------------------------------------------------------

	//Returns the last error in string format. Returns an empty string if there is no error.
	const char* GetLastErrorAsString(unsigned long errorMessageID)
	{
		static char msg[1024];
		*msg = 0;
#ifdef _WIN32
		//Get the error message, if any.
		if (errorMessageID != 0)
		{
			size_t size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, sizeof(msg), NULL);
			msg[size] = 0;
		}
#else
		strncpy(msg, strerror(errno), sizeof(msg));
#endif
		return msg;
	}

	int processExecute2(lua_State* L)
	{
		std::string cmd = luaL_checkstring(L, 1);
#ifdef _WIN32
		cmd = "cmd.exe /c \"" + cmd + "\"";
#else
		cmd = cmd + "/bin/sh " + cmd;
#endif

		const std::string* input = NULL;
		std::string cmd_stdout;
		std::string* output = &cmd_stdout;
		const char* curdir = NULL;
		bool kill_on_exit = true;
		int timeout = 60000;

#if defined(__GNUC__) && !defined(__MINGW32__)

		int outfd[2];
		int infd[2];
		pipe(outfd); // Where the parent is going to write to 
		pipe(infd); // From where parent is going to read 

		// Write to childs stdin 
		if (input && input->size() > 0)
			write(outfd[1], input->c_str(), input->size());

		// eof for stdin
		close(outfd[1]);

		pid_t pid = fork();
		if (pid == 0)
		{
			// its child
			close(STDOUT_FILENO);
			close(STDIN_FILENO);
			dup2(outfd[0], STDIN_FILENO);
			dup2(infd[1], STDOUT_FILENO);

			// Not required for the child 
			close(outfd[0]);
			close(infd[0]);
			close(infd[1]);

			//The exec() functions only return if an error has occurred.
			int rc = execl(s_shell.c_str(), s_shell.c_str(), "-c", cmd.c_str(), NULL);

			// close process
			exit(rc);
		}
		else
		{
			if (wait_for_exit)
			{
				int fd = infd[0];
				int state = 0;

				// wait for child process, 10 sec
				uint32_t t = get_ticks();
				pid_t p = 0;
				while (get_ticks() - t < 10000 && (p = waitpid(pid, &state, WNOHANG)) == 0)
				{
					sleep(10);
				}

				if (p == 0)
				{
					// the script is working too long time, kill it
					kill(pid, 0);
				}

				// Read from childs stdout 
				if (output)
				{
					while (true)
					{
						//poll with no wait time
						struct pollfd fds { .fd = fd, .events = POLLIN };
						int res = poll(&fds, 1, 0);

						//POLLERR is set for some other errors, POLLNVAL is set if the pipe is closed
						if (res < 0 || fds.revents & (POLLERR | POLLNVAL))
							break;

						bool can_read = fds.revents & POLLIN;
						if (can_read == false)
							break;

						char buffer[1024];
						ssize_t inlen = read(fd, buffer, sizeof(buffer));

						string s = string(buffer, inlen);
						*output += s;
					}
				}
			}

			close(infd[0]);
			close(infd[1]);
			close(outfd[0]);
		}
		goto done;
#else

		HANDLE hPipeRead_stdin = NULL;
		HANDLE hPipeRead_stdout = NULL;
		HANDLE hPipeWrite_stdout = NULL;
		DWORD exit_code = -1;
		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFOA si = { sizeof(STARTUPINFOA) };

		if (input || output)
		{
			SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
			saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
			saAttr.lpSecurityDescriptor = NULL;

			// Create a pipe for the child process's STDOUT
			if (!CreatePipe(&hPipeRead_stdout, &hPipeWrite_stdout, &saAttr, 0))
			{
				if (output)
					*output = GetLastErrorAsString(GetLastError());

				goto done;
			}

			if (input && input->size() > 0)
			{
				HANDLE hPipeWrite_stdin = NULL;

				// Create a pipe for the child process's STDIN 
				if (!CreatePipe(&hPipeRead_stdin, &hPipeWrite_stdin, &saAttr, 0))
				{
					*output = GetLastErrorAsString(GetLastError());
					return -1;
				}

				// Ensure the write handle to the pipe for STDIN is not inherited. 
				if (!SetHandleInformation(hPipeWrite_stdin, HANDLE_FLAG_INHERIT, 0))
				{
					goto done;
				}

				// write content to the pipe for the child's STDIN
				DWORD dwWritten = 0;
				if (WriteFile(hPipeWrite_stdin, input->c_str(), input->size(), &dwWritten, NULL) == FALSE)
				{
					*output = GetLastErrorAsString(GetLastError());
					goto done;
				}

				// Close the pipe handle so the child process stops reading. 
				if (!CloseHandle(hPipeWrite_stdin))
				{
					*output = GetLastErrorAsString(GetLastError());
					goto done;
				}
			}
		}

		si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing, Requires STARTF_USESHOWWINDOW in dwFlags.
		si.dwFlags = STARTF_USESHOWWINDOW;
		if (input || output)
		{
			si.hStdOutput = hPipeWrite_stdout;
			si.hStdError = hPipeWrite_stdout;
			si.hStdInput = hPipeRead_stdin;
			si.dwFlags |= STARTF_USESTDHANDLES;
		}

		BOOL fSuccess = CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, curdir, &si, &pi);
		if (!fSuccess)
		{
			DWORD err = GetLastError();
			const char* msg = GetLastErrorAsString(err);
			Rtt_LogException("processExecute: %s, %s", cmd.c_str(), msg);
			if (output)
				*output = msg;

			goto done;
		}

		bool bProcessEnded = false;

		// wait for 60 sec 
		for (auto t = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()); Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()) - t < timeout && bProcessEnded == false;)
		{
			// Give some timeslice (50 ms), so we won't waste 100% CPU.
			bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

			// Even if process exited - we continue reading, if
			// there is some data available over pipe.
			for (;;)
			{
				char buf[1024];
				DWORD dwRead = 0;
				DWORD dwAvail = 0;

				if (!::PeekNamedPipe(hPipeRead_stdout, NULL, 0, NULL, &dwAvail, NULL))
					break;

				if (!dwAvail) // No data available, return
					break;

				if (!::ReadFile(hPipeRead_stdout, buf, min(sizeof(buf) - 1, (int)dwAvail), &dwRead, NULL) || !dwRead)
					// Error, the child process might ended
					break;

				buf[dwRead] = 0;
				
				if (output)
					*output += buf;
			}
		} //for

		GetExitCodeProcess(pi.hProcess, &exit_code);

	done:

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hPipeWrite_stdout);
		CloseHandle(hPipeRead_stdout);
		CloseHandle(hPipeRead_stdin);

		lua_pushinteger(L, exit_code);
		lua_pushstring(L, cmd_stdout.c_str());
		return 2;

#endif
	}

#define kDefaultNumBytes 128

	NintendoAppPackager::NintendoAppPackager(const MPlatformServices& services)
		: Super(services, TargetDevice::kNintendoPlatform)
	{
		lua_State* L = fVM;

#ifdef Rtt_LINUX_ENV
		Lua::RegisterModuleLoader(L, "lpeg", luaopen_lpeg);
		Lua::RegisterModuleLoader(L, "dkjson", Lua::Open< luaload_dkjson >);
		Lua::RegisterModuleLoader(L, "json", Lua::Open< luaload_json >);
		Lua::RegisterModuleLoader(L, "lfs", luaopen_lfs);
		Lua::RegisterModuleLoader(L, "socket.core", luaopen_socket_core);
		Lua::RegisterModuleLoader(L, "socket", Lua::Open< CoronaPluginLuaLoad_socket >);
		Lua::RegisterModuleLoader(L, "socket.ftp", Lua::Open< CoronaPluginLuaLoad_ftp >);
		Lua::RegisterModuleLoader(L, "socket.headers", Lua::Open< CoronaPluginLuaLoad_headers >);
		Lua::RegisterModuleLoader(L, "socket.http", Lua::Open< CoronaPluginLuaLoad_http >);
		Lua::RegisterModuleLoader(L, "socket.url", Lua::Open< CoronaPluginLuaLoad_url >);
		Lua::RegisterModuleLoader(L, "mime.core", luaopen_mime_core);
		Lua::RegisterModuleLoader(L, "mime", Lua::Open< CoronaPluginLuaLoad_mime >);
		Lua::RegisterModuleLoader(L, "ltn12", Lua::Open< CoronaPluginLuaLoad_ltn12 >);
#else
		Lua::RegisterModuleLoader(L, "lpeg", luaopen_lpeg);
		Lua::RegisterModuleLoader(L, "dkjson", Lua::Open< luaload_dkjson >);
		Lua::RegisterModuleLoader(L, "json", Lua::Open< luaload_json >);
		Lua::RegisterModuleLoader(L, "lfs", luaopen_lfs);
		Lua::RegisterModuleLoader(L, "socket.core", luaopen_socket_core);
		Lua::RegisterModuleLoader(L, "socket", Lua::Open< luaload_luasocket_socket >);
		Lua::RegisterModuleLoader(L, "socket.ftp", Lua::Open< luaload_luasocket_ftp >);
		Lua::RegisterModuleLoader(L, "socket.headers", Lua::Open< luaload_luasocket_headers >);
		Lua::RegisterModuleLoader(L, "socket.http", Lua::Open< luaload_luasocket_http >);
		Lua::RegisterModuleLoader(L, "socket.url", Lua::Open< luaload_luasocket_url >);
		Lua::RegisterModuleLoader(L, "mime.core", luaopen_mime_core);
		Lua::RegisterModuleLoader(L, "mime", Lua::Open< luaload_luasocket_mime >);
		Lua::RegisterModuleLoader(L, "ltn12", Lua::Open< luaload_luasocket_ltn12 >);
#endif

		HTTPClient::registerFetcherModuleLoaders(L);
		Lua::DoBuffer(fVM, &luaload_nintendoPackageApp, NULL);
	}

	NintendoAppPackager::~NintendoAppPackager()
	{
	}

	int NintendoAppPackager::Build(AppPackagerParams* _params, const char* tmpDirBase)
	{
		ReadBuildSettings(_params->GetSrcDir());
		if (fNeverStripDebugInfo)
		{
			Rtt_LogException("Note: debug info is not being stripped from application (settings.build.neverStripDebugInfo = true)\n");

			_params->SetStripDebug(false);
		}

		NintendoAppPackagerParams* params = (NintendoAppPackagerParams*)_params;
		Rtt_ASSERT(params);

		time_t startTime = time(NULL);

		bool useStandartResources = params->fUseStandartResources;

		const char tmpTemplate[] = "CLtmpXXXXXX";
		char tmpDir[kDefaultNumBytes]; Rtt_ASSERT(kDefaultNumBytes > (strlen(tmpDirBase) + strlen(tmpTemplate)));

		const char* lastChar = tmpDirBase + strlen(tmpDirBase) - 1;
		if (lastChar[0] == LUA_DIRSEP[0])
		{
			snprintf(tmpDir, kDefaultNumBytes, "%s%s", tmpDirBase, tmpTemplate);
		}
		else
		{
			snprintf(tmpDir, kDefaultNumBytes, "%s" LUA_DIRSEP "%s", tmpDirBase, tmpTemplate);
		}

		String debugBuildProcessPref;
		GetServices().GetPreference("debugBuildProcess", &debugBuildProcessPref);
		int debugBuildProcess = 3;
		if (!debugBuildProcessPref.IsEmpty())
		{
			debugBuildProcess = (int)strtol(debugBuildProcessPref.GetString(), (char**)NULL, 10);
		}


		// This is not as foolproof as mkdtemp() but has the advantage of working on Win32
		if (mkdir(mktemp(tmpDir)) == false)
		{
			// Note that the failing mkdir() that brought us here is a member of the AndroidAppPackager class
			String tmpString;
			tmpString.Set("NintendoAppPackager::Build: failed to create temporary directory\n\n");
			tmpString.Append(tmpDir);
			tmpString.Append("\n");

			Rtt_TRACE_SIM(("%s", tmpString.GetString()));
			params->SetBuildMessage(tmpString.GetString());
			return PlatformAppPackager::kLocalPackagingError;
		}

		lua_State* L = fVM;
		lua_getglobal(L, "nintendoPackageApp"); Rtt_ASSERT(lua_isfunction(L, -1));

		// params
		lua_newtable(L);
		{
			String resourceDir;
			const MPlatform& platform = GetServices().Platform();
			const char* platformName = fServices.Platform().GetDevice().GetPlatformName();

			platform.PathForFile(NULL, MPlatform::kSystemResourceDir, 0, resourceDir);

			lua_pushstring(L, tmpDir);
			lua_setfield(L, -2, "tmpDir");

			lua_pushstring(L, params->GetSrcDir());
			lua_setfield(L, -2, "srcDir");

			lua_pushstring(L, params->GetDstDir());
			lua_setfield(L, -2, "dstDir");

			lua_pushstring(L, params->GetAppName());
			lua_setfield(L, -2, "applicationName");

			lua_pushstring(L, params->GetVersion());
			lua_setfield(L, -2, "versionName");

			lua_pushstring(L, params->GetIdentity());
			lua_setfield(L, -2, "user");

			lua_pushboolean(L, useStandartResources);
			lua_setfield(L, -2, "useStandartResources");

			lua_pushinteger(L, Rtt_BUILD_YEAR);
			lua_setfield(L, -2, "buildYear");

			lua_pushinteger(L, Rtt_BUILD_REVISION);
			lua_setfield(L, -2, "buildRevision");

			lua_pushinteger(L, debugBuildProcess);
			lua_setfield(L, -2, "debugBuildProcess");

			lua_pushlightuserdata(L, (void*)params);		// keep for compileScriptsAndMakeCAR
			lua_setfield(L, -2, "nintendoParams");

			// needs to disable -fno-rtti
			const NintendoAppPackagerParams* nintendoParams = (NintendoAppPackagerParams*)params;

			Rtt_ASSERT(nintendoParams);
			String templateLocation(nintendoParams->fNintendoTemplate.GetString());
			if (templateLocation.IsEmpty())
			{
				fServices.Platform().PathForFile("nintendotemplate.zip", MPlatform::kSystemResourceDir, 0, templateLocation);
			}
			lua_pushstring(L, templateLocation.GetString());
			lua_setfield(L, -2, "templateLocation");

		}

#ifndef Rtt_NO_GUI
		lua_pushcfunction(L, Rtt::processExecute);
		lua_setglobal(L, "processExecute");
		lua_pushcfunction(L, Rtt::processExecute2);
		lua_setglobal(L, "processExecute2");
#endif
		lua_pushcfunction(L, Rtt::prn);
		lua_setglobal(L, "myprint");
		lua_pushcfunction(L, Rtt::CompileScriptsAndMakeCAR);
		lua_setglobal(L, "compileScriptsAndMakeCAR");

		int result = PlatformAppPackager::kNoError;

		// call nintendoPostPackage( params )
		if (!Rtt_VERIFY(0 == Lua::DoCall(L, 1, 1)))
		{
			result = PlatformAppPackager::kLocalPackagingError;
		}
		else
		{
			if (lua_isstring(L, -1))
			{
				result = PlatformAppPackager::kLocalPackagingError;
				const char* msg = lua_tostring(L, -1);
				Rtt_Log("%s\n", msg);
			}
			lua_pop(L, 1);
		}

		// Clean up intermediate files
		rmdir(tmpDir);

		return result;
	}

} // namespace Rtt


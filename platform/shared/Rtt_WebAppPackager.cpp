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

#include "Core/Rtt_Build.h"

#include "Rtt_WebAppPackager.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_WebServicesSession.h"
#include "Rtt_LuaLibSocket.h"
#include "Rtt_Archive.h"
#include "Rtt_FileSystem.h"

#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#if defined(Rtt_WIN_ENV) && !defined(Rtt_LINUX_ENV)
#include "Interop/Ipc/CommandLine.h"
#endif

Rtt_EXPORT int luaopen_lfs (lua_State *L);

extern "C" {
	int luaopen_socket_core(lua_State *L);
	int luaopen_mime_core(lua_State *L);
#ifdef Rtt_LINUX_ENV
	int luaopen_socket_core(lua_State *L);
	int CoronaPluginLuaLoad_ftp(lua_State *L);
	int CoronaPluginLuaLoad_socket(lua_State *L);
	int CoronaPluginLuaLoad_headers(lua_State *L);
	int CoronaPluginLuaLoad_http(lua_State *L);
	int CoronaPluginLuaLoad_mbox(lua_State *L);
	int CoronaPluginLuaLoad_smtp(lua_State *L);
	int CoronaPluginLuaLoad_tp(lua_State *L);
	int CoronaPluginLuaLoad_url(lua_State *L);
	int CoronaPluginLuaLoad_mime(lua_State *L);
	int CoronaPluginLuaLoad_ltn12(lua_State *L);
#endif
}

namespace Rtt
{
#ifndef Rtt_LINUX_ENV
	extern int luaload_luasocket_socket(lua_State *L);
	extern int luaload_luasocket_ftp(lua_State *L);
	extern int luaload_luasocket_headers(lua_State *L);
	extern int luaload_luasocket_http(lua_State *L);
	extern int luaload_luasocket_url(lua_State *L);
	extern int luaload_luasocket_mime(lua_State *L);
	extern int luaload_luasocket_ltn12(lua_State *L);
#endif

	bool CompileScriptsInDirectory( lua_State *L, AppPackagerParams& params, const char *dstDir, const char *srcDir );
	bool FetchDirectoryTreeFilePaths( const char* directoryPath, std::vector<std::string>& filePathCollection );

	int prn(lua_State *L)
	{
		int n = lua_gettop(L);  /* number of arguments */
		int i;
		lua_getglobal(L, "tostring");
		for (i=1; i<=n; i++) {
			const char *s;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  /* get result */
			if (s == NULL)
				return luaL_error(L, LUA_QL("tostring") " must return a string to "
														 LUA_QL("print"));
			if (i>1) printf("\t");
			printf("%s", s);
			lua_pop(L, 1);  /* pop result */
		}
		printf("\n");
		return 0;
	}

	// it's used only for Windows
	int processExecute(lua_State *L)
	{
		int ret = 0;
		const char* cmdBuf = luaL_checkstring(L, 1);

#if defined(Rtt_WIN_ENV) && !defined(Rtt_LINUX_ENV)
		Interop::Ipc::CommandLine::SetOutputCaptureEnabled(false);
		Interop::Ipc::CommandLineRunResult result = Interop::Ipc::CommandLine::RunShellCommandUntilExit(cmdBuf);
		if (result.HasFailed())
		{
			std::string output = result.GetOutput();
			Rtt_Log(output.c_str());
			ret = result.GetExitCode();
		}
#elif defined(Rtt_LINUX_ENV)
		int result = system(cmdBuf);
#endif

		lua_pushinteger(L, ret);
		return 1;
	}

	int CompileScriptsAndMakeCAR(lua_State *L)
	{
		Rtt_ASSERT(lua_isuserdata(L, 1)); 
		AppPackagerParams* p =  (AppPackagerParams*) lua_touserdata( L, 1);
		Rtt_ASSERT(lua_isstring(L, 2)); 
		const char* srcDir = lua_tostring( L, 2);
		Rtt_ASSERT(lua_isstring(L, 3)); 
		const char* dstDir = lua_tostring( L, 3);
		Rtt_ASSERT(lua_isstring(L, 4)); 
		const char* tmpDir = lua_tostring( L, 4);

		// Package build settings parameters.
		Rtt::AppPackagerParams params(p->GetAppName(), p->GetVersion(), p->GetIdentity(), NULL, srcDir, dstDir, NULL, p->GetTargetPlatform(), NULL,	0, 0, NULL, NULL, NULL, true);

		bool rc = CompileScriptsInDirectory(L, params, dstDir, srcDir);
		if (rc)
		{
			// Bundle all of the compiled Lua scripts in the intermediate directory into a "resource.car" file.
			String resourceCarPath(params.GetSrcDir());
			resourceCarPath.AppendPathSeparator();
			resourceCarPath.Append("resource.car");

			// create .car file

			// Fetch all file paths under the given source directory.
			std::vector<std::string> sourceFilePathCollection;
			bool wasSuccessful = FetchDirectoryTreeFilePaths(srcDir, sourceFilePathCollection);
			if (wasSuccessful && sourceFilePathCollection.empty() == false)
			{
				// Allocate enough space for ALL file paths to a string array.
				const char** sourceFilePathArray = new const char*[sourceFilePathCollection.size()];
				int fileToIncludeCount = 0;
				for (int fileIndex = (int)sourceFilePathCollection.size() - 1; fileIndex >= 0; fileIndex--)
				{
					const char* fileToInclude = sourceFilePathCollection.at(fileIndex).c_str();

					// pack only .lu files because all other files will be packed by html5 Lua builder
					int size = strlen(fileToInclude);
					if (size > 3 && strcmp(fileToInclude + size - 3, ".lu") == 0)
					{
						sourceFilePathArray[fileToIncludeCount++] = fileToInclude;
					}
				}

				// Create the "resource.car" archive file containing the files fetched up above.
				Archive::Serialize(resourceCarPath.GetString(), fileToIncludeCount, sourceFilePathArray);

				// Clean up memory allocated up above.
				delete[] sourceFilePathArray;

				// Return true if the archive file was successfully created.
				rc = Rtt_FileExists(resourceCarPath.GetString());
			}
		}

		lua_pushboolean(L, rc);
		return 1;
	}


// TODO: Move webPackageApp.lua out of librtt and into rtt_player in XCode
// Current issue with doing that is this lua file needs to be precompiled into C
// via custom build step --- all .lua files in librtt already do that, so we're
// taking a shortcut for now by putting it under librtt.

// webPackageApp.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in a generated .cpp file. The file also contains the definition of the 
// following function which loads the bytecodes via luaL_loadbuffer.
int luaload_webPackageApp(lua_State* L);


// ----------------------------------------------------------------------------

#define kDefaultNumBytes 128

WebAppPackager::WebAppPackager( const MPlatformServices& services )
:	Super( services, TargetDevice::kWebPlatform )
{
	lua_State *L = fVM;

#ifdef Rtt_LINUX_ENV
	Lua::RegisterModuleLoader( L, "lpeg", luaopen_lpeg );
	Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( L, "json", Lua::Open< luaload_json > );
	Lua::RegisterModuleLoader( L, "lfs", luaopen_lfs );
	Lua::RegisterModuleLoader( L, "socket.core", luaopen_socket_core );
	Lua::RegisterModuleLoader( L, "socket", Lua::Open< CoronaPluginLuaLoad_socket > );
	Lua::RegisterModuleLoader( L, "socket.ftp", Lua::Open< CoronaPluginLuaLoad_ftp > );
	Lua::RegisterModuleLoader( L, "socket.headers", Lua::Open< CoronaPluginLuaLoad_headers > );
	Lua::RegisterModuleLoader( L, "socket.http", Lua::Open< CoronaPluginLuaLoad_http > );
	Lua::RegisterModuleLoader( L, "socket.url", Lua::Open< CoronaPluginLuaLoad_url > );
	Lua::RegisterModuleLoader( L, "mime.core", luaopen_mime_core );
	Lua::RegisterModuleLoader( L, "mime", Lua::Open< CoronaPluginLuaLoad_mime > );
	Lua::RegisterModuleLoader( L, "ltn12", Lua::Open< CoronaPluginLuaLoad_ltn12 > );
#else
	Lua::RegisterModuleLoader( L, "lpeg", luaopen_lpeg );
	Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( L, "json", Lua::Open< luaload_json > );
	Lua::RegisterModuleLoader( L, "lfs", luaopen_lfs );
	Lua::RegisterModuleLoader( L, "socket.core", luaopen_socket_core );
	Lua::RegisterModuleLoader( L, "socket", Lua::Open< luaload_luasocket_socket > );
	Lua::RegisterModuleLoader( L, "socket.ftp", Lua::Open< luaload_luasocket_ftp > );
	Lua::RegisterModuleLoader( L, "socket.headers", Lua::Open< luaload_luasocket_headers > );
	Lua::RegisterModuleLoader( L, "socket.http", Lua::Open< luaload_luasocket_http > );
	Lua::RegisterModuleLoader( L, "socket.url", Lua::Open< luaload_luasocket_url > );
	Lua::RegisterModuleLoader( L, "mime.core", luaopen_mime_core );
	Lua::RegisterModuleLoader( L, "mime", Lua::Open< luaload_luasocket_mime > );
	Lua::RegisterModuleLoader( L, "ltn12", Lua::Open< luaload_luasocket_ltn12 > );
#endif

	Lua::DoBuffer( fVM, & luaload_webPackageApp, NULL );
}

WebAppPackager::~WebAppPackager()
{
}

int WebAppPackager::Build(AppPackagerParams* params, WebServicesSession& session, const char* tmpDirBase)
{
	// needs to disable -fno-rtti
	//const WebAppPackagerParams *webParams = dynamic_cast<WebAppPackagerParams*>(params);
	const WebAppPackagerParams *webParams = (WebAppPackagerParams*) params;
	Rtt_ASSERT(webParams);

	bool useStandartResources = webParams->useStandartResources;
	bool createFBInstantArchive = webParams->createFBInstantArchive;
	time_t startTime = time(NULL);

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
	int debugBuildProcess = 0;
	if (!debugBuildProcessPref.IsEmpty())
	{
		debugBuildProcess = (int) strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
	}


	// This is not as foolproof as mkdtemp() but has the advantage of working on Win32
	char* tmp = mktemp(tmpDir);
	if (Rtt_MakeDirectory(tmp) == false)
	{
		// Note that the failing mkdir() that brought us here is a member of the AndroidAppPackager class
		String tmpString;
		tmpString.Set("WebAppPackager::Build: failed to create temporary directory\n\n");
		tmpString.Append(tmpDir);
		tmpString.Append("\n");

		Rtt_TRACE_SIM(("%s", tmpString.GetString()));
		params->SetBuildMessage(tmpString.GetString());
		return WebServicesSession::kLocalPackagingError;
	}

	lua_State *L = fVM;
	lua_getglobal(L, "webPackageApp"); Rtt_ASSERT(lua_isfunction(L, -1));

	// params
	lua_newtable(L);
	{
		String resourceDir;
		const MPlatform& platform = GetServices().Platform();
		const char *platformName = fServices.Platform().GetDevice().GetPlatformName();

		platform.PathForFile(NULL, MPlatform::kSystemResourceDir, 0, resourceDir);

		lua_pushstring(L, tmpDir);
		lua_setfield(L, -2, "tmpDir");

		lua_pushstring(L, webParams->GetSrcDir());
		lua_setfield(L, -2, "srcDir");

		lua_pushstring(L, webParams->GetDstDir());
		lua_setfield(L, -2, "dstDir");

		lua_pushstring(L, webParams->GetAppName());
		lua_setfield(L, -2, "applicationName");

		lua_pushstring(L, webParams->GetVersion());
		lua_setfield(L, -2, "versionName");

		lua_pushstring(L, webParams->GetIdentity());
		lua_setfield(L, -2, "user");

		lua_pushinteger(L, Rtt_BUILD_YEAR);
		lua_setfield(L, -2, "buildYear");

		lua_pushinteger(L, Rtt_BUILD_REVISION);
		lua_setfield(L, -2, "buildRevision");

		lua_pushinteger(L, debugBuildProcess);
		lua_setfield(L, -2, "debugBuildProcess");

		lua_pushboolean(L, useStandartResources);
		lua_setfield(L, -2, "useStandartResources");

		lua_pushboolean(L, createFBInstantArchive);
		lua_setfield(L, -2, "createFBInstantArchive");

		lua_pushlightuserdata(L, (void*) webParams);		// keep for compileScriptsAndMakeCAR
		lua_setfield(L, -2, "webParams");

		String webtemplateLocation(webParams->webtemplate.GetString());
		if(webtemplateLocation.IsEmpty())
		{
			fServices.Platform().PathForFile( "webtemplate.zip", MPlatform::kSystemResourceDir, 0, webtemplateLocation );
		}
		lua_pushstring( L, webtemplateLocation.GetString() );
		lua_setfield( L, -2, "webtemplateLocation" );

	}

#ifndef Rtt_NO_GUI
	lua_pushcfunction(L, Rtt::processExecute);
	lua_setglobal(L, "processExecute");
#endif
	lua_pushcfunction(L, Rtt::prn);
	lua_setglobal(L, "myprint");
	lua_pushcfunction(L, Rtt::CompileScriptsAndMakeCAR);
	lua_setglobal(L, "compileScriptsAndMakeCAR");

	int result = WebServicesSession::kNoError;

	// call webPostPackage( params )
	if (!Rtt_VERIFY(0 == Lua::DoCall(L, 1, 1)))
	{
		result = WebServicesSession::kLocalPackagingError;
	}
	else
	{
		if (lua_isstring(L, -1))
		{
			result = WebServicesSession::kLocalPackagingError;
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


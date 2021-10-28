//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Assert.h"
#include "Rtt_LinuxAppPackager.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_LuaLibSocket.h"
#include "Rtt_Archive.h"
#include "Rtt_FileSystem.h"
#include "Rtt_HTTPClient.h"
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

Rtt_EXPORT int luaopen_lfs(lua_State *L);

extern "C" {
	int luaopen_socket_core(lua_State *L);
	int luaopen_mime_core(lua_State *L);
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
}

namespace Rtt
{
	bool CompileScriptsInDirectory(lua_State *L, AppPackagerParams &params, const char *dstDir, const char *srcDir);
	bool FetchDirectoryTreeFilePaths(const char* directoryPath, std::vector<std::string> &filePathCollection);
	int processExecute(lua_State *L);
	int luaload_linuxPackageApp(lua_State* L);

	int GenerateResourceCar(lua_State *L)
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
		params.SetStripDebug(p->IsStripDebug());

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

					// pack only .lu and .settings files
					int size = strlen(fileToInclude);

					if (size > 3 && strcmp(fileToInclude + size - 3, ".lu") == 0)
					{
						sourceFilePathArray[fileToIncludeCount++] = fileToInclude;
					}
					else if (size > 9 && strcmp(fileToInclude + size - 9, ".settings") == 0)
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

	static int luaPrint(lua_State *L)
	{
		const char *message = lua_tostring(L, 1);
		char buffer[(strlen(message) * sizeof(const char *)) + 100];
		sprintf(buffer, "%s\n", message);
		Rtt_Log(buffer);

		return 0;
	}

// ----------------------------------------------------------------------------

#define kDefaultNumBytes 128

	LinuxAppPackager::LinuxAppPackager(const MPlatformServices &services)
		:	Super(services, TargetDevice::kLinuxPlatform)
	{
		lua_State *L = fVM;
		Lua::RegisterModuleLoader(L, "lpeg", luaopen_lpeg);
		Lua::RegisterModuleLoader(L, "dkjson", Lua::Open<luaload_dkjson>);
		Lua::RegisterModuleLoader(L, "json", Lua::Open<luaload_json>);
		Lua::RegisterModuleLoader(L, "lfs", luaopen_lfs);
		Lua::RegisterModuleLoader(L, "socket.core", luaopen_socket_core);
		Lua::RegisterModuleLoader(L, "socket", Lua::Open<CoronaPluginLuaLoad_socket>);
		Lua::RegisterModuleLoader(L, "socket.ftp", Lua::Open<CoronaPluginLuaLoad_ftp>);
		Lua::RegisterModuleLoader(L, "socket.headers", Lua::Open<CoronaPluginLuaLoad_headers>);
		Lua::RegisterModuleLoader(L, "socket.http", Lua::Open<CoronaPluginLuaLoad_http>);
		Lua::RegisterModuleLoader(L, "socket.url", Lua::Open<CoronaPluginLuaLoad_url>);
		Lua::RegisterModuleLoader(L, "mime.core", luaopen_mime_core);
		Lua::RegisterModuleLoader(L, "mime", Lua::Open<CoronaPluginLuaLoad_mime>);
		Lua::RegisterModuleLoader(L, "ltn12", Lua::Open<CoronaPluginLuaLoad_ltn12>);
		HTTPClient::registerFetcherModuleLoaders(L);
		Lua::DoBuffer(fVM, & luaload_linuxPackageApp, NULL);
	}

	LinuxAppPackager::~LinuxAppPackager()
	{
	}

	int LinuxAppPackager::Build(AppPackagerParams *_params, const char * tmpDirBase)
	{
		LinuxAppPackagerParams *params = (LinuxAppPackagerParams *)_params;
		Rtt_ASSERT(params);
		bool useWidgetResources = params->fUseWidgetResources;
		bool runAfterBuild = params->fRunAfterBuild;
		bool onlyGetPlugins = params->fOnlyGetPlugins;
		const char *homeDir = NULL;
		struct passwd *pw = getpwuid(getuid());
		time_t startTime = time(NULL);
		int debugBuildProcess = 0;
		String debugBuildProcessPref;

		ReadBuildSettings(_params->GetSrcDir());

		if ((homeDir = getenv("HOME")) == NULL)
		{
			homeDir = getpwuid(getuid())->pw_dir;
		}

		if (fNeverStripDebugInfo && !onlyGetPlugins)
		{
			Rtt_LogException("Note: debug info is not being stripped from application (settings.build.neverStripDebugInfo = true)\n");
			_params->SetStripDebug(false);
		}

		std::string tmpDir(homeDir);
		tmpDir.append("/Documents/Solar2D Built Apps/").append(params->GetAppName());

		GetServices().GetPreference("debugBuildProcess", &debugBuildProcessPref);

		if (!debugBuildProcessPref.IsEmpty())
		{
			debugBuildProcess = (int)strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
		}

		lua_State *L = fVM;
		lua_getglobal(L, "linuxPackageApp");
		Rtt_ASSERT(lua_isfunction(L, -1));

		// params
		lua_newtable(L);
		{
			String resourceDir;
			const MPlatform& platform = GetServices().Platform();
			const char *platformName = fServices.Platform().GetDevice().GetPlatformName();
			platform.PathForFile(NULL, MPlatform::kSystemResourceDir, 0, resourceDir);

			lua_pushstring(L, tmpDir.c_str());
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

			lua_pushboolean(L, useWidgetResources);
			lua_setfield(L, -2, "useWidgetResources");

			lua_pushboolean(L, runAfterBuild);
			lua_setfield(L, -2, "runAfterBuild");

			lua_pushboolean(L, onlyGetPlugins);
			lua_setfield(L, -2, "onlyGetPlugins");

			lua_pushinteger(L, Rtt_BUILD_YEAR);
			lua_setfield(L, -2, "buildYear");

			lua_pushinteger(L, Rtt_BUILD_REVISION);
			lua_setfield(L, -2, "buildRevision");

			lua_pushinteger(L, debugBuildProcess);
			lua_setfield(L, -2, "debugBuildProcess");

			lua_pushlightuserdata(L, (void*) params); // keep for GenerateResourceCar
			lua_setfield(L, -2, "linuxParams");

			const LinuxAppPackagerParams* linuxParams = (LinuxAppPackagerParams*)params;

			Rtt_ASSERT(linuxParams);
			String templateLocation(linuxParams->fDebTemplate.GetString());

			if (templateLocation.IsEmpty() && !onlyGetPlugins)
			{
				fServices.Platform().PathForFile("template_x64.tgz", MPlatform::kSystemResourceDir, 0, templateLocation);
			}

			lua_pushstring(L, templateLocation.GetString());
			lua_setfield(L, -2, "templateLocation");
		}

#ifndef Rtt_NO_GUI
		lua_pushcfunction(L, Rtt::processExecute);
		lua_setglobal(L, "processExecute");
#endif
		lua_pushcfunction(L, HTTPClient::fetch);
		lua_setglobal(L, "_fetch");
		lua_pushcfunction(L, HTTPClient::download);
		lua_setglobal(L, "_download");
		lua_pushcfunction(L, luaPrint);
		lua_setglobal(L, "luaPrint");
		lua_pushcfunction(L, Rtt::GenerateResourceCar);
		lua_setglobal(L, "compileScriptsAndMakeCAR");

		int result = PlatformAppPackager::kNoError;

		// call linuxPostPackage( params )
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

		return result;
	}
} // namespace Rtt

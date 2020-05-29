//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_Win32AppPackager.h"
#include "Core\Rtt_Build.h"
#include "Interop\Ipc\CommandLine.h"
#include "Interop\ApplicationServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "rescle.h"
#include "Rtt_Archive.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_Win32AppPackagerParams.h"
#include "WinString.h"
#include <string>

#if defined( Rtt_NO_GUI )
#include "Rtt_WinConsolePlatform.h"
#endif

namespace Rtt
{

#pragma region Constructors/Destructors
Win32AppPackager::Win32AppPackager(const MPlatformServices& services)
:	PlatformAppPackager(services, TargetDevice::kWin32Platform)
{
}

Win32AppPackager::~Win32AppPackager()
{
}

#pragma endregion


#pragma region Public Methods
int Win32AppPackager::Build(
	AppPackagerParams* paramsPointer, const char* tempDirectoryPath)
{
	// Validate arguments.
	if (!paramsPointer)
	{
		Rtt_ASSERT(0);
		return 2;
	}
	auto win32ParamsPointer = dynamic_cast<Rtt::Win32AppPackagerParams*>(paramsPointer);
	if (!win32ParamsPointer ||
	    Rtt_StringIsEmpty(tempDirectoryPath) ||
	    Rtt_StringIsEmpty(paramsPointer->GetAppName()) ||
	    Rtt_StringIsEmpty(paramsPointer->GetSrcDir()) ||
	    Rtt_StringIsEmpty(paramsPointer->GetDstDir()) ||
	    Rtt_StringIsEmpty(win32ParamsPointer->GetExeFileName()))
	{
		paramsPointer->SetBuildMessage("Build() was given invalid parameters.");
		Rtt_ASSERT(0);
		return 2;
	}
	
	// Read the Corona project's "build.settings" file. This does the following:
	// - Fetches the "netwverStripDebugInfo" setting. Must be fetched before compiling Lua files.
	// - Fetches the "excludeFiles" patterns. Needed to determine if any project assets should be excluded from build.
	fExcludeFilePatterns.clear();
	ReadBuildSettings(paramsPointer->GetSrcDir());

	// Clean the given temp directory if it already exists.
	if (Rtt_FileExists(tempDirectoryPath))
	{
		rmdir(tempDirectoryPath);
	}

	// Fetch the destination directory path. Ensure it does not end with a trailing slash.
	WinString destinationDirectoryPath;
	destinationDirectoryPath.SetUTF8(paramsPointer->GetDstDir());
	destinationDirectoryPath.TrimEnd(L"\\/");

	// Clean the destination directory in case we've built there before.
	if (::PathIsDirectoryW(destinationDirectoryPath.GetUTF16()))
	{
		// Before attempting to delete the destination directory, check if any of its files are currently locked.
		// The simplest way to check is to see if we can rename/move the directory successfully.
		WinString rootTempDirectoryPath(tempDirectoryPath);
		WinString movedDestinationDirectoryPath;
		if (ArePathsOnSameVolume(destinationDirectoryPath.GetUTF16(), rootTempDirectoryPath.GetUTF16()))
		{
			movedDestinationDirectoryPath.SetUTF16(rootTempDirectoryPath.GetUTF16());
			movedDestinationDirectoryPath.Append(L"\\LastBuiltApp");
			::SHCreateDirectoryExW(nullptr, rootTempDirectoryPath.GetUTF16(), nullptr);
		}
		else
		{
			movedDestinationDirectoryPath.SetUTF16(destinationDirectoryPath.GetUTF16());
			movedDestinationDirectoryPath.Append(L".Backup");
		}
		BOOL wasRenamed = ::MoveFileExW(
				destinationDirectoryPath.GetUTF16(), movedDestinationDirectoryPath.GetUTF16(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		if (!wasRenamed)
		{
			// Failed to move/rename the destination directory. This means we cannot build to it because files are locked.
			// Generate an error message explaining why with advise that tells the user how to resolve it.
			std::string message("Unable to overwrite destination directory:\r\n   ");
			message.append(destinationDirectoryPath.GetUTF8());
			auto errorCode = ::GetLastError();
			if (errorCode)
			{
				LPWSTR utf16Buffer = nullptr;
				::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						nullptr, errorCode,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&utf16Buffer, 0, nullptr);
				if (utf16Buffer)
				{
					WinString stringConverter;
					stringConverter.SetUTF16(utf16Buffer);
					message.append("\r\n\r\nReason:\r\n   ");
					message.append(stringConverter.GetUTF8());
					::LocalFree(utf16Buffer);
				}
				switch (errorCode)
				{
					case ERROR_ACCESS_DENIED:
					case ERROR_SHARING_VIOLATION:
					case ERROR_LOCK_VIOLATION:
						message.append(
								"\r\n\r\nThe files in this directory might be locked. Please do the following:\r\n"
								"- Close any open files in this directory.\r\n"
								"- Close any Windows Explorer windows showing this directory.");
						break;
				}
			}
			paramsPointer->SetBuildMessage(message.c_str());
			if (::PathIsDirectoryW(rootTempDirectoryPath.GetUTF16()))
			{
				// Delete the generated temp directory.
				rmdir(rootTempDirectoryPath.GetUTF8());
			}
			return 3;
		}

		// The destination directory's files are not locked. Delete the directory.
		rmdir(movedDestinationDirectoryPath.GetUTF8());
	}
	else if (::PathFileExistsW(destinationDirectoryPath.GetUTF16()))
	{
		// The destination directory has a file with the same name. Delete it.
		::DeleteFileW(destinationDirectoryPath.GetUTF16());
	}

	// Create an intermediate directory to compile scripts to.
	WinString objDirectoryPath;
	objDirectoryPath.SetUTF8(tempDirectoryPath);
	objDirectoryPath.Append("\\obj");
	::SHCreateDirectoryExW(nullptr, objDirectoryPath.GetUTF16(), nullptr);

	// Create a "Bin" directory to copy the binaries to.
	WinString binDirectoryPath;
	binDirectoryPath.SetUTF8(tempDirectoryPath);
	binDirectoryPath.Append("\\Bin");
	::SHCreateDirectoryExW(nullptr, binDirectoryPath.GetUTF16(), nullptr);

	// Create a "Bin\Resources" directory to copy the Corona project's assets to.
	WinString binResourcesDirectoryPath;
	binResourcesDirectoryPath.SetUTF16(binDirectoryPath.GetUTF16());
	binResourcesDirectoryPath.Append("\\Resources");
	::SHCreateDirectoryExW(nullptr, binResourcesDirectoryPath.GetUTF16(), nullptr);

	// Compile Lua scripts and copy the Win32 binaries to the "bin" directory.
	int buildResultCode = 5;
	Win32AppPackager::BuildSettings buildSettings{};
	buildSettings.ParamsPointer = win32ParamsPointer;
	buildSettings.IntermediateDirectoryPath = objDirectoryPath.GetUTF8();
	buildSettings.BinDirectoryPath = binDirectoryPath.GetUTF8();
#if 1
	buildResultCode = DoLocalBuild(buildSettings);
#else
	//TODO: In the future, support cloud builds via the unfinished DoRemoteBuild() method.
	buildResultCode = DoRemoteBuild(buildSettings, webSession);
#endif
	if (buildResultCode != 0)
	{
		rmdir(tempDirectoryPath);
		return buildResultCode;
	}

	// Fetch the EXE file name. Make sure it it conforms to the following:
	// 1) Ends with a *.exe extension.
	// 2) Uses valid Windows file system characters.
	std::wstring utf16ExeFileName;
	{
		// Replace invalid file system characters in the EXE file name.
		Rtt::String utf8EscapedExeFileName;
		PlatformAppPackager::EscapeFileName(win32ParamsPointer->GetExeFileName(), utf8EscapedExeFileName, false);
		WinString exeFileName;
		if (utf8EscapedExeFileName.IsEmpty())
		{
			exeFileName.SetUTF8(win32ParamsPointer->GetExeFileName());
		}
		else
		{
			exeFileName.SetUTF8(utf8EscapedExeFileName.GetString());
		}

		// Make sure the file name ends with a *.exe extension.
		if (Rtt_StringEndsWithNoCase(exeFileName.GetUTF8(), ".exe") == false)
		{
			exeFileName.Append(".exe");
		}
		utf16ExeFileName = exeFileName.GetUTF16();
	}

	// Rename the app template's EXE to what the user specified.
	std::wstring utf16ExeFilePath(binDirectoryPath.GetUTF16());
	utf16ExeFilePath.append(L"\\");
	utf16ExeFilePath.append(utf16ExeFileName);
	{
		std::wstring utf16TemplateExeFilePath(binDirectoryPath.GetUTF16());
		utf16TemplateExeFilePath.append(L"\\Corona.App.exe");
		BOOL wasRenamed = ::MoveFileW(utf16TemplateExeFilePath.c_str(), utf16ExeFilePath.c_str());
		if (!wasRenamed)
		{
			utf16ExeFilePath = utf16TemplateExeFilePath;
		}
	}

	// Update the EXE file's app icon and version information.
	{
		rescle::ResourceUpdater exeUpdater;
		bool wasExeLoaded = exeUpdater.Load(utf16ExeFilePath.c_str());
		if (!wasExeLoaded)
		{
			paramsPointer->SetBuildMessage("Failed to load Win32 app template's EXE file.");
			rmdir(tempDirectoryPath);
			return 2;
		}
		{
			WinString appIconPath;
			appIconPath.SetUTF8(paramsPointer->GetSrcDir());
			appIconPath.Append("\\Icon-win32.ico");
			if (::PathFileExistsW(appIconPath.GetUTF16()))
			{
				exeUpdater.SetIcon(appIconPath.GetUTF16());
			}
		}
		if (Rtt_StringIsEmpty(paramsPointer->GetVersion()) == false)
		{
			WinString versionString;
			versionString.SetUTF8(paramsPointer->GetVersion());
			exeUpdater.SetVersionString(L"ProductVersion", versionString.GetUTF16());
			exeUpdater.SetVersionString(L"FileVersion", versionString.GetUTF16());
			try
			{
				int majorNumber = 0;
				int minorNumber = 0;
				int buildNumber = 0;
				int specialNumber = 0;
				swscanf_s(
						versionString.GetUTF16(), L"%d.%d.%d.%d",
						&majorNumber, &minorNumber, &buildNumber, &specialNumber);
				exeUpdater.SetProductVersion(
						(WORD)majorNumber, (WORD)minorNumber, (WORD)buildNumber, (WORD)specialNumber);
				exeUpdater.SetFileVersion(
						(WORD)majorNumber, (WORD)minorNumber, (WORD)buildNumber, (WORD)specialNumber);
			}
			catch (...) {}
		}
		{
			WinString appName;
			appName.SetUTF8(paramsPointer->GetAppName());
			exeUpdater.SetVersionString(L"ProductName", appName.GetUTF16());
			exeUpdater.SetVersionString(L"InternalName", appName.GetUTF16());
		}
		if (Rtt_StringIsEmpty(win32ParamsPointer->GetAppDescription()) == false)
		{
			WinString appDescription;
			appDescription.SetUTF8(win32ParamsPointer->GetAppDescription());
			exeUpdater.SetVersionString(L"FileDescription", appDescription.GetUTF16());
		}
		if (Rtt_StringIsEmpty(win32ParamsPointer->GetCompanyName()) == false)
		{
			WinString companyName;
			companyName.SetUTF8(win32ParamsPointer->GetCompanyName());
			exeUpdater.SetVersionString(L"CompanyName", companyName.GetUTF16());
		}
		if (Rtt_StringIsEmpty(win32ParamsPointer->GetCopyrightString()) == false)
		{
			WinString copyrightString;
			copyrightString.SetUTF8(win32ParamsPointer->GetCopyrightString());
			exeUpdater.SetVersionString(L"LegalCopyright", copyrightString.GetUTF16());
		}
		exeUpdater.SetVersionString(L"OriginalFilename", utf16ExeFileName.c_str());
		exeUpdater.Commit();
	}

	// Copy the Corona project's assets to the "Bin\Resources" directory.
	bool hasCopiedAssets = false;
	{
		LightPtrArray<const char> excludeFilePatternArray(&fServices.Platform().GetAllocator());
		excludeFilePatternArray.Append("*.lua");
		excludeFilePatternArray.Append("*.lu");
		excludeFilePatternArray.Append("build.settings");
		for (auto iter = fExcludeFilePatterns.begin(); iter != fExcludeFilePatterns.end(); iter++)
		{
			excludeFilePatternArray.Append((*iter).c_str());
		}
		CopyDirectoryTreeSettings copySettings{};
		copySettings.ParamsPointer = paramsPointer;
		copySettings.SourceDirectoryPath = paramsPointer->GetSrcDir();
		copySettings.DestinationDirectoryPath = binResourcesDirectoryPath.GetUTF8();
		copySettings.ExcludeFilePatternArray = &excludeFilePatternArray;
		hasCopiedAssets = CopyDirectoryTree(copySettings);
	}
	if (!hasCopiedAssets)
	{
		return 3;
	}

	// First, attempt to move the built Win32 app folder to the destination directory.
	// Note: We can only do this reliably if both directories are on the same volume/drive.
	BOOL wasMoved = FALSE;
	if (ArePathsOnSameVolume(binDirectoryPath.GetUTF16(), destinationDirectoryPath.GetUTF16()))
	{
		wasMoved = ::MoveFileExW(
				binDirectoryPath.GetUTF16(), destinationDirectoryPath.GetUTF16(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	}

	// Copy the directory tree if we're unable to move the directory to the destination. (Worst performance.)
	// We'll do this if the destination is on a different drive/volume or if the above move failed for some reason.
	if (!wasMoved)
	{
		CopyDirectoryTreeSettings copySettings{};
		copySettings.ParamsPointer = paramsPointer;
		copySettings.SourceDirectoryPath = binDirectoryPath.GetUTF8();
		copySettings.DestinationDirectoryPath = destinationDirectoryPath.GetUTF8();
		wasMoved = CopyDirectoryTree(copySettings) ? true : false;
	}
	buildResultCode = wasMoved ? 0 : 3;

	// Update the system's icon cache.
	// Note: We need to do this in case the app icon was changed for the same EXE file name.
	//       Otherwise, Windows Explorer and the desktop will show the old/cached EXE icon instead.
	DWORD_PTR sendMessageResult;
	::SystemParametersInfoW(SPI_SETICONS, 0, nullptr, SPIF_SENDCHANGE);
	::SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0, SMTO_ABORTIFHUNG, 5000, &sendMessageResult);
	::SHChangeNotify(SHCNE_ASSOCCHANGED, 0, nullptr, nullptr);

	// Delete the build's temp directory.
	rmdir(tempDirectoryPath);

	// Return the build result code.
	// If we return an error, then the package params will provide a reason why via its GetBuildMessage() function.
	return buildResultCode;
}

#pragma endregion


#pragma region Protected Methods
int Win32AppPackager::DoLocalBuild(const Win32AppPackager::BuildSettings& buildSettings)
{
	// Validate arguments.
	if (!buildSettings.ParamsPointer)
	{
		Rtt_ASSERT(0);
		return 2;
	}
	if (!Rtt_FileExists(buildSettings.IntermediateDirectoryPath) || !Rtt_FileExists(buildSettings.BinDirectoryPath))
	{
		buildSettings.ParamsPointer->SetBuildMessage("DoLocalBuild() was given invalid parameters.");
		Rtt_ASSERT(0);
		return 2;
	}

	// Get a UTF-16 path to the bin directory.
	WinString binDirectoryPath;
	binDirectoryPath.SetUTF8(buildSettings.BinDirectoryPath);

#if !defined( Rtt_NO_GUI )
	// Extract the project's plugins to the "bin" directory.
	// This must be done before extracting the Win32 app template so that plugins can't overwrite Corona's DLLs.
	auto runtimePointer = buildSettings.ParamsPointer->GetRuntime();
	if (runtimePointer && runtimePointer->RequiresDownloadablePlugins())
	{
		// Unzip the project's plugins to an intermediate directory.
		WinString intermediatePluginDirectoryPath(buildSettings.IntermediateDirectoryPath);
		intermediatePluginDirectoryPath.Append(L"\\..\\Plugins");
		bool wasUnzipped = UnzipPlugins(
				buildSettings.ParamsPointer, buildSettings.ParamsPointer->GetRuntime(),
				intermediatePluginDirectoryPath.GetUTF8());
		if (!wasUnzipped)
		{
			return 3;
		}

		// Compile the Lua plugins to the intermediate directory.
		// Note: Precompiled *.lu files are copied to the given directory by the below function.
		Win32AppPackagerParams::CoreSettings pluginParamsSettings{};
		pluginParamsSettings.AppName = buildSettings.ParamsPointer->GetAppName();
		pluginParamsSettings.DestinationDirectoryPath = buildSettings.BinDirectoryPath;
		pluginParamsSettings.SourceDirectoryPath = intermediatePluginDirectoryPath.GetUTF8();
		pluginParamsSettings.VersionString = buildSettings.ParamsPointer->GetVersion();
		Win32AppPackagerParams pluginParams(pluginParamsSettings);
		bool wasCompiled = CompileScripts(&pluginParams, buildSettings.IntermediateDirectoryPath);
		if (!wasCompiled)
		{
			if (Rtt_StringIsEmpty(buildSettings.ParamsPointer->GetBuildMessage()))
			{
				buildSettings.ParamsPointer->SetBuildMessage("Failed to compile plugin Lua scripts.");
			}
			return 5;
		}

		// Copy the DLL plugins to the "bin" directory.
		std::wstring pluginPatternMatch(intermediatePluginDirectoryPath.GetUTF16());
		pluginPatternMatch.append(L"\\*.*");
		WIN32_FIND_DATAW findData{};
		auto searchHandle = ::FindFirstFileW(pluginPatternMatch.c_str(), &findData);
		if (searchHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				BOOL wasMoved = false;

				if (_wcsicmp(findData.cFileName, L"plugin") == 0)
				{
					// Handle Lua plugins which are in the "plugin" directory tree.  All we need are the assets as
					// the code is incorporated into resource.car.  This is done by moving the entire "plugin" tree 
					// and then deleting any .lua or .lu files in it

					std::wstring utf16SourceFilePath(intermediatePluginDirectoryPath.GetUTF16());
					utf16SourceFilePath.append(L"\\");
					utf16SourceFilePath.append(findData.cFileName);
					std::wstring utf16DestinationFilePath(binDirectoryPath.GetUTF16());
					utf16DestinationFilePath.append(L"\\corona-plugins");
					CreateDirectoryW(utf16DestinationFilePath.c_str(), NULL);  // create the \corona-plugins directory if needed
					utf16DestinationFilePath.append(L"\\plugin");
					wasMoved = ::MoveFileW(utf16SourceFilePath.c_str(), utf16DestinationFilePath.c_str());

					// Remove .lua and .lu files, we just want the assets
					std::wstring commandLine(L"cmd /c del /s /q \"");
					commandLine.append(utf16DestinationFilePath.c_str());
					commandLine.append(L"\\*.lua\"");
					commandLine.append(L" \"");
					commandLine.append(utf16DestinationFilePath.c_str());
					commandLine.append(L"\\*.lu\"");

					Interop::Ipc::CommandLine::RunUntilExit(commandLine.c_str());
				}
				else
				{
					// It's a native plugin file

					// Determine if the next plugin file's extension should be copied to the "bin" directory.
					// This excludes Lua files and other files that might have accidentally been zipped up with the plugin.
					const wchar_t *kUtf16AllowedExtensions[] = { L".dll", L".exe", L".manifest", nullptr };
					bool shouldCopy = false;
					auto fileNameLength = wcslen(findData.cFileName);
					for (int index = 0; kUtf16AllowedExtensions[index]; index++)
					{
						auto allowedExtensionLength = wcslen(kUtf16AllowedExtensions[index]);
						if (fileNameLength >= allowedExtensionLength)
						{
							auto offset = fileNameLength - allowedExtensionLength;
							if (_wcsicmp(findData.cFileName + offset, kUtf16AllowedExtensions[index]) == 0)
							{
								shouldCopy = true;
								break;
							}
						}
					}
					if (!shouldCopy)
					{
						continue;
					}

					// Copy the DLL plugin related file.
					std::wstring utf16SourceFilePath(intermediatePluginDirectoryPath.GetUTF16());
					utf16SourceFilePath.append(L"\\");
					utf16SourceFilePath.append(findData.cFileName);
					std::wstring utf16DestinationFilePath(binDirectoryPath.GetUTF16());
					utf16DestinationFilePath.append(L"\\");
					utf16DestinationFilePath.append(findData.cFileName);
					wasMoved = ::MoveFileW(utf16SourceFilePath.c_str(), utf16DestinationFilePath.c_str());
				}

				if (!wasMoved)
				{
					std::string message("Failed to include plugin files.");
					auto errorCode = ::GetLastError();
					if (errorCode)
					{
						LPWSTR utf16Buffer = nullptr;
						::FormatMessageW(
							FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							nullptr, errorCode,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPWSTR)&utf16Buffer, 0, nullptr);
						if (utf16Buffer)
						{
							WinString stringConverter;
							stringConverter.SetUTF16(utf16Buffer);
							message.append("\r\n\r\nReason:\r\n   ");
							message.append(stringConverter.GetUTF8());
							::LocalFree(utf16Buffer);
						}
					}
					buildSettings.ParamsPointer->SetBuildMessage(message.c_str());
					return 3;
				}
			} while (::FindNextFileW(searchHandle, &findData));
		}
	}
#endif // ! Rtt_NO_GUI

	// Unzip the Win32 app template to the "bin" directory.
	// This app template contains the pre-compiled exe, libraries, widget assets, etc.
	{
		// Build a UTF-16 path to the Win32 app template zip file.
		std::wstring zipFilePath;
#ifdef Rtt_NO_GUI
		zipFilePath.append(WinConsolePlatform::GetDirectoryPath());
#else
		zipFilePath.append(Interop::ApplicationServices::GetDirectoryPath());
#endif

		zipFilePath.append(L"\\Resources\\AppTemplates\\Win32\\Template.zip");

		// Unzip the app template.
		bool wasUnzipped = UnzipFile(zipFilePath.c_str(), binDirectoryPath.GetUTF16());
		if (!wasUnzipped)
		{
			buildSettings.ParamsPointer->SetBuildMessage("Failed to unzip Win32 app template.");
			Rtt_ASSERT(0);
			return 3;
		}
	}

	// Compile the Corona project's Lua scripts to the intermediate directory.
	bool wasCompiled = CompileScripts(buildSettings.ParamsPointer, buildSettings.IntermediateDirectoryPath);
	if (!wasCompiled)
	{
		if (Rtt_StringIsEmpty(buildSettings.ParamsPointer->GetBuildMessage()))
		{
			buildSettings.ParamsPointer->SetBuildMessage("Failed to compile Lua scripts.");
		}
		return 5;
	}

	// Bundle all of the compiled Lua scripts in the intermediate directory into a "resource.car" file.
	std::string resourceCarPath(buildSettings.BinDirectoryPath);
	resourceCarPath.append("\\Resources\\resource.car");
	bool wasArchived = ArchiveDirectoryTree(
			buildSettings.ParamsPointer, buildSettings.IntermediateDirectoryPath, resourceCarPath.c_str());
	if (!wasArchived)
	{
		if (Rtt_StringIsEmpty(buildSettings.ParamsPointer->GetBuildMessage()))
		{
			buildSettings.ParamsPointer->SetBuildMessage(
					"Failed to package compiled Lua scripts into a \"resource.car\" file.");
		}
		return 3;
	}

	// Return the build result code.
	return 0;
}

int Win32AppPackager::DoRemoteBuild(
	const Win32AppPackager::BuildSettings& buildSettings)
{
	Rtt_ASSERT_NOT_IMPLEMENTED();

	// Validate arguments.
	if (!buildSettings.ParamsPointer)
	{
		Rtt_ASSERT(0);
		return 2;
	}
	if (!Rtt_FileExists(buildSettings.IntermediateDirectoryPath) || !Rtt_FileExists(buildSettings.BinDirectoryPath))
	{
		buildSettings.ParamsPointer->SetBuildMessage("DoRemoteBuild() was given invalid parameters.");
		Rtt_ASSERT(0);
		return 2;
	}

	// Locally compile the Corona project's Lua scripts and bundle them into an "input.zip" file.
	auto inputZipFilePath = Prepackage(buildSettings.ParamsPointer, buildSettings.IntermediateDirectoryPath);
	if (!inputZipFilePath)
	{
		return 3;
	}

//TODO: Add remote build code here.
#if 1
	buildSettings.ParamsPointer->SetBuildMessage("Remote Win32 desktop app builds are not currently supported.");
	int buildResultCode = 5;
#endif

	// Delete the "input.zip" string returned by the Prepackage() method.
	free(inputZipFilePath);

	// Return the build result code.
	return buildResultCode;
}

void Win32AppPackager::OnReadingBuildSettings(lua_State* luaStatePointer, int index)
{
	// Validate.
	if (!luaStatePointer)
	{
		return;
	}

	// Clear the last read settings.
	fExcludeFilePatterns.clear();

	// Fetch the "excludeFiles" patterns.
	lua_getfield(luaStatePointer, index, "excludeFiles");
	if (lua_istable(luaStatePointer, -1))
	{
		// Fetch the patterns from the "all" array.
		lua_getfield(luaStatePointer, -1, "all");
		if (lua_istable(luaStatePointer, -1))
		{
			int luaTableIndex = lua_gettop(luaStatePointer);
			for (lua_pushnil(luaStatePointer); lua_next(luaStatePointer, luaTableIndex); lua_pop(luaStatePointer, 1))
			{
				if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
				{
					const char* stringPointer = lua_tostring(luaStatePointer, -1);
					if (Rtt_StringIsEmpty(stringPointer) == false)
					{
						fExcludeFilePatterns.push_back(std::string(stringPointer));
					}
				}
			}
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the patterns from the "win32" array.
		lua_getfield(luaStatePointer, -1, "win32");
		if (lua_istable(luaStatePointer, -1))
		{
			int luaTableIndex = lua_gettop(luaStatePointer);
			for (lua_pushnil(luaStatePointer); lua_next(luaStatePointer, luaTableIndex); lua_pop(luaStatePointer, 1))
			{
				if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
				{
					const char* stringPointer = lua_tostring(luaStatePointer, -1);
					if (Rtt_StringIsEmpty(stringPointer) == false)
					{
						fExcludeFilePatterns.push_back(std::string(stringPointer));
					}
				}
			}
		}
		lua_pop(luaStatePointer, 1);
	}
	lua_pop(luaStatePointer, 1);
}

bool Win32AppPackager::UnzipFile(const wchar_t* zipFilePath, const wchar_t* destinationPath)
{
	// Validate.
	if (!zipFilePath || (L'\0' == zipFilePath[0]) || !destinationPath || (L'\0' == destinationPath[0]))
	{
		return false;
	}

	// Unzip the file using the "7za.exe" tool installed under the "Corona" directory.
	std::wstring commandLine(L"\"");
#ifdef Rtt_NO_GUI
	commandLine.append(WinConsolePlatform::GetDirectoryPath());
#else
	commandLine.append(Interop::ApplicationServices::GetDirectoryPath());
#endif
	commandLine.append(L"\\7za.exe\" x -y \"");
	commandLine.append(zipFilePath);
	commandLine.append(L"\" -o\"");
	commandLine.append(destinationPath);
	commandLine.append(L"\"");
#ifdef Rtt_NO_GUI
	return (WinConsolePlatform::RunSystemCommand(commandLine) == 0);
#else
	return (Interop::Ipc::CommandLine::RunUntilExit(commandLine.c_str()).GetExitCode() == 0);
#endif
}

bool Win32AppPackager::ArePathsOnSameVolume(const wchar_t* path1, const wchar_t* path2)
{
	// Validate arguments.
	if (!path1 || !path2 || (L'\0' == path1[0]) || (L'\0' == path2[0]))
	{
		return false;
	}

	// Fetch the drive numbers of each directory path.
	int path1DriveNumber = ::PathGetDriveNumberW(path1);
	int path2DriveNumber = ::PathGetDriveNumberW(path2);

	// If the drive number fetched above is -1, then the given path is relative.
	// This would mean its relative to this app's path, meaning its using this app's drive number.
#ifdef Rtt_NO_GUI
	int appDriveNumber = ::PathGetDriveNumberW(WinConsolePlatform::GetDirectoryPath().c_str());
#else
	int appDriveNumber = ::PathGetDriveNumberW(Interop::ApplicationServices::GetDirectoryPath());
#endif
	if (path1DriveNumber < 0)
	{
		path1DriveNumber = appDriveNumber;
	}
	if (path2DriveNumber < 0)
	{
		path2DriveNumber = appDriveNumber;
	}

	// Check if both paths are on the same drive/volume.
	return (path1DriveNumber == path2DriveNumber);
}

#pragma endregion

} // namespace Rtt

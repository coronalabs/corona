//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AndroidAppPackager.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_TargetAndroidAppStore.h"
#include "Rtt_FileSystem.h"
#include "Rtt_JavaHost.h"
#include "Rtt_DeviceBuildData.h"

#include "ListKeyStore.h"

#include <string>

#if defined(Rtt_WIN_ENV) && !defined(Rtt_LINUX_ENV)
#include "stdafx.h"
#include "WinString.h"
#include "Interop/Ipc/CommandLine.h"
#if !defined( Rtt_NO_GUI )
#include "Simulator.h"
#endif
#endif

#if USE_JNI
#include "AntHost.h"
#endif

extern "C"
{
#	include "lfs.h"
}


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#define kDefaultNumBytes 1024

// This is set on the server. This also doesn't belong here.
#define MAX_PERMISSIONS 9

AndroidAppPackagerParams::AndroidAppPackagerParams(
	const char* appName,
	const char* versionName,
	const char* identity,
	const char* provisionFile,
	const char* srcDir,
	const char* dstDir,
	const char* sdkRoot,
	TargetDevice::Platform targetPlatform,
	const char * targetAppStoreName,
	S32 targetVersion,
	const char * customBuildId,
	const char * productId,
	const char * appPackage,
	bool isDistributionBuild,
	const char * keyStore,
	const char * storePassword,
	const char * keyAlias,
	const char * aliasPassword,
    U32 versionCode
)
 :	AppPackagerParams(
		appName, versionName, identity, provisionFile, srcDir, dstDir, sdkRoot,
		targetPlatform, targetAppStoreName, targetVersion, TargetDevice::kAndroidGenericDevice,
		customBuildId, productId, appPackage, isDistributionBuild ),
	fVersionCode(versionCode)
	, fWindowsNonAscii(false)
{
	fKeyStore.Set( keyStore );
	fKeyStorePassword.Set( storePassword );
	fKeyAlias.Set( keyAlias );
	fKeyAliasPassword.Set( aliasPassword );
}

void
AndroidAppPackagerParams::Print()
{
	Super::Print();
	fprintf( stderr,
		"\tKeystore path: '%s'\n"
		"\tKeystore alias: '%s'\n"
		"\tAndroid version code: '%x'\n"
		"\tAndroid app package: '%s'\n",
		GetAndroidKeyStore(),
		GetAndroidKeyAlias(),
		GetVersionCode(),
		GetAppPackage() );
}

	
// ----------------------------------------------------------------------------

// TODO: Move create_build_properties.lua out of librtt and into rtt_player in XCode
// Current issue with doing that is this lua file needs to be precompiled into C
// via custom build step --- all .lua files in librtt already do that, so we're
// taking a shortcut for now by putting it under librtt.

// create_build_properties.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in a generated .cpp file. The file also contains the definition of the
// following function which loads the bytecodes via luaL_loadbuffer.
int luaload_create_build_properties(lua_State* L);

AndroidAppPackager::AndroidAppPackager( const MPlatformServices & services, const char * resourcesDir )
:	PlatformAppPackager( services, TargetDevice::kAndroidPlatform ),
	fResourcesDir( & services.Platform().GetAllocator(), resourcesDir ),
	fIsUsingExpansionFile( false )
//	,
//	fPermissions( & services.Platform().GetAllocator() ),
//	fVersionCode( & services.Platform().GetAllocator() ),
//	fDefaultOrientation( DeviceOrientation::kUnknown ),
//	fSupportsOrientationChange( false )
{
    Lua::RegisterModuleLoader( fVM, "lfs", luaopen_lfs );
    Lua::RegisterModuleLoader( fVM, "lpeg", luaopen_lpeg ); // json depends on lpeg
	Lua::RegisterModuleLoader( fVM, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( fVM, "json", Lua::Open< luaload_json > );

	Lua::DoBuffer( fVM, & luaload_create_build_properties, NULL );
}

AndroidAppPackager::~AndroidAppPackager()
{
}

static const char *
versionToString( int version )
{
	switch ( version )
	{
		case TargetDevice::kAndroidOS1_5:
			return "1.5";
		case TargetDevice::kAndroidOS1_6:
			return "1.6";
		case TargetDevice::kAndroidOS2_0_1:
			return "2.0";
		case TargetDevice::kAndroidOS2_1:
			return "2.1";
		case TargetDevice::kAndroidOS2_2:
			return "2.2";
		case TargetDevice::kAndroidOS2_3_3:
			return "2.3.3";
		case TargetDevice::kAndroidOS4_0_3:
			return "4.0.3";
		default:
			return "unknown";
	}
}

std::string
AndroidAppPackager::EscapeArgument(std::string arg)
{
	std::string result = arg;

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)

	// On macOS escape shell special characters in the strings by replacing single quotes with "'\''" and
	// then enclosing in single quotes
	ReplaceString(result, "'", "'\\''");	// escape single quotes
	result = "'" + result + "'";

#else // Windows

	// On Windows escape shell special characters in the strings by replacing double quotes with "\"" and
	// then enclosing in double quotes
	ReplaceString(result, "\"", "\\\"");	// escape double quotes
	result = "\"" + result + "\"";

#endif
	
	return result;
}

#if defined(Rtt_WIN_ENV) && !defined(Rtt_NO_GUI)
int system_hidden(const char* cmdArgsUtf8)
{
	wchar_t cmdArgs[4096] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, cmdArgsUtf8, -1, cmdArgs, 4096);

	PROCESS_INFORMATION pinfo;
	STARTUPINFO sinfo;
	if (!GetConsoleWindow()) {
		AllocConsole();
		ShowWindow(GetConsoleWindow(), SW_HIDE);
	}

	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	CreateProcess(NULL, cmdArgs,
		NULL, NULL, false,
		0,
		NULL, NULL, &sinfo, &pinfo);
	DWORD ret;
	while (true)
	{
		HANDLE array[1];
		array[0] = pinfo.hProcess;
		ret = MsgWaitForMultipleObjects(1, array, false, INFINITE,
			QS_ALLPOSTMESSAGE);
		if ((ret == WAIT_FAILED) || (ret == WAIT_OBJECT_0))
			break;
		
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DWORD pret;
	GetExitCodeProcess(pinfo.hProcess, &pret);
	return pret;
}
#endif

int
AndroidAppPackager::Build( AppPackagerParams * params, const char * tmpDirBase )
{
	int result = PlatformAppPackager::kBuildError;
	time_t startTime = time(NULL);

    const char tmpTemplate[] = "CLtmpXXXXXX";
	char tmpDir[kDefaultNumBytes]; Rtt_ASSERT( kDefaultNumBytes > ( strlen( tmpDirBase ) + strlen( tmpTemplate ) ) );
	snprintf(tmpDir, kDefaultNumBytes, "%s" LUA_DIRSEP "%s", tmpDirBase, tmpTemplate);

    // This is not as foolproof as mkdtemp() but has the advantage of working on Win32
    if ( mkdir( mktemp(tmpDir) ) )
	{
		char* inputFile = Prepackage( params, tmpDir );

		if (inputFile) //offline build
		{
			const AndroidAppPackagerParams * androidParams = (const AndroidAppPackagerParams *)params;

			std::string gradleGo = "cd ";
#if defined(Rtt_WIN_ENV)
			gradleGo.append("/d ");
#endif
			gradleGo.append(EscapeArgument(tmpDir));
			gradleGo.append(" && cd template &&");
			
#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
			bool java11Installed = 0 == system("JAVA_VERSION=11 /usr/bin/java -version > /dev/null 2>/dev/null");
			if(java11Installed) {
				gradleGo.append(" ./setup.sh && JAVA_VERSION=11 ./gradlew");
			} else {
				Rtt_TRACE_SIM(("WARNING: Java 11 does not seems to be available. If build fails, install Java 1.11."));
				gradleGo.append(" ./setup.sh && ./gradlew");
			}
#else
			gradleGo.append(" setup.bat");
			if (androidParams->IsWindowsNonAsciiUser())
			{
				gradleGo.append(" /T");
			}
			gradleGo.append(" && gradlew.bat");
#endif
			if (androidParams->IsWindowsNonAsciiUser()) {
				std::string gradleDir(tmpDirBase);
				gradleDir += LUA_DIRSEP ".gradle";
				gradleDir = EscapeArgument(gradleDir);

				gradleGo.append(" -g ");
				gradleGo.append(gradleDir);

				gradleGo.append(" -Dgradle.user.home=");
				gradleGo.append(gradleDir);
			}

			gradleGo.append(" buildCoronaApp");
			gradleGo.append(" --no-daemon");
			
			gradleGo.append(" -PconfigureCoronaPlugins=YES");
			gradleGo.append(" -PcoronaBuild=" Rtt_STRING_BUILD);
			
			if (params->IsLiveBuild())
			{
				gradleGo.append(" -PcoronaLiveBuild=YES");
			}

			if (androidParams->IsWindowsNonAsciiUser()) {
				gradleGo.append(" -PcoronaCustomHome=");
				gradleGo.append(EscapeArgument(tmpDirBase));
			}

			gradleGo.append(" -PcoronaResourcesDir=");
			gradleGo.append(EscapeArgument(fResourcesDir.GetString()));
			
			gradleGo.append(" -PcoronaDstDir=");
			gradleGo.append(EscapeArgument(params->GetDstDir()));
			
			gradleGo.append(" -PcoronaTmpDir=");
			gradleGo.append(EscapeArgument(tmpDir));
			
			gradleGo.append(" -PcoronaSrcDir=");
			gradleGo.append(EscapeArgument(params->GetSrcDir()));
			
			String appFileName;
			PlatformAppPackager::EscapeFileName( params->GetAppName(), appFileName );
			gradleGo.append(" -PcoronaAppFileName=");
			gradleGo.append(EscapeArgument(appFileName.GetString()));
			
			gradleGo.append(" -PcoronaAppPackage=");
			gradleGo.append(EscapeArgument(params->GetAppPackage()));

			gradleGo.append(" -PcoronaVersionCode=");
			gradleGo.append(std::to_string(androidParams->GetVersionCode()));
			
			gradleGo.append(" -PcoronaVersionName=");
			gradleGo.append(EscapeArgument(params->GetVersion()));
			
			gradleGo.append(" -PcoronaKeystore=");
			gradleGo.append(EscapeArgument(androidParams->GetAndroidKeyStore()));
			
			gradleGo.append(" -PcoronaKeystorePassword=");
			gradleGo.append(EscapeArgument(androidParams->GetAndroidKeyStorePassword()));
			
			gradleGo.append(" -PcoronaKeyAlias=");
			gradleGo.append(EscapeArgument(androidParams->GetAndroidKeyAlias()));
			
			gradleGo.append(" -PcoronaTargetStore=");
			gradleGo.append(EscapeArgument(androidParams->GetTargetAppStoreName()));

			gradleGo.append(" -PcoronaKeyAliasPassword=");
			if(androidParams->GetAndroidKeyAliasPassword()!=NULL)
			{
				gradleGo.append(EscapeArgument(androidParams->GetAndroidKeyAliasPassword()));
			}
			else
			{
				gradleGo.append(EscapeArgument(androidParams->GetAndroidKeyStorePassword()));
			}
			
			if (fIsUsingExpansionFile &&
				params->GetTargetAppStoreName() &&
				!strcmp(params->GetTargetAppStoreName(), TargetAndroidAppStore::kGoogle.GetStringId()))
			{
				char expansionFileName[255];
				snprintf(
						 expansionFileName, sizeof(expansionFileName) - 1, "main.%d.%s.obb",
						 androidParams->GetVersionCode(), params->GetAppPackage());
				gradleGo.append(" -PcoronaExpansionFileName=");
				gradleGo.append(expansionFileName);
			}
			
			DeviceBuildData& deviceBuildData = params->GetDeviceBuildData( fServices.Platform(), fServices );
			String json( & fServices.Platform().GetAllocator() );
			deviceBuildData.GetJSON( json );
			const size_t maxPath = 600;
			char buildDataFileOutput[maxPath+1];
			snprintf(buildDataFileOutput, maxPath, "%s" LUA_DIRSEP "build.data", tmpDir);
			Rtt::Data<const unsigned char> jsonData(& fServices.Platform().GetAllocator());
			jsonData.Set((unsigned char*)json.GetString(), json.GetLength());
			Rtt_WriteDataToFile(buildDataFileOutput, jsonData);

			gradleGo.append(" -PcoronaBuildData=");
			gradleGo.append(EscapeArgument(buildDataFileOutput));

			String debugBuildProcessPref;
			int debugBuildProcess = 0;
			fServices.GetPreference( "debugBuildProcess", &debugBuildProcessPref );
			
			if (! debugBuildProcessPref.IsEmpty())
			{
				debugBuildProcess = (int) strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
			}
			else
			{
				debugBuildProcess = 0;
			}
			
			gradleGo.append(" --console=plain");
			if(debugBuildProcess == 0) {
				gradleGo.append(" -q");
			}
			
			if (debugBuildProcess > 1)
			{
				// Obfuscate passwords
				std::string placeHolder = EscapeArgument("XXXXXX");
				std::string sanitizedCmdBuf = gradleGo;
				
				String unsanitizedBuildSettingStr;
				fServices.GetPreference( "unsanitizedBuildLog", &unsanitizedBuildSettingStr );
				if(Rtt_StringCompare(unsanitizedBuildSettingStr, "1") != 0) {
					std::string keystorePasswordStr = EscapeArgument(androidParams->GetAndroidKeyStorePassword());
					if (keystorePasswordStr.length() > 0)
					{
						ReplaceString(sanitizedCmdBuf, keystorePasswordStr, placeHolder);
					}
					
					std::string keyaliasPasswordStr;
					if (androidParams->GetAndroidKeyAliasPassword() != NULL)
					{
						keyaliasPasswordStr = EscapeArgument(androidParams->GetAndroidKeyAliasPassword());
					}
					if (keyaliasPasswordStr.length() > 0)
					{
						ReplaceString(sanitizedCmdBuf, keyaliasPasswordStr, placeHolder);
					}
				}
				
				Rtt_Log("Build: running: %s\n", sanitizedCmdBuf.c_str());
			}

#if defined(Rtt_WIN_ENV)
			gradleGo.append(" < NUL ");
#else
			gradleGo.append(" < /dev/null ");
#endif

#if !defined(Rtt_NO_GUI)
			gradleGo.insert(0, "(");
			gradleGo.append(") > ");
			std::string gradleLogFile(tmpDir);
			gradleLogFile.append(LUA_DIRSEP);
			gradleLogFile.append("gradleLogFile.log");
			gradleGo.append(EscapeArgument(gradleLogFile.c_str()));
			gradleGo.append(" 2>&1 ");
#endif

#if defined(Rtt_WIN_ENV)
			gradleGo.insert(0, "\"");
			gradleGo.append("\"");
#if defined(Rtt_NO_GUI)
			result = system(gradleGo.c_str());
#else
			gradleGo.insert(0, "CMD /C ");
			result = system_hidden(gradleGo.c_str());
#endif
#else
			result = system(gradleGo.c_str());
#endif

#if !defined(Rtt_NO_GUI)
			if (debugBuildProcess >= 1 || result != 0)
			{
				FILE *log = Rtt_FileOpen(gradleLogFile.c_str(), "rb");
				if (log) {
					Rtt_FileSeek(log, 0, SEEK_END);
					long sz = Rtt_FileTell(log);
					Rtt_FileSeek(log, 0, SEEK_SET);
					char* buf = new char[sz + 1];
					long read = Rtt_FileRead(buf, sizeof(char), sz, log);
					buf[read] = 0;
					Rtt_Log("%s", buf);
					delete[] buf;
					Rtt_FileClose(log);
				}
				else 
				{
					Rtt_Log("%s", "Unable to open build log file");
				}
			}
#endif
		}
		
		// Clean up intermediate files
		String retainTmpDirStr;
		fServices.GetPreference( "retainBuildTmpDir", &retainTmpDirStr );
		if(Rtt_StringCompare(retainTmpDirStr, "1") != 0) {
        	rmdir( tmpDir );
		}
	}
	else
	{
		// Note that the failing mkdir() that brought us here is a member of the AndroidAppPackager class
		String tmpString;

		tmpString.Set("AndroidAppPackager::Build: failed to create temporary directory\n\n");
		tmpString.Append(tmpDir);
		tmpString.Append("\n");

		Rtt_TRACE_SIM( ( "%s", tmpString.GetString() ) );
		params->SetBuildMessage(tmpString.GetString());
	}

    // Indicate status in the console
    if (PlatformAppPackager::kNoError == result)
    {
		// This isn't an exception but Rtt_Log() is only defined for debug builds
        Rtt_LogException("Android build succeeded in %ld seconds", (time(NULL) - startTime));
    }
    else
    {
		Rtt_LogException("Android build failed (%d) after %ld seconds", result, (time(NULL) - startTime));
    }

	return result;
}

bool
AndroidAppPackager::VerifyConfiguration() const
{
	// TODO: Add code to check existence of various utilities
	return true;
}

/// Called when the "build.settings" file is being read.
/// @param L Pointer to the Lua state that has loaded the build settings table.
/// @param index The index to the "settings" table in the Lua stack.
void
AndroidAppPackager::OnReadingBuildSettings( lua_State *L, int index )
{
	// Fetch the "usesExpansionFile" flag in the "build.settings" file, if provided.
	lua_getfield( L, index, "android" );
	if (lua_istable( L, -1 ))
	{
		lua_getfield( L, -1, "usesExpansionFile" );
		if (lua_isboolean( L, -1 ))
		{
			fIsUsingExpansionFile = lua_toboolean( L, -1 ) ? true : false;
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}

bool
AndroidAppPackager::CreateBuildProperties( const AppPackagerParams& params, const char *tmpDir )
{
	lua_State *L = fVM;
	lua_getglobal( L, "androidCreateProperties" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
	lua_pushstring( L, tmpDir );
	lua_pushstring( L, params.GetAppPackage() );
	lua_pushstring( L, params.GetSrcDir() );
	lua_pushinteger( L, ((AndroidAppPackagerParams&)params).GetVersionCode() );
	lua_pushstring( L, params.GetVersion() );
	lua_pushstring( L, params.GetTargetAppStoreName() );
	lua_pushstring( L, params.GetAppName() );

	bool result = Rtt_VERIFY( 0 == Lua::DoCall( L, 7, 1 ) );
	if ( ! lua_isnil( L, -1 ) )
	{
		Rtt_TRACE_SIM( ( "ERROR: Could not create build.properties:\n\t%s\n", lua_tostring( L, -1 ) ) );
		result = false;
	}
	return result;
}

char *
AndroidAppPackager::Prepackage( AppPackagerParams * params, const char * tmpDir )
{
	char* result = NULL;
	int iresult = -1;

	Rtt_ASSERT( params );

	// Convert build.settings into build.properties
	// And run Android specific pre package script
	String debugBuildProcessPref;
	int debugBuildProcess = 0;
	fServices.GetPreference("debugBuildProcess", &debugBuildProcessPref);

	if (!debugBuildProcessPref.IsEmpty())
	{
		debugBuildProcess = (int)strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
	}
	else
	{
		debugBuildProcess = 0;
	}

	if (debugBuildProcess > 0)
	{
		Rtt_Log("Prepackage: Compiling Lua ...");
	}
	
	if ( CompileScripts( params, tmpDir ) && CreateBuildProperties( * params, tmpDir ) )
	{
		if (! Rtt_StringIsEmpty(GetSplashImageFile()))
		{
			// Note: this logic (copying the splash screen image to the root of the tmp directory) needs
			// to match the logic in buildsys-worker/tools/build3_output_android.sh on the build server
			String splashImageFilename, tmpSplashPath, tmpFilename, tmpDstFilename;
			splashImageFilename.Set(params->GetSrcDir());
			splashImageFilename.AppendPathComponent(GetSplashImageFile());
			tmpSplashPath.Set(GetSplashImageFile());
			tmpFilename.Set(tmpSplashPath.GetLastPathComponent());
			tmpDstFilename.Set(tmpDir);
			tmpDstFilename.AppendPathComponent(tmpFilename);

			if (! Rtt_CopyFile( splashImageFilename, tmpDstFilename ))
			{
				String tmpString;
				tmpString.Set("ERROR: failed to copy splashScreen.image file: ");
				tmpString.Append(GetSplashImageFile());
				//Rtt_Log("%s", tmpString.GetString());
				params->SetBuildMessage( tmpString );

				return NULL;
			}
		}

		std::string liveBuildStr;
		std::string liveBuildTMPManifestFileStr;

		if (params->IsLiveBuild())
		{
			liveBuildStr = "YES";
			liveBuildTMPManifestFileStr = tmpDir;
			liveBuildTMPManifestFileStr.append("/output/assets/_corona_live_build_manifest.txt");
			liveBuildTMPManifestFileStr = EscapeArgument(liveBuildTMPManifestFileStr);
		}
		else
		{
			liveBuildStr = "NO";
		}

		std::string javaCmd;
#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
		javaCmd = "/usr/bin/java";
#else // Windows
		std::string jdkPath = Rtt::JavaHost::GetJdkPath();

		javaCmd = jdkPath + "\\bin\\java.exe";
#endif

		const char kCmdFormat[] = "\"%s\" -Djava.class.path=%s org.apache.tools.ant.launch.Launcher %s -DTEMP_DIR=%s -DSRC_DIR=%s -DBUNDLE_DIR=%s -DLIVE_BUILD=%s -DLIVE_BUILD_MANIFEST_FILE=%s -f %s/build.xml build-input-zip";

		char cmdBuf[20480];

		std::string jarPathStr;
		std::string tmpDirStr = EscapeArgument(tmpDir);
		std::string srcDirStr = EscapeArgument(params->GetSrcDir());
		std::string resourcesDirStr = EscapeArgument(fResourcesDir.GetString());

		jarPathStr.append(fResourcesDir.GetString());
		jarPathStr.append("/");
		jarPathStr.append("ant.jar");

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
		jarPathStr.append(":");
#else // Windows
		jarPathStr.append(";");
#endif
		jarPathStr.append(fResourcesDir.GetString());
		jarPathStr.append("/");
		jarPathStr.append("ant-launcher.jar" );

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
		jarPathStr.append(":");
#else // Windows
		jarPathStr.append(";");
#endif
		jarPathStr.append(fResourcesDir.GetString());
		jarPathStr.append("/");
		jarPathStr.append("AntLiveManifest.jar");
		jarPathStr = EscapeArgument(jarPathStr);

		const char *antDebugFlag = "-q";
		if (debugBuildProcess >= 2)
		{
			antDebugFlag = "-v";
		}
		if (debugBuildProcess > 4)
		{
			antDebugFlag = "-d";
		}

		snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat,
				 javaCmd.c_str(),
				 jarPathStr.c_str(),
				 antDebugFlag,
				 tmpDirStr.c_str(),
				 srcDirStr.c_str(),
				 resourcesDirStr.c_str(),
				 liveBuildStr.c_str(),
				 liveBuildTMPManifestFileStr.c_str(),
				 resourcesDirStr.c_str() );
		
		if (debugBuildProcess > 0)
		{
			Rtt_Log("Prepackage: Building server data file");
		}

		if (debugBuildProcess > 1)
		{
			Rtt_Log("Prepackage: running: %s\n", cmdBuf);
		}

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)

		iresult = system( cmdBuf );
		
#else // Windows

#if !defined( Rtt_NO_GUI )
		WinString cmdBufStr;
		cmdBufStr.SetUTF8(cmdBuf);

		std::wstring utf16CommandLine(cmdBufStr.GetUTF16());

		Interop::Ipc::CommandLine::SetOutputCaptureEnabled(true);
		Interop::Ipc::CommandLineRunResult cmdResult = Interop::Ipc::CommandLine::RunUntilExit(utf16CommandLine.c_str());

		iresult = cmdResult.GetExitCode();

		CSimulatorApp *pApp = ((CSimulatorApp *)AfxGetApp());
		if (pApp!= NULL && pApp->IsStopBuildRequested())
		{
			// A request to stop the build was made while the Java was running
			iresult = -1;
		}

		if (debugBuildProcess > 1)
		{
			std::string output = cmdResult.GetOutput();
			Rtt_Log("%s", output.c_str());
		}
#else
		// due to windows bug whole thing must be quoted for some reason
		int len = strlen(cmdBuf);
		for (int i = len; i>0; i--) {
			cmdBuf[i] = cmdBuf[i - 1];
		}
		cmdBuf[0] = '"';
		cmdBuf[len + 1] = '"';
		cmdBuf[len + 2] = 0;
		iresult = system(cmdBuf);
#endif

#endif

#if USE_JNI
		AntHost	antTask;

		char buildFilePath[1024];
		snprintf( buildFilePath, sizeof(buildFilePath), "%s%sbuild.xml", fResourcesDir.GetString(), LUA_DIRSEP );

		antTask.SetProperty( "TEMP_DIR", tmpDir );
		antTask.SetProperty( "SRC_DIR", params->GetSrcDir() );
		antTask.SetProperty( "BUNDLE_DIR", fResourcesDir.GetString() );
		antTask.SetProperty( "LIVE_BUILD", liveBuildStr.c_str() );
		antTask.SetProperty( "LIVE_BUILD_MANIFEST_FILE", liveBuildTMPManifestFileStr.c_str() );

		String debugBuildProcessPref;
		int debugBuildProcess = 0;
		fServices.GetPreference( "debugBuildProcess", &debugBuildProcessPref );

		if (! debugBuildProcessPref.IsEmpty())
		{
			debugBuildProcess = (int) strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
		}
		else
		{
			debugBuildProcess = 0;
		}

		String antResult;
		int antCode;

		antCode = antTask.AntCall( buildFilePath, "build-input-zip", debugBuildProcess, &antResult );
		Rtt_ASSERT( antCode == 1 );
		if ( antCode == 0 )
		{
			Rtt_TRACE_SIM( ( "ANT build-input-zip failed: %s", antResult.GetString() ) );
		}
		if ( antCode == 1 )
			iresult = 0;
#endif

		if ( iresult == 0 )
		{
			const char kDstName[] = "input.zip";
			size_t resultLen = strlen( tmpDir ) + strlen( kDstName ) + sizeof( LUA_DIRSEP );
			result = (char *) malloc( resultLen + 1 );
			snprintf( result, resultLen, "%s" LUA_DIRSEP "%s", tmpDir, kDstName );
		}
	}

	return result;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IOSAppPackager.h"

#if defined(Rtt_NO_GUI)
// Stub out MacSimulatorServices so CoronaBuilder can use this class without a bunch of ifdeffing
class Rtt::MacSimulatorServices
{
    public:
        void SetBuildMessage(const char *dummy) { };
};
#else
#include "Rtt_MacSimulatorServices.h"
#include "Rtt_LuaLibSimulator.h"
#endif

#include "Rtt_Lua.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_FileSystem.h"
#include "Rtt_DeviceBuildData.h"
#include "Rtt_HTTPClient.h"
#include "XcodeToolHelper.h"

#include <string.h>
#include <time.h>

#include <dirent.h>
#include <sys/stat.h>

Rtt_EXPORT int luaopen_lfs (lua_State *L);
Rtt_EXPORT int luaopen_socket_core (lua_State *L);
Rtt_EXPORT int luaopen_mime_core(lua_State *L);


// ----------------------------------------------------------------------------

namespace Rtt
{

// TODO: Move iPhonePackageApp.lua out of librtt and into rtt_player in XCode
// Current issue with doing that is this lua file needs to be precompiled into C
// via custom build step --- all .lua files in librtt already do that, so we're
// taking a shortcut for now by putting it under librtt.

// iPhonePackageApp.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in a generated .cpp file. The file also contains the definition of the 
// following function which loads the bytecodes via luaL_loadbuffer.
    int luaload_iPhonePackageApp(lua_State* L);
    int luaload_CoronaPListSupport(lua_State* L);
    int luaload_CoronaOfflineiOSPackager(lua_State* L);

// ----------------------------------------------------------------------------

void
IOSAppPackagerParams::Print()
{
	Super::Print();
	fprintf( stderr,
		"\tSDK path: '%s'\n"
		"\tCertificate path: '%s'\n"
		"\tSigning identity: '%s'\n"
		"\tBundle id: '%s'\n"
		"\tBuildSettingsPath: '%s'\n",
		GetSdkRoot(),
		GetProvisionFile(),
		GetIdentity(),
		GetAppPackage(),
		GetBuildSettingsPath() );
}

// ----------------------------------------------------------------------------

#define kDefaultNumBytes 1024

IOSAppPackager::IOSAppPackager( const MPlatformServices& services, MacSimulatorServices *simulatorServices /* = NULL */)
	: PlatformAppPackager( services, TargetDevice::kIPhonePlatform ),
    fSimulatorServices(simulatorServices)
{
	lua_State *L = fVM;

#if ! defined(Rtt_NO_GUI)
    if (fSimulatorServices != NULL)
    {
        lua_pushlightuserdata( L, fSimulatorServices );
        Lua::RegisterModuleLoader( L, "simulator", LuaLibSimulator::Open, 1 );
    }
#endif

	Lua::RegisterModuleLoader( L, "CoronaPListSupport", Lua::Open< luaload_CoronaPListSupport > );
	HTTPClient::registerFetcherModuleLoaders(L);
    
	Lua::DoBuffer( fVM, & luaload_iPhonePackageApp, NULL);
}

IOSAppPackager::~IOSAppPackager()
{
}

int
IOSAppPackager::Build( AppPackagerParams * params, const char* tmpDirBase )
{
	int result = PlatformAppPackager::kBuildError;
	const IOSAppPackagerParams * macParams = (const IOSAppPackagerParams *) params;
	time_t startTime = time(NULL);

    const char tmpTemplate[] = "CLtmpXXXXXX";
	char tmpDir[kDefaultNumBytes]; Rtt_ASSERT( kDefaultNumBytes > ( strlen( tmpDirBase ) + strlen( tmpTemplate ) ) );
	int tmpDirLen = sprintf( tmpDir, "%s%s", tmpDirBase, tmpTemplate );

	const char kCmdFormat[] = "mkdir -p %s";
	char cmd[kDefaultNumBytes]; Rtt_ASSERT( kDefaultNumBytes > ( sizeof( kCmdFormat ) + tmpDirLen ) );
	sprintf( cmd, kCmdFormat, mktemp(tmpDir) );

	if ( Rtt_VERIFY( 0 == system( cmd ) ) )
	{
		char* inputFile = Prepackage( params, tmpDir );

		if ( inputFile )
		{
			const char kOutputName[] = "output.zip";
			size_t tmpDirLen = strlen( tmpDir );
			size_t outputFileLen = tmpDirLen + sizeof(kOutputName) + sizeof( LUA_DIRSEP );
			char* outputFile = (char*)malloc( outputFileLen );
			sprintf( outputFile, "%s" LUA_DIRSEP "%s", tmpDir, kOutputName );

			if(true)
			{
				if (fSimulatorServices != NULL)
				{
					fSimulatorServices->SetBuildMessage("Collecting plugins");
				}

				lua_State *L = fVM;
				
				Lua::DoBuffer( L, & luaload_CoronaOfflineiOSPackager, NULL);
				lua_getglobal(L, "CreateOfflinePackage");
				lua_newtable( L );
				
				lua_pushstring(L, tmpDir);
				lua_setfield(L, -2, "tmpDir");
				
				lua_pushstring(L, outputFile);
				lua_setfield(L, -2, "outputFile");
				
				lua_pushstring(L, inputFile);
				lua_setfield(L, -2, "inputFile");
				
				const char *platform, *modernPlatform, *pluginPlatform;
				bool isAppleTV = false;
				switch (params->GetTargetDevice()) {
					case TargetDevice::kIPhone:
					case TargetDevice::kIPad:
					case TargetDevice::kIOSUniversal:
						platform = "iphoneos";
						modernPlatform = "ios";
						pluginPlatform = "iphone";
						break;
					case TargetDevice::kAppleTV:
						platform = "appletvos";
						modernPlatform = "tvos";
						pluginPlatform = "appletvos";
						isAppleTV = true;
						break;
					case TargetDevice::kIPhoneXCodeSimulator:
					case TargetDevice::kIPadXCodeSimulator:
					case TargetDevice::kIOSUniversalXCodeSimulator:
						platform = "iphonesimulator";
						modernPlatform = "ios-sim";
						pluginPlatform = "iphone-sim";
						break;
					case TargetDevice::kTVOSXCodeSimulator:
						platform = "appletvsimulator";
						modernPlatform = "tvos-sim";
						pluginPlatform = "appletvsimulator";
						isAppleTV = true;
						break;
					default:
						Rtt_ASSERT(0);
						platform = "iphoneos";
					break;
				}
				
				lua_pushstring(L, platform);
				lua_setfield(L, -2, "platform");
				lua_pushstring(L, modernPlatform);
				lua_setfield(L, -2, "modernPlatform");
				lua_pushstring(L, pluginPlatform);
				lua_setfield(L, -2, "pluginPlatform");
				lua_pushboolean(L, isAppleTV);
				lua_setfield(L, -2, "isAppleTV");
				
				lua_pushstring(L, Rtt_MACRO_TO_STRING( Rtt_BUILD_REVISION ) );
				lua_setfield(L, -2, "build");
				
				char templateZip[255];
				setlocale(LC_NUMERIC, "en_US");
				snprintf(templateZip, 255, "%s_%.1f%s.tar.bz", platform, params->GetTargetVersion()/10000.0f, params->GetCustomTemplate());
				lua_pushstring(L, templateZip);
				lua_setfield(L, -2, "template");
				
				Rtt::String resourceDir;
				fServices.Platform().PathForFile(NULL, MPlatform::kSystemResourceDir, 0, resourceDir);
				lua_pushstring(L, resourceDir.GetString());
				lua_setfield(L, -2, "resourceDir");

				lua_pushstring(L, params->GetAppPackage());
				lua_setfield(L, -2, "appPackage");
				
				
				DeviceBuildData & buildData = params->GetDeviceBuildData(fServices.Platform(), fServices);
				Rtt::String json;
				buildData.GetJSON(json);
				lua_pushstring(L, json.GetString());
				lua_setfield(L, -2, "buildData");
				
				String escapedAppName;
                PlatformAppPackager::EscapeFileName( params->GetAppName(), escapedAppName );
                lua_pushstring(L, escapedAppName.GetString());
                lua_setfield(L, -2, "appName");
                
				lua_pushstring(L, params->GetCoronaUser());
				lua_setfield(L, -2, "user");
				
				if ( Rtt_VERIFY( 0 == Lua::DoCall( L, 1, 1 ) ) )
				{
					result = PlatformAppPackager::kNoError;
					if ( lua_isstring( L, -1 ) )
					{
						Rtt_TRACE_SIM( ( "BUILD ERROR: %s\n", lua_tostring( L, -1 ) ) );
						params->SetBuildMessage( lua_tostring( L, -1 ) );
						result = PlatformAppPackager::kBuildError;
					}
				}
				else
				{
					result = PlatformAppPackager::kLocalPackagingError;
				}
				lua_pop( L, 1 );
			}
			if ( PlatformAppPackager::kNoError == result )
			{
				if (fSimulatorServices != NULL)
				{
					fSimulatorServices->SetBuildMessage("Packaging app");
				}
				lua_State *L = fVM;
				lua_getglobal( L, "iPhonePostPackage" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );

				// params
				lua_newtable( L );
				{
					lua_pushstring( L, macParams->GetSrcDir() );
					lua_setfield( L, -2, "srcAssets" );
					
					lua_pushstring( L, tmpDir );
					lua_setfield( L, -2, "tmpDir" );

					lua_pushstring( L, macParams->GetDstDir() );
					lua_setfield( L, -2, "dstDir" );

                    String sanitizedName;
                    PlatformAppPackager::EscapeFileName(macParams->GetAppName(), sanitizedName);
					lua_pushstring( L, sanitizedName.GetString() );
					lua_setfield( L, -2, "dstFile" );

					lua_pushstring( L, macParams->GetAppName() );
					lua_setfield( L, -2, "bundledisplayname" );

					lua_pushstring( L, macParams->GetVersion() );
					lua_setfield( L, -2, "bundleversion" );

					lua_pushstring( L, macParams->GetProvisionFile() );
					lua_setfield( L, -2, "provisionFile" );

					lua_pushstring( L, macParams->GetIdentity() );
					lua_setfield( L, -2, "signingIdentity" );

					lua_pushstring( L, macParams->GetSdkRoot() );
					lua_setfield( L, -2, "sdkRoot" );

					lua_pushinteger( L, macParams->GetTargetDevice() );
					lua_setfield( L, -2, "targetDevice" );

                    lua_pushstring( L, TargetDevice::StringForPlatform( macParams->GetTargetPlatform() ) );
                    lua_setfield( L, -2, "targetPlatform" );
                    
					lua_pushboolean( L, macParams->IsLiveBuild() );
					lua_setfield( L, -2, "liveBuild" );
                    lua_pushboolean( L, macParams->IncludeStandardResources() );
					lua_setfield( L, -2, "includeStandardResources" );

					// By default, assumes ARM architecture, so we need to override
					// when building for Xcode simulator
					bool isSimulator = ( params->GetTargetDevice() >= TargetDevice::kXCodeSimulator );
					if ( isSimulator )
					{
						lua_pushstring( L, "iphonesimulator" );
						lua_setfield( L, -2, "sdkType" );
					}

                    lua_newtable(L);
                    {
                        BOOL debugBuildProcess = ([[NSUserDefaults standardUserDefaults] integerForKey:@"debugBuildProcess"] > 1);
                        NSString* sdkRoot = [XcodeToolHelper getXcodePath];
                        NSString* copypng = [XcodeToolHelper pathForCopyPngUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];
                        NSString* codesign = [XcodeToolHelper pathForCodesignUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];
                        NSString* codesign_allocate = [XcodeToolHelper pathForCodesignAllocateUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];
						NSString *codesign_framework = [XcodeToolHelper pathForCodesignFramework];

                        lua_pushstring( L, [sdkRoot UTF8String] );
                        lua_setfield( L, -2, "sdkroot" );
                        
                        lua_pushstring( L, [copypng UTF8String] );
                        lua_setfield( L, -2, "copypng" );
                        
                        lua_pushstring( L, [codesign UTF8String] );
                        lua_setfield( L, -2, "codesign" );
                        
                        lua_pushstring( L, [codesign_allocate UTF8String] );
                        lua_setfield( L, -2, "codesign_allocate" );

						lua_pushstring( L, [codesign_framework UTF8String] );
						lua_setfield( L, -2, "codesign_framework" );
                    }
                    lua_setfield( L, -2, "xcodetoolhelper" );
				}

                if (fSimulatorServices != NULL)
                {
                    fSimulatorServices->SetBuildMessage("Packaging app");
                }

				// iPhonePostPackage( params )
				if ( ! Rtt_VERIFY( 0 == Lua::DoCall( L, 1, 1 ) ) )
				{
					// The packaging script failed to compile
					result = PlatformAppPackager::kLocalPackagingError;
				}
				else
				{
					if ( lua_isstring( L, -1 ) )
					{
						result = PlatformAppPackager::kLocalPackagingError;
						Rtt_TRACE_SIM( ( "BUILD ERROR: %s\n", lua_tostring( L, -1 ) ) );
                        params->SetBuildMessage( lua_tostring( L, -1 ) );
					}
					lua_pop( L, 1 );
				}

				/*
				// Obtain output.zip file from server 
				const char kCmdFormat[] = "/Volumes/rtt/bin/mac/app_sign/build_output.sh %s %s %s %s %s little";
				char cmd[kDefaultNumBytes + sizeof(kCmdFormat) + strlen( params.appName ) + sizeof(tmpDir) + sizeof() ];
				const char kTemplateFile[] = "/Volumes/rtt/bin/iphone/template.app";
				const char kCertificateFile[] = "/Volumes/rtt/bin/mac/app_sign/build/Debug/foo";
				sprintf(
					cmd,
					kCmdFormat,
					params.appName, kTemplateFile, tmpDir, kCertificateFile, );
				system(  );
				*/
			}
            else
            {
                if (params->GetBuildMessage() == NULL)
                {
                    // If we don't already have a more precise error, use the XMLRPC layer's error message
                    params->SetBuildMessage( "Unknown error" );
                }
            }

			free( outputFile );
			free( inputFile );
		}

		// Clean up intermediate files
		if(!([[NSUserDefaults standardUserDefaults] integerForKey:@"retainBuildTmpDir"] > 1)) {
			sprintf( cmd, "rm -rf \"%s\"", tmpDir );
			(void)Rtt_VERIFY( 0 == system( cmd ) );
		}
	}

exit_gracefully:
    
    // Indicate status in the console
    if (PlatformAppPackager::kNoError == result)
    {
        Rtt_LogException("iOS build succeeded in %ld seconds", (time(NULL) - startTime));
    }
    else
    {
        Rtt_LogException("iOS build failed (%d) after %ld seconds", result, (time(NULL) - startTime));
    }

	return result;
}

int
IOSAppPackager::SendToAppStore( IOSAppPackagerParams *iosParams, const char *itunesConnectUsername, const char *itunesConnectPassword )
{
    int result = PlatformAppPackager::kNoError;
    lua_State *L = fVM;
    lua_getglobal( L, "IOSSendToAppStore" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );

    // params for Lua call
    lua_newtable( L );
    {
        lua_pushstring( L, iosParams->GetSrcDir() );
        lua_setfield( L, -2, "srcAssets" );

        lua_pushstring( L, iosParams->GetDstDir() );
        lua_setfield( L, -2, "dstDir" );

		String sanitizedName;
		PlatformAppPackager::EscapeFileName(iosParams->GetAppName(), sanitizedName);
		lua_pushstring( L, sanitizedName.GetString() );
		lua_setfield( L, -2, "dstFile" );

		lua_pushstring( L, iosParams->GetAppName() );
		lua_setfield( L, -2, "bundledisplayname" );

        lua_pushstring( L, iosParams->GetVersion() );
        lua_setfield( L, -2, "bundleversion" );

        lua_pushstring( L, iosParams->GetIdentity() );
        lua_setfield( L, -2, "signingIdentity" );

        lua_pushinteger( L, iosParams->GetTargetDevice() );
        lua_setfield( L, -2, "targetDevice" );

        lua_pushstring( L, TargetDevice::StringForPlatform( iosParams->GetTargetPlatform() ) );
        lua_setfield( L, -2, "targetPlatform" );

        lua_pushstring( L, Rtt_STRING_BUILD );
        lua_setfield( L, -2, "corona_build_id" );

        lua_pushstring( L, itunesConnectUsername );
        lua_setfield( L, -2, "itc1" );

        lua_pushstring( L, itunesConnectPassword );
        lua_setfield( L, -2, "itc2" );

        lua_newtable(L);
        {
            BOOL debugBuildProcess = ([[NSUserDefaults standardUserDefaults] integerForKey:@"debugBuildProcess"] > 1);
            NSString* sdkRoot = [XcodeToolHelper getXcodePath];
            NSString* codesign = [XcodeToolHelper pathForCodesignUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];
            NSString* productBuild = [XcodeToolHelper pathForProductBuildUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];
            NSString* applicationLoader = [XcodeToolHelper pathForApplicationLoaderUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];

            lua_pushstring( L, [sdkRoot UTF8String] );
            lua_setfield( L, -2, "sdkRoot" );

            lua_pushstring( L, [codesign UTF8String] );
            lua_setfield( L, -2, "codesign" );

            lua_pushstring( L, [productBuild UTF8String] );
            lua_setfield( L, -2, "productbuild" );
            
            lua_pushstring( L, [applicationLoader UTF8String] );
            lua_setfield( L, -2, "applicationLoader" );
        }
        lua_setfield( L, -2, "xcodetoolhelper" );
    }
    
    // IOSSendToAppStore( params )
    if ( ! Rtt_VERIFY( 0 == Lua::DoCall( L, 1, 1 ) ) )
    {
        // The packaging script failed
        result = PlatformAppPackager::kLocalPackagingError;
    }
    else
    {
        if ( lua_isstring( L, -1 ) )
        {
            result = PlatformAppPackager::kLocalPackagingError;
            Rtt_TRACE_SIM( ( "PACKAGING %s\n", lua_tostring( L, -1 ) ) );
            iosParams->SetBuildMessage( lua_tostring( L, -1 ) );
        }
        lua_pop( L, 1 );
    }
    
    return result;
}

const char *
IOSAppPackager::GetBundleId( const char *provisionFile, const char *appName ) const
{
	const char *result = NULL;

	lua_State *L = fVM;
	lua_getglobal( L, "getBundleId" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
	lua_pushstring( L, provisionFile );
	lua_pushstring( L, PlatformAppPackager::EscapeStringForIOS( appName ) );
	lua_pushstring( L, appName );

	if ( Rtt_VERIFY( 0 == Lua::DoCall( L, 3, 1 ) ) )
	{
		const char *s = lua_tostring( L, -1 );
		if ( s )
		{
			// NOTE: This is a slimey trick to guarantee the C-string is valid for use by the caller
			// 'str' is auto-released so the result of [str UTF8String] is valid until the auto-released
			NSString *str = [NSString stringWithExternalString:s];
			result = [str UTF8String];
		}

		lua_pop( L, 1 );
	}

	return result;
}

bool
IOSAppPackager::VerifyConfiguration() const
{
	// Add code to check existence of various utilities
	return true;
}

char* 
IOSAppPackager::Prepackage( AppPackagerParams * params, const char* tmpDir )
{
	char* result = NULL;

	// Copy provision file (if we have one) into tmpDir
	if ( params->GetProvisionFile() )
	{
		bool retflag = CopyProvisionFile( params, tmpDir );
		if ( false == retflag )
		{
			return result;
		}
	}

	// Then Prepackage like before
	result = Super::Prepackage( params, tmpDir );
	
	return result;
}

bool
IOSAppPackager::CopyProvisionFile( const AppPackagerParams * params, const char* tmpDir )
{
	const char kDstName[] = "embedded.mobileprovision";
	String dstFileStr;

	dstFileStr.Append(tmpDir);
	dstFileStr.Append(LUA_DIRSEP);
	dstFileStr.Append(kDstName);

	return Rtt_CopyFile( params->GetProvisionFile(), dstFileStr.GetString() );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


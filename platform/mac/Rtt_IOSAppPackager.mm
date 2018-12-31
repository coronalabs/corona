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
#include "Rtt_WebServicesSession.h"
#include "Rtt_FileSystem.h"

#include "XcodeToolHelper.h"

#include <string.h>
#include <time.h>

#include <dirent.h>
#include <sys/stat.h>

Rtt_EXPORT int luaopen_lfs (lua_State *L);

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
	Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( L, "json", Lua::Open< luaload_json > );
	Lua::RegisterModuleLoader( L, "lpeg", luaopen_lpeg );
	Lua::RegisterModuleLoader( L, "lfs", luaopen_lfs );
    
	Lua::DoBuffer( fVM, & luaload_iPhonePackageApp, NULL);
}

IOSAppPackager::~IOSAppPackager()
{
}

int
IOSAppPackager::Build( AppPackagerParams * params, WebServicesSession& session, const char* tmpDirBase )
{
	int result = WebServicesSession::kBuildError;
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

            if (fSimulatorServices != NULL)
            {
                fSimulatorServices->SetBuildMessage("Communicating with build server");
            }

            // Send params/input.zip via web api
			result = session.BeginBuild( params, inputFile, outputFile );
			if ( WebServicesSession::kNoError == result )
			{
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
						NSString *codesign_framework = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"codesign-framework.sh"];

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
					result = WebServicesSession::kLocalPackagingError;
				}
				else
				{
					if ( lua_isstring( L, -1 ) )
					{
						result = WebServicesSession::kLocalPackagingError;
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
                    params->SetBuildMessage( session.ErrorMessage() );
                }
            }

			free( outputFile );
			free( inputFile );
		}

		// Clean up intermediate files
		sprintf( cmd, "rm -rf \"%s\"", tmpDir );
		(void)Rtt_VERIFY( 0 == system( cmd ) );
	}

exit_gracefully:
    
    // Indicate status in the console
    if (WebServicesSession::kNoError == result)
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
    int result = WebServicesSession::kNoError;
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
        result = WebServicesSession::kLocalPackagingError;
    }
    else
    {
        if ( lua_isstring( L, -1 ) )
        {
            result = WebServicesSession::kLocalPackagingError;
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


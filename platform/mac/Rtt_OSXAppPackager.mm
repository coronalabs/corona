//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_OSXAppPackager.h"

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
#include "Rtt_Runtime.h"

#include "XcodeToolHelper.h"

#include <string.h>
#include <time.h>

#include <dirent.h>
#include <sys/stat.h>

Rtt_EXPORT int luaopen_lfs (lua_State *L);

// ----------------------------------------------------------------------------

namespace Rtt
{

// OSXPackageApp.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in a generated .cpp file. The file also contains the definition of the 
// following function which loads the bytecodes via luaL_loadbuffer.
    int luaload_OSXPackageApp(lua_State* L);
	int luaload_CoronaPListSupport(lua_State* L);

// ----------------------------------------------------------------------------

void
OSXAppPackagerParams::Print()
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

OSXAppPackager::OSXAppPackager( const MPlatformServices& services, MacSimulatorServices *simulatorServices /* = NULL */ )
	: PlatformAppPackager( services, TargetDevice::kOSXPlatform ),
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

	Lua::DoBuffer( fVM, & luaload_OSXPackageApp, NULL);
}

OSXAppPackager::~OSXAppPackager()
{
}

const char*
OSXAppPackager::GetAppTemplatePath()
{
	return [[[XcodeToolHelper pathForResources] stringByAppendingPathComponent:@"OSXAppTemplate.zip"] UTF8String];
}

int
OSXAppPackager::Build( AppPackagerParams * params, const char* tmpDirBase )
{
	int result = PlatformAppPackager::kNoError;
	OSXAppPackagerParams * osxParams = (OSXAppPackagerParams *) params;
	time_t startTime = time(NULL);

	const char tmpTemplate[] = "CLtmpXXXXXX";
	char tmpDir[kDefaultNumBytes + 1]; Rtt_ASSERT( kDefaultNumBytes > ( strlen( tmpDirBase ) + strlen( tmpTemplate ) ) );
	int tmpDirLen = snprintf( tmpDir, kDefaultNumBytes, "%s%s", tmpDirBase, tmpTemplate );
	mktemp(tmpDir);
	
    const char kCmdFormat[] = "mkdir -p %s";
    char cmd[kDefaultNumBytes + 1]; Rtt_ASSERT( kDefaultNumBytes > ( sizeof( kCmdFormat ) + tmpDirLen ) );
    snprintf( cmd, kDefaultNumBytes, kCmdFormat, tmpDir );
    
    const char kTmpResourceCarFormat[] = "%s-resource.car";
    char tmpResourceCar[kDefaultNumBytes + 1]; Rtt_ASSERT( kDefaultNumBytes > ( sizeof( kTmpResourceCarFormat ) + tmpDirLen ) );
    snprintf( tmpResourceCar, kDefaultNumBytes, kTmpResourceCarFormat, tmpDir );
    
    // Create the temporary directory
	if ( Rtt_VERIFY( 0 == system( cmd ) ) )
	{
        osxParams->SetIncludeBuildSettings(true);
		
		String tmpPluginsDir;
		tmpPluginsDir.Set(tmpDir);
		tmpPluginsDir.Append("-plugins");
		String outputDir;
		outputDir.Set(tmpDir);

		if ((result = PrepackagePlugins(osxParams, tmpPluginsDir, outputDir)) != PlatformAppPackager::kNoError)
		{
			return result;
		}
			
        if ( CompileScripts( osxParams, tmpDir ) && ArchiveDirectoryTree(osxParams, tmpDir, tmpResourceCar) )
        {
            lua_State *L = fVM;
            lua_getglobal( L, "OSXPostPackage" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
            
            // params for Lua call
            lua_newtable( L );
            {
                lua_pushstring( L, osxParams->GetSrcDir() );
                lua_setfield( L, -2, "srcAssets" );
                
                lua_pushstring( L, tmpResourceCar );
                lua_setfield( L, -2, "compiledLuaArchive" );
                
				lua_pushstring( L, tmpDir );
				lua_setfield( L, -2, "tmpDir" );
				
				lua_pushstring( L, tmpPluginsDir.GetString() );
				lua_setfield( L, -2, "tmpPluginsDir" );
				
                lua_pushstring( L, osxParams->GetDstDir() );
                lua_setfield( L, -2, "dstDir" );
                
                String sanitizedName;
                PlatformAppPackager::EscapeFileName(osxParams->GetAppName(), sanitizedName);
                lua_pushstring( L, sanitizedName.GetString() );
                lua_setfield( L, -2, "bundledisplayname" );
                
                lua_pushstring( L, osxParams->GetVersion() );
                lua_setfield( L, -2, "bundleversion" );
                
                lua_pushstring( L, osxParams->GetProvisionFile() );
                lua_setfield( L, -2, "provisionFile" );
                
                lua_pushstring( L, osxParams->GetIdentity() );
                lua_setfield( L, -2, "signingIdentity" );
                
                lua_pushinteger( L, osxParams->GetTargetDevice() );
                lua_setfield( L, -2, "targetDevice" );
                
                lua_pushstring( L, TargetDevice::StringForPlatform( osxParams->GetTargetPlatform() ) );
                lua_setfield( L, -2, "targetPlatform" );
                
                lua_pushstring( L, GetAppTemplatePath() );
                lua_setfield( L, -2, "osxAppTemplate" );
                
				lua_pushstring( L, Rtt_STRING_BUILD );
				lua_setfield( L, -2, "corona_build_id" );

                lua_newtable(L);
                {
                    BOOL debugBuildProcess = ([[NSUserDefaults standardUserDefaults] integerForKey:@"debugBuildProcess"] > 1);
                    NSString* sdkRoot = [XcodeToolHelper getXcodePath];
                    NSString* codesign = [XcodeToolHelper pathForCodesignUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];

                    lua_pushstring( L, [sdkRoot UTF8String] );
                    lua_setfield( L, -2, "sdkRoot" );
                    
                    lua_pushstring( L, [codesign UTF8String] );
                    lua_setfield( L, -2, "codesign" );
                 }
                lua_setfield( L, -2, "xcodetoolhelper" );
            }
            
            // OSXPostPackage( params )
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
                    Rtt_TRACE_SIM( ( "BUILD %s\n", lua_tostring( L, -1 ) ) );
                    osxParams->SetBuildMessage( lua_tostring( L, -1 ) );
                }
                lua_pop( L, 1 );
            }
        }
        else
        {
            result = PlatformAppPackager::kLocalPackagingError;
        }

		// Clean up intermediate files
		Rtt_ASSERT(strcmp(tmpDir, "/") != 0);
		snprintf( cmd, kDefaultNumBytes, "rm -rf \"%s\"", tmpDir );
		(void)Rtt_VERIFY( 0 == system( cmd ) );
	}

    // Indicate status in the console
	if (PlatformAppPackager::kNoError == result)
	{
		Rtt_LogException("macOS build succeeded in %ld seconds", (time(NULL) - startTime));
	}
	else
	{
		Rtt_LogException("macOS build failed (%d) after %ld seconds", result, (time(NULL) - startTime));
	}

	return result;
}

const char *
OSXAppPackager::GetBundleId( const char *provisionFile, const char *appName ) const
{
	const char *result = NULL;

	lua_State *L = fVM;
	lua_getglobal( L, "getBundleId" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
	lua_pushstring( L, provisionFile );
	lua_pushstring( L, PlatformAppPackager::EscapeStringForIOS( appName ) );
	if ( Rtt_VERIFY( 0 == Lua::DoCall( L, 2, 1 ) ) )
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
OSXAppPackager::VerifyConfiguration() const
{
	// Add code to check existence of various utilities
	return true;
}

char* 
OSXAppPackager::Prepackage( AppPackagerParams * params, const char* tmpDir )
{
	char* result = NULL;

	// Then Prepackage like before
	// result = Super::Prepackage( params, tmpDir );
	
	return result;
}

int
OSXAppPackager::PrepackagePlugins(OSXAppPackagerParams *params, String& pluginsDir, String&outputDir)
{
	int result = PlatformAppPackager::kNoError;

#if !defined( Rtt_NO_GUI )
	// We don't currently support plugins for CoronaBuilder macOS desktop builds

	Runtime *runtime = params->GetRuntime();

	Rtt_TRACE(("OSXAppPackager::PrepackagePlugins: pluginsDir %s", pluginsDir.GetString()));

	// Extract the project's plugins to the "bin" directory.
	if (runtime != NULL && runtime->RequiresDownloadablePlugins())
	{
		// Unzip the project's plugins to an intermediate directory.
		bool wasUnzipped = UnzipPlugins(params, runtime, pluginsDir.GetString());
		if (!wasUnzipped)
		{
			return PlatformAppPackager::kLocalPackagingError;
		}

		// Compile the Lua plugins to the intermediate directory.
		// Note: Precompiled *.lu files are copied to the given directory by the below function.
		// Note: Uncompiled .lua files are in the same directory, "OSXPostPackage" ignores them
		// Note: Compiling already compiled Lua works fine, the result will be stripped
		OSXAppPackagerParams pluginParamsSettings(params->GetAppName(), "", pluginsDir.GetString(), outputDir.GetString());

		bool wasCompiled = CompileScripts(&pluginParamsSettings, outputDir.GetString());
		
		if (!wasCompiled)
		{
			if (Rtt_StringIsEmpty(pluginParamsSettings.GetBuildMessage()))
			{
				params->SetBuildMessage("Failed to compile plugin Lua scripts.");
			}
			else
			{
				params->SetBuildMessage(pluginParamsSettings.GetBuildMessage());
			}
			
			return PlatformAppPackager::kBuildError;
		}
	}
#endif

	return result;
}

int
OSXAppPackager::PackageForAppStore( OSXAppPackagerParams *osxParams, bool sendToAppStore, const char *itunesConnectUsername, const char *itunesConnectPassword )
{
	int result = PlatformAppPackager::kNoError;
	lua_State *L = fVM;
	lua_getglobal( L, "OSXPackageForAppStore" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );

	// params for Lua call
	lua_newtable( L );
	{
		lua_pushstring( L, osxParams->GetSrcDir() );
		lua_setfield( L, -2, "srcAssets" );

		lua_pushstring( L, osxParams->GetDstDir() );
		lua_setfield( L, -2, "dstDir" );

		String sanitizedName;
		PlatformAppPackager::EscapeFileName(osxParams->GetAppName(), sanitizedName);
		lua_pushstring( L, sanitizedName.GetString() );
		lua_setfield( L, -2, "dstFile" );

		lua_pushstring( L, osxParams->GetAppName() );
		lua_setfield( L, -2, "bundledisplayname" );

		lua_pushstring( L, osxParams->GetVersion() );
		lua_setfield( L, -2, "bundleversion" );

		lua_pushstring( L, osxParams->GetIdentity() );
		lua_setfield( L, -2, "signingIdentity" );

		lua_pushstring( L, osxParams->GetAppSigningIdentity() );
		lua_setfield( L, -2, "appSigningIdentity" );

		lua_pushstring( L, osxParams->GetInstallerSigningIdentity() );
		lua_setfield( L, -2, "installerSigningIdentity" );

		lua_pushinteger( L, osxParams->GetTargetDevice() );
		lua_setfield( L, -2, "targetDevice" );

		lua_pushstring( L, TargetDevice::StringForPlatform( osxParams->GetTargetPlatform() ) );
		lua_setfield( L, -2, "targetPlatform" );

		lua_pushstring( L, Rtt_STRING_BUILD );
		lua_setfield( L, -2, "corona_build_id" );

		lua_pushstring( L, [[[XcodeToolHelper pathForResources] stringByAppendingPathComponent:@"OSXApp.xcent"] UTF8String] );
		lua_setfield( L, -2, "osxAppEntitlements" );

		lua_pushboolean( L, sendToAppStore );
		lua_setfield( L, -2, "sendToAppStore" );
		
		lua_pushstring( L, osxParams->GetProvisionFile() );
		lua_setfield( L, -2, "provisionFile" );

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

	// OSXPostPackage( params )
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
			osxParams->SetBuildMessage( lua_tostring( L, -1 ) );
		}
		lua_pop( L, 1 );
	}

	return result;
}

int
OSXAppPackager::PackageForSelfDistribution( OSXAppPackagerParams *osxParams, bool createDMG )
{
	int result = PlatformAppPackager::kNoError;
	lua_State *L = fVM;
	lua_getglobal( L, "OSXPackageForSelfDistribution" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );

	// params for Lua call
	lua_newtable( L );
	{
		lua_pushstring( L, osxParams->GetSrcDir() );
		lua_setfield( L, -2, "srcAssets" );

		lua_pushstring( L, osxParams->GetDstDir() );
		lua_setfield( L, -2, "dstDir" );

		String sanitizedName;
		PlatformAppPackager::EscapeFileName(osxParams->GetAppName(), sanitizedName);
		lua_pushstring( L, sanitizedName.GetString() );
		lua_setfield( L, -2, "bundledisplayname" );

		lua_pushstring( L, osxParams->GetVersion() );
		lua_setfield( L, -2, "bundleversion" );

		lua_pushstring( L, osxParams->GetIdentity() );
		lua_setfield( L, -2, "signingIdentity" );

		lua_pushstring( L, osxParams->GetAppSigningIdentity() );
		lua_setfield( L, -2, "appSigningIdentity" );

		lua_pushstring( L, osxParams->GetInstallerSigningIdentity() );
		lua_setfield( L, -2, "installerSigningIdentity" );
		
		lua_pushinteger( L, osxParams->GetTargetDevice() );
		lua_setfield( L, -2, "targetDevice" );

		lua_pushstring( L, TargetDevice::StringForPlatform( osxParams->GetTargetPlatform() ) );
		lua_setfield( L, -2, "targetPlatform" );

		lua_pushstring( L, GetAppTemplatePath() );
		lua_setfield( L, -2, "osxAppTemplate" );

		lua_pushstring( L, Rtt_STRING_BUILD );
		lua_setfield( L, -2, "corona_build_id" );

		lua_pushboolean( L, createDMG );
		lua_setfield( L, -2, "createDMG" );

		lua_newtable(L);
		{
            BOOL debugBuildProcess = ([[NSUserDefaults standardUserDefaults] integerForKey:@"debugBuildProcess"] > 1);
			NSString* sdkRoot = [XcodeToolHelper getXcodePath];
			NSString* codesign = [XcodeToolHelper pathForCodesignUsingDeveloperBase:sdkRoot printWarning:debugBuildProcess];

			lua_pushstring( L, [sdkRoot UTF8String] );
			lua_setfield( L, -2, "sdkRoot" );

			lua_pushstring( L, [codesign UTF8String] );
			lua_setfield( L, -2, "codesign" );
		}
		lua_setfield( L, -2, "xcodetoolhelper" );
	}

	// OSXPostPackage( params )
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
			osxParams->SetBuildMessage( lua_tostring( L, -1 ) );
		}
		lua_pop( L, 1 );
	}
	
	return result;
}
	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


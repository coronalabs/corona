//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// Note: error messages and warnings need to be prefaced with "error:" or "warning:" (lowercase)
// to be detected as such by Xcode and Android Studio (and thus flagged in the build log)

#include "Core/Rtt_Build.h"

#include "Rtt_CoronaBuilder.h"
#include "Rtt_LuaLibBuilder.h"

#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_TargetDevice.h"

#include <stdio.h>
#include <string.h>

#include "Rtt_BuildParams.h"
#include "Rtt_AppPackagerFactory.h"
#include "Rtt_PlatformAppPackager.h"
#include "Rtt_DownloadPluginsMain.h"

#include "Rtt_Car.h"

#include "Rtt_FileSystem.h"
#include "Rtt_Archive.h"

#ifdef Rtt_DEBUG
// #warning *** Using test durations ***
const int ONE_DAY_IN_SECONDS = 60; // make things happen every minute instead of every day for testing
#else
const int ONE_DAY_IN_SECONDS = (24 * 60 * 60);
#endif
const int MAX_MISSED_REAUTHS = 7;
const int ONE_WEEK_IN_SECONDS = (7 * ONE_DAY_IN_SECONDS);

#if defined(Rtt_MAC_ENV)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

Rtt_EXPORT int luaopen_lfs (lua_State *L);
int luaload_json(lua_State *L);
int luaload_dkjson(lua_State *L);
int luaload_CoronaBuilder(lua_State *L);
int luaload_CoronaPListSupport(lua_State *L);

#ifdef Rtt_DEBUG
	void lua_stackdump(lua_State* L);
#endif

// ----------------------------------------------------------------------------

class CoronaBuilderParams
{
	public:
		typedef enum _Command
		{
			kUnknownCommand = 0, // Default command
			kAuthorizeCommand,
			kDeauthorizeCommand,
			kCheckAuthorizeCommand,
			kBuildCommand,
			kAppSignCommand,
			kCarCommand,
			kPluginsCommand,
			kVersionCommand,
			
			kNumCommands
		}
		Command;

	public:
		CoronaBuilderParams( int argc, const char *argv[] );

	public:
		Command GetCommand() const { return fCommand; }
		const char* GetCommandArg( int index ) const;
		int GetNumCommandArgs() const { return ( fArgc - fCommandArgBase ); }

	private:
		int fArgc;
		const char **fArgv;
		int fCommandArgBase;
		Command fCommand;
};

CoronaBuilderParams::CoronaBuilderParams( int argc, const char *argv[] )
:	fArgc( argc ),
	fArgv( argv ),
	fCommandArgBase( argc ),
	fCommand( kUnknownCommand )
{
	if ( argc > 1 )
	{
		if ( 0 == Rtt_StringCompareNoCase( argv[1], "authorize" ) )
		{
			fCommand = kAuthorizeCommand;
		}
		else if (0 == Rtt_StringCompareNoCase(argv[1], "deauthorize"))
		{
			fCommand = kDeauthorizeCommand;
		}
		if ( 0 == Rtt_StringCompareNoCase( argv[1], "checkauth" ) )
		{
			fCommand = kCheckAuthorizeCommand;
		}
		else if (0 == Rtt_StringCompareNoCase(argv[1], "build"))
		{
			fCommand = kBuildCommand;
		}
		else if (0 == Rtt_StringCompareNoCase(argv[1], "app_sign"))
		{
			fCommand = kAppSignCommand;
		}
		else if (0 == Rtt_StringCompareNoCase(argv[1], "car"))
		{
			fCommand = kCarCommand;
		}
		else if (0 == Rtt_StringCompareNoCase(argv[1], "plugins"))
		{
			fCommand = kPluginsCommand;
		}
		else if (0 == Rtt_StringCompareNoCase(argv[1], "version"))
		{
			fCommand = kVersionCommand;
		}

		// command arguments start after the command itself
		fCommandArgBase = 2;
	}
}

const char*
CoronaBuilderParams::GetCommandArg( int index ) const
{
	const char *result = NULL;

	if ( index < GetNumCommandArgs() )
	{
		int i = index + fCommandArgBase;
		result = fArgv[i];
	}

	return result;
}

// ----------------------------------------------------------------------------

static const char kStdinPath[] = "";

// Returns path to file with metadata for building. And also format of the file.
static const char*
ParseBuildParams( const CoronaBuilderParams& params, BuildParams::Format& outFormat, int &lastArg )
{
	const char *result = NULL;

	bool isFormatLua = false;

	for ( int i = 0, iMax = params.GetNumCommandArgs(); i < iMax; i++ )
	{
		const char *arg = params.GetCommandArg( i );
		if ( '-' == arg[0] )
		{
			if ( 0 == strcmp( arg, "--lua" ) )
			{
				isFormatLua = true;
			}
			else if ( 0 == strcmp( arg, "-" ) )
			{
				result = kStdinPath;
			}
		}
		else
		{
			result = arg;
			lastArg = i;
			break;
		}
	}

	// If we haven't already set isFormatLua and we have a filename, check to see if its extension is ".lua"
	if (! isFormatLua && result != NULL)
	{
		const char *extn = strrchr(result, '.');

		if(extn != NULL)
		{
			isFormatLua = strcmp(extn, ".lua") == 0;
		}
	}
	
	outFormat = ( isFormatLua ? BuildParams::kLuaFormat : BuildParams::kJsonFormat );

	return result;
}

// ----------------------------------------------------------------------------

CoronaBuilder::CoronaBuilder(
	const MPlatform& platform,
	const MPlatformServices& services )
:	fPlatform( platform ),
	fServices( services ),
	fL( Lua::New( true ) )
{
	Rtt::String offlineModeStr;
	lua_pushlightuserdata( fL, this );
	Lua::RegisterModuleLoader( fL, "builder", LuaLibBuilder::Open, 1 );

	// Open LuaFileSystem
	lua_pushcfunction(fL, luaopen_lfs);
	lua_pushstring(fL, "lfs");
	lua_call(fL, 1, 0);

#if defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS) || defined(CORONABUILDER_OSX)
	Lua::RegisterModuleLoader( fL, "CoronaPListSupport", Lua::Open< luaload_CoronaPListSupport > );
#endif
	Lua::RegisterModuleLoader( fL, "dkjson", Lua::Open< luaload_dkjson >, 0 );
	Lua::RegisterModuleLoader( fL, "json", Lua::Open< luaload_json >, 0 );

	Lua::DoBuffer( fL, &luaload_CoronaBuilder, NULL);
	
}

CoronaBuilder::~CoronaBuilder()
{
	lua_close( fL );

//	delete fAuthorizer;
//	delete fAuthorizerDelegate;
}

void
CoronaBuilder::Usage( const char *arg0 )
{
	String progname(String(arg0).GetLastPathComponent());

	fprintf( stderr,
		"%s command [...]\n"
		"\n", progname.GetString() );

	fprintf( stderr,
		"'command' can be one of:\n"
		"\tauthorize\n"
		"\tdeauthorize\n"
        "\tbuild\n"
		"\tplugins\n"
        "\tversion\n"
		"\n" );
		
	fprintf( stderr,
		"'authorize' has 2 required arguments:\n"
		"\t%s authorize user password\n"
		"\n", progname.GetString());

	fprintf( stderr,
		"'deauthorize' does not require arguments. If you provide arguments, you must supply both.\n"
		"\t%s deauthorize [user password]\n"
		"\n", progname.GetString());

	fprintf( stderr,
		"'build' takes the following arguments:\n"
		"\t%s build [--lua] argsFile\n"
		"\targsFile can be a JSON file or a Lua file. JSON is assumed by default.\n"
		"\targsFile can also be '-' in which case stdin is assumed.\n"
		"\n"
		"\tBelow is an example of an args file in Lua:\n"
		"\t\tlocal params =\n"
		"\t\t{\n"
		"\t\t\tplatform='ios',\n"
		"\t\t\tappName='Hello',\n"
		"\t\t\tappVersion='1.0',\n"
		"\t\t\tdstPath='/Users/appdev/Desktop',\n"
		"\t\t\tcertificatePath='/Users/appdev/Library/MobileDevice/Provisioning Profiles/Dev.mobileprovision',\n"
		"\t\t\tprojectPath='/Applications/Corona/SampleCode/Graphics/Fishies',\n"
		"\t\t}\n"
		"\t\treturn params\n"
		"\n", progname.GetString());

	fprintf( stderr,
			"'plugins' takes the following arguments:\n"
			"\t%s plugins download platform buildSettings dest\n"
			"\tdownload subcommand.\n"
			"\tplatform can be 'iphone' or 'android' (not implemented yet).\n"
			"\tbuildSettings is full path to build.settings file.\n"
			"\tdest is path to Corona.xcconfig for iphone platform.\n"
			"\n", progname.GetString());


	fprintf( stderr,
            "'version' does not take arguments.\n"
            "\t%s version\n"
			"\n", progname.GetString());
}


int
CoronaBuilder::Main( int argc, const char *argv[] )
{
	String debugBuildProcess;
	fServices.GetPreference("debugBuildProcess", &debugBuildProcess);
	if(!debugBuildProcess.IsEmpty()) {
#if defined(Rtt_WIN_ENV)
		if (getenv("DEBUG_BUILD_PROCESS") == NULL) {
			_putenv_s("DEBUG_BUILD_PROCESS", debugBuildProcess);
		}
#else
		setenv("DEBUG_BUILD_PROCESS", debugBuildProcess, 0);
#endif
	}
	int result = -1;
    Rtt::String ticketData;

	fCommandPath.Set(argv[0]);

	CoronaBuilderParams params( argc, argv );

	CoronaBuilderParams::Command cmd = params.GetCommand();

	switch ( cmd )
	{
		case CoronaBuilderParams::kAuthorizeCommand:
		case CoronaBuilderParams::kCheckAuthorizeCommand:
		case CoronaBuilderParams::kDeauthorizeCommand:
		case CoronaBuilderParams::kAppSignCommand:
			fprintf( stderr, "Command %s is no longer supported. Execution skipped.\n", argv[1]);
			break;
		case CoronaBuilderParams::kBuildCommand:
			{
				BuildParams::Format format = BuildParams::kJsonFormat;
				int lastArg = 0;
				const char *path = ParseBuildParams( params, format, lastArg );

				printf("CoronaBuilder: building project with '%s'\n", path);
				BuildParams params( fL, path, format, argc-(lastArg+3), argv+(lastArg+3) );
				if ( params.IsValid() )
				{
					result = Build( params );
				}
				else
				{
					fprintf( stderr, "error: CoronaBuilder: Could not build because of malformed or invalid build arguments\n" );
				}
			}
			break;
		case CoronaBuilderParams::kPluginsCommand:
			{
				DownloadPluginsMain downloader(fL);
				result = downloader.Run(argc - 2, argv + 2 );
			}
			break;
		case CoronaBuilderParams::kCarCommand:
			{
				result = Rtt_CarMain( argc - 1, argv + 1 );
			}
			break;
        case CoronaBuilderParams::kVersionCommand:
            {
                printf("%s\n", Rtt_STRING_BUILD);
            }
            result = 0;
            break;
		default:
			Usage( argv[0] );
			break;
	}

	return result;
}

#ifdef Rtt_DEBUG
void
lua_stackdump(lua_State* L)
{
	Rtt_LUA_STACK_GUARD( L );

	int top = lua_gettop(L);
	int i;

	printf("=== Lua Stack Dump: %d frames ===\n",top);

	for (i = 1; i <= top; i++)
	{  /* repeat for each level */
		int t = lua_type(L, i);
		printf("[%d]: ", i);
		switch (t)
		{
			case LUA_TSTRING:  /* strings */
				printf("string: '%s'\n", lua_tostring(L, i));
				break;
			case LUA_TBOOLEAN:  /* booleans */
				printf("boolean %s\n",lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:  /* numbers */
				printf("number: %g\n", lua_tonumber(L, i));
				break;
			case LUA_TTABLE:  /* tables */
				/* table is in the stack at index 't' */
				lua_pushnil(L);  /* first key */
				printf("table:\n");
				while (lua_next(L, i) != 0)
				{
					/* uses 'key' (at index -2) and 'value' (at index -1) */
					printf("    [%s] - %s\n",
						   lua_tostring(L, -2),
						   lua_typename(L, lua_type(L, -1)));
					/* removes 'value'; keeps 'key' for next iteration */
					lua_pop(L, 1);
				}
				break;
			default:  /* other values */
				printf("%s: %p\n", lua_typename(L, t), lua_topointer(L, i));
				break;
		}
	}
	printf("\n");  /* end the listing */

	lua_settop(L, top);
}
#endif // Rtt_DEBUG


int
CoronaBuilder::Build( const BuildParams& params ) const
{
	int result = kBuildError;

	String msg;

	TargetDevice::Platform targetPlatform = params.GetTargetPlatform();

	if (targetPlatform == TargetDevice::kUnknownPlatform)
	{
		return result;
	}

	{
		AppPackagerFactory factory( fServices );
		PlatformAppPackager *packager = params.CreatePackager( factory, targetPlatform );

		AppPackagerParams *appParams = params.CreatePackagerParams( factory, targetPlatform );
		if ( appParams )
		{
			AppPackagerContext context( (TargetDevice::Platform)appParams->GetTargetPlatform() );
			String tmpDir;
			fPlatform.PathForFile( NULL, MPlatform::kTmpDir, MPlatform::kDefaultPathFlags, tmpDir );

			const char kBuildSettings[] = "build.settings";
			String buildSettingsPath( & fServices.Platform().GetAllocator() );

			buildSettingsPath.Set( appParams->GetSrcDir() );
			buildSettingsPath.Append( LUA_DIRSEP );
			buildSettingsPath.Append( kBuildSettings );

			const char * path = buildSettingsPath.GetString();
			if ( !fServices.Platform().FileExists( path ) )
			{
				path = NULL;
			}

			appParams->SetBuildSettingsPath( path );

			appParams->Print();
			packager->ReadBuildSettings(appParams->GetSrcDir());
			int code = packager->Build( appParams, tmpDir.GetString() );

			if ( 0 == code )
			{
				fprintf( stderr, "\nBuild succeeded [%s]\n",
					appParams->GetDstDir() );

				result = kNoError;
			}
			else
			{
				char resultCode[1024] = {};
				snprintf(resultCode, 1023, "Unknown build error (%d).", code);
				msg.Set( appParams->GetBuildMessage() ? appParams->GetBuildMessage()  : resultCode );
			}
		}
		else
		{
			msg.Set("Could not build because of earlier errors and/or missing build arguments");
		}

		delete appParams;
		delete packager;
	}

	if ( msg.GetLength() > 0 )
	{
		if (! Rtt_StringStartsWith(msg, "ERROR: "))
		{
			// The lower case "error:" makes Xcode notice an error occurred
			fprintf( stderr, "error: CoronaBuilder: " );
		}

		fprintf( stderr, "%s\n", msg.GetString() );
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


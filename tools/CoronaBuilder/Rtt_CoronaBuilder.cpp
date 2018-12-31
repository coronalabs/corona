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

// Note: error messages and warnings need to be prefaced with "error:" or "warning:" (lowercase)
// to be detected as such by Xcode and Android Studio (and thus flagged in the build log)

#include "Core/Rtt_Build.h"

#include "Rtt_CoronaBuilder.h"
#include "Rtt_LuaLibBuilder.h"

#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_ConsoleAuthorizationDelegate.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_TargetDevice.h"

#include <stdio.h>
#include <string.h>

#include "Rtt_BuildParams.h"
#include "Rtt_AppPackagerFactory.h"
#include "Rtt_PlatformAppPackager.h"
#include "Rtt_WebServicesSession.h"
#include "Rtt_DownloadPluginsMain.h"
#include "Rtt_CoronaBuilderSplashControl.h"

#include "Rtt_AppSign.h"
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
ParseBuildParams( const CoronaBuilderParams& params, BuildParams::Format& outFormat )
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
	fUsr(),
	fL( Lua::New( true ) )
//	fAuthorizerDelegate( NULL ),
//	fAuthorizer( NULL )
{
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

void CoronaBuilder::InsertBuildId(const char *carFile)
{
	String url("https://backendapi.coronalabs.com/v1/buildid/native/");
	url.Append(fUsr);
	String build_id, error;
	std::map<std::string, std::string> headers;
	if(GetPlatform().HttpDownload(url.GetString(), build_id, error, headers) && build_id.GetLength())
	{
		String contents("return \"");
		contents.Append(build_id);
		contents.Append("\"");

#ifdef WIN32
		char tmpDirTemplate[_MAX_PATH];
		const char* tmp = getenv("TMP");
		if (tmp == NULL)
		{
			tmp = getenv("TEMP");
		}

		if (tmp)
		{
			_snprintf(tmpDirTemplate, sizeof(tmpDirTemplate), "%s\\CBASCXXXXXX", tmp);
		}
		else
		{
			strcpy(tmpDirTemplate, "\\tmp\\CBASCXXXXXX");
		}
#else
		char tmpDirTemplate[] = "/tmp/CBASCXXXXXX";
#endif
		const char *tmpDirName = Rtt_MakeTempDirectory(tmpDirTemplate);
		String tmpFilename(tmpDirName);
		tmpFilename.AppendPathComponent("_Corona_Build_Number.lu");

		bool writeSuccess = false;
		FILE *fp = Rtt_FileOpen(tmpFilename, "wb");
		if (fp != NULL)
		{
			writeSuccess = (fputs(contents.GetString(), fp) > 0);
		}
		Rtt_FileClose(fp);

		if(writeSuccess)
		{
			const char *srcPaths[1] = { tmpFilename };
			Archive::Serialize( carFile, 1, srcPaths );
		}
	}
}

int
CoronaBuilder::Main( int argc, const char *argv[] )
{
	int result = -1;
    Rtt::String ticketData;

	fCommandPath.Set(argv[0]);

	CoronaBuilderParams params( argc, argv );

	CoronaBuilderParams::Command cmd = params.GetCommand();

	switch ( cmd )
	{
		case CoronaBuilderParams::kAuthorizeCommand:
            // Remove any existing authorization
            fServices.GetPreference( Authorization::kTicketKey, &ticketData );

            if ( ticketData.GetString() )
            {
                Deauthorize( params.GetCommandArg( 0 ), params.GetCommandArg( 1 ) );
            }

			result = Authorize( params.GetCommandArg( 0 ), params.GetCommandArg( 1 ) );
			if ( kNoError == result )
			{
				fprintf( stderr, "Authorization succeeded.\n" );
			}
			break;
		case CoronaBuilderParams::kCheckAuthorizeCommand:
			result = Reauthorize( );
			if ( kNoError == result )
			{
				fprintf( stderr, "Authorization check succeeded.\n" );
			}
			else
			{
				String reauthMesg;

				fServices.GetPreference("reauthMesg", &reauthMesg);
				if (reauthMesg.GetString() != NULL)
				{
					fprintf(stderr, "error: CoronaBuilder: %s\n", reauthMesg.GetString());
				}
			}
			break;
		case CoronaBuilderParams::kDeauthorizeCommand:
			result = Deauthorize( params.GetCommandArg( 0 ), params.GetCommandArg( 1 ) );
			if ( kNoError == result )
			{
				fprintf( stderr, "Deauthorization succeeded.\n" );
			}
			break;
		case CoronaBuilderParams::kBuildCommand:
			{
				BuildParams::Format format = BuildParams::kJsonFormat;
				const char *path = ParseBuildParams( params, format );

				printf("CoronaBuilder: building project with '%s'\n", path);
				BuildParams params( fL, path, format );
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
		case CoronaBuilderParams::kAppSignCommand:
			{
				// CoronaBuilder app_sign sign "developerkey.cert" "resource.corona-archive" "$EXE_PATH" little [platform bundleID]

				ConsoleAuthorizationDelegate delegate;
				if ( VerifyPermission( delegate, kAppSignPermission ) )
				{
					const char *platformName = (argc > 7 && argv[7][0] != '-') ? argv[7] : NULL;
					const char *bundleID = (argc > 8 && argv[8][0] != '-') ? argv[8] : NULL;

					bool sign = (strncmp(argv[2], "sign", 4) == 0);

					if ( sign )
					{
						if(! CanCustomizeSplashScreen(platformName, bundleID))
						{
							String carFile(argv[4]);

							// This information has already been conveyed at this point
							// fprintf(stderr, "NOTE: using the default Corona splash screen\nActivate the Splash Screen Control plugin for user '%s' if you don't want it\nby visiting https://marketplace.coronalabs.com/plugin/splash-screen-control\n", fUsr.GetString());

							Rtt_CoronaBuilderAddSplashControl(carFile);
						}
						InsertBuildId(argv[4]);
					}

					result = Rtt_AppSignMain( argc - 1, argv + 1 );

					// replace all architectures
					while (sign && result == 0 && Rtt_AppSignMain( argc - 1, argv + 1 ) == 0);
				}
				else
				{
					fprintf( stderr, "error: CoronaBuilder: This computer is not properly authorized. You must have an account with an '%s' subscription. Please use the 'authorize' command or check that your subscription type is valid\n", AuthorizationTicket::DisplayStringForSubscription( AuthorizationTicket::kEnterpriseNativeExtensionsSubscription ) );
				}
			}
			break;
		case CoronaBuilderParams::kPluginsCommand:
			{
				ConsoleAuthorizationDelegate delegate;
				if ( VerifyPermission( delegate, kBuildPermission ) )
				{
					DownloadPluginsMain downloader(fL);
					result = downloader.Run(argc - 2, argv + 2, fUsr.GetString() );
				}
				else
				{
					fprintf( stderr, "error: CoronaBuilder: This computer is not properly authorized. You must have an account with an '%s' subscription. Please use the 'authorize' command or check that your subscription type is valid\n", AuthorizationTicket::DisplayStringForSubscription( AuthorizationTicket::kEnterpriseNativeExtensionsSubscription ) );
				}
			}
			break;
		case CoronaBuilderParams::kCarCommand:
			{
				ConsoleAuthorizationDelegate delegate;
				if ( VerifyPermission( delegate, kCarPermission ) )
				{
					result = Rtt_CarMain( argc - 1, argv + 1 );
				}
				else
				{
					fprintf( stderr, "error: CoronaBuilder: This computer is not properly authorized. You must have an account with an '%s' subscription. Please use the 'authorize' command or check that your subscription type is valid\n", AuthorizationTicket::DisplayStringForSubscription( AuthorizationTicket::kEnterpriseNativeExtensionsSubscription ) );
				}
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
/*
const Authorization *
CoronaBuilder::GetAuthorizer() const
{
	if ( ! fAuthorizer )
	{
		fAuthorizerDelegate = new ConsoleAuthorizationDelegate;
		fAuthorizer = new Authorization( fServices, * fAuthorizerDelegate );
	}

	return fAuthorizer;
}
*/

static const char*
InitializeAuthorizer( Authorization& authorizer, ConsoleAuthorizationDelegate& delegate )
{
	const char *errorMsg = NULL;

	if ( authorizer.Initialize() )
	{
		if ( authorizer.VerifyTicket() )
		{
		}
		else
		{
			errorMsg = "Could not launch Corona Builder. Invalid registration.";
		}
	}
	else
	{
		if ( delegate.HasLoginCredentials() )
		{
			errorMsg = "Could not authorize this computer. Please check your Internet connection or your login/password.\nTry putting your password in single-quotes.";
		}
		else if ( ! delegate.IsPreviouslyAuthorized() )
		{
			errorMsg = "You must authorize Corona Builder before performing other commands. Please run Corona Builder with the 'authorize' command.";
		}
		else
		{
			errorMsg = "Could not verify authorization.";
		}
	}

	return errorMsg;
}

static bool
DoesSubscriptionHavePermission( 
	AuthorizationTicket::Subscription sub,
	CoronaBuilder::Permission perm )
{
	return (sub != AuthorizationTicket::kUnknownSubscription);

#if 0
	// This is the old logic used when only Enterprise customers could do certain kinds of builds

	bool result = false;

	switch ( perm )
	{
		case CoronaBuilder::kBuildPermission:
            // Automated builds is a separate product
			result = ( AuthorizationTicket::kEnterpriseAutomatedBuildsSubscription == sub );
			break;
		case CoronaBuilder::kAppSignPermission:
			// TODO: Temporarily allow automated builds to access this permission
			// Until we fix the limitations with authorization
			result = ( AuthorizationTicket::kEnterpriseAutomatedBuildsSubscription == sub
						|| AuthorizationTicket::kEnterpriseNativeExtensionsSubscription == sub );
			break;
		case CoronaBuilder::kCarPermission:
			// TODO: Temporarily allow automated builds to access this permission
			// Until we fix the limitations with authorization
			result = ( AuthorizationTicket::kEnterpriseAutomatedBuildsSubscription == sub
					  || AuthorizationTicket::kEnterpriseNativeExtensionsSubscription == sub );
			break;
		case CoronaBuilder::kSplashScreenPermission:
			// TODO: Temporarily allow automated builds to access this permission
			// Until we fix the limitations with authorization
			result = ( AuthorizationTicket::kEnterpriseAutomatedBuildsSubscription == sub
					  || AuthorizationTicket::kEnterpriseNativeExtensionsSubscription == sub );
			break;
		default:
			break;
	}
	
	return result;
#endif
}


bool
CoronaBuilder::VerifyPermission(
	ConsoleAuthorizationDelegate& delegate,
	Permission perm ) const
{
	bool result = false;

	Authorization authorizer( fServices, delegate );
	const char *errorMsg = InitializeAuthorizer( authorizer, delegate );	

	if ( ! errorMsg )
	{
		const AuthorizationTicket *ticket = authorizer.GetTicket();

		AuthorizationTicket::Subscription sub = ticket->GetSubscription();
		if ( DoesSubscriptionHavePermission( sub, perm ) )
		{
			time_t expiration = ticket->GetExpiration();
			time_t licenseTimeRemaining = expiration - time(NULL);
			int numAttempts = 0;
			time_t lastAttemptTime = 0;
			String authStats;
			fServices.GetPreference( "authKey", &authStats );

			if (authStats.GetString() != NULL &&
				sscanf(authStats.GetString(), "%d %ld", &numAttempts, &lastAttemptTime) != 2)
			{
				// Something is fishy, we have a setting for the key but we can't parse it, assume malfeasance
				lastAttemptTime = 0;
			}

			time_t timeSinceLastCheck = time(NULL) - lastAttemptTime;

#ifdef Rtt_DEBUG
			printf("*** Using test durations (1 day == %d seconds) ***\n", ONE_DAY_IN_SECONDS);
			printf("licenseTimeRemaining: %ld; lastAttemptTime: %s", licenseTimeRemaining, ctime(&lastAttemptTime));
			printf("timeSinceLastCheck: %ld (recheck at %d)\n", timeSinceLastCheck, ONE_DAY_IN_SECONDS);
#endif

			if (licenseTimeRemaining < 0 || timeSinceLastCheck > ONE_DAY_IN_SECONDS)
			{
				if (numAttempts >= MAX_MISSED_REAUTHS)
				{
					// This ticket has expired or it's more than a day since we last checked ...
					// try to reauth to get a new ticket as the server may have new info
					// (this, aside from a slight delay, should be transparent to the user unless
					// their license has been revoked before it's original expiry time on the server)
					if (Reauthorize() != kNoError)
					{
						String reauthMesg;

						fServices.GetPreference("reauthMesg", &reauthMesg);
						if (reauthMesg.GetString() != NULL)
						{
							fprintf(stderr, "\nerror: CoronaBuilder: %s\n", reauthMesg.GetString());
						}

						return false;
					}

					if ( authorizer.Initialize() )
					{
						ticket = authorizer.GetTicket();
						expiration = ticket->GetExpiration();
						licenseTimeRemaining = expiration - time(NULL);
					}
				}
				else
				{
					ReauthorizeAsync();
				}
			}

			if ( licenseTimeRemaining < 0 )
			{
				struct tm *dt = localtime( & expiration );

				char buf[32];
				strftime( buf, sizeof( buf ), "%Y-%m-%d %H%M", dt);
				fprintf( stderr, "error: CoronaBuilder: the '%s' subscription for account '%s'\n\texpired at %s. Please renew your subscription\n",
						AuthorizationTicket::DisplayStringForSubscription( sub ),
						authorizer.GetTicket()->GetUsername(),
						buf );
			}
			else
			{
				if (licenseTimeRemaining < ONE_WEEK_IN_SECONDS)
				{
					fprintf(stderr, "warning: the current authorization will expire in about %ld day%s\n",
							(licenseTimeRemaining / ONE_DAY_IN_SECONDS), ((licenseTimeRemaining / ONE_DAY_IN_SECONDS) == 1 ? "" : "s"));
				}

				// If there's a authorization message waiting, print it
				String reauthMesg;
				fServices.GetPreference("reauthMesg", &reauthMesg);
				if (reauthMesg.GetString() != NULL)
				{
					fprintf(stderr, "warning: %s\n", reauthMesg.GetString());
				}

				result = true; // Success!

				if ( ! fUsr.GetString() )
				{
					fUsr.Set( authorizer.GetTicket()->GetUsername() );
				}
			}
		}
		else if ( sub != AuthorizationTicket::kUnknownSubscription )
		{
			// If no required sub (or ticket doesn't match required sub), then fail
			result = false;
		
			String reauthMesg;
			fServices.GetPreference("reauthMesg", &reauthMesg);
			if (reauthMesg.GetString() != NULL)
			{
				fprintf(stderr, "\nwarning %s\n", reauthMesg.GetString());
			}

			fprintf( stderr, "error: cannot launch CoronaBuilder. The current '%s' subscription does not have sufficient privileges\n",
				AuthorizationTicket::DisplayStringForSubscription( sub ) );
		}
	}
	else
	{
		fprintf( stderr, "error: CoronaBuilder: %s\n", errorMsg );
	}

	return result;
}

int
CoronaBuilder::Authorize( const char *inUsr, const char *inPwd ) const
{
	int result = kAuthorizationError;

    if (inUsr == NULL)
    {
        fprintf(stderr, "error: CoronaBuilder: missing email\n");

        exit( 1 );
    }
    if (inPwd == NULL)
    {
        fprintf(stderr, "error: CoronaBuilder: missing password\n");

        exit( 1 );
    }

	ConsoleAuthorizationDelegate delegate;
	Authorization authorizer( fServices, delegate );

	const char *errorMsg = NULL;
	if ( authorizer.Initialize() )
	{
		// Check for existing authorization
		if ( authorizer.VerifyTicket() )
		{
			const AuthorizationTicket *ticket = authorizer.GetTicket();
			String reauthMesg;

			AuthorizationTicket::Subscription sub = ticket->GetSubscription();

			if ( AuthorizationTicket::kUnknownSubscription != sub )
			{
				fprintf( stderr, "This computer is registered with the '%s' subscription.\n",
					AuthorizationTicket::DisplayStringForSubscription( sub ) );
				switch ( sub )
				{
					case AuthorizationTicket::kEnterpriseAutomatedBuildsSubscription:
					case AuthorizationTicket::kEnterpriseHostedTemplatesSubscription:
					case AuthorizationTicket::kEnterpriseNativeExtensionsSubscription:
						// Remember the last time we successfully reauthed
						RecordSuccessfulAuthorization();
						result = kNoError; // we're good to go
						break;
					default:
						fServices.GetPreference("reauthMesg", &reauthMesg);
						if (reauthMesg.GetString() != NULL)
						{
							fprintf(stderr, "\n%s\n", reauthMesg.GetString());
						}
						fprintf( stderr, "\nCorona Enterprise users: please run the 'deauthorize' command and then 'authorize' again to register this computer.\n" );
						break;
				}
			}
		}
	}
	else
	{
		// No existing authorization
		fServices.SetPreference("reauthMesg", NULL);
		delegate.SetLoginCredentials( inUsr, inPwd );
		errorMsg = InitializeAuthorizer( authorizer, delegate );

		if ( ! errorMsg )
		{
			const AuthorizationTicket *ticket = authorizer.GetTicket();

			AuthorizationTicket::Subscription sub = ticket->GetSubscription();
			if ( AuthorizationTicket::kUnknownSubscription != sub )
			{
				// Success!
				result = kNoError;

				if ( ! fUsr.GetString() )
				{
					fUsr.Set( authorizer.GetTicket()->GetUsername() );
				}
			}
			else
			{
				// If no valid sub, then fail
				errorMsg = "Could not authorize. No valid subscription found.";
			}
		}
	}

	if ( errorMsg )
	{
		fprintf( stderr, "error: CoronaBuilder: %s\n", errorMsg );
	}

	return result;
}

/*
 * CoronaBuilder::ReauthorizeAsync(  )
 *
 * Reauthorize in the background using the stored username and encrypted password to refresh the ticket.
 * Results are written to the user's defaults and will be looked at next time the user runs CoronaBuilder.
 * This prevents delays while checking for authorization.
 */
void
CoronaBuilder::ReauthorizeAsync(  ) const
{
#if defined(Rtt_MAC_ENV)

	pid_t processId;
	if ((processId = fork()) == 0)
	{
		// close std filedescs
		//close(0);
		//close(1);
		//close(2);

		char *app = (char *)fCommandPath.GetString();
		char *arg = (char *)"checkauth";
		char * const argv[] = { app, arg, NULL };
		if (execv(app, argv) < 0)
		{
			perror("execv error");
		}
	}
	else if (processId < 0)
	{
		perror("fork error");
	}

#elif defined(Rtt_WIN_ENV)
	// NEEDSWORK: make this async on Windows
	Reauthorize();
#endif
}

/*
 * CoronaBuilder::Reauthorize(  )
 *
 * Reauthorize using the stored username and encrypted password to refresh the ticket
 */
int
CoronaBuilder::Reauthorize(  ) const
{
	int result = kAuthorizationError;

	ConsoleAuthorizationDelegate delegate;
	Authorization authorizer( fServices, delegate );

	if ( authorizer.Initialize() )
	{
		const AuthorizationTicket *ticket = authorizer.GetTicket();

		if ( fServices.IsInternetAvailable() )
		{
			// We're going to get a definitive answer so any previous message will no longer be valid
			fServices.SetPreference("reauthMesg", NULL);

			if ( ! fUsr.GetString() )
			{
				fUsr.Set( ticket->GetUsername() );
			}
			const char *usr = fUsr.GetString();

			String encryptedPassword;
			String reauthMesg;
			fServices.GetPreference( usr, &encryptedPassword );
			
			result = authorizer.Authorize( usr, encryptedPassword, true, true );

			if (result == kNoError && authorizer.Initialize())
			{
				ticket = authorizer.GetTicket();

				AuthorizationTicket::Subscription sub = ticket->GetSubscription();

				if ( AuthorizationTicket::kUnknownSubscription != sub )
				{
					printf("Current authorization is '%s' for user '%s'\n", AuthorizationTicket::DisplayStringForSubscription(sub), fUsr.GetString() );

					RecordSuccessfulAuthorization();
				}
			}
			else
			{
				// We got an error from the server when trying to login, ensure they are asked to reauth next time
				char buf[BUFSIZ];
				snprintf(buf, BUFSIZ, "%d %ld", MAX_MISSED_REAUTHS + 1, (time(NULL) - ONE_DAY_IN_SECONDS));
				fServices.SetPreference( "authKey", buf );

				const char *reauthMesg = "License reauthorization failed.  You will need to reauthorize this computer";
				fServices.SetPreference("reauthMesg", reauthMesg);
				fprintf(stderr, "%s\n", reauthMesg);
			}
		}
		else
		{
			// No internet connection ... record the failure

			int numAttempts = 0;
			time_t lastAttemptTime = 0;
			char buf[BUFSIZ];

			String authStats;
			fServices.GetPreference( "authKey", &authStats );

			if (authStats.GetString() != NULL &&
				sscanf(authStats.GetString(), "%d %ld", &numAttempts, &lastAttemptTime) != 2)
			{
				// Something is fishy, we have a setting for the key but we can't parse it, assume malfeasance
				numAttempts = MAX_MISSED_REAUTHS;
			}

			// If it's been at least a day since we last recorded an attempt, update or fail
			if (lastAttemptTime < (time(NULL) - ONE_DAY_IN_SECONDS))
			{
				if (lastAttemptTime > 0 && lastAttemptTime < (time(NULL) - ONE_WEEK_IN_SECONDS))
				{
					// It's been over a week since our last successful reauth
					// This is mostly to protect against replay attacks though it will trigger
					// if they don't run for a really long time and then without an internet connection
					snprintf(buf, BUFSIZ, "License reauthorization needs to complete before CoronaBuilder can run.  Connect to the internet and try again");
					fServices.SetPreference("reauthMesg", buf);

					numAttempts = MAX_MISSED_REAUTHS;
					snprintf(buf, BUFSIZ, "%d %ld", numAttempts, (time(NULL) - ONE_DAY_IN_SECONDS));
					fServices.SetPreference( "authKey", buf );

					result = kAuthorizationError;
				}
				else if (numAttempts < MAX_MISSED_REAUTHS)
				{
#ifdef Rtt_DEBUG
					printf("***** adding %d more day(s)\n", (int)((time(NULL) - lastAttemptTime) / ONE_DAY_IN_SECONDS));
#endif
					// Add the number of days since we last checked
					numAttempts = numAttempts + (int)((time(NULL) - lastAttemptTime) / ONE_DAY_IN_SECONDS);
					snprintf(buf, BUFSIZ, "%d %ld", numAttempts, time(NULL));
					fServices.SetPreference( "authKey", buf );

					int numDaysRemaining = (numAttempts > MAX_MISSED_REAUTHS) ? 0 : (MAX_MISSED_REAUTHS - numAttempts);

					snprintf(buf, BUFSIZ, "License reauthorization was unable to access the internet for %d day%s.  You have %d day%s grace period remaining", numAttempts, (numAttempts == 1 ? "" : "s"), numDaysRemaining, (numDaysRemaining == 1 ? "" : "s"));
					fServices.SetPreference("reauthMesg", buf);
					result = kNoError; // we're good to go
				}
				else
				{
					snprintf(buf, BUFSIZ, "License reauthorization was unable to access the internet for %d days.    Connect to the internet and try again", MAX_MISSED_REAUTHS);
					fServices.SetPreference("reauthMesg", buf);
					result = kAuthorizationError;
				}
			}
		}
	}

	return result;
}

void
CoronaBuilder::RecordSuccessfulAuthorization() const
{
	char buf[BUFSIZ];

	snprintf(buf, BUFSIZ, "%d %ld", 0, time(NULL));
	fServices.SetPreference("authKey", buf);
	fServices.SetPreference("reauthMesg", NULL);
}

int
CoronaBuilder::Deauthorize( const char *inUsr, const char *inPwd ) const
{
	int result = kDeauthorizationError;

	const char *msg = NULL;

    if (inUsr == NULL)
    {
        fprintf(stderr, "error: CoronaBuilder: missing email\n");

        exit( 1 );
    }
    if (inPwd == NULL)
    {
        fprintf(stderr, "error: CoronaBuilder: missing password\n");

        exit( 1 );
    }

	if ( fServices.IsInternetAvailable() )
	{
		ConsoleAuthorizationDelegate delegate;
		delegate.SetLoginCredentials( inUsr, inPwd );

		Authorization authorizer( fServices, delegate );
		if ( ! authorizer.Initialize() )
		{
			msg = "Missing authorization data for deauthorization.";
		}
		else
		{
			const AuthorizationTicket *ticket = authorizer.GetTicket();
			const char *usr = ticket ? ticket->GetUsername() : inUsr;

			if ( usr )
			{
				Rtt::String encryptedPassword;
				fServices.GetPreference( usr, &encryptedPassword );
				if ( ! encryptedPassword.GetString() )
				{
					if ( inPwd )
					{
						Rtt::Authorization::Encrypt( inPwd, &encryptedPassword );
					}
				}

				if ( encryptedPassword.GetString() )
				{
					Rtt::String ticketData;
					fServices.GetPreference( Authorization::kTicketKey, &ticketData );

					const char *ticketBuffer = ticketData.GetString();
					if ( ticketBuffer )
					{
						int code = authorizer.Deauthorize(
							usr, encryptedPassword.GetString(), ticketBuffer );

						if ( Authorization::kAuthorizationDeauthorizeSuccess == code )
						{
							result = kNoError;
						}
						else
						{
							if ( Authorization::kAuthorizationDeauthorizeTryLater == code )
							{
								msg = "An error occurred while communicating with the server. Please try again later.";
							}
							else if ( Authorization::kAuthorizationPassword == code )
							{
								msg = "Could not deauthorize. Please contact CoronaLabs at support@coronalabs.com for further assistance.";
							}
							else
							{
								fServices.SetPreference( usr, NULL );
								msg = "Could not deauthorize. The password you entered was incorrect.";
							}
						}
					}
					else
					{
						msg = "No authorization data available to deauthorize.";
					}
				}
				else
				{
					msg = "Password must be supplied to deauthorize.";
				}
			}
			else
			{
				msg = "Username (e-mail) must be supplied to deauthorize.";
			}
		}
	}
	else
	{
		msg = "Could not connect to the server. Please check your Internet connection or try again later.";
	}

	if ( msg )
	{
		fprintf( stderr, "error: CoronaBuilder: %s\n", msg );
	}

	return result;
}

static const char*
CodeForErrorMessage( int code, const char *defaultMsg = NULL )
{
	const char *msg = NULL;

	switch( code )
	{
		case WebServicesSession::kNoError:
			break;
		case WebServicesSession::kApiKeyError:
			msg = "This version is no longer supported for device builds.";
			break;
		case WebServicesSession::kTokenExpiredError:
			msg = "Your computer's clock has the incorrect date and/or time. Please update with the correct time and try again.";
			break;
		case WebServicesSession::kAgreementError:
			msg = "The Corona Connect terms and conditions have changed. Please log onto the web site and agree to the new terms.";
			break;
		case WebServicesSession::kUnverifiedUserError:
			msg = "Your account has not been verified yet. A verification e-mail has been sent to you with further instructions on how to validate your account.";
			break;
		case WebServicesSession::kLoginError:
			msg = "Could not login because the password was incorrect.";
			break;
		case WebServicesSession::kExpiredError:
			msg = "Your subscription to the Corona Developer Program has expired. You must renew to continue building for devices.";
			break;
		case WebServicesSession::kBuildVersionMismatchError:
			// The version of the SDK is newer than any of the templates on the server, so the server doesn't
			// know what template to use. This should not happen in production, but it could happen in development.
			// 
			msg = "There was an error in the build request.";
			break;
		default:
			msg = ( defaultMsg ? defaultMsg : "An error occurred during the build, possibly a Lua compile error" );
			break;
	}

	return msg;
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

bool
CoronaBuilder::CanCustomizeSplashScreen(const char *platformName, const char *bundleID)
{
	int base = lua_gettop(fL);
	bool result = false;
	const char *userid = fUsr.GetString();

	lua_getglobal( fL, "CoronaBuilderDoesUserOwnSplashScreenControlPlugin" ); Rtt_ASSERT( lua_isfunction( fL, -1 ) );
	lua_pushstring( fL, userid );
	lua_pushstring( fL, platformName );
	lua_pushstring( fL, bundleID );

	if (Lua::DoCall( fL, 3, 2 ) == 0)
	{
		//lua_stackdump(fL);
		if (lua_isstring( fL, 2 ))
		{
			// Error message is 2nd value on stack
			const char *errorMsg = lua_tostring( fL, 2 );

			fprintf( stderr, "warning: CoronaBuilder: %s\n", errorMsg );
		}

		result = lua_toboolean( fL, 1 );
	}

	lua_settop( fL, base );

	return result;
}

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

	ConsoleAuthorizationDelegate delegate;
	if ( VerifyPermission( delegate, kBuildPermission ) )
	{
		AppPackagerFactory factory( fServices );
		PlatformAppPackager *packager = params.CreatePackager( factory, targetPlatform );

		AppPackagerParams *appParams = params.CreatePackagerParams( factory, targetPlatform );
		if ( appParams )
		{
			AppPackagerContext context( (TargetDevice::Platform)appParams->GetTargetPlatform() );

			const char *usr = fUsr.GetString();

			String encryptedPassword;
			fServices.GetPreference( usr, &encryptedPassword );

			if ( fServices.IsInternetAvailable() )
			{
				WebServicesSession session( fServices );

				int code = session.LoginWithEncryptedPassword(
					WebServicesSession::CoronaServerUrl(fServices),
					usr,
					encryptedPassword.GetString() );

				if ( WebServicesSession::kNoError == code )
				{
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
					code = packager->Build( appParams, session, tmpDir.GetString() );

					if ( WebServicesSession::kNoError == code )
					{
#if defined(Rtt_WIN_ENV)
						fprintf( stderr, "\nBuild succeeded [%s]\n",
							appParams->GetDstDir() );
#else
						fprintf( stderr, "\nBuild succeeded [%s/%s.%s]\n",
							appParams->GetDstDir(),
							appParams->GetAppName(),
							appParams->GetTargetPlatform() == TargetDevice::kAndroidPlatform ? "apk" : "app" );
#endif

						result = kNoError;
					}
					else
					{
						msg.Set( CodeForErrorMessage(
							code,
							appParams->GetBuildMessage() ) );
					}
				}
				else
				{
					msg.Set(CodeForErrorMessage( code ));
				}
			}
			else
			{
				msg.Set("Unable to connect to internet");
			}
		}
		else
		{
			msg.Set("Could not build because of earlier errors and/or missing build arguments");
		}

		delete appParams;
		delete packager;
	}
	else
	{
		msg.Set("This computer has not been properly authorized. Please use the 'authorize' command or check that your subscription type is valid");
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


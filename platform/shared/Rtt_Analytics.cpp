//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Version.h"
#include "Rtt_Analytics.h"
#include "Rtt_LuaContext.h"
#include "Rtt_MPlatform.h"
#include "Rtt_AuthorizationTicket.h"

#include <string.h>
#include <time.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

	// TODO: Move simulatorAnalytics.lua out of librtt and into rtt_player in XCode
	// Current issue with doing that is this lua file needs to be precompiled into C
	// via custom build step --- all .lua files in librtt already do that, so we're
	// taking a shortcut for now by putting it under librtt.


// simulatorAnalytics.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in a generated .cpp file. The file also contains the definition of the 
// following function which loads the bytecodes via luaL_loadbuffer.
int luaload_simulatorAnalytics(lua_State* L);

// ----------------------------------------------------------------------------

static const char kAnalyticsUrlKey[] = "url";
static const char kAnalyticsDeviceIdKey[] = "deviceId";
static const char kAnalyticsSessionIdKey[] = "sessionId";

Analytics::Analytics( const MPlatform& platform, const char* baseUrl, const char *deviceId )
:	fPlatform( platform ),
	fVM( NULL ),
	fSessionStarted( false ),
	fEnabled( true )
{
	if ( Rtt_VERIFY( baseUrl && deviceId ) )
	{
		fVM = LuaContext::NewLuaContext( & fPlatform.GetAllocator() );
		Rtt_ASSERT( fVM );

		fVM->Initialize( fPlatform, NULL );
		fVM->DoBuffer( & luaload_simulatorAnalytics, false );

		lua_State *L = fVM->L(); Rtt_ASSERT( L );

		size_t baseUrlLen = strlen( baseUrl );
		bool appendSlash = ( '/' != baseUrl[baseUrlLen - 1] );
		lua_pushstring( L, baseUrl );
		if ( appendSlash ) { lua_pushstring( L, "/" ); }
		lua_pushstring( L, "analytics-v2/analytics.php" );
		lua_concat( L, appendSlash ? 3 : 2 );
		lua_setglobal( L, kAnalyticsUrlKey );

		lua_pushstring( L, deviceId );
		lua_setglobal( L, kAnalyticsDeviceIdKey );
	}
}

Analytics::~Analytics()
{
	if ( IsSessionStarted() )
	{
		EndSession();
	}

	Rtt_DELETE( fVM );
}

void
Analytics::Log( int event, int eventdata, const char *eventstring )
{
	if ( fEnabled && fVM )
	{
		lua_State *L = fVM->L();
		lua_getglobal( L, "log" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
		char buffer[16];
		sprintf( buffer, "%d", event );
		lua_pushstring( L, buffer );

		lua_pushinteger( L, eventdata );

		const char kEmptyString[] = "";
		if ( ! eventstring )
		{
			eventstring = kEmptyString;
		}
		lua_pushstring( L, eventstring );
		
		sprintf( buffer, "%lu", time(NULL) );
		lua_pushstring( L, buffer );

		Rtt_VERIFY( 0 == LuaContext::DoCall( L, 4, 0 ) );
	}
}

void
Analytics::BeginSession( const char *sessionId )
{
	if ( fEnabled && fVM )
	{
		lua_State *L = fVM->L();
		lua_pushstring( L, sessionId );
		lua_setglobal( L, kAnalyticsSessionIdKey );

		fSessionStarted = true;
	}
}

void
Analytics::EndSession()
{
	if ( fEnabled && fVM && Rtt_VERIFY( fSessionStarted ) )
	{
		lua_State *L = fVM->L();

		Rtt_ASSERT( lua_gettop( L ) == 0 );

		lua_getglobal( L, kAnalyticsUrlKey ); Rtt_ASSERT( LUA_TSTRING == lua_type( L, -1 ) );
		const char *url = lua_tostring( L, -1 );

		if ( url )
		{
			lua_getglobal( L, "serialize" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
			lua_getglobal( L, kAnalyticsDeviceIdKey ); Rtt_ASSERT( LUA_TSTRING == lua_type( L, -1 ) );
			lua_getglobal( L, kAnalyticsSessionIdKey ); Rtt_ASSERT( LUA_TSTRING == lua_type( L, -1 ) );

			if ( Rtt_VERIFY( 0 == LuaContext::DoCall( L, 2, 1 ) ) )
			{
				const char* result = lua_tostring( L, -1 );

				// Rtt_TRACE( ( "Sending analytics data:\n%s", result ) );

				fPlatform.HttpPost( url, "xml", result );
			}

			// Pop result
			lua_pop( L, 1 );
		}

		// Pop url
		lua_pop( L, 1 );

		Rtt_ASSERT( lua_gettop( L ) == 0 );

		fSessionStarted = false;
	}
}

// Fetches a "Product Version" string constant appropriate for the given authorization ticket.
const char*
Analytics::GetProductVersionNameFrom( const AuthorizationTicket *ticket )
{
	const char* versionName;

	// Define the strings.
	#define Rtt_ANALYTICS_RUN_PREFIX	"version(" Rtt_STRING_VERSION ") build(" Rtt_STRING_BUILD ") "
#if !defined( Rtt_PROJECTOR )
	static const char kVersionNameFull[] = Rtt_ANALYTICS_RUN_PREFIX "sku(sdk)";
	static const char kVersionNameTrial[] = Rtt_ANALYTICS_RUN_PREFIX "sku(sdk-trial)";

	// Determine which version string to return depending on the ticket's current license info.
	versionName = kVersionNameFull;
	if (Rtt_VERIFY(ticket) && ticket->IsTrial())
	{
		versionName = kVersionNameTrial;
	}
	return versionName;
#else
	static const char kVersionNameProjector[] = Rtt_ANALYTICS_RUN_PREFIX "sku(projector)";
	return kVersionNameProjector;
#endif
}

// Writes a message for a built project.
// Argument "messageBuffer" is the string buffer that this function will write to. Must be allocated by caller.
// Argument "targetDeviceVersionId" should be assigned a value from one of TargetDevice's Version enum.
// Argument "targetDeviceTypeId" should be assigned a value from one of TargetDevice's enum values.
// Returns the number of characters written to in "messageBuffer", not including the NULL character.
// Returns -1 if an error occurred such as an invalid argument.
int
Analytics::WriteBuildMessage(char *messageBuffer, int targetDeviceVersionId, int targetDeviceTypeId)
{
	// Validate arguments.
	if (NULL == messageBuffer)
	{
		return -1;
	}

	// Write the string.
	return sprintf(messageBuffer, "v(%d) d(%d)", targetDeviceVersionId, targetDeviceTypeId);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


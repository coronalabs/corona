//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaConsole.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "Rtt_Lua.h"

#include "Rtt_LuaContext.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kSuppressOutput[] = "Rtt_SUPPRESS_LUA_CONSOLE_OUTPUT";

void
LuaConsole::Initialize()
{
	// Force init of readline
	rl_initialize();

	// Check if we should suppress echo
	const char* val = getenv( kSuppressOutput );
	if ( val && 0 != strtol( val, NULL, 10 ) )
	{
		termios termOptions;
		int result;
		result = tcgetattr( STDIN_FILENO, & termOptions ); Rtt_ASSERT( result >= 0 );
		termOptions.c_lflag &= ~ECHO;
		result = tcsetattr( STDIN_FILENO, TCSANOW, & termOptions ); Rtt_ASSERT( result >= 0 );
	}
}

void
LuaConsole::Finalize()
{
}

// ----------------------------------------------------------------------------

static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;

// TODO: remove dependency on global --- is this possible?
static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, LuaContext::lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    LuaContext::l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}

static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, LuaContext::traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}

LuaConsole::LuaConsole( lua_State *L, const Runtime& runtime )
:	fL( L ),
	fRuntime( runtime ),
	fBuffer( NULL ),
	fBufferSize( 0 )
{
}

LuaConsole::~LuaConsole()
{
	if ( fBuffer ) { free( fBuffer ); }
}

void
LuaConsole::operator()()
{
	(void) Rtt_VERIFY( 0 == LuaContext::DoCPCall( fL, & dotty_glue, this ) );
}

static const char*
get_prompt( bool firstline )
{
	static const char kPrompt[] = LUA_PROMPT;
	static const char kPrompt2[] = LUA_PROMPT2;
	return firstline ? kPrompt : kPrompt2;
}

char*
LuaConsole::CheckBuffer( size_t numBytes )
{
	if ( fBufferSize < numBytes )
	{
		fBuffer = (char*)realloc( fBuffer, numBytes );
		fBufferSize = numBytes;
	}

	return fBuffer;
}

#if 0 // NOT USED
static bool
IsSpaceOrTab( char c )
{
	return ' ' == c || '\t' == c;
}

static char*
StripLocalPrefix( char* str )
{
	const char kLocal[] = "local";

	char* result = str; // Store original string in result

	str = strstr( str, kLocal );
	if ( str )
	{
		// Prefix must be all whitespace
		for ( const char* iStr = result; iStr != str; iStr++ )
		{
			char c = *iStr;
			if ( ! IsSpaceOrTab( c ) )
			{
				goto exit_gracefully;
			}
		}

		// Trailing character after "local" must be whitespace
		size_t len = strlen( result );
		size_t i = ( str - result ) + strlen( kLocal );
		if ( i < len )
		{
			char c = result[i];
			if ( IsSpaceOrTab( c ) )
			{
				result = str;
			}
		}
	}

exit_gracefully:
	return result;
}
#endif // 0

bool
LuaConsole::ReadLine( bool firstline )
{
	const char *prmt = get_prompt( firstline );
	char* b = readline( prmt );
	char* lineBuffer = b;

	bool result = ( NULL != b );

	if ( result )
	{
		size_t l = strlen(b);
		if ( l > 0 && b[l-1] == '\n' )  // line ends with newline?
		{
			b[l-1] = '\0';  // remove it
		}

		const char kFormatString[] = "%s";
		const char kFormatReturnString[] = "return %s";
		size_t numBytes = l + 1; // add space for terminating '\0'
		const char* formatString = kFormatString;

		if (firstline && b[0] == '=')  // first line starts with `=' ?
		{
			numBytes += sizeof( kFormatReturnString );
			formatString = kFormatReturnString;
			++b;
		}
/*
		else if ( firstline )
		{
			b = StripLocalPrefix( b );
		}
*/

		char* buffer = CheckBuffer( numBytes );
		sprintf( buffer, formatString, b );

		free( lineBuffer );
	}

	return result;
}

static int 
incomplete( lua_State *L, int status )
{
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}

int
LuaConsole::LoadLine()
{
	lua_State* L = fL;
	lua_settop( L, 0 );
	lua_pushstring( L, fBuffer ); // push command
	int status;
	for (;;) // repeat until gets a complete line
	{
		status = luaL_loadbuffer( L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin" );
		if ( !incomplete( L, status ) ) break;  // cannot try to add lines?
		if ( !ReadLine( false ) )  // no more input?
		{
			return -1;
		}
		lua_pushstring( L, fBuffer ); // push command
		lua_pushliteral(L, "\n");  // add a new line...
		lua_insert(L, -2);  // ...between the two lines
		lua_concat(L, 3);  // join them
	}

	lua_saveline( L, 1 );
	lua_remove( L, 1 );  // remove line

	return status;
}

void
LuaConsole::dotty()
{
	lua_State *L = fL;

	int status = 0;
	const char *oldprogname = progname;
	progname = NULL;
	const Runtime& runtime = fRuntime;
	const MPlatform& platform = runtime.Platform();

	while( -1 != status && ReadLine( true ) )
	{
		// Acquire lock to the runtime while we modify its lua state
		platform.BeginRuntime( runtime );
		{
			status = LoadLine();
			if ( -1 != status )
			{
				if (status == 0) { status = docall(L, 0, 0); }
				report(L, status);
				if (status == 0 && lua_gettop(L) > 0)  // any result to print?
				{
					lua_getglobal(L, "print");
					lua_insert(L, 1);
					if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
					{
						const char* msg = lua_pushfstring(
							L, "error calling " LUA_QL("print") " (%s)", lua_tostring(L, -1));
						LuaContext::l_message( progname, msg );
					}
				}
			}
		}
		platform.EndRuntime( runtime );
	}

	// TODO: Should we lock the runtime for this lua call also???
	// lua_settop(L, 0);  /* clear stack */
	Rtt_ASSERT( 0 == lua_gettop( L ) );

	fputs("\n", stdout);
	fflush(stdout);
	progname = oldprogname;
}

int
LuaConsole::dotty_glue( lua_State *L )
{
	LuaConsole* p = (LuaConsole*)lua_touserdata( L, 1 );
	p->dotty();
	return 0;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


// This is intended to be a minimal, but crucial core file to provide core Lua functionality.
// The intention is to override core Lua functions as needed with implementations defined here.
// Because this is considered "core", minimal dependencies and a stream-lined design are desireable because
// this file will be involved in bootstrap situations with the Lua state where not everything is fully setup.
// Currently the only assumption is that Lua and the standard Lua library are loaded.



#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "Core/Rtt_Config.h"


/* This is the stock Lua print function. 
 * The goal is to reimplement this for every platform that can't use the stock one.
 * Use this one as the fall back definition.
 */

#if defined( Rtt_APPLE_ENV ) 
	/* Apple is defined in the .m file for Obj-C stuff. Make sure that is compiled in for this case. */
extern int Rtt_LuaCoronaBaseLib_print(lua_State *L);

#elif defined ( Rtt_ANDROID_ENV )

#include <android/log.h>

/* Android's log system is annoying for two reasons. 
 1. We must use __android_log_write. (Note: Don't use __android_log_print because we don't % characters to be evaluated.)
 2. __android_log_write only takes one string at a time, but will print timestamps and newlines for each call. There is no easy way to combine strings into one message, so we must do it ourselves the hard way.
*/
static int Rtt_LuaCoronaBaseLib_print(lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_newtable(L); /* create a new table/array to hold the resulting strings to call table.concat on later */
	int concat_table_index = n+1;
	const char* result = NULL;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++)
	{
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
		{
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
							  LUA_QL("print"));
		}
		/* We can build up the string using the standard C library like strcat, 
		 or we can try to be more clever by leveraging Lua table.concat.
		 The former can be messy dealing with malloc/realloc and may not be optimal depending on how reallocs are done.
		 The latter punts the optimization issues to Lua, but the messiness is in dealing with the Lua stack.
		 (This would be easier implementing in Lua, but we would need to push platform details out about which platform this is.)
		 */
		lua_rawseti(L, concat_table_index, i); /* Put the string in our own array which happens to have the exact same indexing as our parameter list. Remember this pops the string off the top of the stack. */
	}
	lua_pop(L, 1); /* pop function 'tostring' */
	
	lua_getglobal(L, "table"); /* get the table library */
	lua_pushliteral(L, "concat");
	lua_gettable(L, -2); /* get table.concat function on top of stack */
	
	lua_pushvalue(L, concat_table_index); /* push the concat array as the first parameter */
	lua_pushliteral(L, "\t"); /* Lua's built-in print separates each argument with tabs. Pass this as the second parameter to concat. */
	lua_call(L, 2, 1);
	result = lua_tostring(L, -1);  /* get result */
	
	/* Finally, we can write the output. Remember that this automatically includes a newline. */
	__android_log_write(ANDROID_LOG_INFO, "Corona", result);
	
	return 0;
}

#elif defined( Rtt_WIN_ENV ) || defined( Rtt_NINTENDO_ENV )

#include "Core\Rtt_Assert.h"
#include <windows.h>

static int Rtt_LuaCoronaBaseLib_print(lua_State *L)
{
	int luaToStringFunctionIndex;
	int concatenationCount;
	int argumentCount = lua_gettop(L);
	int argumentIndex;

	/* Do not continue if no arguments were given to the print() function. */
	if (argumentCount <= 0)
	{
		return 0;
	}

	/* Do not continue if logging is disabled. */
	if (!Rtt_LogIsEnabled())
	{
		return 0;
	}

	/* Push Lua's tostring() function to the top of the stack. */
	lua_getglobal(L, "tostring");
	luaToStringFunctionIndex = lua_gettop(L);

	/* Concatenate all of the print() function's arguments into a single string to the top of the stack. */
	lua_pushstring(L, "");
	concatenationCount = 1;
	for (argumentIndex = 1; argumentIndex <= argumentCount; argumentIndex++)
	{
		/* Push the next argument to the top of the stack as a string. */
		/* Note: Calling Lua's tostring() function converts it to a string without changing the argument's type. */
		lua_pushvalue(L, luaToStringFunctionIndex);
		lua_pushvalue(L, argumentIndex);
		lua_call(L, 1, 1);
		if (lua_isstring(L, -1) == 0)
		{
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
		}
		concatenationCount++;
		
		/* Push a tab character if there are more arguments to concatenate. */
		if (argumentIndex < argumentCount)
		{
			lua_pushstring(L, "\t");
			concatenationCount++;
		}
	}
	lua_pushstring(L, "\n");
	concatenationCount++;
	lua_concat(L, concatenationCount);

	/* Handle the concatenated string. */
	{
		/* Fetch the concatenated string, including any embedded null characters. */
		size_t stringLength = 0;
		const char *luaStringPointer = lua_tolstring(L, -1, &stringLength);
		const char *stringPointer = luaStringPointer;

		/* If the string contains any "\r\n" pairs, then create a new string which replaces them with '\n' characters. */
		/* We do this because stdout is in "text mode" by default, which means it'll replace all "\n" with "\r\n".     */
		/* If we don't remove the '\r' characters, then the outputted string will contain "\r\r\n" line endings.       */
		{
			size_t newStringLength = 0;
			char *newStringPointer = NULL;
			size_t destinationIndex;
			size_t sourceIndex;
			for (sourceIndex = 0, destinationIndex = 0; sourceIndex < stringLength; sourceIndex++)
			{
				/* Check for a \r\n character pair. */
				if (('\r' == luaStringPointer[sourceIndex]) &&
				    ((sourceIndex + 1) < stringLength) && ('\n' == luaStringPointer[sourceIndex + 1]))
				{
					/* Create a copy of the Lua string, if not done already. */
					if (!newStringPointer)
					{
						newStringLength = stringLength;
						newStringPointer = malloc(newStringLength + 1);
						if (!newStringPointer)
						{
							break;
						}
						memcpy_s(newStringPointer, newStringLength, luaStringPointer, sourceIndex);
					}

					/* Skip copying the carriage return and decrement the new string length by 1. */
					newStringLength--;
					continue;
				}

				/* Copy the Lua character to the new string. */
				/* This is only done if at least 1 \r\n pair has been found. */
				if (newStringPointer)
				{
					newStringPointer[destinationIndex] = luaStringPointer[sourceIndex];
				}
				destinationIndex++;
			}
			if (newStringPointer)
			{
				/* Switch the pointer to use the new modified string to be logged down below. */
				newStringPointer[newStringLength] = '\0';
				stringPointer = newStringPointer;
				stringLength = newStringLength;
			}
		}

		/* Print the final concatenated string to stdout and the Visual Studio debugger, if available. */
#ifdef Rtt_WIN_PHONE_ENV
		{
			Rtt_LogHandlerCallback logCallback = Rtt_LogGetHandler();
			if (logCallback)
			{
				/* Note: It's more efficient to invoke this callback directly because it avoids having   */
				/*       the Rtt_LogException() function from doing a sprintf() string format operation. */
				logCallback(stringPointer);
			}
			else
			{
				Rtt_LogException("%s", stringPointer);
			}
		}
#else
		if (IsDebuggerPresent())
		{
			OutputDebugStringA(stringPointer);
		}
		fwrite(stringPointer, sizeof(char), stringLength, stdout);
		fflush(stdout);
#endif

		/* If we've made a modified copy of the Lua string up above, then delete it. */
		if (stringPointer != luaStringPointer)
		{
			free((void*)stringPointer);
		}
	}

	/* Pop the concatenated string. */
	lua_pop(L, 1);
	return 0;
}

#else // fall back default definition

static int Rtt_LuaCoronaBaseLib_print(lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
							  LUA_QL("print"));
		if (i>1) fputs("\t", stdout);
		fputs(s, stdout);
		lua_pop(L, 1);  /* pop result */
	}
	fputs("\n", stdout);
	return 0;
}
#endif /* Rtt_LuaCoronaBaseLib_print platform definitions */


static const luaL_Reg corona_base_funcs[] =
{
	{"print", Rtt_LuaCoronaBaseLib_print},
	{NULL, NULL}
};


int luaopen_coronabaselib(lua_State *L)
{
	luaL_register(L, "coronabaselib", corona_base_funcs);
	return 1;
}





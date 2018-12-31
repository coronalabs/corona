/*
 LuaHashMap
 Copyright (C) 2011 PlayControl Software, LLC. 
 Eric Wing <ewing . public @ playcontrol.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 */

#include "LuaHashMap.h"
#include "lua.h"
#include "lauxlib.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
	/* Not ISO/IEC 9899:1999-compliant. */
	#if !defined(restrict)
		#define restrict
		#define __LUAHASHMAP_RESTRICT_KEYWORD_DEFINED__
	#endif
	
	#if !defined(bool)
		#define bool char
        #define __LUAHASHMAP_BOOL_KEYWORD_DEFINED__
	#endif
	#if !defined(false)
		#define false (bool)0
        #define __LUAHASHMAP_FALSE_KEYWORD_DEFINED__
	#endif
	#if !defined(true)
		#define true (bool)1
        #define __LUAHASHMAP_TRUE_KEYWORD_DEFINED__
	#endif
#else
	#include <stdbool.h>
#endif


#ifdef LUAHASHMAP_DEBUG
#define LUAHASHMAP_ASSERT(e) assert(e)
#else
#define LUAHASHMAP_ASSERT(e)
#endif

struct LuaHashMap
{
	lua_State* luaState;
	lua_Alloc memoryAllocator;
	void* allocatorUserData;
	lua_Integer uniqueTableNameForSharedState;
};


/* Benchmark:
 
 CFTimeInterval start_time = CACurrentMediaTime();
 void* ret_ptr = NULL;	
 for(i=0; i<40000000; i++)
 {
 LuaHashMap_SetValuePointerForKeyPointer(hash_map, (void*)i, (void*)i);
 ret_ptr = LuaHashMap_GetValuePointerForKeyPointer(hash_map, (void*)i);
 LuaHashMap_RemoveKeyPointer(hash_map, ret_ptr);
 }
 CFTimeInterval end_time = CACurrentMediaTime();
 fprintf(stderr, "diff time: %lf", end_time-start_time);
 
 iMac i3, 32-bit with lua_get/settable ~15-16 sec;
 iMac i3, 32-bit with lua_raw/get/set ~14-15 sec;
 iMac i3, 64-bit with lua_get/settable ~13-13.5 sec;
 iMac i3, 64-bit with lua_raw/get/set  ~12.5 sec;
*/ 
/* I don't expect we'll need metamethods so rawget/set is fine. */
#if 1
	#define LUAHASHMAP_SETTABLE lua_rawset
	#define LUAHASHMAP_GETTABLE lua_rawget
#else
	#define LUAHASHMAP_SETTABLE lua_settable
	#define LUAHASHMAP_GETTABLE lua_gettable
#endif

/* Lua 5.2 lua_pushstring returns the internalized string pointer, but 5.1 does not.
 * This is useful to me in a few places so I'm creating a macro.
 */
#if LUA_VERSION_NUM <= 501 /* Lua 5.1 or less */
	#define LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(lua_state, push_string, length, return_internal_string) \
		do { \
			lua_pushlstring(lua_state, push_string, length); \
			return_internal_string = lua_tostring(lua_state, -1); \
		} while(0)
#else /* assuming 502 (5.2) */
	#define LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(lua_state, push_string, length, return_internal_string) return_internal_string = lua_pushlstring(lua_state, push_string, length);
#endif

/* Putting stuff in the global table might be interesting because you could run a Lua script and interact with all the elements added from this API.
 * But unfortunately, Lua 5.2 removed LUA_GLOBALSINDEX and made global access more cumbersome. 
 * This means I have to call slightly more commands to do the same thing as before.
 * This suggests using the LUA_REGISTRYINDEX might have a tiny speed edge in Lua 5.2.
 * For LUA_REGISTRYINDEX, the code for Lua 5.1 and 5.2 is the same.
 */
#ifdef LUAHASHMAP_USE_GLOBAL_TABLE

	#if LUA_VERSION_NUM <= 501 /* Lua 5.1 or less */
		#define LUAHASHMAP_GETGLOBAL_UNIQUESTRING(lua_state, unique_key) lua_rawgeti(lua_state, LUA_GLOBALSINDEX, unique_key)

		#define LUAHASHMAP_SETGLOBAL_UNIQUESTRING(lua_state, unique_key) lua_rawseti(lua_state, LUA_GLOBALSINDEX, unique_key)

		#define LUAHASHMAP_REPLACE_WITH_EMPTY_TABLE(lua_state, unique_key) \
			do { \
				lua_newtable(hash_map->luaState); \
				lua_rawseti(lua_state, LUA_GLOBALSINDEX, unique_key); \
			} while(0)

		/* Made a function because I couldn't figure out how to return a value in a multiline macro scenario. */
		static int Internal_NewGlobalLuaRef(lua_State* lua_state)
		{
			return luaL_ref(lua_state, LUA_GLOBALSINDEX);
		}
		
		#define LUAHASHMAP_GLOBAL_LUA_UNREF(lua_state, unique_key) \ luaL_unref(lua_state, LUA_GLOBALSINDEX, unique_key)


	#else /* assuming 502 (5.2) */
		#define LUAHASHMAP_GETGLOBAL_UNIQUESTRING(lua_state, unique_key) \
			do { \
				lua_pushglobaltable(lua_state); \
				lua_rawgeti(lua_state, -1, unique_key); \
				lua_remove(lua_state, -2); \
			} while(0)

		#define LUAHASHMAP_SETGLOBAL_UNIQUESTRING(lua_state, unique_key) \
			do { \
				lua_pushglobaltable(lua_state); \
				lua_rawseti(lua_state, -1, unique_key); \
				lua_pop(lua_state, 1); \
			} while(0)

		#define LUAHASHMAP_REPLACE_WITH_EMPTY_TABLE(lua_state, unique_key) \
			do { \
				lua_pushglobaltable(lua_state); \
				lua_newtable(hash_map->luaState); \
				lua_rawseti(lua_state, -2, unique_key); \
				lua_pop(lua_state, 1); \
			} while(0)

		/* Made a function because I couldn't figure out how to return a value in a multiline macro scenario. */
		static int Internal_NewGlobalLuaRef(lua_State* lua_state)
		{
			/* need to get gloal table under the top stack object that luaL_ref automatically refers to */
			int ret_val;
			lua_pushglobaltable(lua_state);
			lua_insert(lua_state, -2); /* should swap the global table with the element below it making it the top element */
			ret_val = luaL_ref(lua_state, -2);
			lua_pop(lua_state, 1);
			return ret_val;
		}

		#define LUAHASHMAP_GLOBAL_LUA_UNREF(lua_state, unique_key) \
			do { \
				lua_pushglobaltable(lua_state); \
				luaL_unref(lua_state, -1, unique_key); \
				lua_pop(lua_state, 1); \
			} while(0)

	#endif /* end Lua version check */


#else /* Using LUA_REGISTRYINDEX used by both 5.1 and 5.2 */

	#define LUAHASHMAP_GETGLOBAL_UNIQUESTRING(lua_state, unique_key) lua_rawgeti(lua_state, LUA_REGISTRYINDEX, (int) unique_key)

	#define LUAHASHMAP_SETGLOBAL_UNIQUESTRING(lua_state, unique_key) lua_rawseti(lua_state, LUA_REGISTRYINDEX, unique_key)

	#define LUAHASHMAP_REPLACE_WITH_EMPTY_TABLE(lua_state, unique_key) \
		do { \
			lua_newtable(hash_map->luaState); \
			lua_rawseti(lua_state, LUA_REGISTRYINDEX, (int) unique_key); \
		} while(0)

	/* Made a function because I couldn't figure out how to return a value in a multiline macro scenario. */
	static int Internal_NewGlobalLuaRef(lua_State* lua_state)
	{
		return luaL_ref(lua_state, LUA_REGISTRYINDEX);
	}
	
	#define LUAHASHMAP_GLOBAL_LUA_UNREF(lua_state, unique_key) luaL_unref(lua_state, LUA_REGISTRYINDEX, (int) unique_key)


#endif


static void Internal_InitializeInternalTables(LuaHashMap* hash_map)
{
	/* Create a table in Lua to be our hash map */
	 /* According to Lua Gems performance, Lua tables are created with 0 size by default to not waste memory
	 * so creating different tables shouldn't be that bad.
	 * 
	 * I originally did an experiment to see if integer keys on the global table with 
	 * rawset/geti was faster than the rawset/get with light userdata keys (which 
	 * are also effectively integers). (The light userdata keys were static strings.)
	 * My benchmarks are inconclusive, testing with 10000000 insert/removes.
	 * They appear to be the same speed, though I've seen large timing variations in
	 * both tests.
	 * But in the changes for shared state, I realized luaL_ref basically was what I was doing with 
	 * integer keys. So I removed all this for a luaL_ref based implementation which is more
	 * general, but I lose the explicit separate tables for the 4 key types.
	 * luaL_ref is nice in that is also creates unique table numbers automatically and recycles them as they become free.
	 */
	
	lua_newtable(hash_map->luaState);
	/* The idea is that every LuaHashMap instance gets a unique reference id so they can share the same lua_State. */
	/* Even the first/original hash map created gets its own so all hash maps are effectively peers of each other. */
	hash_map->uniqueTableNameForSharedState = Internal_NewGlobalLuaRef(hash_map->luaState);
}

const LuaHashMapVersion* LuaHashMap_GetLinkedVersion()
{
	static LuaHashMapVersion linked_version;
	LUAHASHMAP_GET_COMPILED_VERSION(&linked_version);
	return(&linked_version);
}

LuaHashMap* LuaHashMap_Create()
{
	LuaHashMap* hash_map;
	lua_State* lua_state = luaL_newstate();
	if(NULL == lua_state)
	{
		return NULL;
	}
	hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	hash_map->luaState = lua_state;

	Internal_InitializeInternalTables(hash_map);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}

LuaHashMap* LuaHashMap_CreateWithAllocator(lua_Alloc the_allocator, void* user_data)
{
	LuaHashMap* hash_map;
	lua_State* lua_state = lua_newstate(the_allocator, user_data);
	if(NULL == lua_state)
	{
		return NULL;
	}

	hash_map = (LuaHashMap*)(*the_allocator)(user_data, NULL, 0, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = lua_state;
	hash_map->memoryAllocator = the_allocator;
	hash_map->allocatorUserData = user_data;

	Internal_InitializeInternalTables(hash_map);
	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


LuaHashMap* LuaHashMap_CreateWithSizeHints(int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	lua_State* lua_state = luaL_newstate();
	if(NULL == lua_state)
	{
		return NULL;
	}
	hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	hash_map->luaState = lua_state;
	
	/* I'm no longer using key_type/value_type. But I wonder if I should reserve it for future changes. */
/*
	switch( 0x0F & key_type)
	{
		case LUAHASHMAP_KEYSTRING_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYPOINTER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYNUMBER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYINTEGER_TYPE:
		{
			break;
		}
		default:
		{
			Internal_InitializeInternalTables(hash_map);
		}
	}
*/
	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = Internal_NewGlobalLuaRef(hash_map->luaState);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


LuaHashMap* LuaHashMap_CreateWithAllocatorAndSizeHints(lua_Alloc the_allocator, void* user_data, int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	lua_State* lua_state = lua_newstate(the_allocator, user_data);
	if(NULL == lua_state)
	{
		return NULL;
	}

	hash_map = (LuaHashMap*)(*the_allocator)(user_data, NULL, 0, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = lua_state;
	hash_map->memoryAllocator = the_allocator;
	hash_map->allocatorUserData = user_data;
	/* I'm no longer using key_type/value_type. But I wonder if I should reserve it for future changes. */
/*
	switch( 0x0F & key_type)
	{
		case LUAHASHMAP_KEYSTRING_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYPOINTER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYNUMBER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYINTEGER_TYPE:
		{
			break;
		}
		default:
		{
			Internal_InitializeInternalTables(hash_map);
		}
	}
*/
	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = Internal_NewGlobalLuaRef(hash_map->luaState);

	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


LuaHashMap* LuaHashMap_CreateShare(LuaHashMap* original_hash_map)
{
	LuaHashMap* hash_map;
	if(NULL == original_hash_map)
	{
		return NULL;
	}
	if(NULL == original_hash_map->memoryAllocator)
	{
		hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	}	
	else
	{
		hash_map = (LuaHashMap*)(*original_hash_map->memoryAllocator)(original_hash_map->allocatorUserData, NULL, 0, sizeof(LuaHashMap));
	}
	if(NULL == hash_map)
	{
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = original_hash_map->luaState;
	hash_map->memoryAllocator = original_hash_map->memoryAllocator;
	hash_map->allocatorUserData = original_hash_map->allocatorUserData;

	Internal_InitializeInternalTables(hash_map);

	return hash_map;
}


LuaHashMap* LuaHashMap_CreateShareWithSizeHints(LuaHashMap* original_hash_map, int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	if(NULL == original_hash_map)
	{
		return NULL;
	}
	if(NULL == original_hash_map->memoryAllocator)
	{
		hash_map = (LuaHashMap*)malloc(sizeof(LuaHashMap));
	}	
	else
	{
		hash_map = (LuaHashMap*)(*original_hash_map->memoryAllocator)(original_hash_map->allocatorUserData, NULL, 0, sizeof(LuaHashMap));
	}
	if(NULL == hash_map)
	{
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = original_hash_map->luaState;
	hash_map->memoryAllocator = original_hash_map->memoryAllocator;
	hash_map->allocatorUserData = original_hash_map->allocatorUserData;

	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = Internal_NewGlobalLuaRef(hash_map->luaState);

	return hash_map;
}


LuaHashMap* LuaHashMap_CreateShareFromLuaState(lua_State* lua_state)
{
	LuaHashMap* hash_map;
	if(NULL == lua_state)
	{
		return NULL;
	}
	hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		return NULL;
	}

	hash_map->luaState = lua_state;
	
	Internal_InitializeInternalTables(hash_map);

	return hash_map;
}


LuaHashMap* LuaHashMap_CreateShareFromLuaStateWithAllocatorAndSizeHints(lua_State* lua_state, lua_Alloc the_allocator, void* user_data, int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	if(NULL == lua_state)
	{
		return NULL;
	}
	if(NULL == the_allocator)
	{
		hash_map = (LuaHashMap*)malloc(sizeof(LuaHashMap));
	}	
	else
	{
		hash_map = (LuaHashMap*)(*the_allocator)(user_data, NULL, 0, sizeof(LuaHashMap));
	}
	if(NULL == hash_map)
	{
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = lua_state;
	hash_map->memoryAllocator = the_allocator;
	hash_map->allocatorUserData = user_data;

	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = Internal_NewGlobalLuaRef(hash_map->luaState);

	return hash_map;
}


LuaHashMap* LuaHashMap_CreateShareFromLuaStateWithSizeHints(lua_State* lua_state, int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	if(NULL == lua_state)
	{
		return NULL;
	}
	hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == hash_map)
	{
		return NULL;
	}

	hash_map->luaState = lua_state;
	
	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = Internal_NewGlobalLuaRef(hash_map->luaState);
	
	return hash_map;
}





/* This version does not close the Lua state since it is shared */
void LuaHashMap_FreeShare(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	LUAHASHMAP_GLOBAL_LUA_UNREF(hash_map->luaState, hash_map->uniqueTableNameForSharedState);
	/* Seems like a good time to force the garbage collector */
	lua_gc(hash_map->luaState, LUA_GCCOLLECT, 0);
	if(NULL != hash_map->memoryAllocator)
	{
		(*hash_map->memoryAllocator)(hash_map->allocatorUserData, hash_map, sizeof(LuaHashMap), 0);
	}
	else
	{
		free(hash_map);
	}
}

void LuaHashMap_Free(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	/* Since we close the lua_State, we don't need to call luaL_unref */
	/* LUAHASHMAP_GLOBAL_LUA_UNREF(hash_map->luaState, hash_map->uniqueTableNameForSharedState); */
	lua_close(hash_map->luaState);
	if(NULL != hash_map->memoryAllocator)
	{
		(*hash_map->memoryAllocator)(hash_map->allocatorUserData, hash_map, sizeof(LuaHashMap), 0);
	}
	else
	{
		free(hash_map);
	}
}

lua_State* LuaHashMap_GetLuaState(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return hash_map->luaState;
}

static const char* Internal_SetValueStringForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* value_string, const char* key_string, size_t value_string_length, size_t key_string_length)
{
	const char* internalized_key_string = NULL;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(hash_map->luaState, key_string, key_string_length, internalized_key_string); /* stack: [key_string, table] */
	lua_pushlstring(hash_map->luaState, value_string, value_string_length); /* stack: [value_string, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);

	return internalized_key_string;
}

const char* LuaHashMap_SetValueStringForKeyString(LuaHashMap* restrict hash_map, const char* value_string, const char* key_string)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	if(NULL == value_string)
	{
		return Internal_SetValueStringForKeyStringWithLength(hash_map, value_string, key_string, 0, strlen(key_string));	
	}
	else
	{
		return Internal_SetValueStringForKeyStringWithLength(hash_map, value_string, key_string, strlen(value_string), strlen(key_string));
	}

}

const char* LuaHashMap_SetValueStringForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* value_string, const char* key_string, size_t value_string_length, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_SetValueStringForKeyStringWithLength(hash_map, value_string, key_string, value_string_length, key_string_length);	
}

static const char* Internal_SetValuePointerForKeyStringWithLength(LuaHashMap* hash_map, void* value_pointer, const char* key_string, size_t key_string_length)
{
	const char* internalized_key_string = NULL;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(hash_map->luaState, key_string, key_string_length, internalized_key_string); /* stack: [key_string, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);

	return internalized_key_string;
}

const char* LuaHashMap_SetValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* key_string)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_SetValuePointerForKeyStringWithLength(hash_map, value_pointer, key_string, strlen(key_string));
}

const char* LuaHashMap_SetValuePointerForKeyStringWithLength(LuaHashMap* hash_map, void* value_pointer, const char* key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_SetValuePointerForKeyStringWithLength(hash_map, value_pointer, key_string, key_string_length);
}

static const char* Internal_SetValueNumberForKeyStringWithLength(LuaHashMap* restrict hash_map, lua_Number value_number, const char* restrict key_string, size_t key_string_length)
{
	const char* internalized_key_string = NULL;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(hash_map->luaState, key_string, key_string_length, internalized_key_string); /* stack: [key_string, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	

	return internalized_key_string;
}

const char* LuaHashMap_SetValueNumberForKeyString(LuaHashMap* restrict hash_map, lua_Number value_number, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_SetValueNumberForKeyStringWithLength(hash_map, value_number, key_string, strlen(key_string));
}

const char* LuaHashMap_SetValueNumberForKeyStringWithLength(LuaHashMap* restrict hash_map, lua_Number value_number, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_SetValueNumberForKeyStringWithLength(hash_map, value_number, key_string, key_string_length);
}

static const char* Internal_SetValueIntegerForKeyStringWithLength(LuaHashMap* restrict hash_map, lua_Integer value_integer, const char* restrict key_string, size_t key_string_length)
{
	const char* internalized_key_string = NULL;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(hash_map->luaState, key_string, key_string_length, internalized_key_string); /* stack: [key_string, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_integer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	

	return internalized_key_string;
}


const char* LuaHashMap_SetValueIntegerForKeyString(LuaHashMap* restrict hash_map, lua_Integer value_integer, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_SetValueIntegerForKeyStringWithLength(hash_map, value_integer, key_string, strlen(key_string));
}

const char* LuaHashMap_SetValueIntegerForKeyStringWithLength(LuaHashMap* restrict hash_map, lua_Integer value_integer, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_SetValueIntegerForKeyStringWithLength(hash_map, value_integer, key_string, key_string_length);
}

static void Internal_SetValueStringForKeyPointerWithLength(LuaHashMap* hash_map, const char* value_string, void* key_pointer, size_t value_string_length)
{
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushlstring(hash_map->luaState, value_string, value_string_length); /* stack: [value_string, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_pointer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValueStringForKeyPointer(LuaHashMap* hash_map, const char* value_string, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == value_string)
	{
		Internal_SetValueStringForKeyPointerWithLength(hash_map, value_string, key_pointer, 0);
	}
	else
	{
		Internal_SetValueStringForKeyPointerWithLength(hash_map, value_string, key_pointer, strlen(value_string));
	}
}

void LuaHashMap_SetValueStringForKeyPointerWithLength(LuaHashMap* hash_map, const char* value_string, void* key_pointer, size_t value_string_length)
{
	if(NULL == hash_map)
	{
		return;
	}
	Internal_SetValueStringForKeyPointerWithLength(hash_map, value_string, key_pointer, value_string_length);

}

void LuaHashMap_SetValueNumberForKeyPointer(LuaHashMap* hash_map, lua_Number value_number, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValueIntegerForKeyPointer(LuaHashMap* hash_map, lua_Integer value_integer, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

static void Internal_SetValueStringForKeyNumberWithLength(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Number key_number, size_t value_string_length)
{
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushlstring(hash_map->luaState, value_string, value_string_length); /* stack: [value_string, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValueStringForKeyNumber(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == value_string)
	{
		Internal_SetValueStringForKeyNumberWithLength(hash_map, value_string, key_number, 0);
	}
	else
	{
		Internal_SetValueStringForKeyNumberWithLength(hash_map, value_string, key_number, strlen(value_string));
	}
}

void LuaHashMap_SetValueStringForKeyNumberWithLength(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Number key_number, size_t value_string_length)
{
	if(NULL == hash_map)
	{
		return;
	}
	Internal_SetValueStringForKeyNumberWithLength(hash_map, value_string, key_number, value_string_length);
}

void LuaHashMap_SetValuePointerForKeyNumber(LuaHashMap* hash_map, void* value_pointer, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_SetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number value_number, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Integer value_integer, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

static void Internal_SetValueStringForKeyIntegerWithLength(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Integer key_integer, size_t value_string_length)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushlstring(hash_map->luaState, value_string, value_string_length); /* stack: [value_string, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValueStringForKeyInteger(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == value_string)
	{
		Internal_SetValueStringForKeyIntegerWithLength(hash_map, value_string, key_integer, 0);
	}
	else
	{
		Internal_SetValueStringForKeyIntegerWithLength(hash_map, value_string, key_integer, strlen(value_string));
	}
}

void LuaHashMap_SetValueStringForKeyIntegerWithLength(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Integer key_integer, size_t value_string_length)
{
	if(NULL == hash_map)
	{
		return;
	}
	Internal_SetValueStringForKeyIntegerWithLength(hash_map, value_string, key_integer, value_string_length);
}

void LuaHashMap_SetValuePointerForKeyInteger(LuaHashMap* hash_map, void* value_pointer, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Number value_number, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_SetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer value_integer, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

static const char* Internal_GetValueStringForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t* value_string_length_return, size_t key_string_length)
{
	const char* ret_val;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlstring(hash_map->luaState, key_string, key_string_length); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	
	ret_val = lua_tolstring(hash_map->luaState, -1, value_string_length_return);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

const char* LuaHashMap_GetValueStringForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyStringWithLength(hash_map, key_string, NULL, strlen(key_string));
}

const char* LuaHashMap_GetValueStringForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t* value_string_length_return, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyStringWithLength(hash_map, key_string, value_string_length_return, key_string_length);
}

static void* Internal_GetValuePointerForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	void* ret_val;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlstring(hash_map->luaState, key_string, key_string_length); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_pointer, table] */

	ret_val = lua_touserdata(hash_map->luaState, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

void* LuaHashMap_GetValuePointerForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_GetValuePointerForKeyStringWithLength(hash_map, key_string, strlen(key_string));
}

void* LuaHashMap_GetValuePointerForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}
	return Internal_GetValuePointerForKeyStringWithLength(hash_map, key_string, key_string_length);
}

static lua_Number Internal_GetValueNumberForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	lua_Number ret_val;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlstring(hash_map->luaState, key_string, key_string_length); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_number, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return (lua_Number)0.0;
	}
	if(NULL == key_string)
	{
		return (lua_Number)0.0;
	}
	return Internal_GetValueNumberForKeyStringWithLength(hash_map, key_string, strlen(key_string));	
}

lua_Number LuaHashMap_GetValueNumberForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return (lua_Number)0.0;
	}
	if(NULL == key_string)
	{
		return (lua_Number)0.0;
	}
	return Internal_GetValueNumberForKeyStringWithLength(hash_map, key_string, key_string_length);
}

static lua_Integer Internal_GetValueIntegerForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	lua_Integer ret_val;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlstring(hash_map->luaState, key_string, key_string_length); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_integer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return 0;
	}
	if(NULL == key_string)
	{
		return 0;
	}
	return Internal_GetValueIntegerForKeyStringWithLength(hash_map, key_string, strlen(key_string));
}

lua_Integer LuaHashMap_GetValueIntegerForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return 0;
	}
	if(NULL == key_string)
	{
		return 0;
	}
	return Internal_GetValueIntegerForKeyStringWithLength(hash_map, key_string, key_string_length);
}

static const char* Internal_GetValueStringForKeyPointerWithLength(LuaHashMap* hash_map, void* key_pointer, size_t* value_string_length_return)
{
	const char* ret_val;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_tolstring(hash_map->luaState, -1, value_string_length_return);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

const char* LuaHashMap_GetValueStringForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyPointerWithLength(hash_map, key_pointer, NULL);
}

const char* LuaHashMap_GetValueStringForKeyPointerWithLength(LuaHashMap* hash_map, void* key_pointer, size_t* value_string_length_return)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyPointerWithLength(hash_map, key_pointer, value_string_length_return);
}

void* LuaHashMap_GetValuePointerForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	void* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	lua_Number ret_val;
	if(NULL == hash_map)
	{
		return 0.0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	lua_Integer ret_val;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

static const char* Internal_GetValueStringForKeyNumberWithLength(LuaHashMap* hash_map, lua_Number key_number, size_t* value_string_length_return)
{
	const char* ret_val;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_pointer, table] */
	ret_val = lua_tolstring(hash_map->luaState, -1, value_string_length_return);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

const char* LuaHashMap_GetValueStringForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyNumberWithLength(hash_map, key_number, NULL);
}

const char* LuaHashMap_GetValueStringForKeyNumberWithLength(LuaHashMap* hash_map, lua_Number key_number, size_t* value_string_length_return)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyNumberWithLength(hash_map, key_number, value_string_length_return);
}

void* LuaHashMap_GetValuePointerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	void* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_pointer, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	lua_Number ret_val;
	if(NULL == hash_map)
	{
		return 0.0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_number, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	lua_Integer ret_val;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_integer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

static const char* Internal_GetValueStringForKeyIntegerWithLength(LuaHashMap* hash_map, lua_Integer key_integer, size_t* value_string_length_return)
{
	const char* ret_val;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_string, table] */
	ret_val = lua_tolstring(hash_map->luaState, -1, value_string_length_return);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

const char* LuaHashMap_GetValueStringForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyIntegerWithLength(hash_map, key_integer, NULL);
}

const char* LuaHashMap_GetValueStringForKeyIntegerWithLength(LuaHashMap* hash_map, lua_Integer key_integer, size_t* value_string_length_return)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return Internal_GetValueStringForKeyIntegerWithLength(hash_map, key_integer, value_string_length_return);
}

void* LuaHashMap_GetValuePointerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	void* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_pointer, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	lua_Number ret_val;
	if(NULL == hash_map)
	{
		return 0.0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_number, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	lua_Integer ret_val;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_integer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}



static void Internal_RemoveKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlstring(hash_map->luaState, key_string, key_string_length); /* stack: [key_string, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=nil; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}

void LuaHashMap_RemoveKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}
	Internal_RemoveKeyStringWithLength(hash_map, key_string, strlen(key_string));
}

void LuaHashMap_RemoveKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}
	Internal_RemoveKeyStringWithLength(hash_map, key_string, key_string_length);
}

void LuaHashMap_RemoveKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=nil; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_RemoveKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=nil; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_RemoveKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=nil; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

static bool Internal_ExistsKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	bool ret_val;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlstring(hash_map->luaState, key_string, key_string_length); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value, table] */
	
	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

bool LuaHashMap_ExistsKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return false;
	}
	if(NULL == key_string)
	{
		return false;
	}
	return Internal_ExistsKeyStringWithLength(hash_map, key_string, strlen(key_string));	
}

bool LuaHashMap_ExistsKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return false;
	}
	if(NULL == key_string)
	{
		return false;
	}
	return Internal_ExistsKeyStringWithLength(hash_map, key_string, key_string_length);
}

bool LuaHashMap_ExistsKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	bool ret_val;
	if(NULL == hash_map)
	{
		return false;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */

	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

bool LuaHashMap_ExistsKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	bool ret_val;
	if(NULL == hash_map)
	{
		return false;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_pointer, table] */

	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

bool LuaHashMap_ExistsKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	bool ret_val;
	if(NULL == hash_map)
	{
		return false;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_pointer, table] */
	
	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

void Internal_Clear(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		lua_pop(hash_map->luaState, 1); /* pop the value because we don't need it; stack: [key table] */
		/* duplicate the key because we want to save a copy of it to be used for the next round of lua_next */
		lua_pushvalue(hash_map->luaState, -1); /* stack: [key key table] */
		lua_pushnil(hash_map->luaState); /* stack: [nil, key, key, table] */
		LUAHASHMAP_SETTABLE(hash_map->luaState, -4);  /* table[key_pointer]=nil; stack: [key table] */
		
		/* key is at the top of the stack, ready for next round of lua_next() */
	}
	lua_pop(hash_map->luaState, 1); /* pop the table */
}

void LuaHashMap_Clear(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
/*	Internal_InitializeInternalTables(hash_map); */
	/* If we simply replace the current table with a new one, we lose the internal current size of the hash
	 * which might be painful to reallocate if the user intends to refill the hash with the same amount of data.
	 * So instead, it is probably better to iterate through all keys and remove them.
	 * If the user wants to blow away everything for more speed, they can always destroy the hashmap and create a new one.
     * Lua Gems verifies that removing entries only nils entries and doesn't rehash so 
     * it is safe to iterate via lua_next while removing items.
	 * See Purge for clearing that reclaims memory.
	 */

	Internal_Clear(hash_map, (int) hash_map->uniqueTableNameForSharedState);
	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	LUAHASHMAP_ASSERT(true == LuaHashMap_IsEmpty(hash_map));
}


void LuaHashMap_Purge(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	/* Unlike Clear, this version replaces the existing table with a completely new one.
	 * This effectively purges the memory since Lua normally doesn't reclaim memory when nil-ing an entry.
	 * The presumption here is you really want the memory back.
	 */
	LUAHASHMAP_REPLACE_WITH_EMPTY_TABLE(hash_map->luaState, hash_map->uniqueTableNameForSharedState);

	/* Now seems to be a reasonable time to invoke garbage collection. */
	lua_gc(hash_map->luaState, LUA_GCCOLLECT, 0);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	LUAHASHMAP_ASSERT(true == LuaHashMap_IsEmpty(hash_map));
}


static bool Internal_IsEmpty(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	bool is_empty;
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	if(lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		is_empty = false;
		/* pop key, value, and table */
		lua_pop(hash_map->luaState, 3);
	}
	else
	{
		is_empty = true;
		/* pop table */
		lua_pop(hash_map->luaState, 1);
	}

	return is_empty;
}

bool LuaHashMap_IsEmpty(LuaHashMap* hash_map)
{
	bool is_empty = true;
	if(NULL == hash_map)
	{
		return true;
	}

	is_empty = Internal_IsEmpty(hash_map, (int) hash_map->uniqueTableNameForSharedState);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);		

	return is_empty;
}


static bool Internal_IteratorIsAlreadyNext(LuaHashMapIterator* hash_iterator)
{
	hash_iterator->isNext = false;
	
	/* This is my clue as to whether we are at an end iterator or not */
	if(LUA_TNONE == hash_iterator->keyType)
	{
		hash_iterator->atEnd = true;
		/* Memory should already be set correctly */
		return false;
	}
	/* Assumption: Remove already set the currentKey/Value and types so all we need to do is flip the isNext flag. */
	/* Note: There is a corner case where the cached values could be stale when the user is expecting them to be fresh.
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Removing: Price of %s: %lf\n", 
			LuaHashMap_GetCachedKeyStringAtIterator(&hash_iterator), 
			LuaHashMap_GetCachedValueNumberAtIterator(&hash_iterator));
		LuaHashMap_RemoveAtIterator(&hash_iterator);
		// Change a value in the hash that happens to be the next position.
		LuaHashMap_SetValue*ForKey*(...);
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
	
	 In this case, the value was already cached in RemoveAtIterator so in the next loop, the value is stale.
	 However, since this technically wasn't using the iterator APIs to set the value, this is one of the cases GetCached is documented to miss.
	 For performance reasons, I don't really want to re-fetch the value here. I presume the common case is to iterate and remove
	 so it doesn't make sense to do a double lookup.
	 */
	return true;	
}

static bool Internal_IteratorNext(LuaHashMapIterator* hash_iterator)
{
	LuaHashMap* hash_map = hash_iterator->hashMap;
	LuaHashMap_InternalGlobalKeyType table_name = hash_iterator->whichTable;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	
	 /* first key */
	if(LUA_TSTRING == hash_iterator->keyType)
	{
		lua_pushlstring(hash_map->luaState, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		lua_pushlightuserdata(hash_map->luaState, hash_iterator->currentKey.thePointer);
	}
	/* Note: This could be either a number or integer since they are the same to Lua.
	 * Since this is just for internal use, I don't need to distinguish.
	 */
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		lua_pushnumber(hash_map->luaState, hash_iterator->currentKey.theNumber);
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		lua_pop(hash_map->luaState, 1);
		return false;
	}

	if(lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		hash_iterator->atEnd = false;
		hash_iterator->keyType = lua_type(hash_map->luaState, -2);
		switch(hash_iterator->keyType)
		{
			case LUA_TSTRING:
			{
				hash_iterator->currentKey.theString.stringPointer = lua_tolstring(hash_map->luaState, -2, &hash_iterator->currentKey.theString.stringLength);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				hash_iterator->currentKey.thePointer = lua_touserdata(hash_map->luaState, -2);
				break;
			}
			case LUA_TNUMBER:
			{
				/* Note: Like above, I can't easily distinguish between a number and integer.
				 * So I will treat things as a number since that is the Lua default and cast later as necessary.
				 */
				hash_iterator->currentKey.theNumber = lua_tonumber(hash_map->luaState, -2);
				break;
			}
			default:
			{
				/* Clear the largest field to make sure every thing is cleared. */
				memset(&hash_iterator->currentKey, 0, sizeof(union LuaHashMapKeyValueType));
			}
		}
		hash_iterator->valueType = lua_type(hash_map->luaState, -1);
		switch(hash_iterator->valueType)
		{
			case LUA_TSTRING:
			{
				hash_iterator->currentValue.theString.stringPointer = lua_tolstring(hash_map->luaState, -1, &hash_iterator->currentValue.theString.stringLength);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				hash_iterator->currentValue.thePointer = lua_touserdata(hash_map->luaState, -1);
				break;
			}
			case LUA_TNUMBER:
			{
				/* Note: Like above, I can't easily distinguish between a number and integer.
				 * So I will treat things as a number since that is the Lua default and cast later as necessary.
				 */
				hash_iterator->currentValue.theNumber = lua_tonumber(hash_map->luaState, -1);
				break;
			}
			default:
			{
				/* Clear the largest field to make sure every thing is cleared. */
				memset(&hash_iterator->currentValue, 0, sizeof(union LuaHashMapKeyValueType));
			}
		}		
		/* pop key, value, and table */
		lua_pop(hash_map->luaState, 3);
	}
	else
	{
		hash_iterator->atEnd = true;
		hash_iterator->keyType = LUA_TNONE;
		hash_iterator->valueType = LUA_TNONE;
		/* Clear the largest field to make sure every thing is cleared. */
		memset(&hash_iterator->currentKey, 0, sizeof(union LuaHashMapKeyValueType));
		memset(&hash_iterator->currentValue, 0, sizeof(union LuaHashMapKeyValueType));

		/* pop table */
		lua_pop(hash_map->luaState, 1);
	}
	
	/* return true if more keys follow, false if we're at the end */
	return (false == hash_iterator->atEnd);
}

static LuaHashMapIterator Internal_GetIteratorBegin(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	LuaHashMapIterator the_iterator;
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = table_name;
	the_iterator.keyType = LUA_TNONE;
	the_iterator.valueType = LUA_TNONE;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	
	lua_pushnil(hash_map->luaState);  /* first key */
	if(lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		the_iterator.atEnd = false;
		the_iterator.keyType = lua_type(hash_map->luaState, -2);
		switch(the_iterator.keyType)
		{
			case LUA_TSTRING:
			{
				the_iterator.currentKey.theString.stringPointer = lua_tolstring(hash_map->luaState, -2, &the_iterator.currentKey.theString.stringLength);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				the_iterator.currentKey.thePointer = lua_touserdata(hash_map->luaState, -2);
				break;
			}
			case LUA_TNUMBER:
			{
				/* Note: Like above, I can't easily distinguish between a number and integer.
				 * So I will treat things as a number since that is the Lua default and cast later as necessary.
				 */
				the_iterator.currentKey.theNumber = lua_tonumber(hash_map->luaState, -2);
				break;
			}
			default:
			{
				the_iterator.currentKey.thePointer = NULL;
			}
		}
		the_iterator.valueType = lua_type(hash_map->luaState, -1);
		switch(the_iterator.valueType)
		{
			case LUA_TSTRING:
			{
				the_iterator.currentValue.theString.stringPointer = lua_tolstring(hash_map->luaState, -1, &the_iterator.currentValue.theString.stringLength);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				the_iterator.currentValue.thePointer = lua_touserdata(hash_map->luaState, -1);
				break;
			}
			case LUA_TNUMBER:
			{
				/* Note: Like above, I can't easily distinguish between a number and integer.
				 * So I will treat things as a number since that is the Lua default and cast later as necessary.
				 */
				the_iterator.currentValue.theNumber = lua_tonumber(hash_map->luaState, -1);
				break;
			}
			default:
			{
				the_iterator.currentValue.thePointer = NULL;
			}
		}

		/* pop key, value, and table */
		lua_pop(hash_map->luaState, 3);
	}
	else
	{
		the_iterator.atEnd = true;
		the_iterator.currentKey.thePointer = NULL;
		the_iterator.currentValue.thePointer = NULL;

		/* pop table */
		lua_pop(hash_map->luaState, 1);
	}
	
	return the_iterator;
}

static LuaHashMapIterator Internal_GetIteratorEnd(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	LuaHashMapIterator the_iterator;
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = table_name;
	the_iterator.atEnd = true;
	the_iterator.keyType = LUA_TNONE;	
	the_iterator.valueType = LUA_TNONE;	
	/* Clear the largest field to make sure every thing is cleared. */
	memset(&the_iterator.currentKey, 0, sizeof(union LuaHashMapKeyValueType));
	memset(&the_iterator.currentValue, 0, sizeof(union LuaHashMapKeyValueType));
	return the_iterator;
}

bool LuaHashMap_IteratorNext(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return false;
	}
	if(true == hash_iterator->atEnd)
	{
		return false;
	}
	if(true == hash_iterator->isNext)
	{
		return Internal_IteratorIsAlreadyNext(hash_iterator);
	}
	return Internal_IteratorNext(hash_iterator);
}

static LuaHashMapIterator Internal_CreateBadIterator()
{
	LuaHashMapIterator the_iterator;
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.whichTable = LUA_NOREF;
	the_iterator.atEnd = true;
	the_iterator.keyType = LUA_TNONE;
	the_iterator.valueType = LUA_TNONE;
	return the_iterator;
}

bool LuaHashMap_IteratorIsNotFound(const LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return true;
	}
	/* To make this distinct from a good iterator and an end iterator, whichTable==LUA_NOREF seems to be the only unique characteristic. */
	if(LUA_NOREF == hash_iterator->whichTable)
	{
		return true;
	}
	else
	{
		return false;		
	}
}

LuaHashMapIterator LuaHashMap_GetIteratorAtBegin(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorBegin(hash_map, (int) hash_map->uniqueTableNameForSharedState);
}

LuaHashMapIterator LuaHashMap_GetIteratorAtEnd(LuaHashMap* hash_map)
{
	return Internal_GetIteratorEnd(hash_map, (int) hash_map->uniqueTableNameForSharedState);
}

void Internal_SetCurrentValueInIteratorFromStackIndex(LuaHashMapIterator* the_iterator, int stack_index)
{
	int value_type = lua_type(the_iterator->hashMap->luaState, -1);
	switch(value_type)
	{
		case LUA_TSTRING:
		{
			the_iterator->currentValue.theString.stringPointer = lua_tolstring(the_iterator->hashMap->luaState, stack_index, &the_iterator->currentValue.theString.stringLength);
			break;			
		}
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		{
			the_iterator->currentValue.thePointer = lua_touserdata(the_iterator->hashMap->luaState, stack_index);
			break;
			
		}
		case LUA_TNUMBER:
		{
			the_iterator->currentValue.theNumber = lua_tonumber(the_iterator->hashMap->luaState, stack_index);
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			return;
		}
	}
	the_iterator->valueType = value_type;

}

static LuaHashMapIterator Internal_GetIteratorForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	const char* internalized_key_string = NULL;
	int value_type;
	LuaHashMapIterator the_iterator;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	/* pushes the string on the stack and sets internalized_key_string to the internalized Lua string pointer. */
	LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(hash_map->luaState, key_string, key_string_length, internalized_key_string); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value, table] */
	
	
	value_type = lua_type(hash_map->luaState, -1);
	if(LUA_TNIL == value_type)
	{
		/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
		lua_pop(hash_map->luaState, 2);
		LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
		return Internal_CreateBadIterator();
	}
	
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = (LuaHashMap_InternalGlobalKeyType) hash_map->uniqueTableNameForSharedState;
	the_iterator.keyType = LUA_TSTRING;
	/* Make sure to use the Lua internalized string and not the passed in string in case the string passed into this function gets released. */
	the_iterator.currentKey.theString.stringPointer = internalized_key_string;
	the_iterator.currentKey.theString.stringLength = key_string_length;

	switch(value_type)
	{
		case LUA_TSTRING:
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			
			/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
			lua_pop(hash_map->luaState, 2);
			LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
			return Internal_CreateBadIterator();
		}
	}
	
	Internal_SetCurrentValueInIteratorFromStackIndex(&the_iterator, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
		
	return the_iterator;
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	if(NULL == key_string)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorForKeyStringWithLength(hash_map, key_string, strlen(key_string));
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	if(NULL == key_string)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorForKeyStringWithLength(hash_map, key_string, key_string_length);
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	int value_type;
	LuaHashMapIterator the_iterator;
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	
	value_type = lua_type(hash_map->luaState, -1);
	if(LUA_TNIL == value_type)
	{
		/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
		lua_pop(hash_map->luaState, 2);
		LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
		return Internal_CreateBadIterator();
	}
	
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = (LuaHashMap_InternalGlobalKeyType) hash_map->uniqueTableNameForSharedState;
	the_iterator.keyType = LUA_TLIGHTUSERDATA;		
	the_iterator.currentKey.thePointer = key_pointer;

	switch(value_type)
	{
		case LUA_TSTRING:
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			
			/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
			lua_pop(hash_map->luaState, 2);
			LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
			return Internal_CreateBadIterator();
		}
	}
	
	Internal_SetCurrentValueInIteratorFromStackIndex(&the_iterator, -1);
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
		
	return the_iterator;
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	int value_type;
	LuaHashMapIterator the_iterator;
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_pointer, table] */
	
	value_type = lua_type(hash_map->luaState, -1);
	if(LUA_TNIL == value_type)
	{
		/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
		lua_pop(hash_map->luaState, 2);
		LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
		return Internal_CreateBadIterator();
	}
	
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = (LuaHashMap_InternalGlobalKeyType) hash_map->uniqueTableNameForSharedState;
	the_iterator.keyType = LUA_TNUMBER;
	the_iterator.currentKey.theNumber = key_number;

	switch(value_type)
	{
		case LUA_TSTRING:
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			
			/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
			lua_pop(hash_map->luaState, 2);
			LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
			return Internal_CreateBadIterator();
		}
	}
	
	Internal_SetCurrentValueInIteratorFromStackIndex(&the_iterator, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
		
	return the_iterator;
}



LuaHashMapIterator LuaHashMap_GetIteratorForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	int value_type;
	LuaHashMapIterator the_iterator;
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_pointer, table] */
	
	value_type = lua_type(hash_map->luaState, -1);
	if(LUA_TNIL == value_type)
	{
		/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
		lua_pop(hash_map->luaState, 2);
		LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
		return Internal_CreateBadIterator();
	}
	
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = (LuaHashMap_InternalGlobalKeyType) hash_map->uniqueTableNameForSharedState;
	/* Note: I might be able to benefit from the information that this is an integer,
	 * but since this is not the only entry point to create an iterator,
	 * its usefulness is limited.
	 */
	the_iterator.keyType = LUA_TNUMBER;
	the_iterator.currentKey.theNumber = (lua_Number)key_integer;		
	/*		the_iterator.currentKey.theInteger = key_integer; */

	switch(value_type)
	{
		case LUA_TSTRING:
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);

			/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
			lua_pop(hash_map->luaState, 2);
			LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);			
			return Internal_CreateBadIterator();
		}
	}
	
	Internal_SetCurrentValueInIteratorFromStackIndex(&the_iterator, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
		
	return the_iterator;
}

/*
static int Internal_safestrcmp(const char* str1, const char* str2)
{
	if(NULL == str1 && NULL == str2)
	{
		return 0;
	}
	else if(NULL == str1)
	{
		return 1;
	}
	else if(NULL == str2)
	{
		return -1;
	}
	else
	{
		return strcmp(str1, str2);
	}
}
*/

/* Can be faster than safestrcmp since it can use the string length to compare and doesn't need to return greater-than/less-than. */
static bool Internal_safestrequal(const char* str1, size_t length1, const char* str2, size_t length2)
{
	if(NULL == str1 && NULL == str2)
	{
		return true;
	}
	else if(length1 != length2)
	{
		return false;
	}
	else if(NULL == str1)
	{
		return false;
	}
	else if(NULL == str2)
	{
		return false;
	}
	else
	{
		return (0 == strcmp(str1, str2));
	}
}

bool LuaHashMap_IteratorIsEqual(const LuaHashMapIterator* hash_iterator1, const LuaHashMapIterator* hash_iterator2)
{
	if(NULL == hash_iterator1 && NULL == hash_iterator2)
	{
		return true;
	}
	else if(NULL == hash_iterator1)
	{
		return false;
	}
	else if(NULL == hash_iterator2)
	{
		return false;
	}
	else
	{
		/* Note: Don't test against keyType because when I create an End iterator, I don't know the type. */
		bool ret_flag = (hash_iterator1->hashMap == hash_iterator2->hashMap)
			&& (hash_iterator1->whichTable == hash_iterator2->whichTable)
			&& (hash_iterator1->atEnd == hash_iterator2->atEnd)
		;
		if(false == ret_flag)
		{
			return false;
		}
		/* If the iterators are both atEnd, we can consider them equal. */
		if(true == hash_iterator1->atEnd)
		{
			return true;
		}
		 
		
		if(LUA_TSTRING == hash_iterator1->keyType)
		{
			return (0 == Internal_safestrequal(hash_iterator1->currentKey.theString.stringPointer, hash_iterator1->currentKey.theString.stringLength, hash_iterator2->currentKey.theString.stringPointer, hash_iterator2->currentKey.theString.stringLength));
		}
		else if(LUA_TLIGHTUSERDATA == hash_iterator1->keyType)
		{
			return (hash_iterator1->currentKey.thePointer == hash_iterator2->currentKey.thePointer);
		}
		else if(LUA_TNUMBER == hash_iterator1->keyType)
		{
			return (hash_iterator1->currentKey.theNumber == hash_iterator2->currentKey.theNumber);
		}
		else
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			return false;
		}
	}
}


/* Helper function for LuaHashMap_SetValue*AtIterator. This pushes the table and key onto the stack provided from the iterator.
 * It is expected to be immediately followed by pushing the value and calling settable.
 * This should not fail. Validation checking is expected to be done before this.
 */
void Internal_PushTableAndKeyInIterator(LuaHashMapIterator* hash_iterator)
{
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_iterator->hashMap->luaState, hash_iterator->hashMap->uniqueTableNameForSharedState); /* stack: [table] */

	switch(hash_iterator->keyType)
	{
		case LUA_TSTRING:
		{
			lua_pushlstring(hash_iterator->hashMap->luaState, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength); /* stack: [key_string, table] */
			break;
		}
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		{
			lua_pushlightuserdata(hash_iterator->hashMap->luaState, hash_iterator->currentKey.thePointer); /* stack: [key_pointer, table] */
			break;
		}
		case LUA_TNUMBER:
		{
			/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
			 * If this is supposed to be an integer, I technically should be calling LuaHashMap_SetValue*ForKeyInteger..
			 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
			 * In stock Lua cases, I don't think there will be a real difference.
			 * To solve this, I would need to know the intention either by an explicit function, 
			 * or I need to track the intention using a declaration/hint in creation, 
			 * or I need to flag the first use of a type and save it.
			 */
			lua_pushnumber(hash_iterator->hashMap->luaState, hash_iterator->currentKey.theNumber); /* stack: [key_number, table] */
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			
			/* pop LUAHASHMAP_GETGLOBAL_UNIQUESTRING */
			lua_pop(hash_iterator->hashMap->luaState, 1);
		}
	}
}

static void Internal_SetValueStringAtIteratorWithLength(LuaHashMapIterator* restrict hash_iterator, const char* restrict value_string, size_t value_string_length)
{
	/* Early error checking so I can call Internal_PushTableAndKeyInIterator without worrying */
	switch(hash_iterator->keyType)
	{
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		case LUA_TSTRING:
		{
			if(NULL == hash_iterator->currentKey.theString.stringPointer)
			{
				LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);			
				return;
			}
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
			return;
		}
	}
	
	hash_iterator->valueType = LUA_TSTRING;
	
	Internal_PushTableAndKeyInIterator(hash_iterator); /* stack: [key, table] */
	LUAHASHMAP_PUSHLSTRING_AND_ASSIGNINTERNALSTRING(hash_iterator->hashMap->luaState, value_string, value_string_length, hash_iterator->currentValue.theString.stringPointer); /* stack: [value_string, key, table] */
	LUAHASHMAP_SETTABLE(hash_iterator->hashMap->luaState, -3);  /* table[key]=value_string; stack: [table] */
	
	hash_iterator->currentValue.theString.stringLength = value_string_length; /* Don't forget to save the length */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_iterator->hashMap->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
}

void LuaHashMap_SetValueStringAtIterator(LuaHashMapIterator* restrict hash_iterator, const char* restrict value_string)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	if(NULL == hash_iterator->hashMap)
	{
		return;
	}
	if(true == hash_iterator->isNext)
	{
		return;
	}
	Internal_SetValueStringAtIteratorWithLength(hash_iterator, value_string, strlen(value_string));
}

void LuaHashMap_SetValueStringAtIteratorWithLength(LuaHashMapIterator* restrict hash_iterator, const char* restrict value_string, size_t value_string_length)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	if(NULL == hash_iterator->hashMap)
	{
		return;
	}
	if(true == hash_iterator->isNext)
	{
		return;
	}
	Internal_SetValueStringAtIteratorWithLength(hash_iterator, value_string, value_string_length);
}


void LuaHashMap_SetValuePointerAtIterator(LuaHashMapIterator* hash_iterator, void* value_pointer)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	if(NULL == hash_iterator->hashMap)
	{
		return;
	}
	if(true == hash_iterator->isNext)
	{
		return;
	}
	
	/* Early error checking so I can call Internal_PushTableAndKeyInIterator without worrying */
	switch(hash_iterator->keyType)
	{
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		case LUA_TSTRING:
		{
			if(NULL == hash_iterator->currentKey.theString.stringPointer)
			{
				LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);			
				return;
			}
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
			return;
		}
	}
	
	hash_iterator->valueType = LUA_TLIGHTUSERDATA;
	
	Internal_PushTableAndKeyInIterator(hash_iterator); /* stack: [key, table] */
	lua_pushlightuserdata(hash_iterator->hashMap->luaState, value_pointer); /* stack: [value_pointer, key, table] */
	LUAHASHMAP_SETTABLE(hash_iterator->hashMap->luaState, -3);  /* table[key]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_iterator->hashMap->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
}

void LuaHashMap_SetValueNumberAtIterator(LuaHashMapIterator* hash_iterator, lua_Number value_number)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	if(NULL == hash_iterator->hashMap)
	{
		return;
	}
	if(true == hash_iterator->isNext)
	{
		return;
	}
	
	/* Early error checking so I can call Internal_PushTableAndKeyInIterator without worrying */
	switch(hash_iterator->keyType)
	{
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		case LUA_TSTRING:
		{
			if(NULL == hash_iterator->currentKey.theString.stringPointer)
			{
				LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);			
				return;
			}
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
			return;
		}
	}
	
	hash_iterator->valueType = LUA_TNUMBER;
	
	Internal_PushTableAndKeyInIterator(hash_iterator); /* stack: [key, table] */
	lua_pushnumber(hash_iterator->hashMap->luaState, value_number); /* stack: [value_number, key, table] */
	LUAHASHMAP_SETTABLE(hash_iterator->hashMap->luaState, -3);  /* table[key]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_iterator->hashMap->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
}

void LuaHashMap_SetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator, lua_Integer value_integer)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	if(NULL == hash_iterator->hashMap)
	{
		return;
	}
	if(true == hash_iterator->isNext)
	{
		return;
	}
	
	/* Early error checking so I can call Internal_PushTableAndKeyInIterator without worrying */
	switch(hash_iterator->keyType)
	{
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		case LUA_TSTRING:
		{
			if(NULL == hash_iterator->currentKey.theString.stringPointer)
			{
				LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);			
				return;
			}
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
			return;
		}
	}

	hash_iterator->valueType = LUA_TNUMBER;
	
	Internal_PushTableAndKeyInIterator(hash_iterator); /* stack: [key, table] */
	lua_pushinteger(hash_iterator->hashMap->luaState, value_integer); /* stack: [value_integer, key, table] */
	LUAHASHMAP_SETTABLE(hash_iterator->hashMap->luaState, -3);  /* table[key]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_iterator->hashMap->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
}


const char* LuaHashMap_GetKeyStringAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        return hash_iterator->currentKey.theString.stringPointer;
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return NULL;
	}
}

const char* LuaHashMap_GetKeyStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* key_string_length_return)
{
	if(NULL == hash_iterator)
	{
		if(NULL != key_string_length_return)
		{
			*key_string_length_return = 0;
		}
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		if(NULL != key_string_length_return)
		{
			*key_string_length_return = 0;
		}
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		if(NULL != key_string_length_return)
		{
			*key_string_length_return = 0;
		}
		return NULL;
	}

	if(NULL != key_string_length_return)
	{
		*key_string_length_return = hash_iterator->currentKey.theString.stringLength;
	}

	if(LUA_TSTRING == hash_iterator->keyType)
	{
        return hash_iterator->currentKey.theString.stringPointer;
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return NULL;
	}
}

size_t LuaHashMap_GetKeyStringLengthAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        return hash_iterator->currentKey.theString.stringLength;
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return 0;
	}
}

void* LuaHashMap_GetKeyPointerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	
	if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        return hash_iterator->currentKey.thePointer;
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return NULL;
	}
}

lua_Number LuaHashMap_GetKeyNumberAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0.0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0.0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0.0;
	}
	
	if(LUA_TNUMBER == hash_iterator->keyType)
	{
        return hash_iterator->currentKey.theNumber;
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return 0.0;
	}
}

lua_Integer LuaHashMap_GetKeyIntegerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0;
	}
	
	if(LUA_TNUMBER == hash_iterator->keyType)
	{
        return (lua_Integer)hash_iterator->currentKey.theNumber;
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return 0;
	}
}

static const char* Internal_GetValueStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* value_string_length_return)
{
	const char* ret_val = NULL;
	size_t ret_string_length = 0;
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValueStringForKeyStringWithLength(hash_iterator->hashMap, hash_iterator->currentKey.theString.stringPointer, &ret_string_length, hash_iterator->currentKey.theString.stringLength);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValueStringForKeyPointerWithLength(hash_iterator->hashMap, hash_iterator->currentKey.thePointer, &ret_string_length);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        ret_val = LuaHashMap_GetValueStringForKeyNumberWithLength(hash_iterator->hashMap, hash_iterator->currentKey.theNumber, &ret_string_length);
        /* return LuaHashMap_GetValueStringForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.theInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}
	hash_iterator->currentValue.theString.stringPointer = ret_val;
	hash_iterator->currentValue.theString.stringLength = ret_string_length;
	if(NULL != value_string_length_return)
	{
		*value_string_length_return = ret_string_length;
	}
	hash_iterator->valueType = LUA_TSTRING;
	return ret_val;
}


const char* LuaHashMap_GetValueStringAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	return Internal_GetValueStringAtIteratorWithLength(hash_iterator, NULL);
}

const char* LuaHashMap_GetValueStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* value_string_length_return)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	return Internal_GetValueStringAtIteratorWithLength(hash_iterator, value_string_length_return);
}

void* LuaHashMap_GetValuePointerAtIterator(LuaHashMapIterator* hash_iterator)
{
	void* ret_val = NULL;
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValuePointerForKeyStringWithLength(hash_iterator->hashMap, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValuePointerForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.thePointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        ret_val = LuaHashMap_GetValuePointerForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.theNumber);
        /* return LuaHashMap_GetValuePointerForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.theInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return NULL;
	}
	hash_iterator->currentValue.thePointer = ret_val;
	hash_iterator->valueType = LUA_TLIGHTUSERDATA;
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberAtIterator(LuaHashMapIterator* hash_iterator)
{
	lua_Number ret_val = 0.0;
	if(NULL == hash_iterator)
	{
		return 0.0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0.0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0.0;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValueNumberForKeyStringWithLength(hash_iterator->hashMap, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValueNumberForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.thePointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        ret_val = LuaHashMap_GetValueNumberForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.theNumber);
        /* return LuaHashMap_GetValueNumberForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.theInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return 0.0;
	}
	hash_iterator->currentValue.theNumber = ret_val;
	hash_iterator->valueType = LUA_TNUMBER;
	return ret_val;
}


lua_Integer LuaHashMap_GetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator)
{
	lua_Integer ret_val = 0;
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValueIntegerForKeyStringWithLength(hash_iterator->hashMap, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        ret_val = LuaHashMap_GetValueIntegerForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.thePointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        ret_val = LuaHashMap_GetValueIntegerForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.theNumber);
        /* return LuaHashMap_GetValueIntegerForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.theInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return 0;
	}
	hash_iterator->currentValue.theNumber = (lua_Number)ret_val;
	hash_iterator->valueType = LUA_TNUMBER;
	return ret_val;
}

const char* LuaHashMap_GetCachedValueStringAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	if(LUA_TSTRING != hash_iterator->valueType)
	{
		return NULL;
	}
	return hash_iterator->currentValue.theString.stringPointer;
}

const char* LuaHashMap_GetCachedValueStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* value_string_length_return)
{
	if(NULL == hash_iterator)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}
	if(LUA_TSTRING != hash_iterator->valueType)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}

	if(NULL != value_string_length_return)
	{
		*value_string_length_return = hash_iterator->currentKey.theString.stringLength;
	}

	return hash_iterator->currentValue.theString.stringPointer;
}

size_t LuaHashMap_GetCachedValueStringLengthAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0;
	}
	if(LUA_TSTRING != hash_iterator->valueType)
	{
		return 0;
	}
	return hash_iterator->currentValue.theString.stringLength;
}


void* LuaHashMap_GetCachedValuePointerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	if(LUA_TLIGHTUSERDATA != hash_iterator->valueType)
	{
		return NULL;
	}
	return hash_iterator->currentValue.thePointer;
}

lua_Number LuaHashMap_GetCachedValueNumberAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0.0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0.0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0.0;
	}
	if(LUA_TNUMBER != hash_iterator->valueType)
	{
		return 0.0;
	}
	return hash_iterator->currentValue.theNumber;
}

lua_Integer LuaHashMap_GetCachedValueIntegerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0;
	}
	if(LUA_TNUMBER != hash_iterator->valueType)
	{
		return 0;
	}
	return (lua_Integer)hash_iterator->currentValue.theNumber;
}


bool LuaHashMap_ExistsAtIterator(LuaHashMapIterator* hash_iterator)
{
	int value_type;
	if(NULL == hash_iterator)
	{
		return false;
	}
	if(true == hash_iterator->atEnd)
	{
		return false;
	}
	if(true == hash_iterator->isNext)
	{
		return false;
	}
	
	switch(hash_iterator->keyType)
	{
		case LUA_TSTRING:
		{
			break;
			
		}
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		{
			break;
			
		}
		case LUA_TNUMBER:
		{
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			return false;
		}
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_iterator->hashMap->luaState, hash_iterator->hashMap->uniqueTableNameForSharedState); /* stack: [table] */

	switch(hash_iterator->keyType)
	{
		case LUA_TSTRING:
		{
			lua_pushlstring(hash_iterator->hashMap->luaState, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength); /* stack: [key_string, table] */
			break;

		}
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		{
			lua_pushlightuserdata(hash_iterator->hashMap->luaState, hash_iterator->currentKey.thePointer); /* stack: [key_pointer, table] */

			break;

		}
		case LUA_TNUMBER:
		{
			lua_pushnumber(hash_iterator->hashMap->luaState, hash_iterator->currentKey.theNumber); /* stack: [key_number, table] */
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			/* pop LUAHASHMAP_GETGLOBAL_UNIQUESTRING */
			lua_pop(hash_iterator->hashMap->luaState, 1);
			LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);			
			return false;
		}
	}
	
	LUAHASHMAP_GETTABLE(hash_iterator->hashMap->luaState, -2);  /* table[key]; stack: [value, table] */

	value_type = lua_type(hash_iterator->hashMap->luaState, -1);
	
	/* Cache the value before returning */
	Internal_SetCurrentValueInIteratorFromStackIndex(hash_iterator, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_iterator->hashMap->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);			
	
	
	switch(value_type)
	{
		case LUA_TSTRING:
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}

	
}

void LuaHashMap_RemoveAtIterator(LuaHashMapIterator* hash_iterator)
{
	union LuaHashMapKeyValueType next_key = { 0 };
	union LuaHashMapKeyValueType next_value = { 0 };
	int next_key_type = LUA_TNONE;
	int next_value_type = LUA_TNONE;
	
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	if(true == hash_iterator->isNext)
	{
		return;
	}
	
	switch(hash_iterator->keyType)
	{
		case LUA_TSTRING:
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		case LUA_TNUMBER:
		{
			break;
		}
		default:
		{
			return;
		}
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_iterator->hashMap->luaState, hash_iterator->hashMap->uniqueTableNameForSharedState); /* stack: [table] */
	switch(hash_iterator->keyType)
	{
		case LUA_TSTRING:
		{
			lua_pushlstring(hash_iterator->hashMap->luaState, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength); /* stack: [key table] */
			break;
		}
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		{
			lua_pushlightuserdata(hash_iterator->hashMap->luaState, hash_iterator->currentKey.thePointer); /* stack: [key table] */
			break;
		}
		case LUA_TNUMBER:
		{
			lua_pushnumber(hash_iterator->hashMap->luaState, hash_iterator->currentKey.theNumber); /* stack: [key table] */
			break;
		}
		default:
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			/* pop LUAHASHMAP_GETGLOBAL_UNIQUESTRING */
			lua_pop(hash_iterator->hashMap->luaState, 1);
			LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
			return;
		}
	}
	
	/* Before setting the value to nil, we want to fetch/save the next key so iterating while removing can be done. */
	
	/* For performance, duplicate the just pushed key on the stack because we'll need to come back to it when we remove the value */
	/* This will avoid re-inserting from the C-side which may hopefully skip some strlen() and string comparisons in the string case. */
	/* It also saves us from needing to figure out what type the key is again. */
	lua_pushvalue(hash_iterator->hashMap->luaState, -1); /* stack: [key key table] */

	/* Using the duplicated key on top of the stack, use lua_next to find the next key/value pair. */
	if(lua_next(hash_iterator->hashMap->luaState, -3) != 0)
	{
		/* stack: [next_value next_key key table] */
		next_key_type = lua_type(hash_iterator->hashMap->luaState, -2); 
		next_value_type = lua_type(hash_iterator->hashMap->luaState, -1);
		
		/* Save the next key in a temporary variable */
		switch(next_key_type)
		{
			case LUA_TSTRING:
			{
				next_key.theString.stringPointer = lua_tolstring(hash_iterator->hashMap->luaState, -2, &next_key.theString.stringLength);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				next_key.thePointer = lua_touserdata(hash_iterator->hashMap->luaState, -2);
				break;
			}
			case LUA_TNUMBER:
			{
				next_key.theNumber = lua_tonumber(hash_iterator->hashMap->luaState, -2);
				break;
			}
			default:
			{
				break;
			}
		}
		
		/* Save the next value in a temporary variable */
		switch(next_value_type)
		{
			case LUA_TSTRING:
			{
				next_value.theString.stringPointer = lua_tolstring(hash_iterator->hashMap->luaState, -1, &next_value.theString.stringLength);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				next_value.thePointer = lua_touserdata(hash_iterator->hashMap->luaState, -1);
				break;
			}
			case LUA_TNUMBER:
			{
				next_value.theNumber = lua_tonumber(hash_iterator->hashMap->luaState, -1);
				break;
			}
			default:
			{
				break;
			}
		}

		/* Now pop the next key/value and go back to our regularly scheduled program. */
		lua_pop(hash_iterator->hashMap->luaState, 2); /* stack: [key table] */
	}
	else
	{
		/* No next, so we are at the end of the hash. */
		/* stack: [key table] */
		
		memset(&next_key, 0, sizeof(union LuaHashMapKeyValueType));
		memset(&next_value, 0, sizeof(union LuaHashMapKeyValueType));
		next_key_type = LUA_TNONE;
		next_value_type = LUA_TNONE;
	}
	
	
	/* Now back to the regularly scheduled program of removing the key/value */
	/* stack: [key table] */
	lua_pushnil(hash_iterator->hashMap->luaState); /* stack: [nil, key, table] */
	LUAHASHMAP_SETTABLE(hash_iterator->hashMap->luaState, -3);  /* table[key_string]=nil; stack: [table] */

	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_iterator->hashMap->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_iterator->hashMap->luaState) == 0);
	
	/* Now that we've removed the key/value pair, the current iterator is invalid.
	 * In the worst case, Lua will collect the key so when we try to use IteratorNext which has a stale key,
	 * it will fail when it tries to use lua_next with that stale key.
	 * To get around this, we mark this iterator as 'invalid' and save the next key so IteratorNext knows it needs to
	 * do something special because the normal current key has been invalidated/destroyed.
	 * I will use an isNext flag to denote this special case and reuse the currentKey and currentValue fields to store the next values.
	 * If there is no next, I will use the type LUA_TNONE to infer that there is no next in the IteratorNext implementation.
	 */
	hash_iterator->isNext = true;
	hash_iterator->keyType = next_key_type;
	hash_iterator->valueType = next_value_type;
	hash_iterator->currentKey = next_key;
	hash_iterator->currentValue = next_value;
}

static size_t Internal_Count(LuaHashMap* hash_map)
{
	size_t total_count = 0;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		total_count++;
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}

	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_Count(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return 0;
	}
	return Internal_Count(hash_map);
}

int LuaHashMap_GetKeyTypeAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return LUA_TNONE;
	}
	return hash_iterator->keyType;
}

int LuaHashMap_GetValueTypeAtIterator(LuaHashMapIterator* hash_iterator)
{
	int ret_val;
	LuaHashMap* hash_map;
	if(NULL == hash_iterator)
	{
		return LUA_TNONE;
	}
	if(true == hash_iterator->atEnd)
	{
		return LUA_TNONE;
	}
	if(true == hash_iterator->isNext)
	{
		return LUA_TNONE;
	}
	
	hash_map = hash_iterator->hashMap;
	if(NULL == hash_map)
	{
		return LUA_TNONE;
	}

	if(LUA_TSTRING == hash_iterator->keyType)
	{
		LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
		lua_pushlstring(hash_map->luaState, hash_iterator->currentKey.theString.stringPointer, hash_iterator->currentKey.theString.stringLength); /* stack: [key_string, table] */
		LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
		lua_pushlightuserdata(hash_map->luaState, hash_iterator->currentKey.thePointer); /* stack: [key_string, table] */
		LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't distinguish between a number and integer. */		
		LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
		lua_pushnumber(hash_map->luaState, hash_iterator->currentKey.theNumber); /* stack: [key_string, table] */
		LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return LUA_TNONE;
	}
	
	ret_val = lua_type(hash_map->luaState, -1);
	hash_iterator->valueType = ret_val;
	/* Cache the value in the iterator */
	if(LUA_TSTRING == hash_iterator->valueType)
	{
		hash_iterator->currentValue.theString.stringPointer = lua_tolstring(hash_map->luaState, -1, &hash_iterator->currentValue.theString.stringLength);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->valueType)
	{
		hash_iterator->currentValue.thePointer = lua_touserdata(hash_map->luaState, -1);
	}
	else if(LUA_TNUMBER == hash_iterator->valueType)
	{
		hash_iterator->currentValue.theNumber = lua_tonumber(hash_map->luaState, -1);
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return LUA_TNONE;
	}
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

int LuaHashMap_GetCachedValueTypeAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return LUA_TNONE;
	}
	if(true == hash_iterator->atEnd)
	{
		return LUA_TNONE;
	}
	if(true == hash_iterator->isNext)
	{
		return LUA_TNONE;
	}
	
	return hash_iterator->valueType;
}




/************************************ DEPRECATED *********************************************************/
size_t LuaHashMap_GetKeysString(LuaHashMap* hash_map, const char* keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_tostring(hash_map->luaState, -2);
		}

		total_count++;

		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}

	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_GetKeysPointer(LuaHashMap* hash_map, void* keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_touserdata(hash_map->luaState, -2);
		}

		total_count++;

		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}

	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_GetKeysNumber(LuaHashMap* hash_map, lua_Number keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	
	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_tonumber(hash_map->luaState, -2);
		}
		
		total_count++;
		
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}
	
	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_GetKeysInteger(LuaHashMap* hash_map, lua_Integer keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	
	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_tointeger(hash_map->luaState, -2);
		}
		
		total_count++;
		
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}
	
	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}
/************************************ END DEPRECATED *********************************************************/




#if defined(__LUAHASHMAP_RESTRICT_KEYWORD_DEFINED__)
	#undef restrict
	#undef __LUAHASHMAP_RESTRICT_KEYWORD_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_BOOL_KEYWORD_DEFINED__)
	#undef bool
	#undef __LUAHASHMAP_BOOL_KEYWORD_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_FALSE_KEYWORD_DEFINED__)
	#undef false
	#undef __LUAHASHMAP_FALSE_KEYWORD_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_TRUE_KEYWORD_DEFINED__)
	#undef true
	#undef __LUAHASHMAP_TRUE_KEYWORD_DEFINED__
#endif



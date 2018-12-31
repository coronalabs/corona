/*
** LuaProfiler
** Copyright Kepler Project 2005-2007 (http://www.keplerproject.org/luaprofiler)
** $Id: lua50_profiler.c,v 1.13 2008/05/19 18:36:23 mascarenhas Exp $
*/

/*****************************************************************************
lua50_profiler.c:
   Lua version dependent profiler interface
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clocks.h"
#include "core_profiler.h"
#include "function_meter.h"

#include "lua.h"
#include "lauxlib.h"

/* Indices for the main profiler stack and for the original exit function */
static int exit_id;
static int profstate_id;

/* Forward declaration */
static float calcCallTime(lua_State *L);

/* called by Lua (via the callhook mechanism) */
static void callhook(lua_State *L, lua_Debug *ar) {
  int currentline;
  lua_Debug previous_ar;
  lprofP_STATE* S;
  lua_pushlightuserdata(L, &profstate_id);
  lua_gettable(L, LUA_REGISTRYINDEX);
  S = (lprofP_STATE*)lua_touserdata(L, -1);

  if (lua_getstack(L, 1, &previous_ar) == 0) {
    currentline = -1;
  } else {
    lua_getinfo(L, "l", &previous_ar);
    currentline = previous_ar.currentline;
  }
      
  lua_getinfo(L, "nS", ar);

  if (!ar->event) {
    /* entering a function */
    lprofP_callhookIN(S, (char *)ar->name,
		      (char *)ar->source, ar->linedefined,
		      currentline);
  }
  else { /* ar->event == "return" */
    lprofP_callhookOUT(S);
  }
}


/* Lua function to exit politely the profiler                               */
/* redefines the lua exit() function to not break the log file integrity    */
/* The log file is assumed to be valid if the last entry has a stack level  */
/* of 1 (meaning that the function 'main' has been exited)                  */
static void exit_profiler(lua_State *L) {
  lprofP_STATE* S;
  lua_pushlightuserdata(L, &profstate_id);
  lua_gettable(L, LUA_REGISTRYINDEX);
  S = (lprofP_STATE*)lua_touserdata(L, -1);
  /* leave all functions under execution */
  while (lprofP_callhookOUT(S)) ;
  /* call the original Lua 'exit' function */
  lua_pushlightuserdata(L, &exit_id);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua_call(L, 0, 0);
}

/* Our new coroutine.create function  */
/* Creates a new profile state for the coroutine */
#if 0
static int coroutine_create(lua_State *L) {
  lprofP_STATE* S;
  lua_State *NL = lua_newthread(L);
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1), 1,
		"Lua function expected");
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  /* Inits profiler and sets profiler hook for this coroutine */
  S = lprofM_init();
  lua_pushlightuserdata(L, NL);
  lua_pushlightuserdata(L, S);
  lua_settable(L, LUA_REGISTRYINDEX);
  lua_sethook(NL, (lua_Hook)callhook, LUA_MASKCALL | LUA_MASKRET, 0);
  return 1;	
}
#endif

static int profiler_pause(lua_State *L) {
  lprofP_STATE* S;
  lua_pushlightuserdata(L, &profstate_id);
  lua_gettable(L, LUA_REGISTRYINDEX);
  S = (lprofP_STATE*)lua_touserdata(L, -1);
  lprofM_pause_function(S);
  return 0;
}

static int profiler_resume(lua_State *L) {
  lprofP_STATE* S;
  lua_pushlightuserdata(L, &profstate_id);
  lua_gettable(L, LUA_REGISTRYINDEX);
  S = (lprofP_STATE*)lua_touserdata(L, -1);
  lprofM_pause_function(S);
  return 0;
}

static int profiler_init(lua_State *L) {
  lprofP_STATE* S;
  const char* outfile;
  float function_call_time;

  function_call_time = calcCallTime(L);

  outfile = NULL;
  if(lua_gettop(L) == 1)
    outfile = luaL_checkstring(L, -1);

  lua_sethook(L, (lua_Hook)callhook, LUA_MASKCALL | LUA_MASKRET, 0);
  /* init with default file name and printing a header line */
  if (!(S=lprofP_init_core_profiler(outfile, 1, function_call_time))) {
    luaL_error(L,"LuaProfiler error: output file could not be opened!");
    lua_pushnil(L);
    return 1;
  }

  lua_pushlightuserdata(L, &profstate_id);
  lua_pushlightuserdata(L, S);
  lua_settable(L, LUA_REGISTRYINDEX);
	
  /* use our own exit function instead */
  lua_getglobal(L, "os");
  lua_pushlightuserdata(L, &exit_id);
  lua_pushstring(L, "exit");
  lua_gettable(L, -3);
  lua_settable(L, LUA_REGISTRYINDEX);
  lua_pushstring(L, "exit");
  lua_pushcfunction(L, (lua_CFunction)exit_profiler);
  lua_settable(L, -3);

#if 0
  /* use our own coroutine.create function instead */
  lua_getglobal(L, "coroutine");
  lua_pushstring(L, "create");
  lua_pushcfunction(L, (lua_CFunction)coroutine_create);
  lua_settable(L, -3);
#endif

  /* the following statement is to simulate how the execution stack is */
  /* supposed to be by the time the profiler is activated when loaded  */
  /* as a library.                                                     */

  lprofP_callhookIN(S, "profiler_init", "(C)", -1, -1);
	
  lua_pushboolean(L, 1);
  return 1;
}

static int profiler_stop(lua_State *L) {
  lprofP_STATE* S;
  lua_sethook(L, (lua_Hook)callhook, 0, 0);
  lua_pushlightuserdata(L, &profstate_id);
  lua_gettable(L, LUA_REGISTRYINDEX);
  if(!lua_isnil(L, -1)) {
    S = (lprofP_STATE*)lua_touserdata(L, -1);
    /* leave all functions under execution */
    while (lprofP_callhookOUT(S));
    lprofP_close_core_profiler(S);
    lua_pushboolean(L, 1);
  } else { lua_pushboolean(L, 0); }
  return 1;
}

/* calculates the approximate time Lua takes to call a function */
static float calcCallTime(lua_State *L) {
  clock_t timer;
  char lua_code[] = "                                     \
                   function lprofT_mesure_function()    \
                   local i                              \
                                                        \
                      local t = function()              \
                      end                               \
                                                        \
                      i = 1                             \
                      while (i < 100000) do             \
                         t()                            \
                         i = i + 1                      \
                      end                               \
                   end                                  \
                                                        \
                   lprofT_mesure_function()             \
                   lprofT_mesure_function = nil         \
                 ";

  lprofC_start_timer(&timer);
  luaL_dostring(L, lua_code);
  return lprofC_get_seconds(timer) / (float) 100000;
}

static const luaL_reg prof_funcs[] = {
  { "pause", profiler_pause },
  { "resume", profiler_resume },
  { "start", profiler_init },
  { "stop", profiler_stop },
  { NULL, NULL }
};

int luaopen_profiler(lua_State *L) {
  luaL_openlib(L, "profiler", prof_funcs, 0);
  lua_pushliteral (L, "_COPYRIGHT");
  lua_pushliteral (L, "Copyright (C) 2003-2007 Kepler Project");
  lua_settable (L, -3);
  lua_pushliteral (L, "_DESCRIPTION");
  lua_pushliteral (L, "LuaProfiler is a time profiler designed to help finding bottlenecks in your Lua program.");
  lua_settable (L, -3);
  lua_pushliteral (L, "_NAME");
  lua_pushliteral (L, "LuaProfiler");
  lua_settable (L, -3);
  lua_pushliteral (L, "_VERSION");
  lua_pushliteral (L, "2.0.1");
  lua_settable (L, -3);
  return 1;
}

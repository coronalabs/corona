/*
 * $Id: javavm.h 131 2012-01-23 20:25:29Z andre@naef.com $
 * Provides the Java VM module. See LICENSE.txt for license terms.
 */

#ifndef JNLUA_JAVAVM_INCLUDED
#define JNLUA_JAVAVM_INCLUDED

#include <lua.h>

/**
 * Opens the Java VM module in a Lua state.
 *
 * @param L the Lua state
 * @return the number of results
 */
LUALIB_API int luaopen_javavm (lua_State *L);

#endif /* JNLUA_JAVAVM_INCLUDED */

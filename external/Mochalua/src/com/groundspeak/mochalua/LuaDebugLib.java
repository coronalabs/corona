// Copyright (c) 2008 Groundspeak, Inc.

// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
package com.groundspeak.mochalua;

/**
 *
 * @author a.fornwald
 */
public class LuaDebugLib {

    public static final String LUA_DBLIBNAME = "debug";
    private static Character KEY_HOOK = new Character ( 'h' );

    public static final class hookf implements lua_Hook {

        public void Call ( lua_State thread, lua_Debug ar ) {
            String hooknames[] = new String[] {
                "call",
                "return",
                "line",
                "count",
                "tail return"
            };
            LuaAPI.lua_pushlightuserdata ( thread, KEY_HOOK );
            LuaAPI.lua_rawget ( thread, LuaAPI.LUA_REGISTRYINDEX );
            LuaAPI.lua_pushlightuserdata ( thread, thread );
            LuaAPI.lua_rawget ( thread, -2 );
            if ( LuaAPI.lua_isfunction ( thread, -1 ) ) {
                LuaAPI.lua_pushstring ( thread, hooknames[( int ) ar.m_iEvent] );
                if ( ar.m_iCurrentLine >= 0 ) {
                    LuaAPI.lua_pushinteger ( thread, ar.m_iCurrentLine );
                }
                else {
                    LuaAPI.lua_pushnil ( thread );
                }
                //LuaAPI.lua_assert( LuaAPI.lua_getinfo( thread, "lS", ar ) );
                LuaAPI.lua_call ( thread, 2, 0 );
            }
        }
    }

    public static final class db_getfenv implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_getfenv ( thread, 1 );
            return 1;
        }
    }

    private static String unmakemask ( int mask ) {
        char smask[] = new char[ 5 ];
        int i = 0;
        if ( ( mask & LuaAPI.LUA_MASKCALL ) != 0 )
            smask[i ++] = 'c';
        if ( ( mask & LuaAPI.LUA_MASKRET ) != 0 )
            smask[i ++] = 'r';
        if ( ( mask & LuaAPI.LUA_MASKLINE ) != 0 )
            smask[i ++] = 'l';
        smask[i] = '\0';
        return new String ( smask );
    }

    private static void gethooktable ( lua_State thread ) {
        LuaAPI.lua_pushlightuserdata ( thread, KEY_HOOK );
        LuaAPI.lua_rawget ( thread, LuaAPI.LUA_REGISTRYINDEX );
        if (  ! LuaAPI.lua_istable ( thread, -1 ) ) {
            LuaAPI.lua_pop ( thread, 1 );
            LuaAPI.lua_createtable ( thread, 0, 1 );
            LuaAPI.lua_pushlightuserdata ( thread, KEY_HOOK );
            LuaAPI.lua_pushvalue ( thread, -2 );
            LuaAPI.lua_rawset ( thread, LuaAPI.LUA_REGISTRYINDEX );
        }
    }

    public static final class db_gethook implements JavaFunction {

        public int Call ( lua_State thread ) {
            int arg;
            lua_State L1 = null;

            if ( LuaAPI.lua_isthread ( thread, 1 ) ) {
                L1 = LuaAPI.lua_tothread ( thread, 1 );
            }
            else {
                L1 = thread;
            }

            int mask = LuaAPI.lua_gethookmask ( L1 );
            lua_Hook hook = LuaAPI.lua_gethook ( L1 );
            if ( hook != null &&  ! ( hook instanceof hookf ) ) // external hook? 
            {
                LuaAPI.lua_pushliteral ( thread, "external hook" );
            }
            else {
                gethooktable ( thread );
                LuaAPI.lua_pushlightuserdata ( thread, L1 );
                LuaAPI.lua_rawget ( thread, -2 );   // get hook 

                LuaAPI.lua_remove ( thread, -2 );  // remove hook table

            }
            LuaAPI.lua_pushstring ( thread, unmakemask ( mask ) );
            LuaAPI.lua_pushinteger ( thread, LuaAPI.lua_gethookcount ( L1 ) );
            return 3;
        }
    }

    private static void settabss ( lua_State thread, String i, String v ) {
        LuaAPI.lua_pushstring ( thread, v );
        LuaAPI.lua_setfield ( thread, -2, i );
    }

    private static void settabsi ( lua_State thread, String i, int v ) {
        LuaAPI.lua_pushinteger ( thread, v );
        LuaAPI.lua_setfield ( thread, -2, i );
    }

    private static void treatstackoption ( lua_State thread, lua_State L1, String fname ) {
        if ( thread == L1 ) {
            LuaAPI.lua_pushvalue ( thread, -2 );
            LuaAPI.lua_remove ( thread, -3 );
        }
        else {
            LuaAPI.lua_xmove ( L1, thread, 1 );
        }
        LuaAPI.lua_setfield ( thread, -2, fname );
    }

    public static final class db_getinfo implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_Debug ar = new lua_Debug ();
            int arg;
            lua_State L1 = null;

            if ( LuaAPI.lua_isthread ( thread, 1 ) ) {
                arg = 1;
                L1 = LuaAPI.lua_tothread ( thread, 1 );
            }
            else {
                arg = 0;
                L1 = thread;
            }

            String options = LuaAPI.luaL_optstring ( thread, arg + 2, "flnSu" );
            if ( LuaAPI.lua_isnumber ( thread, arg + 1 ) ) {
                if ( LuaAPI.lua_getstack ( L1, ( int ) LuaAPI.lua_tointeger ( thread, arg + 1 ), ar ) == 0 ) {
                    LuaAPI.lua_pushnil ( thread );  /* level out of range */
                    return 1;
                }
            }
            else if ( LuaAPI.lua_isfunction ( thread, arg + 1 ) ) {
                LuaAPI.lua_pushstring ( thread, ">" + options );
                options = LuaAPI.lua_tostring ( thread, -1 );
                LuaAPI.lua_pushvalue ( thread, arg + 1 );
                LuaAPI.lua_xmove ( thread, L1, 1 );
            }
            else {
                return LuaAPI.luaL_argerror ( thread, arg + 1, "function or level expected" );
            }

            if ( LuaAPI.lua_getinfo ( L1, options, ar ) == 0 ) {
                return LuaAPI.luaL_argerror ( thread, arg + 2, "invalid option" );
            }
            LuaAPI.lua_createtable ( thread, 0, 2 );
            if ( options.indexOf ( 'S' ) != -1 ) {
                settabss ( thread, "source", ar.m_strSource );
                settabss ( thread, "short_src", ar.m_strShortSource );
                settabsi ( thread, "linedefined", ar.m_iLineDefined );
                settabsi ( thread, "lastlinedefined", ar.m_iLastLineDefined );
                settabss ( thread, "what", ar.m_strWhat );
            }
            if ( options.indexOf ( 'l' ) != -1 ) {
                settabsi ( thread, "currentline", ar.m_iCurrentLine );
            }
            if ( options.indexOf ( 'u' ) != -1 ) {
                settabsi ( thread, "nups", ar.m_UpValuesQuantity );
            }
            if ( options.indexOf ( 'n' ) != -1 ) {
                settabss ( thread, "name", ar.m_strName );
                settabss ( thread, "namewhat", ar.m_strNameWhat );
            }
            if ( options.indexOf ( 'L' ) != -1 ) {
                treatstackoption ( thread, L1, "activelines" );
            }
            if ( options.indexOf ( 'f' ) != -1 ) {
                treatstackoption ( thread, L1, "func" );
            }
            return 1;  /* return table */
        }
    }

    public static final class db_getlocal implements JavaFunction {

        public int Call ( lua_State thread ) {
            int arg;
            lua_Debug ar = new lua_Debug ();
            lua_State L1 = null;

            if ( LuaAPI.lua_isthread ( thread, 1 ) ) {
                arg = 1;
                L1 = LuaAPI.lua_tothread ( thread, 1 );
            }
            else {
                arg = 0;
                L1 = thread;
            }

            if ( LuaAPI.lua_getstack ( L1, LuaAPI.luaL_checkint ( thread, arg + 1 ), ar ) == 0 ) /* out of range? */ {
                return LuaAPI.luaL_argerror ( thread, arg + 1, "level out of range" );
            }

            String name = LuaAPI.lua_getlocal ( L1, ar, LuaAPI.luaL_checkint ( thread, arg + 2 ) );
            if ( name != null ) {
                LuaAPI.lua_xmove ( L1, thread, 1 );
                LuaAPI.lua_pushstring ( thread, name );
                LuaAPI.lua_pushvalue ( thread, -2 );
                return 2;
            }
            else {
                LuaAPI.lua_pushnil ( thread );
                return 1;
            }
        }
    }

    public static final class db_getregistry implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushvalue ( thread, LuaAPI.LUA_REGISTRYINDEX );
            return 1;
        }
    }

    public static final class db_getmetatable implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 1 );
            if (  ! LuaAPI.lua_getmetatable ( thread, 1 ) ) {
                LuaAPI.lua_pushnil ( thread );  /* no metatable */
            }
            return 1;
        }
    }

    private static int auxupvalue ( lua_State thread, int get ) {
        int n = LuaAPI.luaL_checkint ( thread, 2 );
        LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TFUNCTION );
        if ( LuaAPI.lua_isjavafunction ( thread, 1 ) ) {
            return 0;  /* cannot touch C upvalues from Lua */
        }

        String name = get != 0 ? LuaAPI.lua_getupvalue ( thread, 1, n ) : LuaAPI.lua_setupvalue ( thread, 1, n );
        if ( name == null ) {
            return 0;
        }
        LuaAPI.lua_pushstring ( thread, name );
        LuaAPI.lua_insert ( thread,  - ( get + 1 ) );
        return get + 1;
    }

    public static final class db_getupvalue implements JavaFunction {

        public int Call ( lua_State thread ) {
            return auxupvalue ( thread, 1 );
        }
    }

    public static final class db_setfenv implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 2, LuaAPI.LUA_TTABLE );
            LuaAPI.lua_settop ( thread, 2 );
            if ( LuaAPI.lua_setfenv ( thread, 1 ) == false ) {
                LuaAPI.luaL_error ( thread, "\"setfenv\" cannot change environment of given object" );
            }

            return 1;
        }
    }

    private static int makemask ( String smask, int count ) {
        int mask = 0;
        if ( smask.indexOf ( 'c' ) != -1 )
            mask |= LuaAPI.LUA_MASKCALL;
        if ( smask.indexOf ( 'r' ) != -1 )
            mask |= LuaAPI.LUA_MASKRET;
        if ( smask.indexOf ( 'l' ) != -1 )
            mask |= LuaAPI.LUA_MASKLINE;
        if ( count > 0 )
            mask |= LuaAPI.LUA_MASKCOUNT;
        return mask;
    }

    public static final class db_sethook implements JavaFunction {

        public int Call ( lua_State thread ) {
            int arg, mask, count;
            lua_Hook func = null;
            lua_State L1 = null;
            if ( LuaAPI.lua_isthread ( thread, 1 ) ) {
                arg = 1;
                L1 = LuaAPI.lua_tothread ( thread, 1 );
            }
            else {
                arg = 0;
                L1 = thread;
            }
            if ( LuaAPI.lua_isnoneornil ( thread, arg + 1 ) ) {
                LuaAPI.lua_settop ( thread, arg + 1 );
                func = null;
                mask = 0;
                count = 0;  /* turn off hooks */
            }
            else {
                String smask = LuaAPI.luaL_checkstring ( thread, arg + 2 );
                LuaAPI.luaL_checktype ( thread, arg + 1, LuaAPI.LUA_TFUNCTION );
                count = LuaAPI.luaL_optint ( thread, arg + 3, 0 );
                func = new hookf ();
                mask = makemask ( smask, count );
            }
            gethooktable ( thread );
            LuaAPI.lua_pushlightuserdata ( thread, L1 );
            LuaAPI.lua_pushvalue ( thread, arg + 1 );
            LuaAPI.lua_rawset ( thread, -3 );  /* set new hook */
            LuaAPI.lua_pop ( thread, 1 );  /* remove hook table */
            LuaAPI.lua_sethook ( L1, func, mask, count );  /* set hooks */
            return 0;
        }
    }

    public static final class db_setlocal implements JavaFunction {

        public int Call ( lua_State thread ) {
            int arg;
            lua_State L1 = null;
            if ( LuaAPI.lua_isthread ( thread, 1 ) ) {
                arg = 1;
                L1 = LuaAPI.lua_tothread ( thread, 1 );
            }
            else {
                arg = 0;
                L1 = thread;
            }
            lua_Debug ar = new lua_Debug ();
            if ( LuaAPI.lua_getstack ( L1, LuaAPI.luaL_checkint ( thread, arg + 1 ), ar ) == 0 ) /* out of range? */ {
                return LuaAPI.luaL_argerror ( thread, arg + 1, "level out of range" );
            }

            LuaAPI.luaL_checkany ( thread, arg + 3 );
            LuaAPI.lua_settop ( thread, arg + 3 );
            LuaAPI.lua_xmove ( thread, L1, 1 );
            LuaAPI.lua_pushstring ( thread, LuaAPI.lua_setlocal ( L1, ar, LuaAPI.luaL_checkint ( thread, arg + 2 ) ) );
            return 1;
        }
    }

    public static final class db_setmetatable implements JavaFunction {

        public int Call ( lua_State thread ) {
            int t = LuaAPI.lua_type ( thread, 2 );
            LuaAPI.luaL_argcheck ( thread, t == LuaAPI.LUA_TNIL || t == LuaAPI.LUA_TTABLE, 2,
                "nil or table expected" );
            LuaAPI.lua_settop ( thread, 2 );
            LuaAPI.lua_pushboolean ( thread, LuaAPI.lua_setmetatable ( thread, 1 ) );
            return 1;
        }
    }

    public static final class db_setupvalue implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 3 );
            return auxupvalue ( thread, 0 );
        }
    }
    private static final int LEVELS1 = 12;	/* size of the first part of the stack */

    private static final int LEVELS2 = 10;	/* size of the second part of the stack */


    public static final class db_errorfb implements JavaFunction {

        public int Call ( lua_State thread ) {
            int level;
            int firstpart = 1;  /* still before eventual `...' */
            int arg;
            lua_State L1 = null;

            if ( LuaAPI.lua_isthread ( thread, 1 ) ) {
                arg = 1;
                L1 = LuaAPI.lua_tothread ( thread, 1 );
            }
            else {
                arg = 0;
                L1 = thread;
            }

            lua_Debug ar = new lua_Debug ();
            if ( LuaAPI.lua_isnumber ( thread, arg + 2 ) ) {
                level = ( int ) LuaAPI.lua_tointeger ( thread, arg + 2 );
                LuaAPI.lua_pop ( thread, 1 );
            }
            else {
                level = ( thread == L1 ) ? 1 : 0;  /* level 0 may be this own function */
            }
            if ( LuaAPI.lua_gettop ( thread ) == arg ) {
                LuaAPI.lua_pushliteral ( thread, "" );
            }
            else if (  ! LuaAPI.lua_isstring ( thread, arg + 1 ) ) {
                return 1;  /* message is not a string */
            }
            else {
                LuaAPI.lua_pushliteral ( thread, "\n" );
            }
            LuaAPI.lua_pushliteral ( thread, "stack traceback:" );
            while ( LuaAPI.lua_getstack ( L1, level ++, ar ) != 0 ) {
                if ( ( level > LEVELS1 ) && ( firstpart != 0 ) ) {
                    /* no more than `LEVELS2' more levels? */
                    if ( LuaAPI.lua_getstack ( L1, level + LEVELS2, ar ) == 0 ) {
                        level --;  /* keep going */
                    }
                    else {
                        LuaAPI.lua_pushliteral ( thread, "\n\t..." );  /* too many levels */
                        while ( LuaAPI.lua_getstack ( L1, level + LEVELS2, ar ) != 0 ) /* find last levels */ {
                            level ++;
                        }
                    }
                    firstpart = 0;
                    continue;
                }
                LuaAPI.lua_pushliteral ( thread, "\n\t" );
                LuaAPI.lua_getinfo ( L1, "Snl", ar );
                LuaAPI.lua_pushstring ( thread, ar.m_strShortSource + ":" );
                if ( ar.m_iCurrentLine > 0 ) {
                    LuaAPI.lua_pushstring ( thread, ar.m_iCurrentLine + ":" );
                }
                if ( ar.m_strNameWhat != null && ar.m_strNameWhat.length () != 0 && ar.m_strNameWhat.charAt ( 0 ) != '\0' ) /* is there a name? */ {
                    LuaAPI.lua_pushstring ( thread, " in function '" + ar.m_strName + "'" );
                }
                else {
                    if ( ar.m_strWhat != null && ar.m_strWhat.length () != 0 && ar.m_strWhat.charAt ( 0 ) == 'm' ) /* main? */ {
                        LuaAPI.lua_pushfstring ( thread, " in main chunk" );
                    }
                    else if ( ar.m_strWhat != null && ar.m_strWhat.length () != 0 && ( ar.m_strWhat.charAt ( 0 ) == 'C' || ar.m_strWhat.charAt ( 0 ) == 't' ) ) {
                        LuaAPI.lua_pushliteral ( thread, " ?" );  /* C function or tail call */
                    }
                    else {
                        LuaAPI.lua_pushfstring ( thread, " in function <" + ar.m_strShortSource + ":" + ar.m_iLineDefined + ">" );
                    }
                }
                LuaAPI.lua_concat ( thread, LuaAPI.lua_gettop ( thread ) - arg );
            }
            LuaAPI.lua_concat ( thread, LuaAPI.lua_gettop ( thread ) - arg );
            return 1;
        }
    }

    public static final class luaopen_debug implements JavaFunction {

        public int Call ( lua_State thread ) {
            luaL_Reg[] luaReg = new luaL_Reg[] {
                new luaL_Reg ( "getfenv", new db_getfenv () ),
                new luaL_Reg ( "gethook", new db_gethook () ),
                new luaL_Reg ( "getinfo", new db_getinfo () ),
                new luaL_Reg ( "getlocal", new db_getlocal () ),
                new luaL_Reg ( "getregistry", new db_getregistry () ),
                new luaL_Reg ( "getmetatable", new db_getmetatable () ),
                new luaL_Reg ( "getupvalue", new db_getupvalue () ),
                new luaL_Reg ( "setfenv", new db_setfenv () ),
                new luaL_Reg ( "sethook", new db_sethook () ),
                new luaL_Reg ( "setlocal", new db_setlocal () ),
                new luaL_Reg ( "setmetatable", new db_setmetatable () ),
                new luaL_Reg ( "setupvalue", new db_setupvalue () ),
                new luaL_Reg ( "traceback", new db_errorfb () ),
            };

            LuaAPI.luaL_register ( thread, LUA_DBLIBNAME, luaReg );
            return 1;
        }
    }

    public static final class traceback implements JavaFunction {

        public int Call ( lua_State thread ) {
            if (  ! LuaAPI.lua_isstring ( thread, 1 ) ) /* 'message' not a string? */ {
                return 1;  /* keep it intact */
            }
            LuaAPI.lua_getfield ( thread, LuaAPI.LUA_GLOBALSINDEX, "debug" );
            if (  ! LuaAPI.lua_istable ( thread, -1 ) ) {
                LuaAPI.lua_pop ( thread, 1 );
                return 1;
            }
            LuaAPI.lua_getfield ( thread, -1, "traceback" );
            if (  ! LuaAPI.lua_isfunction ( thread, -1 ) ) {
                LuaAPI.lua_pop ( thread, 2 );
                return 1;
            }
            LuaAPI.lua_pushvalue ( thread, 1 );  /* pass error message */
            LuaAPI.lua_pushinteger ( thread, 2 );  /* skip this function and traceback */
            LuaAPI.lua_call ( thread, 2, 1 );  /* call debug.traceback */
            return 1;
        }
    }
}

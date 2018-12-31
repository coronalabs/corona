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

import java.io.ByteArrayInputStream;

/**
 *
 * @author a.fornwald
 */
class LuaBaseLib {

    public static final String LUA_COLIBNAME = "coroutine";
    public static final int CO_RUN = 0; /* running */

    public static final int CO_SUS = 1; /* suspended */

    public static final int CO_NOR = 2; /* 'normal' (it resumed another coroutine) */

    public static final int CO_DEAD = 3;
    public static final String statnames[] = { "running", "suspended", "normal", "dead" };
    /* thread status; 0 is OK */
    public static final int LUA_YIELD = 1;
    public static final int LUA_ERRRUN = 2;
    public static final int LUA_ERRSYNTAX = 3;
    public static final int LUA_ERRMEM = 4;
    public static final int LUA_ERRERR = 5;

    public static final class luaB_setmetatable implements JavaFunction {

        static int a = 0;

        public int Call ( lua_State thread ) {
            int t = LuaAPI.lua_type ( thread, 2 );
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.luaL_argcheck ( thread, t == LuaAPI.LUA_TNIL || t == LuaAPI.LUA_TTABLE, 2, "nil or table expected" );

            if ( LuaAPI.luaL_getmetafield ( thread, 1, "__metatable" ) ) {
                LuaAPI.luaL_error ( thread, "cannot change a protected metatable" );
            }
            LuaAPI.lua_settop ( thread, 2 );
            LuaAPI.lua_setmetatable ( thread, 1 );
            return 1;
        }
    }

    public static final class luaB_assert implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 1 );
            if (  ! LuaAPI.lua_toboolean ( thread, 1 ) ) {
                return LuaAPI.luaL_error ( thread, LuaAPI.luaL_optstring ( thread, 2, "assertion failed!" ) );
            }
            return LuaAPI.lua_gettop ( thread );
        }
    }

    public static final class luaB_collectgarbage implements JavaFunction {

        public int Call ( lua_State thread ) {
            String[] opts = {
                "stop",
                "restart",
                "collect",
                "count",
                "step",
                "setpause",
                "setstepmul",
            };
            int[] optsnum = {
                LuaAPI.LUA_GCSTOP,
                LuaAPI.LUA_GCRESTART,
                LuaAPI.LUA_GCCOLLECT,
                LuaAPI.LUA_GCCOUNT,
                LuaAPI.LUA_GCSTEP,
                LuaAPI.LUA_GCSETPAUSE,
                LuaAPI.LUA_GCSETSTEPMUL
            };
            int o = LuaAPI.luaL_checkoption ( thread, 1, "collect", opts );
            int ex = LuaAPI.luaL_optint ( thread, 2, 0 );
            int res = LuaAPI.lua_gc ( thread, optsnum[o], ex );
            switch ( optsnum[o] ) {
                case LuaAPI.LUA_GCCOUNT: {
                    int b = LuaAPI.lua_gc ( thread, LuaAPI.LUA_GCCOUNTB, 0 );
                    LuaAPI.lua_pushnumber ( thread, res + b / 1024 );
                    return 1;
                }
                case LuaAPI.LUA_GCSTEP: {
                    LuaAPI.lua_pushboolean ( thread, res != 0 );
                    return 1;
                }
                default: {
                    LuaAPI.lua_pushnumber ( thread, res );
                    return 1;
                }
            }
        }
    }

    public static final class luaB_dofile implements JavaFunction {

        public int Call ( lua_State thread ) {
            String fname = LuaAPI.luaL_optstring ( thread, 1, null );
            int n = LuaAPI.lua_gettop ( thread );
            if ( LuaAPI.luaL_loadfile ( thread, fname ) != 0 ) {
                LuaAPI.lua_error ( thread );
            }
            LuaAPI.lua_call ( thread, 0, LuaAPI.LUA_MULTRET );
            return LuaAPI.lua_gettop ( thread ) - n;
        }
    }

    public static final class luaB_error implements JavaFunction {

        public int Call ( lua_State thread ) {
            int level = LuaAPI.luaL_optinteger ( thread, 2, 1 );
            LuaAPI.lua_settop ( thread, 1 );
            if ( LuaAPI.lua_isstring ( thread, 1 ) && level > 0 ) // add extra information?
            {
                LuaAPI.luaL_where ( thread, level );
                LuaAPI.lua_pushvalue ( thread, 1 );
                LuaAPI.lua_concat ( thread, 2 );
            }
            return LuaAPI.lua_error ( thread );
        }
    }

    public static final class luaB_type implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 1 );
            LuaAPI.lua_pushstring ( thread, LuaAPI.luaL_typename ( thread, 1 ) );
            return 1;
        }
    }

    public static final class luaB_print implements JavaFunction {

        int a = 0;

        public int Call ( lua_State thread ) {
            int n = LuaAPI.lua_gettop ( thread );  // number of arguments

            int i;
            LuaAPI.lua_getglobal ( thread, "tostring" );
            for ( i = 1; i <= n; i ++ ) {
                String s;
                LuaAPI.lua_pushvalue ( thread, -1 );  /* function to be called */
                LuaAPI.lua_pushvalue ( thread, i );   /* value to print */
                LuaAPI.lua_call ( thread, 1, 1 );
                s = LuaAPI.lua_tostring ( thread, -1 );  /* get result */
                if ( s == null ) {
                    return LuaAPI.luaL_error ( thread, "\"tostring\" must return a string to \"print\"" );
                }
                if ( i > 1 ) {
                    System.out.print ( "\t" );
                }

                System.out.print ( s );
                LuaAPI.lua_pop ( thread, 1 );  /* pop result */
            }
            System.out.println ();
            return 0;
        }
    }

    public static final class luaB_tonumber implements JavaFunction {

        private static boolean errno;
        private static String endptr = null;

        private static long strtoul ( String nptr, int base ) {
            endptr = "s";
            errno = false;

            String s = nptr;
            long acc;
            char c;
            long cutoff;
            boolean neg = false;
            int any;
            int cutlim;

            s = s.trim ();

            if ( s.length () == 0 ) {
                endptr = nptr;
                errno = true;
                return 0;
            }
            c = s.charAt ( 0 );
            s = s.substring ( 1 );

            if ( c == '-' ) {
                neg = true;
                s = s.substring ( 1 );
            }
            else if ( c == '+' ) {
                s = s.substring ( 1 );
            }
            if ( ( base == 0 || base == 16 ) && c == 0 && ( s.charAt ( 0 ) == 'x' || s.charAt ( 0 ) == 'X' ) ) {
                c = s.charAt ( 1 );
                s = s.substring ( 2 );
                base = 16;
            }
            if ( base == 0 ) {
                base = c == '0' ? 8 : 10;
            }

            cutoff = ( long ) Long.MAX_VALUE / ( long ) base;
            cutlim = ( int ) ( ( long ) Long.MAX_VALUE % ( long ) base );
            for ( acc = 0  , any = 0;;) {
                if ( '0' <= c && c <= '9' )//if (isdigit(c))
                {
                    c -= '0';
                }
                else if ( 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' )//else if (isalpha(c))
                {
                    c -= 'A' <= c && c <= 'Z' ? 'A' - 10 : 'a' - 10;
                }
                else {
                    break;
                }

                if ( ( int ) c >= base ) {
                    break;
                }

                if ( any < 0 || acc > cutoff || ( acc == cutoff && ( int ) c > cutlim ) ) {
                    any = -1;
                }
                else {
                    any = 1;
                    acc *= base;
                    acc += c;
                }

                //c = *s++
                if ( s.length () == 0 ) {
                    break;
                }
                c = s.charAt ( 0 );
                s = s.substring ( 1 );
            }
            if ( any < 0 ) {
                acc = Long.MAX_VALUE;
                errno = true;
            }
            else if ( neg == true ) {
                acc =  - acc;
            }
            if ( endptr != null ) {
                if ( any != 0 ) {
                    endptr = c + s;
                }
                else {
                    endptr = nptr;
                }
            //*endptr = (char *) (any ? s - 1 : nptr);
            }
            return acc;
        }

        public int Call ( lua_State thread ) {
            int base = LuaAPI.luaL_optinteger ( thread, 2, 10 );
            if ( base == 10 ) {
                LuaAPI.luaL_checkany ( thread, 1 );
                if ( LuaAPI.lua_isnumber ( thread, 1 ) ) {
                    LuaAPI.lua_pushnumber ( thread, LuaAPI.lua_tonumber ( thread, 1 ) );
                    return 1;
                }
            }
            else {
                String s1 = LuaAPI.luaL_checkstring ( thread, 1 );
                LuaAPI.luaL_argcheck ( thread, 2 <= base && base <= 36, 2, "base out of range" );
                long n = strtoul ( s1, base );
                if ( s1.equals ( endptr ) == false ) {  // at least one valid digit?

                    LuaAPI.lua_pushnumber ( thread, n );
                    return 1;
                }
            }

            LuaAPI.lua_pushnil ( thread );
            return 1;
        }
    }

    public static final class luaB_tostring implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 1 );

            if ( LuaAPI.luaL_callmeta ( thread, 1, "__tostring" ) ) {
                return 1;
            }
            switch ( LuaAPI.lua_type ( thread, 1 ) ) {
                case LuaAPI.LUA_TNUMBER: {
                    LuaAPI.lua_pushstring ( thread, LuaAPI.lua_tostring ( thread, 1 ) );
                    break;
                }
                case LuaAPI.LUA_TSTRING: {
                    LuaAPI.lua_pushvalue ( thread, 1 );
                    break;
                }
                case LuaAPI.LUA_TBOOLEAN: {
                    LuaAPI.lua_pushstring ( thread, ( LuaAPI.lua_toboolean ( thread, 1 ) ? "true" : "false" ) );
                    break;
                }
                case LuaAPI.LUA_TNIL: {
                    LuaAPI.lua_pushstring ( thread, "nil" );
                    break;
                }
                default: {
                    final String toString = LuaAPI.luaL_typename ( thread, 1 ) + ": " + LuaAPI.lua_topointer ( thread, 1 ).hashCode ();
                    LuaAPI.lua_pushstring ( thread, toString );
                    break;
                }
            }
            return 1;
        }
    }

    public static final class luaB_getfenv implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_Debug mutableValue = getfunc ( thread, 1 );

            if ( LuaAPI.lua_isjavafunction ( thread, -1 ) &&
                mutableValue.m_iCallInfoIndex != 0 &&
                thread.GetCallInfosStack ()[mutableValue.m_iCallInfoIndex].GetTailCalls () == 0 )/* is a C function? */ {
                LuaAPI.lua_pushvalue ( thread, LuaAPI.LUA_GLOBALSINDEX );  /* return the thread's global env. */
            }
            else {
                LuaAPI.lua_getfenv ( thread, -1 );
            }
            return 1;
        }
    }

    public static final class luaB_getmetatable implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 1 );
            if (  ! LuaAPI.lua_getmetatable ( thread, 1 ) ) {
                LuaAPI.lua_pushnil ( thread );
                return 1;  /* no metatable */
            }
            LuaAPI.luaL_getmetafield ( thread, 1, "__metatable" );
            return 1;  /* returns either __metatable field (if present) or metatable */
        }
    }

    public static final class luaB_loadfile implements JavaFunction {

        public int Call ( lua_State thread ) {
            String fname = LuaAPI.luaL_optstring ( thread, 1, null );
            if ( LuaAPI.luaL_loadfile ( thread, fname ) == 0 ) {
                return 1;
            }
            else {
                LuaAPI.lua_pushnil ( thread );
                LuaAPI.lua_insert ( thread, -2 );  /* put before error message */
                return 2;  /* return nil plus error message */
            }
        }
    }

    public static final class luaB_next implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.lua_settop ( thread, 2 );  /* create a 2nd argument if there isn't one */
            if ( LuaAPI.lua_next ( thread, 1 ) ) {
                return 2;
            }
            else {
                LuaAPI.lua_pushnil ( thread );
                return 1;
            }
        }
    }

    public static final class luaB_pcall implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 1 );
            final int status = LuaAPI.lua_pcall ( thread, LuaAPI.lua_gettop ( thread ) - 1, LuaAPI.LUA_MULTRET, 0 );
            LuaAPI.lua_pushboolean ( thread, ( status == 0 ) );
            LuaAPI.lua_insert ( thread, 1 );
            return LuaAPI.lua_gettop ( thread );  /* return status + all results */
        }
    }

    public static final class luaB_select implements JavaFunction {

        public int Call ( lua_State thread ) {
            int n = LuaAPI.lua_gettop ( thread );
            if ( LuaAPI.lua_type ( thread, 1 ) == LuaAPI.LUA_TSTRING &&
                LuaAPI.lua_tostring ( thread, 1 ).charAt ( 0 ) == '#' ) {
                LuaAPI.lua_pushinteger ( thread, n - 1 );
                return 1;
            }
            else {
                int i = LuaAPI.luaL_checkint ( thread, 1 );
                if ( i < 0 ) {
                    i = n + i;
                }
                else if ( i > n ) {
                    i = n;
                }

                LuaAPI.luaL_argcheck ( thread, 1 <= i, 1, "index out of range" );

                return n - i;
            }
        }
    }

    private static lua_Debug getfunc ( lua_State thread, int opt ) {
        if ( LuaAPI.lua_isfunction ( thread, 1 ) ) {
            LuaAPI.lua_pushvalue ( thread, 1 );
        }
        else {
            lua_Debug ar = new lua_Debug ();
            int level = ( opt != 0 ) ? LuaAPI.luaL_optint ( thread, 1, 1 ) : LuaAPI.luaL_checkint ( thread, 1 );
            LuaAPI.luaL_argcheck ( thread, level >= 0, 1, "level must be non-negative" );
            if ( LuaAPI.lua_getstack ( thread, level, ar ) == 0 ) {
                LuaAPI.luaL_argerror ( thread, 1, "invalid level" );
            }
            LuaAPI.lua_getinfo ( thread, "f", ar );
            if ( LuaAPI.lua_isnil ( thread, -1 ) ) {
                LuaAPI.luaL_error ( thread, "no function environment for tail call at level " + level );
            }

            return ar;
        }
        return null;
    }

    public static final class luaB_setfenv implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 2, LuaAPI.LUA_TTABLE );
            getfunc ( thread, 0 );
            LuaAPI.lua_pushvalue ( thread, 2 );
            if ( LuaAPI.lua_isnumber ( thread, 1 ) && LuaAPI.lua_tonumber ( thread, 1 ) == 0 ) {
                /* change environment of current thread */
                LuaAPI.lua_pushthread ( thread );
                LuaAPI.lua_insert ( thread, -2 );
                LuaAPI.lua_setfenv ( thread, -2 );
                return 0;
            }
            else if ( LuaAPI.lua_isjavafunction ( thread, -2 ) || LuaAPI.lua_setfenv ( thread, -2 ) == false ) {
                LuaAPI.luaL_error ( thread, "\"setfenv\" cannot change environment of given object" );
                return 1;
            }

            return 1;
        }
    }

    public static final class luaB_xpcall implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 2 );
            LuaAPI.lua_settop ( thread, 2 );
            LuaAPI.lua_insert ( thread, 1 );  /* put error function under function to be called */
            final int status = LuaAPI.lua_pcall ( thread, 0, LuaAPI.LUA_MULTRET, 1 );
            LuaAPI.lua_pushboolean ( thread, ( status == 0 ) );
            LuaAPI.lua_replace ( thread, 1 );
            return LuaAPI.lua_gettop ( thread );  /* return status + all results */
        }
    }

    public static final class luaB_rawget implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.luaL_checkany ( thread, 2 );
            LuaAPI.lua_settop ( thread, 2 );
            LuaAPI.lua_rawget ( thread, 1 );
            return 1;
        }
    }

    public static final class luaB_rawset implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.luaL_checkany ( thread, 2 );
            LuaAPI.luaL_checkany ( thread, 3 );
            LuaAPI.lua_settop ( thread, 3 );
            LuaAPI.lua_rawset ( thread, 1 );
            return 1;
        }
    }

    public static final class luaB_rawequal implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkany ( thread, 1 );
            LuaAPI.luaL_checkany ( thread, 2 );
            LuaAPI.lua_pushboolean ( thread, LuaAPI.lua_rawequal ( thread, 1, 2 ) );
            return 1;
        }
    }

    public static final class luaB_unpack implements JavaFunction {

        public int Call ( lua_State thread ) {
            int i, e, n;
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            i = LuaAPI.luaL_optint ( thread, 2, 1 );
            e = LuaAPI.lua_isnoneornil ( thread, 3 ) ? LuaAPI.luaL_getn ( thread, 1 ) : LuaAPI.luaL_checkint ( thread, 3 );

            n = e - i + 1;  /* number of elements */
            if ( n <= 0 ) {
                return 0;  /* empty range */
            }

            LuaAPI.luaL_checkstack ( thread, n, "table too big to unpack" );
            for (; i <= e; i ++ ) /* push arg[i...e] */ {
                LuaAPI.lua_rawgeti ( thread, 1, i );
            }
            return n;
        }
    }

    public static final class luaB_newproxy implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_settop ( thread, 1 );
            LuaAPI.lua_newuserdata ( thread, null, 0 );  // FIXME: 0 == null |-> create proxy

            if ( LuaAPI.lua_toboolean ( thread, 1 ) == false ) {
                return 1;  // no metatable

            }
            else if ( LuaAPI.lua_isboolean ( thread, 1 ) ) {
                LuaAPI.lua_newtable ( thread );  // create a new metatable `m' ...

                LuaAPI.lua_pushvalue ( thread, -1 );  // ... and mark `m' as a valid metatable

                LuaAPI.lua_pushboolean ( thread, true );
                LuaAPI.lua_rawset ( thread, LuaAPI.lua_upvalueindex ( 1 ) );  // weaktable[m] = true

            }
            else {
                boolean validproxy = false;  // to check if weaktable[metatable(u)] == true

                if ( LuaAPI.lua_getmetatable ( thread, 1 ) ) {
                    LuaAPI.lua_rawget ( thread, LuaAPI.lua_upvalueindex ( 1 ) );
                    validproxy = LuaAPI.lua_toboolean ( thread, -1 );
                    LuaAPI.lua_pop ( thread, 1 );  // remove value

                }
                LuaAPI.luaL_argcheck ( thread, validproxy, 1, "boolean or proxy expected" );
                LuaAPI.lua_getmetatable ( thread, 1 );  // metatable is valid; get it

            }
            LuaAPI.lua_setmetatable ( thread, 2 );
            return 1;
        }
    }

    private static void auxopen ( lua_State thread, String name, JavaFunction f, JavaFunction u ) {
        LuaAPI.lua_pushjavafunction ( thread, u );
        LuaAPI.lua_pushjavafunction ( thread, f, 1 );
        LuaAPI.lua_setfield ( thread, -2, name );
    }

    public static final class luaB_ipairs implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.lua_pushvalue ( thread, LuaAPI.lua_upvalueindex ( 1 ) );  /* return generator, */
            LuaAPI.lua_pushvalue ( thread, 1 );  /* state, */
            LuaAPI.lua_pushinteger ( thread, 0 );  /* and initial value */
            return 3;
        }
    }

    public static final class ipairsaux implements JavaFunction {

        public int Call ( lua_State thread ) {
            int i = LuaAPI.luaL_checkint ( thread, 2 );
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            i ++;  /* next value */
            LuaAPI.lua_pushinteger ( thread, i );
            LuaAPI.lua_rawgeti ( thread, 1, i );
            return ( LuaAPI.lua_isnil ( thread, -1 ) ) ? 0 : 2;
        }
    }

    public static final class luaB_pairs implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.lua_pushvalue ( thread, LuaAPI.lua_upvalueindex ( 1 ) );  /* return generator, */
            LuaAPI.lua_pushvalue ( thread, 1 );  /* state, */
            LuaAPI.lua_pushnil ( thread );  /* and initial value */
            return 3;
        }
    }

    private static int load_aux ( lua_State thread, int status ) {
        if ( status == 0 ) /* OK? */ {
            return 1;
        }
        else {
            LuaAPI.lua_pushnil ( thread );
            LuaAPI.lua_insert ( thread, -2 );  /* put before error message */
            return 2;  /* return nil plus error message */
        }
    }

    public static final class luaB_load implements JavaFunction {

        private static String generic_reader ( lua_State thread ) {
            LuaAPI.luaL_checkstack ( thread, 2, "too many nested functions" );
            LuaAPI.lua_pushvalue ( thread, 1 );  /* get function */
            LuaAPI.lua_call ( thread, 0, 1 );  /* call it */
            if ( LuaAPI.lua_isnil ( thread, -1 ) ) {
                return null;
            }
            else if ( LuaAPI.lua_isstring ( thread, -1 ) ) {
                LuaAPI.lua_replace ( thread, 3 );  /* save string in a reserved stack slot */
                return LuaAPI.lua_tolstring ( thread, 3 );
            }
            else {
                LuaAPI.luaL_error ( thread, "reader function must return a string" );
            }
            return null;  /* to avoid warnings */
        }

        public int Call ( lua_State thread ) {
            int status;
            String cname = LuaAPI.luaL_optstring ( thread, 2, "=(load)" );
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TFUNCTION );
            LuaAPI.lua_settop ( thread, 3 );  /* function, eventual name, plus one reserved slot */

            String strChunk = generic_reader ( thread );

            ByteArrayInputStream bais = new ByteArrayInputStream ( strChunk.getBytes () );
            status = LuaAPI.lua_load ( thread, bais, null, cname );
            return load_aux ( thread, status );
        }
    }

    public static final class luaB_loadstring implements JavaFunction {

        public int Call ( lua_State thread ) {
            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            int iStrLen = LuaStringLib.StringLength ( s );
            String chunkname = LuaAPI.luaL_optstring ( thread, 2, s );
            return load_aux ( thread, LuaAPI.luaL_loadbuffer ( thread, s, chunkname ) );
        }
    }

    public static final class luaB_gcinfo implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushinteger ( thread, LuaAPI.lua_gc ( thread, LuaAPI.LUA_GCCOUNT, 0 ) );
            return 1;
        }
    }

    public static void base_open ( lua_State thread ) {
        luaL_Reg[] luaReg = new luaL_Reg[] {
            new luaL_Reg ( "assert", new luaB_assert () ),
            new luaL_Reg ( "collectgarbage", new luaB_collectgarbage () ),
            new luaL_Reg ( "dofile", new luaB_dofile () ),
            new luaL_Reg ( "error", new luaB_error () ),
            new luaL_Reg ( "gcinfo", new luaB_gcinfo () ), // Deprecated. User collectgarbage instead.
            new luaL_Reg ( "getfenv", new luaB_getfenv () ),
            new luaL_Reg ( "getmetatable", new luaB_getmetatable () ),
            new luaL_Reg ( "loadfile", new luaB_loadfile () ),
            new luaL_Reg ( "load", new luaB_load () ),
            new luaL_Reg ( "loadstring", new luaB_loadstring () ),
            new luaL_Reg ( "next", new luaB_next () ),
            new luaL_Reg ( "pcall", new luaB_pcall () ),
            new luaL_Reg ( "print", new luaB_print () ),
            new luaL_Reg ( "rawequal", new luaB_rawequal () ),
            new luaL_Reg ( "rawget", new luaB_rawget () ),
            new luaL_Reg ( "rawset", new luaB_rawset () ),
            new luaL_Reg ( "select", new luaB_select () ),
            new luaL_Reg ( "setfenv", new luaB_setfenv () ),
            new luaL_Reg ( "setmetatable", new luaB_setmetatable () ),
            new luaL_Reg ( "tonumber", new luaB_tonumber () ),
            new luaL_Reg ( "tostring", new luaB_tostring () ),
            new luaL_Reg ( "type", new luaB_type () ),
            new luaL_Reg ( "unpack", new luaB_unpack () ),
            new luaL_Reg ( "xpcall", new luaB_xpcall () ),
        };

        /* set global _G */
        LuaAPI.lua_pushvalue ( thread, LuaAPI.LUA_GLOBALSINDEX );
        LuaAPI.lua_setglobal ( thread, "_G" );
        /* open lib into global table */
        LuaAPI.luaL_register ( thread, "_G", luaReg );
        LuaAPI.lua_pushliteral ( thread, LuaAPI.LUA_VERSION );
        LuaAPI.lua_setglobal ( thread, "_VERSION" );  /* set global _VERSION */
        /* `ipairs' and `pairs' need auxliliary functions as upvalues */
        auxopen ( thread, "ipairs", new luaB_ipairs (), new ipairsaux () );
        auxopen ( thread, "pairs", new luaB_pairs (), new luaB_next () );
        // `newproxy' needs a weaktable as upvalue

        LuaAPI.lua_createtable ( thread, 0, 1 );  // new table `w'

        LuaAPI.lua_pushvalue ( thread, -1 );  // `w' will be its own metatable

        LuaAPI.lua_setmetatable ( thread, -2 );
        LuaAPI.lua_pushliteral ( thread, "kv" );
        LuaAPI.lua_setfield ( thread, -2, "__mode" );  // metatable(w).__mode = "kv"

        LuaAPI.lua_pushjavafunction ( thread, new luaB_newproxy (), 1 );
        LuaAPI.lua_setglobal ( thread, "newproxy" );  //set global `newproxy'

    }

    public static final class luaB_auxwrap implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_State co = LuaAPI.lua_tothread ( thread, LuaAPI.lua_upvalueindex ( 1 ) );
            int r = auxresume ( thread, co, LuaAPI.lua_gettop ( thread ) );
            if ( r < 0 ) {
                if ( LuaAPI.lua_isstring ( thread, -1 ) ) /* error object is a string? */ {
                    LuaAPI.luaL_where ( thread, 1 );  /* add extra info */
                    LuaAPI.lua_insert ( thread, -2 );
                    LuaAPI.lua_concat ( thread, 2 );
                }
                LuaAPI.lua_error ( thread );  /* propagate error */
            }
            return r;
        }
    }

    public static final class luaB_cocreate implements JavaFunction {

        public int Call ( lua_State thread ) {
            Object func = thread.GetObjectValue ( 1 );
            if (  ! ( func instanceof Function ) ) {
                return -1;
            }
            Function function = ( Function ) func;
            if ( function.IsJavaFunction () ) {
                return -1;
            }
            lua_State newThread = LuaAPI.lua_newthread ( thread );
            LuaAPI.lua_pushvalue ( thread, 1 );
            LuaAPI.lua_xmove ( thread, newThread, 1 );
            return 1;
        }
    }

    static int costatus ( lua_State thread, lua_State co ) {
        if ( thread == co )
            return CO_RUN;
        switch ( LuaAPI.lua_status ( co ) ) {
            case LUA_YIELD:
                return CO_SUS;
            case 0: {

                /*if (LuaAPI.lua_getstack(co, 0, &ar) > 0)  /* does it have frames? */
                /*return CO_NOR;  /* it is running */
                /*else */                if ( LuaAPI.lua_gettop ( co ) == 0 )
                    return CO_DEAD;
                else
                    return CO_SUS;  /* initial state */
            }
            default:  /* some error occured */
                return CO_DEAD;
        }
    }

    public static int auxresume ( lua_State thread, lua_State co, int narg ) {
        int status = costatus ( thread, co );
        if (  ! LuaAPI.lua_checkstack ( co, narg ) ) {
            LuaAPI.luaL_error ( thread, "too many arguments to resume" );
        }
        if ( status != CO_SUS ) {
            LuaAPI.lua_pushfstring ( thread, "cannot resume " + statnames[status] + " coroutine" );
            return -1;  /* error flag */
        }
        LuaAPI.lua_xmove ( thread, co, narg );
        co.setNCCalls ( thread.getNCCalls () );
        status = LuaAPI.lua_resume ( co, narg );
        if ( status == 0 || status == LUA_YIELD ) {
            int nres = LuaAPI.lua_gettop ( co );
            if (  ! LuaAPI.lua_checkstack ( thread, nres ) )
                LuaAPI.luaL_error ( thread, "too many results to resume" );
            LuaAPI.lua_xmove ( co, thread, nres );  /* move yielded values */
            return nres;
        }
        else {
            LuaAPI.lua_xmove ( co, thread, 1 );  /* move error message */
            return -1;  /* error flag */
        }
    }

    public static final class luaB_coresume implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_State co = LuaAPI.lua_tothread ( thread, 1 );
            int r;
            LuaAPI.luaL_argcheck ( thread, ( co instanceof lua_State ), 1, "coroutine expected" );
            r = auxresume ( thread, co, LuaAPI.lua_gettop ( thread ) - 1 );
            if ( r < 0 ) {
                LuaAPI.lua_pushboolean ( thread, false );
                LuaAPI.lua_insert ( thread, -2 );
                return 2;  /* return false + error message */
            }
            else {
                LuaAPI.lua_pushboolean ( thread, true );
                LuaAPI.lua_insert ( thread,  - ( r + 1 ) );
                return r + 1;  /* return true + `resume' returns */
            }

        /*
        lua_State *co = lua_tothread(L, 1);
        int r;
        luaL_argcheck(L, co, 1, "coroutine expected");
        r = auxresume(L, co, lua_gettop(L) - 1);
        if (r < 0) {
        lua_pushboolean(L, 0);
        lua_insert(L, -2);
        return 2;  /* return false + error message
        }
        else {
        lua_pushboolean(L, 1);
        lua_insert(L, -(r + 1));
        return r + 1;  /* return true + `resume' returns
        }
         */
        }
    }

    public static final class luaB_corunning implements JavaFunction {

        public int Call ( lua_State thread ) {
            if ( LuaAPI.lua_pushthread ( thread ) ) {
                LuaAPI.lua_pushnil ( thread );  /* main thread is not a coroutine */
            }
            return 1;
        }
    }

    public static final class luaB_costatus implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_State co = LuaAPI.lua_tothread ( thread, 1 );
            LuaAPI.luaL_argcheck ( thread, ( co instanceof lua_State ), 1, "coroutine expected" );
            LuaAPI.lua_pushstring ( thread, statnames[costatus ( thread, co )] );
            return 1;
        }
    }

    public static final class luaB_cowrap implements JavaFunction {

        public int Call ( lua_State thread ) {
            new luaB_cocreate ().Call ( thread );
            LuaAPI.lua_pushjavafunction ( thread, new luaB_auxwrap (), 1 );
            return 1;
        }
    }

    public static final class luaB_yield implements JavaFunction {

        public int Call ( lua_State thread ) {
            return LuaAPI.lua_yield ( thread, LuaAPI.lua_gettop ( thread ) );
        }
    }

    public static final class luaopen_base implements JavaFunction {

        public int Call ( lua_State thread ) {
            luaL_Reg[] luaReg = new luaL_Reg[] {
                new luaL_Reg ( "create", new luaB_cocreate () ),
                new luaL_Reg ( "resume", new luaB_coresume () ),
                new luaL_Reg ( "running", new luaB_corunning () ),
                new luaL_Reg ( "status", new luaB_costatus () ),
                new luaL_Reg ( "wrap", new luaB_cowrap () ),
                new luaL_Reg ( "yield", new luaB_yield () ),
            };

            base_open ( thread );
            LuaAPI.luaL_register ( thread, LUA_COLIBNAME, luaReg );
            return 2;
        }
    }

    public static Double ConvertToDouble ( Object value ) {
        try {
            if ( value instanceof Double ) {
                return ( Double ) value;
            }
            else if ( value instanceof String ) {
                return new Double ( Double.parseDouble ( ( String ) value ) );
            }

            return null;
        }
        catch ( RuntimeException ex ) {
            return null;
        }
    }

    public static String ConvertToString ( Object value ) {
        if ( value instanceof String ) {
            return ( String ) value;
        }
        if ( value instanceof Double ) {
            Double doubleValue = ( ( Double ) value );
            if ( doubleValue.doubleValue () - doubleValue.intValue () == 0 ) {
                return String.valueOf ( doubleValue.intValue () );
            }
            else {
                return String.valueOf ( doubleValue.doubleValue () );
            }
        }
        return null;
    }
}

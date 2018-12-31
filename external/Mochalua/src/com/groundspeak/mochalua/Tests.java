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
 * @author p.pavelko
 */
class Tests {

    private static lua_State lua_state = null;
    private static int Trick = 0;

    /** Creates a new instance of Tests */
    public Tests () {
    }
    public static final String delimits = " \t\n,;";

    public static void skip ( StringBuffer pc ) {
        while ( pc.length () > 0 && pc.charAt ( 0 ) != '\0' && delimits.indexOf ( pc.charAt ( 0 ) ) != -1 ) {
            pc.deleteCharAt ( 0 );
        }
    }

    static int getnum_aux ( lua_State thread, StringBuffer pc ) {
        int res = 0;
        int sig = 1;
        skip ( pc );
        if ( pc.length () > 0 && pc.charAt ( 0 ) == '.' ) {
            res = ( int ) LuaAPI.lua_tonumber ( thread, -1 );
            LuaAPI.lua_pop ( thread, 1 );
            pc.deleteCharAt ( 0 );
            return res;
        }
        else if ( pc.length () > 0 && pc.charAt ( 0 ) == '-' ) {
            sig = -1;
            pc.deleteCharAt ( 0 );
        }

        while ( pc.length () > 0 && Character.isDigit ( pc.charAt ( 0 ) ) ) {
            char c = pc.charAt ( 0 );
            pc.deleteCharAt ( 0 );
            res = res * 10 + c - '0';
        }
        return sig * res;
    }

    public static String getname_aux ( StringBuffer buff, StringBuffer pc ) {
        int i = 0;
        skip ( pc );
        while ( pc.length () > 0 && pc.charAt ( 0 ) != '\0' && delimits.indexOf ( pc.charAt ( 0 ) ) == -1 ) {
            char c = pc.charAt ( 0 );
            pc.deleteCharAt ( 0 );
            buff.append ( c );
            i ++;
        }
        return buff.toString ();
    }

    public static int getindex_aux ( lua_State thread, StringBuffer pc ) {
        skip ( pc );
        char c = pc.length () > 0 ? pc.charAt ( 0 ) : 0;
        switch ( c ) {
            case 'R': {
                pc.deleteCharAt ( 0 );
                return LuaAPI.LUA_REGISTRYINDEX;
            }
            case 'G': {
                pc.deleteCharAt ( 0 );
                return LuaAPI.LUA_GLOBALSINDEX;
            }
            case 'E': {
                pc.deleteCharAt ( 0 );
                return LuaAPI.LUA_ENVIRONINDEX;
            }
            case 'U': {
                pc.deleteCharAt ( 0 );
                return LuaAPI.lua_upvalueindex ( getnum_aux ( thread, pc ) );
            }
            default: {
                return getnum_aux ( thread, pc );
            }
        }
    }

    public static int getnum ( lua_State thread, StringBuffer pc ) {
        return getnum_aux ( thread, pc );
    }

    public static String getname ( StringBuffer buff, StringBuffer pc ) {
        return getname_aux ( buff, pc );
    }

    public static int getindex ( lua_State thread, StringBuffer pc ) {
        return getindex_aux ( thread, pc );
    }

    public static boolean EQ ( String s1, String inst ) {
        if ( s1.equals ( inst ) )
            return true;
        else
            return false;
    }

    public static void setnameval ( lua_State thread, String name, int val ) {
        LuaAPI.lua_pushstring ( thread, name );
        LuaAPI.lua_pushinteger ( thread, val );
        LuaAPI.lua_settable ( thread, -3 );
    }

    public static final class testC implements JavaFunction {

        public int Call ( lua_State thread ) {
            StringBuffer buff = new StringBuffer ();
            lua_State thread_1;
            StringBuffer pc = new StringBuffer ();

            if ( LuaAPI.lua_isthread ( thread, 1 ) ) {
                thread_1 = LuaAPI.lua_tothread ( thread, 1 );
                pc.append ( LuaAPI.luaL_checkstring ( thread, 2 ) );
            }
            else {
                thread_1 = thread;
                pc.append ( LuaAPI.luaL_checkstring ( thread, 1 ) );
            }
            for (;;) {
                buff.delete ( 0, buff.length () );
                String inst = getname ( buff, pc );
                if ( EQ ( "", inst ) ) {
                    return 0;
                }
                else if ( EQ ( "isnumber", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_isnumber ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "isstring", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_isstring ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "istable", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_istable ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "iscfunction", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_isjavafunction ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "isfunction", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_isfunction ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "isuserdata", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_isuserdata ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "isudataval", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_islightuserdata ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "isnil", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_isnil ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "isnull", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_isnone ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "tonumber", inst ) ) {
                    LuaAPI.lua_pushnumber ( thread_1, LuaAPI.lua_tonumber ( thread_1, getindex ( thread, pc ) ) );
                }
                else if ( EQ ( "tostring", inst ) ) {
                    String s = LuaAPI.lua_tostring ( thread_1, getindex ( thread, pc ) );
                    LuaAPI.lua_pushstring ( thread_1, s );
                }
                else if ( EQ ( "objsize", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_objlen ( thread_1, getindex ( thread, pc ) ) );
                }
                else if ( EQ ( "tocfunction", inst ) ) {
                    LuaAPI.lua_pushjavafunction ( thread_1, LuaAPI.lua_tojavafunction ( thread_1, getindex ( thread, pc ) ) );
                }
                else if ( EQ ( "return", inst ) ) {
                    return getnum ( thread, pc );
                }
                else if ( EQ ( "gettop", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_gettop ( thread_1 ) );
                }
                else if ( EQ ( "settop", inst ) ) {
                    LuaAPI.lua_settop ( thread_1, getnum ( thread, pc ) );
                }
                else if ( EQ ( "pop", inst ) ) {
                    LuaAPI.lua_pop ( thread_1, getnum ( thread, pc ) );
                }
                else if ( EQ ( "pushnum", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, getnum ( thread, pc ) );
                }
                else if ( EQ ( "pushstring", inst ) ) {
                    LuaAPI.lua_pushstring ( thread_1, getname ( buff, pc ) );
                }
                else if ( EQ ( "pushnil", inst ) ) {
                    LuaAPI.lua_pushnil ( thread_1 );
                }
                else if ( EQ ( "pushbool", inst ) ) {
                    LuaAPI.lua_pushboolean ( thread_1, getnum ( thread, pc ) > 0 ? true : false );
                }
                else if ( EQ ( "newuserdata", inst ) ) {
                    LuaAPI.lua_newuserdata ( thread_1, null, getnum ( thread, pc ) );
                /*
                int size = getnum( thread, pc );
                StringBuffer sb = new StringBuffer();
                while (size-- > 0)
                {
                sb.append(' ');
                }
                int iSize = LuaAPI.luaL_checkint( thread, 1 );
                LuaAPI.lua_newuserdata(thread_1, sb.toString(), iSize );
                 * */
                }
                else if ( EQ ( "tobool", inst ) ) {
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.lua_toboolean ( thread_1, getindex ( thread, pc ) ) == true ? 1 : 0 );
                }
                else if ( EQ ( "pushvalue", inst ) ) {
                    LuaAPI.lua_pushvalue ( thread_1, getindex ( thread, pc ) );
                }
                else if ( EQ ( "pushcclosure", inst ) ) {
                    LuaAPI.lua_pushjavafunction ( thread_1, new testC (), getnum ( thread, pc ) );
                }
                else if ( EQ ( "remove", inst ) ) {
                    LuaAPI.lua_remove ( thread_1, getnum ( thread, pc ) );
                }
                else if ( EQ ( "insert", inst ) ) {
                    LuaAPI.lua_insert ( thread_1, getnum ( thread, pc ) );
                }
                else if ( EQ ( "replace", inst ) ) {
                    LuaAPI.lua_replace ( thread_1, getindex ( thread, pc ) );
                }
                else if ( EQ ( "gettable", inst ) ) {
                    LuaAPI.lua_gettable ( thread_1, getindex ( thread, pc ) );
                }
                else if ( EQ ( "settable", inst ) ) {
                    LuaAPI.lua_settable ( thread_1, getindex ( thread, pc ) );
                }
                else if ( EQ ( "next", inst ) ) {
                    LuaAPI.lua_next ( thread_1, -2 );
                }
                else if ( EQ ( "concat", inst ) ) {
                    LuaAPI.lua_concat ( thread_1, getnum ( thread, pc ) );
                }
                else if ( EQ ( "lessthan", inst ) ) {
                    int a = getindex ( thread, pc );
                    LuaAPI.lua_pushboolean ( thread_1, LuaAPI.lua_lessthan ( thread_1, a, getindex ( thread, pc ) ) );
                }
                else if ( EQ ( "equal", inst ) ) {
                    int a = getindex ( thread, pc );
                    LuaAPI.lua_pushboolean ( thread_1, LuaAPI.lua_equal ( thread_1, a, getindex ( thread, pc ) ) );
                }
                else if ( EQ ( "rawcall", inst ) ) {
                    int narg = getnum ( thread, pc );
                    int nres = getnum ( thread, pc );
                    LuaAPI.lua_call ( thread_1, narg, nres );
                }
                else if ( EQ ( "call", inst ) ) {
                    int narg = getnum ( thread, pc );
                    int nres = getnum ( thread, pc );
                    LuaAPI.lua_pcall ( thread_1, narg, nres, 0 );
                }
                else if ( EQ ( "loadstring", inst ) ) {
                    String s = LuaAPI.luaL_checklstring ( thread_1, getnum ( thread, pc ) );
                    LuaAPI.luaL_loadbuffer ( thread_1, s, s );
                }
                else if ( EQ ( "loadfile", inst ) ) {
                    LuaAPI.luaL_loadfile ( thread_1, LuaAPI.luaL_checkstring ( thread_1, getnum ( thread, pc ) ) );
                }
                else if ( EQ ( "setmetatable", inst ) ) {
                    LuaAPI.lua_setmetatable ( thread_1, getindex ( thread, pc ) );
                }
                else if ( EQ ( "getmetatable", inst ) ) {
                    if ( LuaAPI.lua_getmetatable ( thread_1, getindex ( thread, pc ) ) == false ) {
                        LuaAPI.lua_pushnil ( thread_1 );
                    }
                }
                else if ( EQ ( "type", inst ) ) {
                    LuaAPI.lua_pushstring ( thread_1, LuaAPI.luaL_typename ( thread_1, getnum ( thread, pc ) ) );
                }
                else if ( EQ ( "getn", inst ) ) {
                    int i = getindex ( thread, pc );
                    LuaAPI.lua_pushinteger ( thread_1, LuaAPI.luaL_getn ( thread_1, i ) );
                }
                else if ( LuaAPI.LUA_COMPAT_GETN && ( EQ ( "setn", inst ) ) ) {

                    int i = getindex ( thread, pc );
                    int n = ( int ) LuaAPI.lua_tonumber ( thread_1, -1 );
                    LuaAPI.luaL_setn ( thread_1, i, n );
                    LuaAPI.lua_pop ( thread_1, 1 );
                }
                else if ( EQ ( "throw", inst ) ) {
                    LuaAPI.luaL_error ( thread_1, "C++" );
                }
                else {
                    LuaAPI.luaL_error ( thread, "unknown instruction " + buff.toString () );
                }

                int k = 0;
                k ++;
            }
        }
    }

    public static final class coresume implements JavaFunction {

        public int Call ( lua_State thread ) {
            int status;
            lua_State co = LuaAPI.lua_tothread ( thread, 1 );
            LuaAPI.luaL_argcheck ( thread, co != null, 1, "coroutine expected" );
            status = LuaAPI.lua_resume ( co, 0 );
            if ( status != 0 ) {
                LuaAPI.lua_pushboolean ( thread, false );
                LuaAPI.lua_insert ( thread, -2 );
                return 2;  /* return false + error message */
            }
            else {
                LuaAPI.lua_pushboolean ( thread, true );
                return 1;
            }
        }
    }

    /*
     ** {======================================================
     ** tests auxlib functions
     ** =======================================================
     */
    public static final class auxgsub implements JavaFunction {

        public int Call ( lua_State thread ) {
            String s1 = LuaAPI.luaL_checkstring ( thread, 1 );
            String s2 = LuaAPI.luaL_checkstring ( thread, 2 );
            String s3 = LuaAPI.luaL_checkstring ( thread, 3 );
            LuaAPI.lua_settop ( thread, 3 );
            LuaAPI.luaL_gsub ( thread, s1, s2, s3 );
            //LuaAPI.lua_assert(LuaAPI.lua_gettop(thread) == 4);
            return 1;
        }
    }

    public static final class tref implements JavaFunction {

        public int Call ( lua_State thread ) {
            int level = LuaAPI.lua_gettop ( thread );
            int lock = LuaAPI.luaL_optint ( thread, 2, 1 );
            LuaAPI.luaL_checkany ( thread, 1 );
            LuaAPI.lua_pushvalue ( thread, 1 );
            LuaAPI.lua_pushinteger ( thread, LuaAPI.lua_ref ( thread, lock ) );
            //lua_assert(LuaAPI.lua_gettop(thread) == level+1);  /* +1 for result */
            return 1;
        }
    }

    public static final class getref implements JavaFunction {

        public int Call ( lua_State thread ) {
            int level = LuaAPI.lua_gettop ( thread );
            LuaAPI.lua_getref ( thread, LuaAPI.luaL_checkint ( thread, 1 ) );
            //lua_assert(LuaAPI.lua_gettop(thread) == level+1);
            return 1;
        }
    }

    public static final class unref implements JavaFunction {

        public int Call ( lua_State thread ) {
            int level = LuaAPI.lua_gettop ( thread );
            LuaAPI.lua_unref ( thread, LuaAPI.luaL_checkint ( thread, 1 ) );
            //lua_assert(lua_gettop(thread) == level);
            return 0;
        }
    }

    public static final class upvalue implements JavaFunction {

        public int Call ( lua_State thread ) {
            int n = LuaAPI.luaL_checkint ( thread, 2 );
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TFUNCTION );
            if ( LuaAPI.lua_isnoneornil ( thread, 3 ) ) {
                String name = LuaAPI.lua_getupvalue ( thread, 1, n );
                if ( name == null ) {
                    return 0;
                }
                LuaAPI.lua_pushstring ( thread, name );
                return 2;
            }
            else {
                String name = LuaAPI.lua_setupvalue ( thread, 1, n );
                LuaAPI.lua_pushstring ( thread, name );
                return 1;
            }
        }
    }

    public static final class pushuserdata implements JavaFunction {

        public int Call ( lua_State thread ) {
            //LuaAPI.lua_pushlightuserdata(thread, LuaAPI.lua_touserdata(thread, 1));
            LuaAPI.lua_pushlightuserdata ( thread, new Double ( LuaAPI.luaL_checkinteger ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class lua_checkmemory implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return 0;
        }
    }

    public static final class closestate implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_State thread_1 = LuaAPI.lua_tothread ( thread, 1 );
            LuaAPI.lua_close ( thread_1 );
            return 0;
        }
    }

    public static final class d2s implements JavaFunction {

        public int Call ( lua_State thread ) {
            double d = LuaAPI.luaL_checknumber ( thread, 1 );
            LuaAPI.lua_pushnumber ( thread, d );
            String str = LuaAPI.lua_tostring ( thread, LuaAPI.lua_gettop ( thread ) - 1 );
            LuaAPI.lua_pop ( thread, 1 );
            LuaAPI.lua_pushlstring ( thread, str, str != null ? str.length () : 0 );
            return 1;
        }
    }

    public static final class doonnewstack implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_State thread_1 = LuaAPI.lua_newthread ( thread );
            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            int status = LuaAPI.luaL_loadbuffer ( thread_1, s, s );
            if ( status == 0 )
                status = LuaAPI.lua_pcall ( thread_1, 0, 0, 0 );
            LuaAPI.lua_pushinteger ( thread, status );
            return 1;
        }
    }

    public static final class doremote implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_State thread_1 = LuaAPI.lua_tothread ( thread, 1 );
            String code = LuaAPI.luaL_checklstring ( thread, 2 );
            int status;
            LuaAPI.lua_settop ( thread_1, 0 );
            status = LuaAPI.luaL_loadbuffer ( thread_1, code, code );
            if ( status == 0 ) {
                status = LuaAPI.lua_pcall ( thread_1, 0, LuaAPI.LUA_MULTRET, 0 );
            }
            if ( status != 0 ) {
                LuaAPI.lua_pushnil ( thread );
                LuaAPI.lua_pushinteger ( thread, status );
                LuaAPI.lua_pushstring ( thread, LuaAPI.lua_tostring ( thread_1, -1 ) );
                return 3;
            }
            else {
                int i = 0;
                while (  ! LuaAPI.lua_isnone ( thread_1,  ++ i ) ) {
                    LuaAPI.lua_pushstring ( thread, LuaAPI.lua_tostring ( thread_1, i ) );
                }
                LuaAPI.lua_pop ( thread_1, i - 1 );
                return i - 1;
            }
        }
    }

    public static final class get_gccolor implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class gcstate implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class hash_query implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class int2fb_aux implements JavaFunction {

        public int Call ( lua_State thread ) {
            int b = LuaAPI.luaO_int2fb ( LuaAPI.luaL_checkint ( thread, 1 ) );
            LuaAPI.lua_pushinteger ( thread, b );
            LuaAPI.lua_pushinteger ( thread, LuaAPI.luaO_fb2int ( b ) );
            return 2;
        }
    }

    public static final class get_limits implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_createtable ( thread, 0, 5 );
            setnameval ( thread, "BITS_INT", LuaAPI.LUAI_BITSINT );
            setnameval ( thread, "LFPF", LuaAPI.LFIELDS_PER_FLUSH );
            setnameval ( thread, "MAXVARS", LuaAPI.LUAI_MAXVARS );
            setnameval ( thread, "MAXSTACK", LuaAPI.MAXSTACK );
            setnameval ( thread, "MAXUPVALUES", LuaAPI.LUAI_MAXUPVALUES );
            setnameval ( thread, "NUM_OPCODES", LuaAPI.NUM_OPCODES );
            return 1;
        }
    }

    public static final class listcode implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class listk implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class listlocals implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class loadlib implements JavaFunction {

        public int Call ( lua_State thread ) {
            luaL_Reg[] libs = new luaL_Reg[] {
                new luaL_Reg ( "baselibopen", new LuaBaseLib.luaopen_base () ),
                //new luaL_Reg( "dblibopen", new luaopen_debug() ),
                new luaL_Reg ( "iolibopen", new LuaIOLib.luaopen_io () ),
                new luaL_Reg ( "mathlibopen", new LuaMathLib.luaopen_math () ),
                new luaL_Reg ( "strlibopen", new LuaStringLib.luaopen_string () ),
                new luaL_Reg ( "tablibopen", new LuaTableLib.luaopen_table () ),
                new luaL_Reg ( "packageopen", new LuaPackageLib.luaopen_package () ),
            };

            lua_State thread_1 = LuaAPI.lua_tothread ( thread, 1 );
            LuaAPI.lua_pushvalue ( thread_1, LuaAPI.LUA_GLOBALSINDEX );
            LuaAPI.luaL_register ( thread_1, null, libs );
            return 0;
        }
    }

    public static final class log2_aux implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushinteger ( thread, LuaAPI.luaO_log2 ( LuaAPI.luaL_checkint ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class newstate implements JavaFunction {

        public int Call ( lua_State thread ) {
            lua_State thread_1 = LuaAPI.lua_newstate ();
            if ( thread_1 != null ) {
                LuaAPI.lua_pushlightuserdata ( thread, thread_1 );
            }
            else
                LuaAPI.lua_pushnil ( thread );
            return 1;
        }
    }

    public static final class newuserdata implements JavaFunction {

        public int Call ( lua_State thread ) {
            int size = LuaAPI.luaL_checkint ( thread, 1 );
            LuaAPI.lua_newuserdata ( thread, new Double ( size ), size );
            return 1;
        }
    }

    public static final class num2int implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushinteger ( thread, LuaAPI.lua_tointeger ( thread, 1 ) );
            return 1;
        }
    }

    public static final class string_query implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class table_query implements JavaFunction {

        private static int sizenode ( Table t ) {
            int iSize = 0;
            Object key = t.GetNext ( null );

            while ( key != null ) {
                if ( t.CanBeIndexToArray ( key ) == false ) {
                    iSize ++;
                }

                key = t.GetNext ( key );
            }
            return iSize;
        }

        public int Call ( lua_State thread ) {
            /*CallInfo currentCallInfo = thread.GetCurrentCallInfo();
            int i = LuaAPI.luaL_optint( thread, 2, -1 );
            LuaAPI.luaL_checktype( thread, 1, LuaAPI.LUA_TTABLE );
            
            Table table = ( Table )currentCallInfo.GetValue( 1 );
            
            if( i == -1 )
            {
            LuaAPI.lua_pushinteger( thread, table.GetArraySize() );
            LuaAPI.lua_pushinteger( thread, sizenode( table) );
            //FIXME: LuaAPI.lua_pushinteger( thread, i );
            }
            else if( i < table.GetArraySize() )
            {
            LuaAPI.lua_pushinteger( thread, i );
            // FIXME:
            LuaAPI.lua_pushlightuserdata( thread, table.GetValueNum( i - 1 ) );
            LuaAPI.lua_pushnil( thread );
            }
            else if( ( i -= table.GetArraySize() ) < sizenode( table ) )
            {
            if( !LuaAPI.IsNilOrNull(table))
            }*/
            /*
            const Table *t;
            int i = luaL_optint(L, 2, -1);
            luaL_checktype(L, 1, LUA_TTABLE);
            t = hvalue(obj_at(L, 1));
            if (i == -1) {
            lua_pushinteger(L, t->sizearray);
            lua_pushinteger(L, luaH_isdummy(t->node) ? 0 : sizenode(t));
            lua_pushinteger(L, t->lastfree - t->node);
            }
            else if (i < t->sizearray) {
            lua_pushinteger(L, i);
            luaA_pushobject(L, &t->array[i]);
            lua_pushnil(L); 
            }
            else if ((i -= t->sizearray) < sizenode(t)) {
            if (!ttisnil(gval(gnode(t, i))) ||
            ttisnil(gkey(gnode(t, i))) ||
            ttisnumber(gkey(gnode(t, i)))) {
            luaA_pushobject(L, key2tval(gnode(t, i)));
            }
            else
            lua_pushliteral(L, "<undef>");
            luaA_pushobject(L, gval(gnode(t, i)));
            if (gnext(&t->node[i]))
            lua_pushinteger(L, gnext(&t->node[i]) - t->node);
            else
            lua_pushnil(L);
            }
            return 3;
             */
            return 0;
        }
    }

    public static final class s2d implements JavaFunction {

        public int Call ( lua_State thread ) {
            try {
                LuaAPI.lua_pushnumber ( thread, Double.parseDouble ( LuaAPI.luaL_checkstring ( thread, 1 ) ) );
                return 1;
            }
            catch ( NumberFormatException ex ) {
                LuaAPI.luaL_error ( thread, "s2d: Error: string not a number" );
                return -1;
            }
        }
    }

    public static final class setyhook implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class stacklevel implements JavaFunction {

        public int Call ( lua_State thread ) {
            //TODO:
            return -1;
        }
    }

    public static final class mem_query implements JavaFunction {

        public int Call ( lua_State thread ) {
            return 0;
        }
    }

    public static final class settrick implements JavaFunction {

        public int Call ( lua_State thread ) {
            Trick = LuaAPI.lua_tointeger ( thread, 1 );
            return 0;
        }
    }

    public static final class udataval implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushinteger ( thread, ( ( Double ) LuaAPI.lua_touserdata ( thread, 1 ) ).intValue () );
            return 1;
        }
    }

    public static final class luaB_opentests implements JavaFunction {

        public int Call ( lua_State thread ) {
            luaL_Reg[] tests_funcs = new luaL_Reg[] {
                new luaL_Reg ( "checkmemory", new lua_checkmemory () ),
                new luaL_Reg ( "closestate", new closestate () ),
                new luaL_Reg ( "d2s", new d2s () ),
                new luaL_Reg ( "doonnewstack", new doonnewstack () ),
                new luaL_Reg ( "doremote", new doremote () ),
                new luaL_Reg ( "gccolor", new get_gccolor () ),
                new luaL_Reg ( "gcstate", new gcstate () ),
                new luaL_Reg ( "getref", new getref () ),
                new luaL_Reg ( "gsub", new auxgsub () ),
                new luaL_Reg ( "hash", new hash_query () ),
                new luaL_Reg ( "int2fb", new int2fb_aux () ),
                new luaL_Reg ( "limits", new get_limits () ),
                new luaL_Reg ( "listcode", new listcode () ),
                new luaL_Reg ( "listk", new listk () ),
                new luaL_Reg ( "listlocals", new listlocals () ),
                new luaL_Reg ( "loadlib", new loadlib () ),
                new luaL_Reg ( "log2", new log2_aux () ),
                new luaL_Reg ( "newstate", new newstate () ),
                new luaL_Reg ( "newuserdata", new newuserdata () ),
                new luaL_Reg ( "num2int", new num2int () ),
                new luaL_Reg ( "pushuserdata", new pushuserdata () ),
                new luaL_Reg ( "querystr", new string_query () ),
                new luaL_Reg ( "querytab", new table_query () ),
                new luaL_Reg ( "ref", new tref () ),
                new luaL_Reg ( "resume", new coresume () ),
                new luaL_Reg ( "s2d", new s2d () ),
                new luaL_Reg ( "setyhook", new setyhook () ),
                new luaL_Reg ( "stacklevel", new stacklevel () ),
                new luaL_Reg ( "testC", new testC () ),
                new luaL_Reg ( "totalmem", new mem_query () ),
                new luaL_Reg ( "trick", new settrick () ),
                new luaL_Reg ( "udataval", new udataval () ),
                new luaL_Reg ( "unref", new unref () ),
                new luaL_Reg ( "upvalue", new upvalue () ),
            };

            lua_state = thread;  /* keep first state to be opened */
            LuaAPI.luaL_register ( thread, "T", tests_funcs );
            return 0;
        }
    }
}

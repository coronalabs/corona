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
class LuaTableLib {

    public static final String LUA_TABLIBNAME = "table";

    public static final class tconcat implements JavaFunction {

        public int Call ( lua_State thread ) {
            StringBuffer b = new StringBuffer ();
            int i, last;
            String sep = LuaAPI.luaL_optlstring ( thread, 2, "" );
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            i = LuaAPI.luaL_optint ( thread, 3, 1 );
            last = LuaAPI.luaL_opt ( thread, new LuaAPI.luaL_checkint (), 4, LuaAPI.luaL_getn ( thread, 1 ) );
            for (; i <= last; i ++ ) {
                LuaAPI.lua_rawgeti ( thread, 1, i );
                LuaAPI.luaL_argcheck ( thread, LuaAPI.lua_isstring ( thread, -1 ), 1, "table contains non-strings" );
                //Object o = thread.GetValue(thread.GetObjectsStackTop() - 1);
                b.append ( LuaAPI.lua_tostring ( thread, -1 ) );
                if ( i != last ) {
                    b.append ( sep );
                    LuaAPI.lua_pop ( thread, 1 );
                }
            }
            LuaAPI.lua_pushstring ( thread, b.toString () );
            return 1;
        }
    }

    public static final class foreach implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.luaL_checktype ( thread, 2, LuaAPI.LUA_TFUNCTION );
            LuaAPI.lua_pushnil ( thread );  /* first key */
            while ( LuaAPI.lua_next ( thread, 1 ) ) {
                LuaAPI.lua_pushvalue ( thread, 2 );  /* function */
                LuaAPI.lua_pushvalue ( thread, -3 );  /* key */
                LuaAPI.lua_pushvalue ( thread, -3 );  /* value */
                LuaAPI.lua_call ( thread, 2, 1 );
                if (  ! LuaAPI.lua_isnil ( thread, -1 ) ) {
                    return 1;
                }
                LuaAPI.lua_pop ( thread, 2 );  /* remove value and result */
            }
            return 0;
        }
    }

    public static final class foreachi implements JavaFunction {

        public int Call ( lua_State thread ) {
            int i;
            int n = aux_getn ( thread, 1 );
            LuaAPI.luaL_checktype ( thread, 2, LuaAPI.LUA_TFUNCTION );
            for ( i = 1; i <= n; i ++ ) {
                LuaAPI.lua_pushvalue ( thread, 2 );  /* function */
                LuaAPI.lua_pushinteger ( thread, i );  /* 1st argument */
                LuaAPI.lua_rawgeti ( thread, 1, i );  /* 2nd argument */
                LuaAPI.lua_call ( thread, 2, 1 );
                if (  ! LuaAPI.lua_isnil ( thread, -1 ) ) {
                    return 1;
                }
                LuaAPI.lua_pop ( thread, 1 );  /* remove nil result */
            }
            return 0;
        }
    }

    public static final class getn implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushinteger ( thread, aux_getn ( thread, 1 ) );
            return 1;
        }
    }

    public static int aux_getn ( lua_State thread, int n ) {
        LuaAPI.luaL_checktype ( thread, n, LuaAPI.LUA_TTABLE );
        return LuaAPI.luaL_getn ( thread, n );
    }

    public static final class maxn implements JavaFunction {

        public int Call ( lua_State thread ) {
            double max = 0;
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.lua_pushnil ( thread );  /* first key */
            while ( LuaAPI.lua_next ( thread, 1 ) ) {
                LuaAPI.lua_pop ( thread, 1 );  /* remove value */
                if ( LuaAPI.lua_type ( thread, -1 ) == LuaAPI.LUA_TNUMBER ) {
                    double v = LuaAPI.lua_tonumber ( thread, -1 );
                    if ( v > max ) {
                        max = v;
                    }
                }
            }
            LuaAPI.lua_pushnumber ( thread, max );
            return 1;
        }
    }

    public static final class tinsert implements JavaFunction {

        public int Call ( lua_State thread ) {
            int e = aux_getn ( thread, 1 ) + 1;  /* first empty element */
            int pos;  /* where to insert new element */
            switch ( LuaAPI.lua_gettop ( thread ) ) {
                case 2: /* called with only 2 arguments */ {
                    pos = e;  /* insert new element at the end */
                    break;
                }
                case 3: {
                    int i;
                    pos = LuaAPI.luaL_checkint ( thread, 2 );  /* 2nd argument is the position */
                    if ( pos > e ) {
                        e = pos;  /* `grow' array if necessary */
                    }
                    for ( i = e; i > pos; i -- ) /* move up elements */ {
                        LuaAPI.lua_rawgeti ( thread, 1, i - 1 );
                        LuaAPI.lua_rawseti ( thread, 1, i );  /* t[i] = t[i-1] */
                    }
                    break;
                }
                default: {
                    return LuaAPI.luaL_error ( thread, "wrong number of arguments to '\"insert'\"" );
                }
            }
            LuaAPI.luaL_setn ( thread, 1, e );  /* new size */
            LuaAPI.lua_rawseti ( thread, 1, pos );  /* t[pos] = v */
            return 0;
        }
    }

    public static final class tremove implements JavaFunction {

        public int Call ( lua_State thread ) {
            int e = aux_getn ( thread, 1 );
            int pos = LuaAPI.luaL_optint ( thread, 2, e );
            if (  ! ( 1 <= pos && pos <= e ) ) /* position is outside bounds? */ {
                return 0;  /* nothing to remove */
            }
            LuaAPI.luaL_setn ( thread, 1, e - 1 );  /* t.n = n-1 */
            LuaAPI.lua_rawgeti ( thread, 1, pos );  /* result = t[pos] */
            for (; pos < e; pos ++ ) {
                LuaAPI.lua_rawgeti ( thread, 1, pos + 1 );
                LuaAPI.lua_rawseti ( thread, 1, pos );  /* t[pos] = t[pos+1] */
            }
            LuaAPI.lua_pushnil ( thread );
            LuaAPI.lua_rawseti ( thread, 1, e );  /* t[e] = nil */
            return 1;
        }
    }

    public static final class setn implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            LuaAPI.luaL_error ( thread, "'setn' is obsolete" );
            LuaAPI.lua_pushvalue ( thread, 1 );
            return 1;
        }
    }

    public static final class sort implements JavaFunction {

        public int Call ( lua_State thread ) {
            int n = aux_getn ( thread, 1 );
            if (  ! LuaAPI.lua_checkstack ( thread, 40 ) ) {
                LuaAPI.luaL_error ( thread, "stack overflow ()" );
            }
            if (  ! LuaAPI.lua_isnoneornil ( thread, 2 ) )  /* is there a 2nd argument? */

                LuaAPI.luaL_checktype ( thread, 2, LuaAPI.LUA_TFUNCTION );
            LuaAPI.lua_settop ( thread, 2 );  /* make sure there is two arguments */
            auxsort ( thread, 1, n );
            return 0;
        }
    }

    public static void auxsort ( lua_State thread, int l, int u ) {
        while ( l < u ) /* for tail recursion */ {
            int i, j;
            /* sort elements a[l], a[(l+u)/2] and a[u] */
            LuaAPI.lua_rawgeti ( thread, 1, l );
            LuaAPI.lua_rawgeti ( thread, 1, u );
            if ( sort_comp ( thread, -1, -2 ) ) /* a[u] < a[l]? */ {
                set2 ( thread, l, u );  /* swap a[l] - a[u] */
            }
            else {
                LuaAPI.lua_pop ( thread, 2 );
            }
            if ( u - l == 1 ) {
                break;  /* only 2 elements */
            }
            i = ( l + u ) / 2;
            LuaAPI.lua_rawgeti ( thread, 1, i );
            LuaAPI.lua_rawgeti ( thread, 1, l );
            if ( sort_comp ( thread, -2, -1 ) ) /* a[i]<a[l]? */ {
                set2 ( thread, i, l );
            }
            else {
                LuaAPI.lua_pop ( thread, 1 );  /* remove a[l] */
                LuaAPI.lua_rawgeti ( thread, 1, u );
                if ( sort_comp ( thread, -1, -2 ) ) /* a[u]<a[i]? */ {
                    set2 ( thread, i, u );
                }
                else {
                    LuaAPI.lua_pop ( thread, 2 );
                }
            }
            if ( u - l == 2 ) {
                break;  /* only 3 elements */
            }
            LuaAPI.lua_rawgeti ( thread, 1, i );  /* Pivot */
            LuaAPI.lua_pushvalue ( thread, -1 );
            LuaAPI.lua_rawgeti ( thread, 1, u - 1 );
            set2 ( thread, i, u - 1 );
            /* a[l] <= P == a[u-1] <= a[u], only need to sort from l+1 to u-2 */
            i = l;
            j = u - 1;
            for (;;) /* invariant: a[l..i] <= P <= a[j..u] */ {
                /* repeat ++i until a[i] >= P */
                LuaAPI.lua_rawgeti ( thread, 1,  ++ i );
                while ( sort_comp ( thread, -1, -2 ) ) {
                    if ( i > u ) {
                        LuaAPI.luaL_error ( thread, "invalid order function for sorting" );
                    }
                    LuaAPI.lua_pop ( thread, 1 );  /* remove a[i] */
                    LuaAPI.lua_rawgeti ( thread, 1,  ++ i );
                }
                /* repeat --j until a[j] <= P */
                LuaAPI.lua_rawgeti ( thread, 1,  -- j );
                while ( sort_comp ( thread, -3, -1 ) ) {
                    if ( j < l ) {
                        LuaAPI.luaL_error ( thread, "invalid order function for sorting" );
                    }
                    LuaAPI.lua_pop ( thread, 1 );  /* remove a[j] */
                    LuaAPI.lua_rawgeti ( thread, 1,  -- j );
                }
                if ( j < i ) {
                    LuaAPI.lua_pop ( thread, 3 );  /* pop pivot, a[i], a[j] */
                    break;
                }
                set2 ( thread, i, j );
            }
            LuaAPI.lua_rawgeti ( thread, 1, u - 1 );
            LuaAPI.lua_rawgeti ( thread, 1, i );
            set2 ( thread, u - 1, i );  /* swap pivot (a[u-1]) with a[i] */
            /* a[l..i-1] <= a[i] == P <= a[i+1..u] */
            /* adjust so that smaller half is in [j..i] and larger one in [l..u] */
            if ( i - l < u - i ) {
                j = l;
                i = i - 1;
                l = i + 2;
            }
            else {
                j = i + 1;
                i = u;
                u = j - 2;
            }
            auxsort ( thread, j, i );  /* call recursively the smaller one */
        }  /* repeat the routine for the larger one */
    }

    public static void set2 ( lua_State thread, int i, int j ) {
        LuaAPI.lua_rawseti ( thread, 1, i );
        LuaAPI.lua_rawseti ( thread, 1, j );
    }

    static boolean sort_comp ( lua_State thread, int a, int b ) {
        if (  ! LuaAPI.lua_isnil ( thread, 2 ) ) /* function? */ {
            boolean res;
            LuaAPI.lua_pushvalue ( thread, 2 );
            LuaAPI.lua_pushvalue ( thread, a - 1 );  /* -1 to compensate function */
            LuaAPI.lua_pushvalue ( thread, b - 2 );  /* -2 to compensate function and `a' */
            LuaAPI.lua_call ( thread, 2, 1 );
            res = LuaAPI.lua_toboolean ( thread, -1 );
            LuaAPI.lua_pop ( thread, 1 );
            return res;
        }
        else /* a < b? */ {
            return LuaAPI.lua_lessthan ( thread, a, b );
        }
    }

    public static final class luaopen_table implements JavaFunction {

        public int Call ( lua_State thread ) {

            luaL_Reg[] luaReg = new luaL_Reg[] {
                new luaL_Reg ( "concat", new tconcat () ),
                new luaL_Reg ( "foreach", new foreach () ),
                new luaL_Reg ( "foreachi", new foreachi () ),
                new luaL_Reg ( "getn", new getn () ),
                new luaL_Reg ( "maxn", new maxn () ),
                new luaL_Reg ( "insert", new tinsert () ),
                new luaL_Reg ( "remove", new tremove () ),
                new luaL_Reg ( "setn", new setn () ),
                new luaL_Reg ( "sort", new sort () ),
            };

            LuaAPI.luaL_register ( thread, LUA_TABLIBNAME, luaReg );
            return 1;
        }
    }
}

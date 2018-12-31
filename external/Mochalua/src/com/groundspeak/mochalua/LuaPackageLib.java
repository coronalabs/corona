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

import javax.microedition.io.Connector;
import javax.microedition.io.file.FileConnection;

/**
 *
 * @author a.fornwald
 */
class LuaPackageLib {

    public static final String LUA_DIRSEP = "/";
    public static final String LUA_PATHSEP = ";";
    public static final String LUA_PATH_MARK = "?";
    public static final String LUA_EXECDIR = "!";
    public static final String LUA_IGMARK = "-";
    public static final String LUA_PATH = "LUA_PATH";
    //public static final String LUA_PATH_DEFAULT	    =   "/resource/?.out;/lua/?.out;?.out;";
    public static final String LUA_PATH_DEFAULT = "resource/?.out;lua/?.out;?.out;";
    public static final boolean LUA_COMPAT_LOADLIB = false;
    public static final String LUA_LOADLIBNAME = "package";

    public static final class ll_loadlib implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnil ( thread );
            LuaAPI.lua_pushstring ( thread, "The specified module could not be opened" );
            LuaAPI.lua_pushstring ( thread, "open" );
            return 3;
        }
    }

    public static final class ll_seeall implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
            if (  ! LuaAPI.lua_getmetatable ( thread, 1 ) ) {
                LuaAPI.lua_createtable ( thread, 0, 1 ); /* create new metatable */
                LuaAPI.lua_pushvalue ( thread, -1 );
                LuaAPI.lua_setmetatable ( thread, 1 );
            }
            LuaAPI.lua_pushvalue ( thread, LuaAPI.LUA_GLOBALSINDEX );
            LuaAPI.lua_setfield ( thread, -2, "__index" );  /* mt.__index = _G */
            return 0;
        }
    }

    public static final class ll_module implements JavaFunction {

        private static void modinit ( lua_State thread, String modname ) {
            LuaAPI.lua_pushvalue ( thread, -1 );
            LuaAPI.lua_setfield ( thread, -2, "_M" );  /* module._M = module */
            LuaAPI.lua_pushstring ( thread, modname );
            LuaAPI.lua_setfield ( thread, -2, "_NAME" );
            //dot = strrchr(modname, '.');  /* look for last dot in module name */
            int iLen = modname.lastIndexOf ( '.' );
            if ( iLen == -1 ) {
                iLen = 0;
            }
            else {
                iLen ++;
            }
            /* set _PACKAGE as package name (full module name minus last part) */
            LuaAPI.lua_pushlstring ( thread, modname, iLen );
            LuaAPI.lua_setfield ( thread, -2, "_PACKAGE" );
        }

        private static void setfenv ( lua_State thread ) {
            lua_Debug ar = new lua_Debug ();
            LuaAPI.lua_getstack ( thread, 1, ar );
            LuaAPI.lua_getinfo ( thread, "f", ar );
            LuaAPI.lua_pushvalue ( thread, -2 );
            LuaAPI.lua_setfenv ( thread, -2 );
            LuaAPI.lua_pop ( thread, 1 );
        }

        static void dooptions ( lua_State thread, int n ) {
            int i;
            for ( i = 2; i <= n; i ++ ) {
                LuaAPI.lua_pushvalue ( thread, i );  /* get option (a function) */
                LuaAPI.lua_pushvalue ( thread, -2 );  /* module */
                LuaAPI.lua_call ( thread, 1, 0 );
            }
        }

        public int Call ( lua_State thread ) {
            String modname = LuaAPI.luaL_checkstring ( thread, 1 );
            int loaded = LuaAPI.lua_gettop ( thread ) + 1;  /* index of _LOADED table */
            LuaAPI.lua_getfield ( thread, LuaAPI.LUA_REGISTRYINDEX, "_LOADED" );
            LuaAPI.lua_getfield ( thread, loaded, modname );  /* get _LOADED[modname] */
            if ( LuaAPI.lua_istable ( thread, -1 ) == false ) /* not found? */ {
                LuaAPI.lua_pop ( thread, 1 );  /* remove previous result */
                /* try global variable (and create one if it does not exist) */
                if ( LuaAPI.luaL_findtable ( thread, LuaAPI.LUA_GLOBALSINDEX, modname, 1 ) != null ) {
                    return LuaAPI.luaL_error ( thread, "name conflict for module \"" + modname + "\"" );
                }
                LuaAPI.lua_pushvalue ( thread, -1 );
                LuaAPI.lua_setfield ( thread, loaded, modname );  /* _LOADED[modname] = new table */
            }
            /* check whether table already has a _NAME field */
            LuaAPI.lua_getfield ( thread, -1, "_NAME" );
            if ( LuaAPI.lua_isnil ( thread, -1 ) == false ) /* is table an initialized module? */ {
                LuaAPI.lua_pop ( thread, 1 );
            }
            else /* no; initialize it */ {
                LuaAPI.lua_pop ( thread, 1 );
                modinit ( thread, modname );
            }
            LuaAPI.lua_pushvalue ( thread, -1 );
            setfenv ( thread );
            dooptions ( thread, loaded - 1 );
            return 0;
        }
    }

    public static final class ll_require implements JavaFunction {

        private static Double sentinel = new Double ( 0 );

        public int Call ( lua_State thread ) {
            String name = LuaAPI.luaL_checkstring ( thread, 1 );
            int i;
            LuaAPI.lua_settop ( thread, 1 );  /* _LOADED table will be at index 2 */
            LuaAPI.lua_getfield ( thread, LuaAPI.LUA_REGISTRYINDEX, "_LOADED" );
            LuaAPI.lua_getfield ( thread, 2, name );
            if ( LuaAPI.lua_toboolean ( thread, -1 ) ) {  /* is it there? */
                if ( LuaAPI.lua_touserdata ( thread, -1 ) == sentinel ) /* check loops */ {
                    LuaAPI.luaL_error ( thread, "loop or previous error loading module \"" + name + "\"" );
                }
                return 1;  /* package is already loaded */
            }
            /* else must load it; iterate over available loaders */
            LuaAPI.lua_getfield ( thread, LuaAPI.LUA_ENVIRONINDEX, "loaders" );
            if ( LuaAPI.lua_istable ( thread, -1 ) == false ) {
                LuaAPI.luaL_error ( thread, "'package.loaders' must be a table" );
            }
            LuaAPI.lua_pushliteral ( thread, "" );  /* error message accumulator */
            for ( i = 1;; i ++ ) {
                LuaAPI.lua_rawgeti ( thread, -2, i );  /* get a loader */
                if ( LuaAPI.lua_isnil ( thread, -1 ) == true ) {
                    LuaAPI.luaL_error ( thread, "module " + name + " not found:" + LuaAPI.lua_tostring ( thread, -2 ) );
                }
                LuaAPI.lua_pushstring ( thread, name );
                LuaAPI.lua_call ( thread, 1, 1 );  /* call it */
                if ( LuaAPI.lua_isfunction ( thread, -1 ) ) /* did it find module? */ {
                    break;  /* module loaded successfully */
                }
                else if ( LuaAPI.lua_isstring ( thread, -1 ) ) /* loader returned error message? */ {
                    LuaAPI.lua_concat ( thread, 2 );  /* accumulate it */
                }
                else {
                    LuaAPI.lua_pop ( thread, 1 );
                }
            }

            LuaAPI.lua_pushlightuserdata ( thread, sentinel );
            LuaAPI.lua_setfield ( thread, 2, name );  /* _LOADED[name] = sentinel */
            LuaAPI.lua_pushstring ( thread, name );  /* pass name as argument to module */
            LuaAPI.lua_call ( thread, 1, 1 );  /* run loaded module */
            if ( LuaAPI.lua_isnil ( thread, -1 ) == false ) /* non-nil return? */ {
                LuaAPI.lua_setfield ( thread, 2, name );  /* _LOADED[name] = returned value */
            }
            LuaAPI.lua_getfield ( thread, 2, name );
            if ( LuaAPI.lua_touserdata ( thread, -1 ) == sentinel ) /* module did not set a value? */ {
                LuaAPI.lua_pushboolean ( thread, true );  /* use true as result */
                LuaAPI.lua_pushvalue ( thread, -1 );  /* extra copy to be returned */
                LuaAPI.lua_setfield ( thread, 2, name );  /* _LOADED[name] = true */
            }
            return 1;
        }
    }

    public static final class loader_preload implements JavaFunction {

        public int Call ( lua_State thread ) {
            String name = LuaAPI.luaL_checkstring ( thread, 1 );
            LuaAPI.lua_getfield ( thread, LuaAPI.LUA_ENVIRONINDEX, "preload" );
            if ( LuaAPI.lua_istable ( thread, -1 ) == false ) {
                LuaAPI.luaL_error ( thread, "'package.preload' must be a table" );
            }
            LuaAPI.lua_getfield ( thread, -1, name );
            if ( LuaAPI.lua_isnil ( thread, -1 ) ) /* not found? */ {
                LuaAPI.lua_pushfstring ( thread, "\n\tno field package.preload['" + name + "']" );
            }
            return 1;
        }
    }

    private static void loaderror ( lua_State thread, String filename ) {
        LuaAPI.luaL_error ( thread, "error loading module " + LuaAPI.lua_tostring ( thread, 1 ) + " from file " + filename + ":\n\t" + LuaAPI.lua_tostring ( thread, -1 ) );
    }

    public static boolean checkIsFileExistOnStorage ( String filename ) {
        FileConnection fileConnection = null;
        if (  ! filename.startsWith ( "file://" ) )
            filename = "file://" + filename;

        try {
            fileConnection = ( FileConnection ) Connector.open ( filename );
            if ( fileConnection == null )
                return false;
            if ( fileConnection.exists () ) {
                return true;
            }
            else
                return false;
        }
        catch ( Exception ex ) {
            return false;
        }
        finally {
            if ( fileConnection != null ) {
                try {
                    fileConnection.close ();
                }
                catch ( Exception e ) {
                }
                finally {
                    fileConnection = null;
                }
            }
        }
    }

    public static final class loader_Lua implements JavaFunction {

        private static boolean readable ( String filename ) {
            if ( checkIsFileExistOnStorage ( filename ) ) {
                return true;
            }
            else if ( Class.class.getClass ().getResourceAsStream ( filename ) != null ) {
                return true;
            }
            return false;
        }

        private static String pushnexttemplate ( lua_State thread, String path ) {
            String l;
            while ( path.length () != 0 && path.substring ( 0, 1 ).equals ( LUA_PATHSEP ) ) {
                path = path.substring ( 1 );
            }
            if ( path.equals ( "" ) ) {
                return null;
            }

            int iInstanceOfPath = path.indexOf ( LUA_PATHSEP );
            if ( iInstanceOfPath == -1 ) {
                l = "";
            }
            else {
                l = path.substring ( iInstanceOfPath );
            }

            LuaAPI.lua_pushlstring ( thread, path, iInstanceOfPath );
            return l;
        }

        private static String findfile ( lua_State thread, String name, String pname ) {
            String path;
            name = LuaAPI.luaL_gsub ( thread, name, ".", LUA_DIRSEP );
            LuaAPI.lua_getfield ( thread, LuaAPI.LUA_ENVIRONINDEX, pname );
            path = LuaAPI.lua_tostring ( thread, -1 );
            if ( path == null ) {
                LuaAPI.luaL_error ( thread, "\"package." + pname + "\" must be a string" );
            }
            LuaAPI.lua_pushliteral ( thread, "" );  /* error accumulator */
            while ( ( path = pushnexttemplate ( thread, path ) ) != null ) {
                String filename = LuaAPI.luaL_gsub ( thread, LuaAPI.lua_tostring ( thread, -1 ), LUA_PATH_MARK, name );
                filename = LUA_DIRSEP + filename;
                LuaAPI.lua_remove ( thread, -2 );  /* remove path template */
                if ( readable ( filename ) ) /* does file exist and is readable? */ {
                    return filename;  /* return that file name */
                }
                LuaAPI.lua_pushfstring ( thread, "\n\tno file " + filename );
                LuaAPI.lua_remove ( thread, -2 );  /* remove file name */
                LuaAPI.lua_concat ( thread, 2 );  /* add entry to possible error message */
            }
            return null;  /* not found */
        }

        public int Call ( lua_State thread ) {
            String filename = "";
            String name = LuaAPI.luaL_checkstring ( thread, 1 );
            filename = findfile ( thread, name, "path" );
            if ( filename == null ) {
                return 1;  /* library not found in this path */
            }
            if ( LuaAPI.luaL_loadfile ( thread, filename ) != 0 ) {
                loaderror ( thread, filename );
            }
            return 1;  /* library loaded successfully */
        }
    }

    /*public static final class loader_C implements JavaFunction
    {
    public int Call( lua_State thread )
    {
    // TODO:
    return 1;
    }
    }
    
    public static final class loader_Croot implements JavaFunction
    {
    public int Call( lua_State thread )
    {
    // TODO:
    return 1;
    }
    }*/
    public static final class luaopen_package implements JavaFunction {

        private static void setpath ( lua_State thread, String fieldname, String envname, String def ) {
            String path = null;//getenv( envname );

            LuaAPI.lua_pushstring ( thread, def );  /* use default */
            LuaAPI.lua_setfield ( thread, -2, fieldname );
        }

        public int Call ( lua_State thread ) {

            luaL_Reg[] luaRegLL = new luaL_Reg[] {
                new luaL_Reg ( "module", new ll_module () ),
                new luaL_Reg ( "require", new ll_require () ),
            };

            luaL_Reg[] luaRegPK = new luaL_Reg[] {
                new luaL_Reg ( "loadlib", new ll_loadlib () ),
                new luaL_Reg ( "seeall", new ll_seeall () ),
            };

            JavaFunction[] loaders = new JavaFunction[] {
                new loader_preload (),
                new loader_Lua (),
            };

            int i;
            /* create new type _LOADLIB */
            LuaAPI.luaL_newmetatable ( thread, "_LOADLIB" );
            // FIXME:
            //LuaAPI.lua_pushjavafunction( thread, new gctm() );
            //LuaAPI.lua_setfield( thread, -2, "__gc" );
	    /* create `package' table */
            LuaAPI.luaL_register ( thread, LUA_LOADLIBNAME, luaRegPK );

            if ( LUA_COMPAT_LOADLIB == true ) {
                LuaAPI.lua_getfield ( thread, -1, "loadlib" );
                LuaAPI.lua_setfield ( thread, LuaAPI.LUA_GLOBALSINDEX, "loadlib" );
            }

            LuaAPI.lua_pushvalue ( thread, -1 );
            LuaAPI.lua_replace ( thread, LuaAPI.LUA_ENVIRONINDEX );
            /* create `loaders' table */
            LuaAPI.lua_createtable ( thread, 0, loaders.length );
            /* fill it with pre-defined loaders */
            for ( i = 0; i < loaders.length; i ++ ) {
                LuaAPI.lua_pushjavafunction ( thread, loaders[i] );
                LuaAPI.lua_rawseti ( thread, -2, i + 1 );
            }
            LuaAPI.lua_setfield ( thread, -2, "loaders" );  /* put it in field `loaders' */
            setpath ( thread, "path", LUA_PATH, LUA_PATH_DEFAULT );  /* set field `path' */
            //setpath( thread, "cpath", LUA_CPATH, LUA_CPATH_DEFAULT ); /* set field `cpath' */
	    /* store config information */
            LuaAPI.lua_pushliteral ( thread, LUA_DIRSEP + "\n" + LUA_PATHSEP + "\n" + LUA_PATH_MARK + "\n" + LUA_EXECDIR + "\n" + LUA_IGMARK );
            LuaAPI.lua_setfield ( thread, -2, "config" );
            /* set field `loaded' */
            LuaAPI.luaL_findtable ( thread, LuaAPI.LUA_REGISTRYINDEX, "_LOADED", 2 );
            LuaAPI.lua_setfield ( thread, -2, "loaded" );
            /* set field `preload' */
            LuaAPI.lua_newtable ( thread );
            LuaAPI.lua_setfield ( thread, -2, "preload" );
            LuaAPI.lua_pushvalue ( thread, LuaAPI.LUA_GLOBALSINDEX );
            LuaAPI.luaL_register ( thread, null, luaRegLL );  /* open lib into global table */
            LuaAPI.lua_pop ( thread, 1 );
            return 1;  /* return 'package' table */
        }
    }
}

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

import java.util.Calendar;
import java.util.TimeZone;
import javax.microedition.io.Connector;
import javax.microedition.io.file.FileConnection;

/**
 *
 * @author a.fornwald
 */
class LuaOSLib {

    public static final String LUA_OSLIBNAME = "os";
    private static final int CLOCKS_PER_SEC = 1000;   // MSVC++ .net FIXME: is it right


    public static final class os_clock implements JavaFunction {

        public int Call ( lua_State thread ) {
            // FIXME: System.currentTimeMillis() - might be a lost of persition
            LuaAPI.lua_pushnumber ( thread, ( ( double ) Calendar.getInstance ().getTime ().getTime () ) / ( double ) CLOCKS_PER_SEC );
            return 1;
        }
    }

    public static final class os_date implements JavaFunction {

        private void setfield ( lua_State thread, String key, int value ) {
            LuaAPI.lua_pushinteger ( thread, value );
            LuaAPI.lua_setfield ( thread, -2, key );
        }

        public int Call ( lua_State thread ) {
            int si = 0;
            String s = LuaAPI.luaL_optstring ( thread, 1, "%c" );
            //long t = luaL_opt( thread, ( long )LuaAPI.luaL_checknumber, 2,  );
            long t = LuaAPI.lua_isnoneornil ( thread, 2 ) ? ( long ) Calendar.getInstance ().getTime ().getTime () : ( long ) LuaAPI.luaL_checknumber ( thread, 2 );
            Calendar calendar = null;
            if ( s.charAt ( si ) == '!' ) // UTC?
            {
                calendar = Calendar.getInstance ( TimeZone.getTimeZone ( "UTC" ) );
                si ++;  // skip `!'

            }
            else {
                calendar = Calendar.getInstance ( TimeZone.getTimeZone ( "GMT" ) );
            }

            if ( calendar == null ) // invalid calendar?
            {
                LuaAPI.lua_pushnil ( thread );
            }
            else if ( s.substring ( si, 2 + si ).equals ( "*t" ) ) {
                LuaAPI.lua_createtable ( thread, 0, 8 );  // 9 = number of fields

                setfield ( thread, "sec", calendar.get ( Calendar.SECOND ) );
                setfield ( thread, "min", calendar.get ( Calendar.MINUTE ) );
                setfield ( thread, "hour", calendar.get ( Calendar.HOUR ) );
                setfield ( thread, "day", calendar.get ( Calendar.DAY_OF_MONTH ) );
                setfield ( thread, "month", calendar.get ( Calendar.MONTH ) + 1 );
                setfield ( thread, "year", calendar.get ( Calendar.YEAR ) );
                setfield ( thread, "wday", calendar.get ( Calendar.DAY_OF_WEEK ) - 1 );

                int dayOfTheYear = getDayOfYear ( calendar );
                setfield ( thread, "yday", dayOfTheYear );

            //LuaAPI.setboolfield( thread, "isdst", calendar.get( Calendar. ) );	// FIXME:
            }
            else {

                char cc;
                luaL_Buffer b = new luaL_Buffer ();
                LuaAPI.luaL_buffinit ( thread, b );
                int stringIndex;
                int strLen = s.length ();
                for ( stringIndex = 0; stringIndex < strLen; stringIndex ++ ) {
                    if ( s.charAt ( stringIndex ) != '%' || s.charAt ( stringIndex + 1 ) == '\0' ) /* no conversion specifier? */ {
                        LuaAPI.luaL_addchar ( b, s.charAt ( stringIndex ) );
                    }
                    else {
                        cc = s.charAt (  ++ stringIndex );
                        StringBuffer buff = new StringBuffer ();
                        strftime ( buff, cc, calendar );
                        LuaAPI.luaL_addlstring ( b, buff.toString (), buff.length () );
                    }
                }
                LuaAPI.luaL_pushresult ( b );
            }
            return 1;
        }
    }

    private static int getDayOfYear ( Calendar cal ) {
        Calendar calendar2 = Calendar.getInstance ();
        calendar2.setTime ( cal.getTime () );
        calendar2.set ( Calendar.MONTH, 0 );
        calendar2.set ( Calendar.DAY_OF_MONTH, 1 );
        long diff = ( Calendar.getInstance ().getTime ().getTime () - calendar2.getTime ().getTime () );

        long f = diff;
        double g = ( double ) diff / ( double ) ( 1000 * 24 * 60 * 60 );

        Double d = new Double ( g );
        if ( d.doubleValue () - d.intValue () != 0 ) {
            return d.intValue () + 1;
        }
        else {
            return d.intValue ();
        }
    }

    private static int getWeekOfYear ( Calendar cal, boolean sun ) {
        Calendar calendar2 = Calendar.getInstance ();
        calendar2.setTime ( cal.getTime () );
        calendar2.set ( Calendar.MONTH, 0 );
        calendar2.set ( Calendar.DAY_OF_MONTH, 1 );
        int dayOfWeek = calendar2.get ( Calendar.DAY_OF_WEEK );
        if ( sun ) {
            if ( dayOfWeek != Calendar.SUNDAY ) {
                calendar2.set ( Calendar.DAY_OF_MONTH, ( 7 - dayOfWeek ) + 1 );
            }
        }
        else {
            if ( dayOfWeek != Calendar.MONDAY ) {
                calendar2.set ( Calendar.DAY_OF_MONTH, ( 7 - dayOfWeek + 1 ) + 1 );
            }
        }
        long diff = ( Calendar.getInstance ().getTime ().getTime () - calendar2.getTime ().getTime () );

        long f = diff;
        double g = ( double ) diff / ( double ) ( 1000 * 24 * 60 * 60 * 7 );

        Double d = new Double ( g );
        if ( d.doubleValue () - d.intValue () != 0 ) {
            return d.intValue () + 1;
        }
        else {
            return d.intValue ();
        }
    }

    private static int strftime ( StringBuffer sb, char format, Calendar cal ) {
        switch ( format ) {
            case 'a':
                 {
                    int dayOfMonth = cal.get ( Calendar.DAY_OF_WEEK );
                    switch ( dayOfMonth ) {
                        case Calendar.MONDAY:
                             {
                                sb.append ( "Mon" );
                            }
                            break;
                        case Calendar.TUESDAY:
                             {
                                sb.append ( "Tue" );
                            }
                            break;
                        case Calendar.WEDNESDAY:
                             {
                                sb.append ( "Wed" );
                            }
                            break;
                        case Calendar.THURSDAY:
                             {
                                sb.append ( "Thu" );
                            }
                            break;
                        case Calendar.FRIDAY:
                             {
                                sb.append ( "Fri" );
                            }
                            break;
                        case Calendar.SATURDAY:
                             {
                                sb.append ( "Sat" );
                            }
                            break;
                        case Calendar.SUNDAY:
                             {
                                sb.append ( "Sun" );
                            }
                            break;
                    }
                }
                break;
            case 'A':
                 {
                    int dayOfMonth = cal.get ( Calendar.DAY_OF_WEEK );
                    switch ( dayOfMonth ) {
                        case Calendar.MONDAY:
                             {
                                sb.append ( "Monday" );
                            }
                            break;
                        case Calendar.TUESDAY:
                             {
                                sb.append ( "Tuesday" );
                            }
                            break;
                        case Calendar.WEDNESDAY:
                             {
                                sb.append ( "Wednesday" );
                            }
                            break;
                        case Calendar.THURSDAY:
                             {
                                sb.append ( "Thursday" );
                            }
                            break;
                        case Calendar.FRIDAY:
                             {
                                sb.append ( "Friday" );
                            }
                            break;
                        case Calendar.SATURDAY:
                             {
                                sb.append ( "Satyrday" );
                            }
                            break;
                        case Calendar.SUNDAY:
                             {
                                sb.append ( "Sunday" );
                            }
                            break;
                    }
                }
                break;
            case 'b':
                 {
                    int month = cal.get ( Calendar.MONTH );
                    switch ( month ) {
                        case Calendar.JANUARY:
                             {
                                sb.append ( "Jan" );
                            }
                            break;
                        case Calendar.FEBRUARY:
                             {
                                sb.append ( "Feb" );
                            }
                            break;
                        case Calendar.MARCH:
                             {
                                sb.append ( "Mar" );
                            }
                            break;
                        case Calendar.APRIL:
                             {
                                sb.append ( "Apr" );
                            }
                            break;
                        case Calendar.MAY:
                             {
                                sb.append ( "May" );
                            }
                            break;
                        case Calendar.JUNE:
                             {
                                sb.append ( "Jun" );
                            }
                            break;
                        case Calendar.JULY:
                             {
                                sb.append ( "Jul" );
                            }
                            break;
                        case Calendar.AUGUST:
                             {
                                sb.append ( "Aug" );
                            }
                            break;
                        case Calendar.SEPTEMBER:
                             {
                                sb.append ( "Sep" );
                            }
                            break;
                        case Calendar.OCTOBER:
                             {
                                sb.append ( "Oct" );
                            }
                            break;
                        case Calendar.NOVEMBER:
                             {
                                sb.append ( "Nov" );
                            }
                            break;
                        case Calendar.DECEMBER:
                             {
                                sb.append ( "Dec" );
                            }
                            break;
                    }
                }
                break;
            case 'B':
                 {
                    int month = cal.get ( Calendar.MONTH );
                    switch ( month ) {
                        case Calendar.JANUARY:
                             {
                                sb.append ( "January" );
                            }
                            break;
                        case Calendar.FEBRUARY:
                             {
                                sb.append ( "February" );
                            }
                            break;
                        case Calendar.MARCH:
                             {
                                sb.append ( "March" );
                            }
                            break;
                        case Calendar.APRIL:
                             {
                                sb.append ( "April" );
                            }
                            break;
                        case Calendar.MAY:
                             {
                                sb.append ( "May" );
                            }
                            break;
                        case Calendar.JUNE:
                             {
                                sb.append ( "June" );
                            }
                            break;
                        case Calendar.JULY:
                             {
                                sb.append ( "July" );
                            }
                            break;
                        case Calendar.AUGUST:
                             {
                                sb.append ( "August" );
                            }
                            break;
                        case Calendar.SEPTEMBER:
                             {
                                sb.append ( "September" );
                            }
                            break;
                        case Calendar.OCTOBER:
                             {
                                sb.append ( "October" );
                            }
                            break;
                        case Calendar.NOVEMBER:
                             {
                                sb.append ( "November" );
                            }
                            break;
                        case Calendar.DECEMBER:
                             {
                                sb.append ( "December" );
                            }
                            break;
                    }
                }
                break;
            case 'c':
                 {
                    sb.append ( cal.getTime ().toString () );
                }
                break;
            case 'd':
                 {
                    sb.append ( cal.get ( Calendar.DAY_OF_MONTH ) );
                }
                break;
            case 'H':
                 {
                    sb.append ( cal.get ( Calendar.HOUR_OF_DAY ) );
                }
                break;
            case 'I':
                 {
                    sb.append ( cal.get ( Calendar.HOUR ) );
                }
                break;
            case 'j':
                 {
                    sb.append ( getDayOfYear ( cal ) );
                }
                break;
            case 'm':
                 {
                    sb.append ( ( cal.get ( Calendar.MONTH ) + 1 ) );
                }
                break;
            case 'M':
                 {
                    sb.append ( cal.get ( Calendar.MINUTE ) );
                }
                break;
            case 'p':
                 {
                    sb.append ( ( cal.get ( Calendar.AM_PM ) == Calendar.AM ) ? "AM" : "PM" );
                }
                break;
            case 'S':
                 {
                    sb.append ( cal.get ( Calendar.SECOND ) );
                }
                break;
            case 'U':
                 {
                    sb.append ( getWeekOfYear ( cal, true ) );
                }
                break;
            case 'w':
                 {
                    sb.append ( ( cal.get ( Calendar.DAY_OF_WEEK ) - 1 ) );
                }
                break;
            case 'W':
                 {
                    sb.append ( getWeekOfYear ( cal, false ) );
                }
                break;
            case 'x':
                 {
                    sb.append ( cal.get ( Calendar.MONTH ) );
                    sb.append ( "/" );
                    sb.append ( cal.get ( Calendar.DAY_OF_MONTH ) );
                    sb.append ( "/" );
                    String str = "" + cal.get ( Calendar.YEAR );
                    sb.append ( str.substring ( 2, str.length () ) );
                }
                break;
            case 'X':
                 {
                    sb.append ( cal.get ( Calendar.HOUR_OF_DAY ) );
                    sb.append ( ":" );
                    sb.append ( cal.get ( Calendar.MINUTE ) );
                    sb.append ( ":" );
                    sb.append ( cal.get ( Calendar.SECOND ) );
                }
                break;
            case 'y':
                 {
                    String str = "" + cal.get ( Calendar.YEAR );
                    sb.append ( str.substring ( 2, str.length () ) );
                }
                break;
            case 'Y':
                 {
                    sb.append ( cal.get ( Calendar.YEAR ) );
                }
                break;
            case 'Z':
                 {
                    sb.append ( cal.getTimeZone ().getID () );
                }
                break;
        }
        return 1;
    }

    public static final class os_difftime implements JavaFunction {

        public long difftime ( long t1, long t2 ) {
            return ( t1 - t2 ) / CLOCKS_PER_SEC;
        }

        public int Call ( lua_State thread ) {
            // FIXME: loss of persition
            LuaAPI.lua_pushnumber ( thread, difftime ( ( long ) ( LuaAPI.luaL_checknumber ( thread, 1 ) ), ( long ) ( LuaAPI.luaL_optnumber ( thread, 2, 0 ) ) ) );
            return 1;
        }
    }

    public static final class os_execute implements JavaFunction {

        public int Call ( lua_State thread ) {
            throw new LuaRuntimeException ( "Error: function not implemented yet" );
        }
    }

    public static final class os_exit implements JavaFunction {

        public int Call ( lua_State thread ) {
            LVM.exit ();
            return 0;
        }
    }

    public static final class os_getenv implements JavaFunction {

        public int Call ( lua_State thread ) {
            throw new LuaRuntimeException ( "Error: function not implemented yet" );
        }
    }

    public static int os_pushresult ( lua_State thread, boolean i, String filename ) {
        if ( i ) {
            LuaAPI.lua_pushboolean ( thread, true );
            return 1;
        }
        else {
            LuaAPI.lua_pushnil ( thread );
            LuaAPI.lua_pushstring ( thread, filename );
            return 2;
        }
    }

    public static final class os_remove implements JavaFunction {

        public int Call ( lua_State thread ) {
            boolean bResult = true;
            String filename = LuaAPI.luaL_checkstring ( thread, 1 );
            FileConnection fileConnection = null;
            try {
                fileConnection = ( FileConnection ) Connector.open ( filename );
                if ( fileConnection.exists () ) {
                    fileConnection.delete ();
                }
                else {
                    bResult = false;
                }
            }
            catch ( Exception ex ) {
                bResult = false;
            }
            finally {
                if ( fileConnection != null ) {
                    try {
                        fileConnection.close ();
                    }
                    catch ( Exception ex ) {
                        bResult = false;
                    }
                    fileConnection = null;
                }
            }

            return os_pushresult ( thread, bResult, filename );
        }
    }

    public static final class os_rename implements JavaFunction {

        public int Call ( lua_State thread ) {
            boolean bResult = true;
            String fromname = LuaAPI.luaL_checkstring ( thread, 1 );
            String toname = LuaAPI.luaL_checkstring ( thread, 2 );
            int index = toname.lastIndexOf ( '/' );
            if ( index != -1 ) {
                toname = toname.substring ( index + 1, toname.length () );
            }

            FileConnection fileConnection = null;
            try {
                fileConnection = ( FileConnection ) Connector.open ( fromname );
                if ( fileConnection.exists () ) {
                    fileConnection.rename ( toname );
                }
                else {
                    bResult = false;
                }
            }
            catch ( Exception ex ) {
                bResult = false;
            }
            finally {
                if ( fileConnection != null ) {
                    try {
                        fileConnection.close ();
                    }
                    catch ( Exception ex ) {
                        bResult = false;
                    }
                    fileConnection = null;
                }
            }

            return os_pushresult ( thread, bResult, fromname );
        }
    }
    private static TimeZone m_TimeZone = TimeZone.getTimeZone ( "GTM" );
    ;

    public static final class os_setlocale implements JavaFunction {

        private String setlocale ( int category, String locale ) {
            if ( category != LC_TIME ) {
                throw new LuaRuntimeException ( "os_setlocale: category not supported" );
            }

            if ( locale.equalsIgnoreCase ( "C" ) ) {
                m_TimeZone = TimeZone.getTimeZone ( "GTM" );
            }
            else if ( locale.equals ( "" ) ) {
                m_TimeZone = TimeZone.getDefault ();
            }
            else {
                m_TimeZone = TimeZone.getTimeZone ( locale );
            }

            return locale;
        }
        private static final int LC_ALL = 0;
        private static final int LC_COLLATE = 1;
        private static final int LC_CTYPE = 2;
        private static final int LC_MONETARY = 3;
        private static final int LC_NUMERIC = 4;
        private static final int LC_TIME = 5;
        private static int[] cat = {
            LC_ALL,
            LC_COLLATE,
            LC_CTYPE,
            LC_MONETARY,
            LC_NUMERIC,
            LC_TIME
        };
        private static String[] catnames = {
            "all",
            "collate",
            "ctype",
            "monetary",
            "numeric",
            "time",
        };

        public int Call ( lua_State thread ) {
            String l = LuaAPI.luaL_optstring ( thread, 1, null );
            int op = LuaAPI.luaL_checkoption ( thread, 2, "all", catnames );
            LuaAPI.lua_pushstring ( thread, setlocale ( cat[op], l ) );
            return 1;
        }
    }

    public static final class os_time implements JavaFunction {

        private static int getfield ( lua_State thread, String key, int d ) {
            int res;
            LuaAPI.lua_getfield ( thread, -1, key );
            if ( LuaAPI.lua_isnumber ( thread, -1 ) ) {
                res = ( int ) LuaAPI.lua_tointeger ( thread, -1 );
            }
            else {
                if ( d < 0 ) {
                    return LuaAPI.luaL_error ( thread, "field " + key + " missing in date table" );
                }
                res = d;
            }
            LuaAPI.lua_pop ( thread, 1 );
            return res;
        }

        public int Call ( lua_State thread ) {
            long t;
            if ( LuaAPI.lua_isnoneornil ( thread, 1 ) ) /* called without args? */ {
                t = Calendar.getInstance ().getTime ().getTime ();
            }
            else {

                Calendar calendar = Calendar.getInstance ( m_TimeZone );

                LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TTABLE );
                LuaAPI.lua_settop ( thread, 1 );  /* make sure table is at the top */
                calendar.set ( Calendar.SECOND, getfield ( thread, "sec", 0 ) );
                calendar.set ( Calendar.MINUTE, getfield ( thread, "min", 0 ) );
                calendar.set ( Calendar.HOUR, getfield ( thread, "hour", 0 ) );
                calendar.set ( Calendar.DAY_OF_MONTH, getfield ( thread, "day", 0 ) );
                calendar.set ( Calendar.MONTH, getfield ( thread, "month", 0 ) - 1 );
                calendar.set ( Calendar.YEAR, getfield ( thread, "year", 0 ) );
                calendar.set ( Calendar.DAY_OF_WEEK, getfield ( thread, "wday", 0 ) + 1 );
                //calendar.set( Calendar.SECOND, getboolfield( thread, "isdst", 0 ) );    // FIXME
                t = calendar.getTime ().getTime ();
            }

            LuaAPI.lua_pushnumber ( thread, ( double ) t );
            return 1;
        }
    }

    public static final class os_tmpname implements JavaFunction {

        public int Call ( lua_State thread ) {
            throw new LuaRuntimeException ( "Error: function not implemented yet" );
        }
    }

    public static final class luaopen_os implements JavaFunction {

        public int Call ( lua_State thread ) {

            luaL_Reg[] luaReg = new luaL_Reg[] {
                new luaL_Reg ( "clock", new os_clock () ),
                new luaL_Reg ( "date", new os_date () ),
                new luaL_Reg ( "difftime", new os_difftime () ),
                new luaL_Reg ( "execute", new os_execute () ),
                new luaL_Reg ( "exit", new os_exit () ),
                new luaL_Reg ( "getenv", new os_getenv () ),
                new luaL_Reg ( "remove", new os_remove () ),
                new luaL_Reg ( "rename", new os_rename () ),
                new luaL_Reg ( "setlocale", new os_setlocale () ),
                new luaL_Reg ( "time", new os_time () ),
                new luaL_Reg ( "tmpname", new os_tmpname () ),
            };

            LuaAPI.luaL_register ( thread, LUA_OSLIBNAME, luaReg );
            return 1;
        }
    }
}

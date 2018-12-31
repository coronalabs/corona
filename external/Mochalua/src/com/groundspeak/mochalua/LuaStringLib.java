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
class LuaStringLib {

    public static final String LUA_STRLIBNAME = "string";
    public static final String SPECIALS = "^$*+?.([%-";
    public static final int LUA_MAXCAPTURES = 32;
    public static final char L_ESC = '%';
    public static final String FLAGS = "-+ #0";
    public static final String LUA_INTFRMLEN = "l";
    public static final int CAP_UNFINISHED = ( -1 );
    public static final int CAP_POSITION = ( -2 );

    public static int posrelat ( int pos, int len ) {
        /* relative string position: negative means back from end */
        return ( pos >= 0 ) ? pos : len + pos + 1;
    }

    public static class Capture {

        public StringPointer init;
        public int len;
    }

    public static class MatchState {

        public MatchState () {
            capture = new Capture[ LUA_MAXCAPTURES ];
            for ( int i = 0; i < LUA_MAXCAPTURES; i ++ ) {
                capture[i] = new Capture ();
            }
        }
        public StringPointer src_init;  /* init of source string */

        public int endIndex; /* end (`\0') of source string */

        public lua_State thread;
        public int level;  /* total number of captures (finished or unfinished) */

        public Capture[] capture;
    }

    public static class StringPointer {

        public static StringPointer copyStringPointer ( StringPointer sp ) {
            StringPointer newSP = new StringPointer ();
            newSP.setOriginalString ( sp.getOriginalString () );
            newSP.setIndex ( sp.getIndex () );
            return newSP;
        }
        private String string;
        private int index = 0;

        public StringPointer () {
        }

        public int getIndex () {
            return index;
        }

        public void setIndex ( int ind ) {
            index = ind;
        }

        public String getOriginalString () {
            return string;
        }

        public void setOriginalString ( String orStr ) {
            string = orStr;
        }

        public String getString () {
            return string.substring ( index, string.length () );
        }

        public String getString ( int i ) {
            return string.substring ( index + i, string.length () );
        }

        public char getChar () {
            if ( index >= string.length () )
                return '\0';
            else
                return string.charAt ( index );
        }

        public int length () {
            return string.length () - index;
        }

        public int postIncrStringI ( int num ) {
            int oldIndex = index;
            index += num;
            return oldIndex;
        }

        public int preIncrStringI ( int num ) {
            index += num;
            return index;
        }

        public char postIncrString ( int num ) {
            char c;
            if ( index >= string.length () )
                c = '\0';
            else
                c = string.charAt ( index );
            index += num;
            return c;
        }

        public char preIncrString ( int num ) {
            index += num;
            if ( index >= string.length () )
                return '\0';
            else
                return string.charAt ( index );
        }

        public char getChar ( int strIndex ) {
            if ( index + strIndex >= string.length () )
                return '\0';
            else
                return string.charAt ( index + strIndex );
        }
    }

    public static final int StringLength ( String strText ) {
        if ( strText == null ) {
            return 0;
        }
        return strText.length ();
    }

    public static final class str_byte implements JavaFunction {

        public int Call ( lua_State thread ) {
            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            int iStrLen = StringLength ( s );
            int posi = posrelat ( LuaAPI.luaL_optinteger ( thread, 2, 1 ), iStrLen );
            int pose = posrelat ( LuaAPI.luaL_optinteger ( thread, 3, posi ), iStrLen );
            int n, i;
            if ( posi <= 0 ) {
                posi = 1;
            }
            if ( pose > iStrLen ) {
                pose = iStrLen;
            }
            if ( posi > pose ) {
                return 0;  /* empty interval; return no values */
            }
            n = ( pose - posi + 1 );
            if ( posi + n <= pose ) /* overflow? */ {
                LuaAPI.luaL_error ( thread, "string slice too long" );
            }
            LuaAPI.luaL_checkstack ( thread, n, "string slice too long" );
            for ( i = 0; i < n; i ++ ) {
                LuaAPI.lua_pushinteger ( thread, ( int ) s.charAt ( posi + i - 1 ) );
            }
            return n;
        }
    }

    public static final class str_char implements JavaFunction {

        public int Call ( lua_State thread ) {
            int n = LuaAPI.lua_gettop ( thread );  /* number of arguments */
            int i;
            StringBuffer b = new StringBuffer ();
            for ( i = 1; i <= n; i ++ ) {
                int c = LuaAPI.luaL_checkint ( thread, i );
                LuaAPI.luaL_argcheck ( thread, ( ( char ) c ) == c, i, "invalid value" );
                b.append ( ( char ) c );
            }
            LuaAPI.lua_pushstring ( thread, b.toString () );
            return 1;
        }
    }

    private static final class writer implements lua_Writer {

        public int Call ( lua_State thread, String pData, int iSize, Object userData ) {
            LuaAPI.luaL_addlstring ( ( luaL_Buffer ) userData, ( String ) pData, iSize );
            return 0;
        }
    }

    public static final class str_dump implements JavaFunction {

        public int Call ( lua_State thread ) {
            luaL_Buffer b = new luaL_Buffer ();
            LuaAPI.luaL_checktype ( thread, 1, LuaAPI.LUA_TFUNCTION );
            LuaAPI.lua_settop ( thread, 1 );
            LuaAPI.luaL_buffinit ( thread, b );
            if ( LuaAPI.lua_dump ( thread, new writer (), b ) != 0 ) {
                LuaAPI.luaL_error ( thread, "unable to dump given function" );
            }
            LuaAPI.luaL_pushresult ( b );
            return 1;
        }
    }

    public static final class str_find implements JavaFunction {

        public int Call ( lua_State thread ) {
            return str_find_aux ( thread, 1 );
        }
    }

    private static String strpbrk ( StringPointer str1, String str2 ) {
        char[] c = new char[ 1 ];
        for ( int i = 0; i < str2.length (); i ++ ) {
            c[0] = str2.charAt ( i );
            String a = new String ( c );
            if ( str1.getString ().indexOf ( a ) != -1 ) {
                return a;
            }
        }
        return null;
    }

    public static int memcmp ( StringPointer ptr1, StringPointer ptr2, int num ) {
        for ( int i = 0; i < num; i ++ ) {
            char a = ptr1.getChar ( i );
            char b = ptr2.getChar ( i );
            if ( a > b ) {
                return 1;
            }
            else if ( a < b ) {
                return -1;
            }
        }
        return 0;
    }

    public static StringPointer memchr ( StringPointer ptr, char value, int num ) {
        String str = ptr.getString ();
        int index = str.indexOf ( value );
        if ( index != -1 && index < num ) {
            StringPointer retVal = StringPointer.copyStringPointer ( ptr );
            retVal.postIncrString ( index );
            return retVal;
        }
        else {
            return null;
        }
    }

    static StringPointer lmemfind ( StringPointer s1, int l1, StringPointer s2, int l2 ) {
        if ( l2 == 0 ) {
            return s1;  /* empty strings are everywhere */
        }
        else if ( l2 > l1 ) {
            return null;  /* avoids a negative `l1' */
        }
        else {
            StringPointer init;  /* to search for a `*s2' inside `s1' */
            l2 --;  /* 1st char will be checked by `memchr' */
            l1 = l1 - l2;  /* `s2' cannot be found after that */
            while ( l1 > 0 && ( init = memchr ( s1, s2.getChar (), l1 ) ) != null ) {
                init.postIncrString ( 1 ); /* 1st char is already checked */
                StringPointer sp = StringPointer.copyStringPointer ( s2 );
                sp.postIncrString ( 1 );
                if ( memcmp ( init, sp, l2 ) == 0 ) {
                    init.postIncrString ( -1 );
                    return init;
                }
                else /* correct `l1' and `s1' to try again */ {
                    l1 -= s1.length () - init.length ();
                    s1 = StringPointer.copyStringPointer ( init );
                }
            }
            return null;  /* not found */
        }
    }

    public static void push_onecapture ( MatchState ms, int i, StringPointer s, StringPointer e ) {
        if ( i >= ms.level ) {
            if ( i == 0 ) /* ms->level == 0, too */ {
                LuaAPI.lua_pushlstring ( ms.thread, s.getString (), s.length () - e.length () );  /* add whole match */
            }
            else {
                LuaAPI.luaL_error ( ms.thread, "invalid capture index" );
            }
        }
        else {
            int l = ms.capture[i].len;
            if ( l == CAP_UNFINISHED ) {
                LuaAPI.luaL_error ( ms.thread, "unfinished capture" );
            }
            if ( l == CAP_POSITION ) {
                LuaAPI.lua_pushinteger ( ms.thread, ms.src_init.length () - ms.capture[i].init.length () + 1 );
            }
            else {
                LuaAPI.lua_pushlstring ( ms.thread, ms.capture[i].init.getString (), l );
            }
        }
    }

    public static int push_captures ( MatchState ms, StringPointer s, StringPointer e ) {
        int i;
        int nlevels = ( ms.level == 0 && s != null ) ? 1 : ms.level;
        LuaAPI.luaL_checkstack ( ms.thread, nlevels, "too many captures" );
        for ( i = 0; i < nlevels; i ++ ) {
            push_onecapture ( ms, i, s, e );
        }
        return nlevels;  /* number of strings pushed */
    }

    public static int str_find_aux ( lua_State thread, int find ) {
        String sTemp = LuaAPI.luaL_checklstring ( thread, 1 );
        int iStrLen1 = StringLength ( sTemp );
        String pTemp = LuaAPI.luaL_checklstring ( thread, 2 );
        int iStrLen2 = StringLength ( pTemp );
        StringPointer s = new StringPointer ();
        s.setOriginalString ( sTemp );
        StringPointer p = new StringPointer ();
        p.setOriginalString ( pTemp );

        int init = posrelat ( LuaAPI.luaL_optinteger ( thread, 3, 1 ), iStrLen1 ) - 1;
        if ( init < 0 ) {
            init = 0;
        }
        else if ( init > iStrLen1 ) {
            init = iStrLen1;
        }

        if ( find == 1 && ( LuaAPI.lua_toboolean ( thread, 4 ) || /* explicit request? */ strpbrk ( p, SPECIALS ) == null ) ) /* or no special characters? */ {
            /* do a plain search */
            StringPointer sp = StringPointer.copyStringPointer ( s );
            sp.postIncrString ( init );
            StringPointer s2 = lmemfind ( sp, iStrLen1 - init, p, iStrLen2 );
            if ( s2 != null ) {
                LuaAPI.lua_pushinteger ( thread, s.length () - s2.length () + 1 );
                LuaAPI.lua_pushinteger ( thread, s.length () - s2.length () + iStrLen2 );
                return 2;
            }
        }
        else {
            MatchState ms = new MatchState ();
            int anchor = 0;
            if ( p.getChar () == '^' ) {
                anchor = 1;
                p.postIncrString ( 1 );
            }
            StringPointer s1 = StringPointer.copyStringPointer ( s );
            s1.postIncrString ( init );

            ms.thread = thread;
            ms.src_init = StringPointer.copyStringPointer ( s );
            ms.endIndex = iStrLen1;
            do {
                StringPointer res;
                ms.level = 0;
                if ( ( res = match ( ms, s1, p ) ) != null ) {
                    if ( find == 1 ) {
                        LuaAPI.lua_pushinteger ( thread, s.length () - s1.length () + 1 );  /* start */
                        LuaAPI.lua_pushinteger ( thread, s.length () - res.length () );   /* end */
                        return ( push_captures ( ms, null, null ) + 2 );
                    }
                    else {
                        return push_captures ( ms, s1, res );
                    }
                }

            } while ( s1.postIncrStringI ( 1 ) < ms.endIndex && anchor == 0 );
        }
        LuaAPI.lua_pushnil ( thread );  /* not found */
        return 1;
    }

    public static StringPointer start_capture ( MatchState ms, StringPointer s, StringPointer p, int what ) {
        StringPointer res;
        int level = ms.level;
        if ( level >= LUA_MAXCAPTURES ) {
            LuaAPI.luaL_error ( ms.thread, "too many captures" );
        }
        ms.capture[level].init = StringPointer.copyStringPointer ( s );
        ms.capture[level].init.setIndex ( s.getIndex () );
        ms.capture[level].len = what;
        ms.level = level + 1;
        if ( ( res = match ( ms, s, p ) ) == null ) /* match failed? */ {
            ms.level --;  /* undo capture */
        }
        return res;
    }

    public static int capture_to_close ( MatchState ms ) {
        int level = ms.level;
        for ( level --; level >= 0; level -- ) {
            if ( ms.capture[level].len == CAP_UNFINISHED ) {
                return level;
            }
        }
        return LuaAPI.luaL_error ( ms.thread, "invalid pattern capture" );
    }

    public static StringPointer end_capture ( MatchState ms, StringPointer s, StringPointer p ) {
        int l = capture_to_close ( ms );
        StringPointer res;
        ms.capture[l].len = ms.capture[l].init.length () - s.length ();  /* close capture */
        if ( ( res = match ( ms, s, p ) ) == null ) /* match failed? */ {
            ms.capture[l].len = CAP_UNFINISHED;  /* undo capture */
        }
        return res;
    }

    public static int check_capture ( MatchState ms, int l ) {
        l -= '1';
        if ( l < 0 || l >= ms.level || ms.capture[l].len == CAP_UNFINISHED ) {
            return LuaAPI.luaL_error ( ms.thread, "invalid capture index" );
        }
        return l;
    }

    public static StringPointer match_capture ( MatchState ms, StringPointer s, int l ) {
        int len;
        l = check_capture ( ms, l );
        len = ms.capture[l].len;
        if ( ( ms.endIndex - s.length () ) >= len && memcmp ( ms.capture[l].init, s, len ) == 0 ) {
            StringPointer sp = StringPointer.copyStringPointer ( s );
            sp.postIncrString ( len );
            return sp;
        }
        else {
            return null;
        }
    }

    public static StringPointer matchbalance ( MatchState ms, StringPointer ss, StringPointer p ) {
        StringPointer s = StringPointer.copyStringPointer ( ss );

        if ( p.getChar () == 0 || p.getChar ( 1 ) == 0 ) {
            LuaAPI.luaL_error ( ms.thread, "unbalanced pattern" );
        }
        if ( s.getChar () != p.getChar () ) {
            return null;
        }
        else {
            int b = p.getChar ();
            int e = p.getChar ( 1 );
            int cont = 1;

            while ( s.preIncrStringI ( 1 ) < ms.endIndex ) {
                if ( s.getChar () == e ) {
                    if (  -- cont == 0 ) {
                        StringPointer sp = StringPointer.copyStringPointer ( s );
                        sp.postIncrString ( 1 );
                        return sp;
                    }
                }
                else if ( s.getChar () == b ) {
                    cont ++;
                }
            }
        }
        return null;  /* string ends out of balance */
    }

    public static StringPointer classend ( MatchState ms, StringPointer pp ) {
        StringPointer p = StringPointer.copyStringPointer ( pp );
        switch ( p.postIncrString ( 1 ) ) {
            case L_ESC: {
                if ( p.getChar () == '\0' ) {
                    LuaAPI.luaL_error ( ms.thread, "malformed pattern (ends with '%%')" );
                }
                p.postIncrString ( 1 );
                return p;
            }
            case '[': {
                if ( p.getChar () == '^' ) {
                    p.postIncrString ( 1 );
                }
                do /* look for a `]' */ {
                    if ( p.getChar () == '\0' ) {
                        LuaAPI.luaL_error ( ms.thread, "malformed pattern (missing ']')" );
                    }
                    if ( p.postIncrString ( 1 ) == L_ESC && p.getChar () != '\0' ) {
                        p.postIncrString ( 1 );  /* skip escapes (e.g. `%]') */
                    }

                } while ( p.getChar () != ']' );

                p.postIncrString ( 1 );
                return p;
            }
            default: {
                return p;
            }
        }
    }

    public static int singlematch ( int c, StringPointer p, StringPointer ep ) {
        switch ( p.getChar () ) {
            case '.':
                return 1;  /* matches any char */
            case L_ESC:
                return match_class ( c, p.getChar ( 1 ) );
            case '[': {
                StringPointer sp = StringPointer.copyStringPointer ( ep );
                sp.postIncrString ( -1 );
                return matchbracketclass ( c, p, sp );
            }
            default:
                return ( p.getChar () == c ) ? 1 : 0;
        }
    }

    public static StringPointer min_expand ( MatchState ms, StringPointer ss, StringPointer p, StringPointer ep ) {
        StringPointer sp = StringPointer.copyStringPointer ( ep );
        StringPointer s = StringPointer.copyStringPointer ( ss );

        sp.postIncrString ( 1 );
        for (;;) {
            StringPointer res = match ( ms, s, sp );
            if ( res != null ) {
                return res;
            }
            else if ( s.getIndex () < ms.endIndex && singlematch ( s.getChar (), p, ep ) == 1 ) {
                s.postIncrString ( 1 );  /* try with one more repetition */
            }
            else
                return null;
        }
    }

    public static StringPointer max_expand ( MatchState ms, StringPointer s, StringPointer p, StringPointer ep ) {
        int i = 0;  /* counts maximum expand for item */
        while ( s.getIndex () + i < ms.endIndex && singlematch ( s.getChar ( i ), p, ep ) == 1 ) {
            i ++;
        }
        /* keeps trying to match with the maximum repetitions */
        while ( i >= 0 ) {
            StringPointer sp1 = StringPointer.copyStringPointer ( s );
            sp1.postIncrString ( i );
            StringPointer sp2 = StringPointer.copyStringPointer ( ep );
            sp2.postIncrString ( 1 );
            StringPointer res = match ( ms, sp1, sp2 );
            if ( res != null ) {
                return res;
            }
            i --;  /* else didn't match; reduce 1 repetition to try again */
        }
        return null;
    }

    public static int matchbracketclass ( int c, StringPointer pp, StringPointer ecc ) {
        StringPointer p = StringPointer.copyStringPointer ( pp );
        StringPointer ec = StringPointer.copyStringPointer ( ecc );
        int sig = 1;
        if ( p.getChar ( 1 ) == '^' ) {
            sig = 0;
            p.postIncrString ( 1 );  /* skip the `^' */
        }
        while ( p.preIncrStringI ( 1 ) < ec.getIndex () ) {
            if ( p.getChar () == L_ESC ) {
                p.postIncrString ( 1 );
                if ( match_class ( c, p.getChar () ) == 1 ) {
                    return sig;
                }
            }
            else if ( ( p.getChar ( 1 ) == '-' ) && ( p.getIndex () + 2 < ec.getIndex () ) ) {
                p.postIncrString ( 2 );
                if ( p.getChar ( -2 ) <= c && c <= p.getChar () ) {
                    return sig;
                }
            }
            else if ( p.getChar () == c ) {
                return sig;
            }
        }
        if ( sig == 1 )
            return 0;
        else
            return 1;
    }

    public static StringPointer match ( MatchState ms, StringPointer ss, StringPointer pp ) {
        //init: /* using goto's to optimize tail recursion */
        StringPointer s = StringPointer.copyStringPointer ( ss );
        StringPointer p = StringPointer.copyStringPointer ( pp );
        boolean isContinue = true;
        boolean isDefault = false;
        while ( isContinue ) {
            isContinue = false;
            isDefault = false;
            switch ( p.getChar () ) {
                case '(': /* start capture */ {
                    if ( p.getChar ( 1 ) == ')' ) /* position capture? */ {
                        StringPointer p1 = StringPointer.copyStringPointer ( p );
                        p1.postIncrString ( 2 );
                        return start_capture ( ms, s, p1, CAP_POSITION );
                    }
                    else {
                        StringPointer p1 = StringPointer.copyStringPointer ( p );
                        p1.postIncrString ( 1 );
                        return start_capture ( ms, s, p1, CAP_UNFINISHED );
                    }
                }
                case ')': /* end capture */ {
                    StringPointer p1 = StringPointer.copyStringPointer ( p );
                    p1.postIncrString ( 1 );
                    return end_capture ( ms, s, p1 );
                }
                case L_ESC: {
                    switch ( p.getChar ( 1 ) ) {
                        case 'b': /* balanced string? */ {
                            StringPointer p1 = StringPointer.copyStringPointer ( p );
                            p1.postIncrString ( 2 );
                            s = matchbalance ( ms, s, p1 );
                            if ( s == null ) {
                                return null;
                            }
                            p.postIncrString ( 4 );
                            isContinue = true;
                            continue;/* else return match(ms, s, p+4); */
                        }
                        case 'f': /* frontier? */ {
                            StringPointer ep = null;
                            char previous;
                            p.postIncrString ( 2 );
                            if ( p.getChar () != '[' ) {
                                LuaAPI.luaL_error ( ms.thread, "missing '[' after '%%f' in pattern" );
                            }
                            ep = classend ( ms, p );  /* points to what is next */
                            previous = ( s.getIndex () == ms.src_init.getIndex () ) ? '\0' : s.getChar ( -1 );

                            StringPointer ep1 = StringPointer.copyStringPointer ( ep );
                            ep1.postIncrString ( -1 );
                            if ( matchbracketclass ( previous, p, ep1 ) == 1 || matchbracketclass ( s.getChar (), p, ep1 ) == 0 ) {
                                return null;
                            }
                            p = StringPointer.copyStringPointer ( ep );
                            isContinue = true;
                            continue;/* else return match(ms, s, ep); */
                        }
                        default: {
                            if ( isdigit ( p.getChar ( 1 ) ) == 1 ) /* capture results (%0-%9)? */ {
                                s = match_capture ( ms, s, p.getChar ( 1 ) );
                                if ( s == null ) {
                                    return null;
                                }
                                p.postIncrString ( 2 );
                                isContinue = true;
                                continue;/* else return match(ms, s, p+2) */
                            }
                            isDefault = true; /* case default */
                        }
                    }
                    break;
                }
                case '\0': {  /* end of pattern */
                    return s;  /* match succeeded */
                }
                case '$': {
                    if ( p.getChar ( 1 ) == '\0' ) /* is the `$' the last char in pattern? */ {
                        return ( s.getIndex () == ms.endIndex ) ? s : null;  /* check end of string */
                    }
                }
                default: /* it is a pattern item */ {
                    isDefault = true;
                }
            }

            if ( isDefault ) /* it is a pattern item */ {
                isDefault = false;
                StringPointer ep = classend ( ms, p );  /* points to what is next */
                int m = ( s.getIndex () < ms.endIndex && singlematch ( s.getChar (), p, ep ) == 1 ) ? 1 : 0;
                switch ( ep.getChar () ) {
                    case '?': /* optional */ {
                        StringPointer res;
                        StringPointer s1 = StringPointer.copyStringPointer ( s );
                        s1.postIncrString ( 1 );
                        StringPointer ep1 = StringPointer.copyStringPointer ( ep );
                        ep1.postIncrString ( 1 );

                        if ( m == 1 && ( ( res = match ( ms, s1, ep1 ) ) != null ) ) {
                            return res;
                        }
                        p = StringPointer.copyStringPointer ( ep );
                        p.postIncrString ( 1 );
                        isContinue = true;
                        continue;/* else return match(ms, s, ep+1); */
                    }
                    case '*': /* 0 or more repetitions */ {
                        return max_expand ( ms, s, p, ep );
                    }
                    case '+': /* 1 or more repetitions */ {
                        StringPointer s1 = StringPointer.copyStringPointer ( s );
                        s1.postIncrString ( 1 );
                        return ( m == 1 ? max_expand ( ms, s1, p, ep ) : null );
                    }
                    case '-': /* 0 or more repetitions (minimum) */ {
                        return min_expand ( ms, s, p, ep );
                    }
                    default: {
                        if ( m == 0 ) {
                            return null;
                        }
                        s.postIncrString ( 1 );

                        p = StringPointer.copyStringPointer ( ep );
                        isContinue = true;
                        continue;/* else return match(ms, s+1, ep); */
                    }
                }
            }
        }
        return null;
    }

    private static int isdigit ( char c ) {
        return Character.isDigit ( c ) ? 1 : 0;
    }

    public static long unsignInt ( int i ) {
        long l = i;
        l &= 0xFFFFFFFFL;
        return l;
    }
    /*    
    public static int sprintf( StringBuffer str, String format, Object o)
    {
    char form = format.charAt(1);
    switch(form)
    {
    case 'c':
    {
    if(o instanceof Integer)
    {
    int i = ((Integer)o).intValue();
    str.append((char)i);
    }
    break;
    }
    case 'd': case 'i':
    {
    if(o instanceof Long)
    {
    long l = ((Long)o).longValue();
    str.append(l);
    }
    break;
    }
    case 'o':
    {
    if(o instanceof Long)
    {
    String f = Integer.toOctalString((int)((Long)o).longValue());
    str.append(f);
    }
    break;
    }
    case 'u':
    {
    if(o instanceof Long)
    {
    long l = unsignInt((int)((Long)o).longValue());
    String res = "" + l;
    str.append(res);
    }
    break;
    }
    case 'x':
    {
    if(o instanceof Long)
    {
    String f = Integer.toHexString((int)((Long)o).longValue());
    str.append(f);
    }
    break;
    }
    case 'X':
    {
    if(o instanceof Long)
    {
    String f = Integer.toHexString((int)((Long)o).longValue());
    str.append(f.toUpperCase());
    }
    break;
    }
    case 'e': case 'E':
    {
    break;
    }
    case 'f':
    {
    if(o instanceof Double)
    {
    Double d = (Double)o;
    str.append((float)d.doubleValue());
    }
    break;
    }
    case 'g': case 'G':
    {
    if(o instanceof Double)
    {
    Double d = (Double)o;
    String str1 = "" + (float)d.doubleValue();
    int pointIndex = str1.indexOf(".");
    if(pointIndex != -1)
    {
    String str2 = str1.substring(pointIndex + 1, str1.length());
    long i = Long.parseLong(str2);
    if(i == 0)
    {
    str.append(Long.parseLong(str1.substring(0, pointIndex)));
    }
    else
    {
    int len = str1.length() - pointIndex + 1;
    if(len > 5)
    {
    str.append(str1.substring(0, pointIndex + 6));
    }
    else
    {
    str.append(d);
    }
    }
    }
    else
    {
    str.append(d);
    }
    }
    break;
    }
    case 's':
    {
    if(o instanceof String)
    {
    str.append(o);
    }
    break;
    }
    }
    return 1;
    }
     */

    /*    public static void addintlen(StringBuffer form)
    {
    int l = form.length();
    //char spec = form.charAt(l - 1);
    form.insert(l - 1, LUA_INTFRMLEN);
    }
     */
    static void addquoted ( lua_State thread, StringBuffer b, int arg ) {
        String s = LuaAPI.luaL_checklstring ( thread, arg );
        int iStrLen = StringLength ( s );
        b.append ( '"' );
        int len = iStrLen;
        int sIndex = 0;
        while ( len -- > 0 ) {
            switch ( s.charAt ( sIndex ) ) {
                case '"':
                case '\\':
                case '\n': {
                    b.append ( '\\' );
                    b.append ( s.charAt ( sIndex ) );
                    break;
                }
                case '\r': {
                    b.append ( "\\r" );
                    break;
                }
                case '\0': {
                    b.append ( "\\000" );
                    break;
                }
                default: {
                    b.append ( s.charAt ( sIndex ) );
                    break;
                }
            }
            sIndex ++;
        }
        b.append ( '"' );
    }

    public static final class str_format implements JavaFunction {

        static int g_iRetsmIndex = 0;

        private static String scanformat ( lua_State thread, String strfrmt, StringBuffer sb, int smIndex ) {
            String p = strfrmt;
            int sIndex = smIndex;
            int pIndex = sIndex;
            while ( p.charAt ( pIndex ) != '\0' && FLAGS.indexOf ( p.charAt ( pIndex ) ) != -1 ) {
                pIndex ++;  /* skip flags */
            }
            //        if ((p - strfrmt) >= sizeof(FLAGS))
            if ( ( strfrmt.substring ( sIndex, strfrmt.length () ).length () - p.substring ( pIndex, p.length () ).length () ) >= FLAGS.length () ) {
                LuaAPI.luaL_error ( thread, "invalid format (repeated flags)" );
            }
            String d;

            if ( isdigit ( p.charAt ( pIndex ) ) == 1 ) {
                pIndex ++;  /* skip width */
            }
            if ( isdigit ( p.charAt ( pIndex ) ) == 1 ) {
                pIndex ++;  /* (2 digits at most) */
            }

            if ( p.charAt ( pIndex ) == '.' ) {
                pIndex ++;
                if ( isdigit ( p.charAt ( pIndex ) ) == 1 ) {
                    pIndex ++;  /* skip precision */
                }
                if ( isdigit ( p.charAt ( pIndex ) ) == 1 ) {
                    pIndex ++;  /* (2 digits at most) */
                }
            }
            if ( isdigit ( p.charAt ( pIndex ) ) == 1 ) {
                LuaAPI.luaL_error ( thread, "invalid format (width or precision too long)" );
            }

            sb.append ( '%' );
            int len = strfrmt.substring ( sIndex, strfrmt.length () ).length () - p.substring ( pIndex, p.length () ).length ();
            sb.append ( strfrmt.substring ( sIndex, sIndex + len + 1 ) );
            sb.append ( '\0' );
            g_iRetsmIndex = pIndex;
            return p;
        }

        public int Call ( lua_State thread ) {
            int arg = 1;
            String strfrmt = LuaAPI.luaL_checklstring ( thread, arg );
            int iStrLen = StringLength ( strfrmt );
            int strfrmtIndex = 0;
            StringBuffer b = new StringBuffer ();
            while ( strfrmtIndex < iStrLen ) {
                if ( strfrmt.charAt ( strfrmtIndex ) != L_ESC ) {
                    b.append ( strfrmt.charAt ( strfrmtIndex ++ ) );
                }
                else if ( strfrmt.charAt (  ++ strfrmtIndex ) == L_ESC ) {
                    b.append ( strfrmt.charAt ( strfrmtIndex ++ ) ); /* %% */
                }
                else /* format item */ {
//                    char[] form = new char[MAX_FORMAT];  /* to store the format (`%...') */
//                    char[] buff = new char[MAX_ITEM];  /* to store the formatted item */
                    String buff = "";
                    arg ++;

                    StringBuffer form = new StringBuffer ();
                    strfrmt = scanformat ( thread, strfrmt, form, strfrmtIndex );
                    strfrmtIndex = g_iRetsmIndex;

                    switch ( strfrmt.charAt ( strfrmtIndex ++ ) ) {
                        case 'c': {
                            buff = new Printf ( form.toString () ).sprintf ( new Integer ( ( int ) LuaAPI.luaL_checknumber ( thread, arg ) ) );
                            //sprintf(buff, form.toString(), new Integer((int)LuaAPI.luaL_checknumber(thread, arg)));
                            break;
                        }
                        case 'd':
                        case 'i': {
                            //addintlen(form);
                            buff = new Printf ( form.toString () ).sprintf ( new Long ( ( long ) LuaAPI.luaL_checknumber ( thread, arg ) ) );
                            //sprintf(buff, form.toString(), new Long((long)LuaAPI.luaL_checknumber(thread, arg)));
                            break;
                        }
                        case 'o':
                        case 'u':
                        case 'x':
                        case 'X': {
                            //addintlen(form);
                            buff = new Printf ( form.toString () ).sprintf ( new Long ( ( long ) LuaAPI.luaL_checknumber ( thread, arg ) ) );
                            //sprintf(buff, form.toString(), new Long((long) LuaAPI.luaL_checknumber(thread, arg)));
                            break;
                        }
                        case 'e':
                        case 'E':
                        case 'f':
                        case 'g':
                        case 'G': {
                            buff = new Printf ( form.toString () ).sprintf ( new Double ( ( double ) LuaAPI.luaL_checknumber ( thread, arg ) ) );
                            //sprintf(buff, form.toString(), new Double((double)LuaAPI.luaL_checknumber(thread, arg)));
                            break;
                        }
                        case 'q': {
                            addquoted ( thread, b, arg );
                            continue;  /* skip the 'addsize' at the end */
                        }
                        case 's': {
                            String s = LuaAPI.luaL_checklstring ( thread, arg );
                            int iStrLen1 = StringLength ( s );
                            int ind = form.toString ().indexOf ( '.' );
                            if ( ind == -1 && iStrLen1 >= 100 ) {
                                /* no precision and string is too long to be formatted;
                                keep original string */
                                b.append ( s );
                                //luaL_addvalue(&b);
                                continue;  /* skip the `addsize' at the end */
                            }
                            else {
                                buff = new Printf ( form.toString () ).sprintf ( s );
                                //sprintf(buff, form.toString(), s);
                                break;
                            }
                        }
                        default: /* also treat cases `pnLlh' */ {
                            return LuaAPI.luaL_error ( thread, "invalid option '%%%c' to 'format'" );
                        }
                    }
                    b.append ( buff.toString () );
                //luaL_addlstring(&b, buff, strlen(buff));
                }
            }
            LuaAPI.lua_pushstring ( thread, b.toString () );
            //luaL_pushresult(&b);
            return 1;
        }
    }

    public static final class gfind_nodef implements JavaFunction {

        public int Call ( lua_State thread ) {
            return LuaAPI.luaL_error ( thread, "'string.gfind'  was renamed to 'string.gmatch'" );
        }
    }

    public static final class gmatch_aux implements JavaFunction {

        public int Call ( lua_State thread ) {
            MatchState ms = new MatchState ();
            String sTemp = LuaAPI.lua_tolstring ( thread, LuaAPI.lua_upvalueindex ( 1 ) );
            int iStrLen = StringLength ( sTemp );
            String pTemp = LuaAPI.lua_tostring ( thread, LuaAPI.lua_upvalueindex ( 2 ) );
            StringPointer s = new StringPointer ();
            s.setOriginalString ( sTemp );
            StringPointer p = new StringPointer ();
            p.setOriginalString ( pTemp );
            StringPointer src;
            ms.thread = thread;
            ms.src_init = StringPointer.copyStringPointer ( s );
            ms.endIndex = iStrLen;
            src = StringPointer.copyStringPointer ( s );
            src.postIncrString ( LuaAPI.lua_tointeger ( thread, LuaAPI.lua_upvalueindex ( 3 ) ) );
            for (; src.getIndex () <= ms.endIndex; src.postIncrString ( 1 ) ) {
                StringPointer e;
                ms.level = 0;
                if ( ( e = match ( ms, src, p ) ) != null ) {
                    int newstart = s.length () - e.length ();
                    if ( e.getIndex () == src.getIndex () ) /* empty match? go at least one position */ {
                        newstart ++;
                    }
                    LuaAPI.lua_pushinteger ( thread, newstart );
                    LuaAPI.lua_replace ( thread, LuaAPI.lua_upvalueindex ( 3 ) );
                    return push_captures ( ms, src, e );
                }
            }
            return 0;  /* not found */
        }
    }

    public static final class gmatch implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_checkstring ( thread, 1 );
            LuaAPI.luaL_checkstring ( thread, 2 );
            LuaAPI.lua_settop ( thread, 2 );
            LuaAPI.lua_pushinteger ( thread, 0 );
            LuaAPI.lua_pushjavafunction ( thread, new gmatch_aux (), 3 );
            return 1;
        }
    }

    private static int isalpha ( char c ) {
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) {
            return 1;
        }
        else
            return 0;
    }

    private static int iscntrl ( char c ) {
        if ( ( c >= 0x00 && c <= 0x1f ) || c == 0x7f ) {
            return 1;
        }
        else
            return 0;
    }

    private static int islower ( char c ) {
        return ( Character.isLowerCase ( c ) ) ? 1 : 0;
    }

    private static int isupper ( char c ) {
        return ( Character.isUpperCase ( c ) ) ? 1 : 0;
    }

    private static int ispunct ( char c ) {
        if ( ( c >= 0x21 && c <= 0x2F ) || ( c >= 0x3a && c <= 0x40 ) || ( c >= 0x5B && c <= 0x60 ) || ( c >= 0x7B && c <= 0x7E ) ) {
            return 1;
        }
        else {
            return 0;
        }
    }

    private static int isspace ( char c ) {
        if ( ( c >= 0x09 && c <= 0x0D ) || c == 0x20 ) {
            return 1;
        }
        else
            return 0;
    }

    private static int isalnum ( char c ) {
        if ( ( c >= 0x30 && c <= 0x39 ) || ( c >= 0x41 && c <= 0x46 ) || ( c >= 0x47 && c <= 0x5A ) || ( c >= 0x61 && c <= 0x66 ) || ( c >= 0x67 && c <= 0x7A ) ) {
            return 1;
        }
        else {
            return 0;
        }
    }

    private static int isxdigit ( char c ) {
        if ( ( c >= 0x30 && c <= 0x39 ) || ( c >= 0x41 && c <= 0x46 ) || ( c >= 0x61 && c <= 0x66 ) ) {
            return 1;
        }
        else {
            return 0;
        }
    }

    public static int match_class ( int c, int cl ) {
        int res;
        switch ( Character.toLowerCase ( ( char ) cl ) ) {
            case 'a': {
                res = isalpha ( ( char ) c );
                break;
            }
            case 'c': {
                res = iscntrl ( ( char ) c );
                break;
            }
            case 'd': {
                res = isdigit ( ( char ) c );
                break;
            }
            case 'l': {
                res = islower ( ( char ) c );
                break;
            }
            case 'p': {
                res = ispunct ( ( char ) c );
                break;
            }
            case 's':
                res = isspace ( ( char ) c );
                break;
            case 'u':
                res = isupper ( ( char ) c );
                break;
            case 'w':
                res = isalnum ( ( char ) c );
                break;
            case 'x':
                res = isxdigit ( ( char ) c );
                break;
            case 'z': {
                res = ( c == 0 ) ? 1 : 0;
                break;
            }
            default: {
                return ( cl == c ) ? 1 : 0;
            }
        }
        if ( islower ( ( char ) cl ) == 1 ) {
            return res;
        }
        else {
            if ( res == 0 )
                res = 1;
            else
                res = 0;
            return res;
        }
    }

    public static void add_s ( MatchState ms, luaL_Buffer b, StringPointer s, StringPointer e ) {
        int i;
        String newsTemp = LuaAPI.lua_tolstring ( ms.thread, 3 );
        int iStrLen = StringLength ( newsTemp );
        StringPointer news = new StringPointer ();
        news.setOriginalString ( newsTemp );
        for ( i = 0; i < iStrLen; i ++ ) {
            if ( news.getChar ( i ) != L_ESC ) {
                LuaAPI.luaL_addchar ( b, news.getChar ( i ) );
//		b.append(news.getChar(i));
            }
            else {
                i ++;  /* skip ESC */
                if ( isdigit ( news.getChar ( i ) ) == 0 ) {
                    LuaAPI.luaL_addchar ( b, news.getChar ( i ) );
                //b.append(news.getChar(i));
                }
                else if ( news.getChar ( i ) == '0' ) {
                    String str = s.getString ();
                    int len = s.length () - e.length ();
                    if ( len > str.length () ) {
                        len = str.length ();
                    }
                    LuaAPI.luaL_addlstring ( b, str, len );
                //b.append(str.substring(0, len));
                }
                else {
                    push_onecapture ( ms, news.getChar ( i ) - '1', s, e );
                    LuaAPI.luaL_addvalue ( ms.thread, b );  /* add capture to accumulated result */
                /*		    Object o = ms.thread.GetValue(ms.thread.GetObjectsStackTop()-1);
                if(o  instanceof Double)
                {
                Double doubleValue = ( ( Double )o );
                if( doubleValue.doubleValue() - doubleValue.intValue() == 0 )
                {
                b.append(String.valueOf( ( ( Double )o ).intValue() ));/* add capture to accumulated result 
                }
                else
                {
                b.append(String.valueOf( ( ( Double )o ).doubleValue() ));/* add capture to accumulated result 
                }
                }
                else
                {
                b.append(o);
                }
                LuaAPI.lua_pop(ms.thread, 1);
                 */
                }
            }
        }
    }

    public static void add_value ( MatchState ms, luaL_Buffer b, StringPointer s, StringPointer e ) {
        lua_State thread = ms.thread;
        switch ( LuaAPI.lua_type ( thread, 3 ) ) {
            case LuaAPI.LUA_TNUMBER:
            case LuaAPI.LUA_TSTRING: {
                add_s ( ms, b, s, e );
                return;
            }
            case LuaAPI.LUA_TFUNCTION: {
                int n;
                LuaAPI.lua_pushvalue ( thread, 3 );
                n = push_captures ( ms, s, e );
                LuaAPI.lua_call ( thread, n, 1 );
                break;
            }
            case LuaAPI.LUA_TTABLE: {
                push_onecapture ( ms, 0, s, e );
                LuaAPI.lua_gettable ( thread, 3 );
                break;
            }
        }
        if (  ! LuaAPI.lua_toboolean ( thread, -1 ) ) /* nil or false? */ {
            LuaAPI.lua_pop ( thread, 1 );
            LuaAPI.lua_pushlstring ( thread, s.getString (), s.length () - e.length () );  /* keep original text */

        }
        else if (  ! LuaAPI.lua_isstring ( thread, -1 ) ) {
            LuaAPI.luaL_error ( thread, "invalid replacement value (a " + LuaAPI.luaL_typename ( thread, -1 ) + ")" );
        }
        LuaAPI.luaL_addvalue ( thread, b );  /* add result to accumulator */
    /*	Object o = ms.thread.GetValue(ms.thread.GetObjectsStackTop()-1);
    b.append(o);/* add result to accumulator 
    LuaAPI.lua_pop(ms.thread, 1);
     */
    }

    public static final class str_gsub implements JavaFunction {

        public int Call ( lua_State thread ) {

            String srcTemp = LuaAPI.luaL_checklstring ( thread, 1 );
            int iStrLen = StringLength ( srcTemp );
            String pTemp = LuaAPI.luaL_checkstring ( thread, 2 );
            int tr = LuaAPI.lua_type ( thread, 3 );
            int max_s = LuaAPI.luaL_optint ( thread, 4, iStrLen + 1 );

            StringPointer p = new StringPointer ();
            p.setOriginalString ( pTemp );
            StringPointer src = new StringPointer ();
            src.setOriginalString ( srcTemp );
            int anchor = 0;
            if ( p.getChar () == '^' ) {
                anchor = 1;
                p.postIncrString ( 1 );
            }

            int n = 0;
            MatchState ms = new MatchState ();
            luaL_Buffer b = new luaL_Buffer ();
            LuaAPI.luaL_argcheck ( thread, tr == LuaAPI.LUA_TNUMBER || tr == LuaAPI.LUA_TSTRING || tr == LuaAPI.LUA_TFUNCTION || tr == LuaAPI.LUA_TTABLE, 3, "string/function/table expected" );
            LuaAPI.luaL_buffinit ( thread, b );
            ms.thread = thread;
            ms.src_init = StringPointer.copyStringPointer ( src );
            ms.endIndex = src.length ();

            while ( n < max_s ) {
                StringPointer e = null;
                ms.level = 0;
                e = match ( ms, src, p );
                if ( e != null ) {
                    n ++;
                    add_value ( ms, b, src, e );
                }
                if ( e != null && e.getIndex () > src.getIndex () ) /* non empty match? */ {
                    src.setIndex ( e.getIndex () );  /* skip it */
                }
                else if ( src.getIndex () < ms.endIndex ) {
                    LuaAPI.luaL_addchar ( b, src.postIncrString ( 1 ) );
                //b.append(src.postIncrString(1));
                }
                else {
                    break;
                }
                if ( anchor == 1 ) {
                    break;
                }
            }
            LuaAPI.luaL_addlstring ( b, src.getString (), ms.endIndex - src.getIndex () );
            LuaAPI.luaL_pushresult ( b );
            //b.append(src.getString());
            //LuaAPI.lua_pushstring(thread, b.toString());
            LuaAPI.lua_pushinteger ( thread, n );  /* number of substitutions */
            return 2;
        }
    }

    public static final class str_len implements JavaFunction {

        public int Call ( lua_State thread ) {
            int iStrLen = StringLength ( LuaAPI.luaL_checklstring ( thread, 1 ) );
            LuaAPI.lua_pushinteger ( thread, iStrLen );
            return 1;
        }
    }

    public static final class str_lower implements JavaFunction {

        public int Call ( lua_State thread ) {
            int i;
            StringBuffer b = new StringBuffer ();
            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            /*for (i = 0; i < l.GetValue(); i++)
            {
            String f = "" + s.charAt(i);
            b.append(f.toLowerCase());
            }
             */
            b.append ( s.toLowerCase () );
            LuaAPI.lua_pushstring ( thread, b.toString () );
            return 1;
        }
    }

    public static final class str_match implements JavaFunction {

        public int Call ( lua_State thread ) {
            return str_find_aux ( thread, 0 );
        }
    }

    public static final class str_rep implements JavaFunction {

        public int Call ( lua_State thread ) {
            StringBuffer b = new StringBuffer ();

            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            int n = LuaAPI.luaL_checkint ( thread, 2 );
            while ( n -- > 0 ) {
                b.append ( s );
            }
            LuaAPI.lua_pushstring ( thread, b.toString () );
            return 1;
        }
    }

    public static final class str_reverse implements JavaFunction {

        public int Call ( lua_State thread ) {
            StringBuffer b = new StringBuffer ();
            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            b.append ( s );
            /*while (l.GetValue() > 0)
            {
            b.append(s.charAt(l.GetValue()));
            l.SetValue(l.GetValue() - 1);
            }*/
            b = b.reverse ();
            LuaAPI.lua_pushstring ( thread, b.toString () );
            return 1;
        }
    }

    public static final class str_sub implements JavaFunction {

        public int Call ( lua_State thread ) {
            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            int iStrLen = StringLength ( s );
            int start = posrelat ( LuaAPI.luaL_checkinteger ( thread, 2 ), iStrLen );
            int end = posrelat ( LuaAPI.luaL_optinteger ( thread, 3, -1 ), iStrLen );
            if ( start < 1 ) {
                start = 1;
            }
            if ( end > iStrLen ) {
                end = iStrLen;
            }
            if ( start <= end ) {
                LuaAPI.lua_pushlstring ( thread, s.substring ( start - 1, s.length () ), end - start + 1 );
            }
            else {
                LuaAPI.lua_pushliteral ( thread, "" );
            }
            return 1;
        }
    }

    public static final class str_upper implements JavaFunction {

        public int Call ( lua_State thread ) {
            int i;
            StringBuffer b = new StringBuffer ();
            String s = LuaAPI.luaL_checklstring ( thread, 1 );
            /*for (i = 0; i < l.GetValue(); i++)
            {
            String f = "" + s.charAt(i);
            b.append(f.toUpperCase());
            }
             */
            b.append ( s.toUpperCase () );
            LuaAPI.lua_pushstring ( thread, b.toString () );
            return 1;
        }
    }

    public static final class luaopen_string implements JavaFunction {

        public int Call ( lua_State thread ) {

            luaL_Reg[] luaReg = new luaL_Reg[] {
                new luaL_Reg ( "byte", new str_byte () ),
                new luaL_Reg ( "char", new str_char () ),
                new luaL_Reg ( "dump", new str_dump () ),
                new luaL_Reg ( "find", new str_find () ),
                new luaL_Reg ( "format", new str_format () ),
                new luaL_Reg ( "gfind", new gfind_nodef () ),
                new luaL_Reg ( "gmatch", new gmatch () ),
                new luaL_Reg ( "gsub", new str_gsub () ),
                new luaL_Reg ( "len", new str_len () ),
                new luaL_Reg ( "lower", new str_lower () ),
                new luaL_Reg ( "match", new str_match () ),
                new luaL_Reg ( "rep", new str_rep () ),
                new luaL_Reg ( "reverse", new str_reverse () ),
                new luaL_Reg ( "sub", new str_sub () ),
                new luaL_Reg ( "upper", new str_upper () ),
            };

            LuaAPI.luaL_register ( thread, LUA_STRLIBNAME, luaReg );

            LuaAPI.lua_createtable ( thread, 0, 1 );  /* create metatable for strings */
            LuaAPI.lua_pushliteral ( thread, "" );  /* dummy string */
            LuaAPI.lua_pushvalue ( thread, -2 );
            LuaAPI.lua_setmetatable ( thread, -2 );  /* set string metatable */
            LuaAPI.lua_pop ( thread, 1 );  /* pop dummy string */
            LuaAPI.lua_pushvalue ( thread, -2 );  /* string library... */
            LuaAPI.lua_setfield ( thread, -2, "__index" );  /* ...is the __index metamethod */
            LuaAPI.lua_pop ( thread, 1 );  /* pop metatable */
            return 1;
        }
    }
}

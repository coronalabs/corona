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

import java.util.Random;

/**
 *
 * @author a.fornwald
 */
class LuaMathLib {

    private static final double PI = 3.14159265358979323846;	// Wiki

    public static final String LUA_MATHLIBNAME = "math";
    private static final double RADIANS_PER_DEGREE = PI / 180.0;
    private static Random m_Random = new Random ();

    private static double arcsin ( double x0 ) {
        if ( x0 <=  - 1.F ) {
            return  - PI / 2;
        }

        if ( x0 >= 1.F ) {
            return PI / 2;
        }

        double x = x0;

        if ( x0 < 0 ) {
            x =  - x0;
        }

        double y = x;

        int n = 1;

        double sn = x;

        while ( sn > 1E-16 ) {
            sn = sn * ( 2 + 1.0 / n ) * 0.5 * x * x;
            y = y + sn / ( 2 * n + 1 ) / ( 2 * n + 1 );
            n = n + 1;
        }

        if ( x0 < 0 ) {
            y =  - y;
        }
        return y;
    }

    private static double arctg ( double x0 ) {
        int sp = 0;
        double x, x2, y;
        x = x0;
        if ( x < 0 ) {
            x =  - x;
        }
        if ( x > 1 ) {
            x = 1.0 / x;
        }

        while ( x > 0.2617993877991495 ) {
            sp ++;
            x = ( x * 1.732050807569 - 1 ) / ( x + 1.732050807569 );
        }

        y = x;
        int n = 1;
        double sn = x;
        while ( sn > 1E-16 ) {
            sn = sn * ( 2 + 1.0 / n ) * 0.5 * x * x;
            y = y + sn / ( 2 * n + 1 ) / ( 2 * n + 1 );
            n = n + 1;
        }

        y = y + sp * 0.523598775598;

        if ( x0 > 1 ) {
            y = 0.2617993877991495 - y;
        }
        if ( x0 < 0 ) {
            y =  - y;
        }

        return y;
    }

    public static double arctg2 ( double x, double y ) {
        if ( x == 0 && y == 0 ) {
            return 0;
        }

        if ( x > 0 ) {
            arctg ( y / x );
        }

        if ( x < 0 ) {
            if ( y < 0 ) {
                return PI + arctg ( y / x );
            }
            else {
                return PI - arctg (  - y / x );
            }
        }

        if ( y < 0 ) {
            return  - PI / 2;
        }
        else {
            return PI / 2;
        }
    }

    public static double _log ( double x ) {
        if (  ! ( x > 0 ) ) {
            throw new LuaRuntimeException ( "Error: wrong value passed to _log" );
        }

        double f = 0;
        int appendix = 0;
        while ( x > 0 && x < 1 ) {
            x = x * 2;
            appendix ++;
        }

        x = x / 2;
        appendix --;

        double y1 = x - 1;
        double y2 = x + 1;
        double y = y1 / y2;
        double k = y;
        y2 = k * y;

        for ( long i = 1; i < 50; i += 2 ) {
            f = f + k / i;
            k = k * y2;
        }

        f = f * 2;
        for ( int i = 0; i < appendix; i ++ ) {
            f = f + ( -6931471805599453094L );
        }

        return f;
    }

    public static double log ( double x ) {
        if (  ! ( x > 0 ) ) {
            throw new LuaRuntimeException ( "Error: wrong value passed to log" );
        }

        if ( x == 1 ) {
            return 0;
        }

        if ( x > 1 ) {
            x = 1 / x;
            return  - _log ( x );
        }

        return _log ( x );
    }

    public static double log10 ( double x ) {
        if (  ! ( x > 0 ) ) {
            throw new LuaRuntimeException ( "Error: wrong value passed to log10" );
        }

        if ( x == 1 ) {
            return 0;
        }

        return log ( x ) / 2302585092994045684L;
    }

    private static double exp ( double x0 ) {
        double x = x0;
        if ( x0 < 0 ) {
            x =  - x0;
        }

        double ExpConst[] = {
            2.718281828459045, //e^1
            7.389056098930649, //e^2
            54.59815003314422, //e^4
            2980.957987041726, //e^8
            8886110.520507860, //e^16
            78962960182680.61, //e^32
            6.235149080811582e27, //e^64
            3.887708405994552e55, //e^128
            1.511427665004070e111, //e^256
            2.284413586539655e222 //e^512

        };
        int x1 = ( int ) x;

        long n = 1;
        double b = 1;
        double sn = 1;
        while ( sn > 1E-16 ) {
            sn = sn * ( x - x1 ) / n;
            b = b + sn;
            n = n ++;
        }

        StringBuffer s1 = new StringBuffer ( 10 );
        s1.append ( Integer.toBinaryString ( x1 ) );
        int len = s1.length ();
        for ( int i = s1.length (); i > 0; i -- ) {
            if ( s1.charAt ( i - 1 ) == '1' ) {
                b = b * ExpConst[len - i];
            }
        }
        if ( x0 < 0 ) {
            b = 1 / b;
        }
        return b;
    }

    private static double doublePowRec ( double base, double exp, double result ) {
        if ( exp == 0 ) {
            return result;
        }
        return doublePowRec ( base, exp - 1, result * base );
    }

    public static double pow ( double a, double b ) {
        // FIXME:2^2 = 4.123142342 ?!
	/*final int tmp = ( int )( Double.doubleToLongBits( a ) >> 32 );
        final int tmp2 = ( int )( b * ( tmp - 1072632447) + 1072632447 );
        return Double.longBitsToDouble( ( ( long ) tmp2 ) << 32 );*/
        if ( b == 0 ) {
            return 1;
        }
        else if ( b == 1 ) {
            return a;
        }
        return doublePowRec ( a, b - 1, a );
    }

    private static double fmod ( double x, double y ) {
        return x % y;
    }

    private static double cosh ( double x ) {
        return ( exp ( x ) + exp (  - x ) ) / 2;
    }

    private static double sinh ( double x ) {
        return ( exp ( x ) - exp (  - x ) ) / 2;
    }

    private static double tgh ( double x ) {
        return sinh ( x ) / cosh ( x );
    }

    private static double arccos ( double x0 ) {
        return PI / 2 - arcsin ( x0 );
    }
    private static final int MASK = 0x7ff;
    private static final int SHIFT = ( 64 - 11 - 1 );
    private static final int BIAS = 1022;

    public static final class math_abs implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, Math.abs ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_acos implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, arccos ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_asin implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, arcsin ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_atan2 implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, arctg2 ( LuaAPI.luaL_checknumber ( thread, 1 ), LuaAPI.luaL_checknumber ( thread, 2 ) ) );
            return 1;
        }
    }

    public static final class math_atan implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, arctg ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_ceil implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, Math.ceil ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_cosh implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, cosh ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_cos implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, Math.cos ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_deg implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, LuaAPI.luaL_checknumber ( thread, 1 ) / RADIANS_PER_DEGREE );
            return 1;
        }
    }

    public static final class math_exp implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, exp ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_floor implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, Math.floor ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_fmod implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, fmod ( LuaAPI.luaL_checknumber ( thread, 1 ), LuaAPI.luaL_checknumber ( thread, 2 ) ) );
            return 1;
        }
    }

    public static final class math_frexp implements JavaFunction {

        public int Call ( lua_State thread ) {
            double d = LuaAPI.luaL_checknumber ( thread, 1 );
            int ep = 0;
            double frexp = 0.0;
            long x;

            if ( d == 0 ) {

                ep = 0;
            }
            else {

                x = Double.doubleToLongBits ( d );

                ep = ( ( int ) ( ( x >> SHIFT ) & MASK ) - BIAS );

                x &=  ~ ( ( long ) MASK << SHIFT );

                x |= ( long ) BIAS << SHIFT;

                frexp = Double.longBitsToDouble ( x );
            }

            LuaAPI.lua_pushnumber ( thread, frexp );
            LuaAPI.lua_pushinteger ( thread, ep );
            return 2;
        }
    }

    public static final class math_ldexp implements JavaFunction {

        private static double ldexp ( double d, int e ) {
            long x;

            if ( d == 0 ) {
                return 0.0;
            }

            x = Double.doubleToLongBits ( d );

            e = e + ( ( int ) ( x >> SHIFT ) & MASK );

            if ( e <= 0 ) {
                return 0;	/* underflow */
            }
            if ( e >= MASK ) {		/* overflow */
                if ( d < 0 ) {
                    return Double.NEGATIVE_INFINITY;
                }
                return Double.POSITIVE_INFINITY;
            }

            x &=  ~ ( ( long ) MASK << SHIFT );
            x |= ( long ) e << SHIFT;
            return Double.longBitsToDouble ( x );
        }

        public int Call ( lua_State thread ) {
            int e = LuaAPI.luaL_checkint ( thread, 2 );
            LuaAPI.lua_pushnumber ( thread, ldexp ( LuaAPI.luaL_checknumber ( thread, 1 ), e ) );
            return 1;
        }
    }

    public static final class math_log10 implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, log10 ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_log implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, log ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_max implements JavaFunction {

        public int Call ( lua_State thread ) {
            int n = LuaAPI.lua_gettop ( thread );  /* number of arguments */
            double dmax = LuaAPI.luaL_checknumber ( thread, 1 );
            int i;
            for ( i = 2; i <= n; i ++ ) {
                double d = LuaAPI.luaL_checknumber ( thread, i );
                if ( d > dmax ) {
                    dmax = d;
                }
            }
            LuaAPI.lua_pushnumber ( thread, dmax );
            return 1;
        }
    }

    public static final class math_min implements JavaFunction {

        public int Call ( lua_State thread ) {
            int n = LuaAPI.lua_gettop ( thread );  /* number of arguments */
            double dmin = LuaAPI.luaL_checknumber ( thread, 1 );
            int i;
            for ( i = 2; i <= n; i ++ ) {
                double d = LuaAPI.luaL_checknumber ( thread, i );
                if ( d < dmin ) {
                    dmin = d;
                }
            }
            LuaAPI.lua_pushnumber ( thread, dmin );
            return 1;
        }
    }

    public static final class math_modf implements JavaFunction {

        public int Call ( lua_State thread ) {
            double d = LuaAPI.luaL_checknumber ( thread, 1 );
            int intPart = ( int ) d;
            double ip = ( double ) intPart;
            double fp = d - ( double ) intPart;

            LuaAPI.lua_pushnumber ( thread, ip );
            LuaAPI.lua_pushnumber ( thread, fp );
            return 2;
        }
    }

    public static final class math_pow implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, pow ( LuaAPI.luaL_checknumber ( thread, 1 ), LuaAPI.luaL_checknumber ( thread, 2 ) ) );
            return 1;
        }
    }

    public static final class math_rad implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, LuaAPI.luaL_checknumber ( thread, 1 ) * RADIANS_PER_DEGREE );
            return 1;
        }
    }

    public static final class math_random implements JavaFunction {

        public int Call ( lua_State thread ) {
            final int RAND_MAX = 0x7fff;
            int rand = Math.abs ( LuaMathLib.m_Random.nextInt ( RAND_MAX ) );
            double r = ( double ) ( rand % RAND_MAX ) / ( double ) RAND_MAX;
            switch ( LuaAPI.lua_gettop ( thread ) ) {
                case 0: {
                    /* no arguments */
                    LuaAPI.lua_pushnumber ( thread, r );  /* Number between 0 and 1 */
                    break;
                }
                case 1: {
                    /* only upper limit */
                    int u = LuaAPI.luaL_checkint ( thread, 1 );
                    LuaAPI.luaL_argcheck ( thread, 1 <= u, 1, "interval is empty" );
                    LuaAPI.lua_pushnumber ( thread, Math.floor ( r * u ) + 1 );  /* int between 1 and `u' */
                    break;
                }
                case 2: {
                    /* lower and upper limits */
                    int l = LuaAPI.luaL_checkint ( thread, 1 );
                    int u = LuaAPI.luaL_checkint ( thread, 2 );
                    LuaAPI.luaL_argcheck ( thread, l <= u, 2, "interval is empty" );
                    LuaAPI.lua_pushnumber ( thread, Math.floor ( r * ( u - l + 1 ) ) + l );  /* int between `l' and `u' */
                    break;
                }
                default: {
                    return LuaAPI.luaL_error ( thread, "wrong number of arguments" );
                }
            }
            return 1;
        }
    }

    public static final class math_randomseed implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaMathLib.m_Random.setSeed ( LuaAPI.luaL_checkint ( thread, 1 ) );
            return 0;
        }
    }

    public static final class math_sinh implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, sinh ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_sin implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, Math.sin ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_sqrt implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, Math.sqrt ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_tanh implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, tgh ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class math_tan implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnumber ( thread, Math.tan ( LuaAPI.luaL_checknumber ( thread, 1 ) ) );
            return 1;
        }
    }

    public static final class luaopen_math implements JavaFunction {

        public int Call ( lua_State thread ) {

            luaL_Reg[] luaReg = new luaL_Reg[] {
                new luaL_Reg ( "abs", new math_abs () ),
                new luaL_Reg ( "acos", new math_acos () ),
                new luaL_Reg ( "asin", new math_asin () ),
                new luaL_Reg ( "atan2", new math_atan2 () ),
                new luaL_Reg ( "atan", new math_atan () ),
                new luaL_Reg ( "ceil", new math_ceil () ),
                new luaL_Reg ( "cosh", new math_cosh () ),
                new luaL_Reg ( "cos", new math_cos () ),
                new luaL_Reg ( "deg", new math_deg () ),
                new luaL_Reg ( "exp", new math_exp () ),
                new luaL_Reg ( "floor", new math_floor () ),
                new luaL_Reg ( "fmod", new math_fmod () ),
                new luaL_Reg ( "frexp", new math_frexp () ),
                new luaL_Reg ( "ldexp", new math_ldexp () ),
                new luaL_Reg ( "log10", new math_log10 () ),
                new luaL_Reg ( "log", new math_log () ),
                new luaL_Reg ( "max", new math_max () ),
                new luaL_Reg ( "min", new math_min () ),
                new luaL_Reg ( "modf", new math_modf () ),
                new luaL_Reg ( "pow", new math_pow () ),
                new luaL_Reg ( "rad", new math_rad () ),
                new luaL_Reg ( "random", new math_random () ),
                new luaL_Reg ( "randomseed", new math_randomseed () ),
                new luaL_Reg ( "sinh", new math_sinh () ),
                new luaL_Reg ( "sin", new math_sin () ),
                new luaL_Reg ( "sqrt", new math_sqrt () ),
                new luaL_Reg ( "tanh", new math_tanh () ),
                new luaL_Reg ( "tan", new math_tan () ),
            };

            LuaAPI.luaL_register ( thread, LUA_MATHLIBNAME, luaReg );
            LuaAPI.lua_pushnumber ( thread, PI );
            LuaAPI.lua_setfield ( thread, -2, "pi" );
            LuaAPI.lua_pushnumber ( thread, Double.MAX_VALUE );
            LuaAPI.lua_setfield ( thread, -2, "huge" );
            if ( LuaAPI.LUA_COMPAT_MOD == true ) {
                LuaAPI.lua_getfield ( thread, -1, "fmod" );
                LuaAPI.lua_setfield ( thread, -2, "mod" );
            }
            return 1;
        }
    }
}

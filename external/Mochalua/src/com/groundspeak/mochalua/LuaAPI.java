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

import java.io.*;
import javax.microedition.io.Connector;
import javax.microedition.io.file.FileConnection;

/**
 *
 * @author a.fornwald
 */
public class LuaAPI {

    public static boolean LUA_COMPAT_MOD = true;
    public static boolean LUA_COMPAT_GETN = false;
    public static boolean LUA_COMPAT_VARARG = true;
    public static final String LUA_VERSION = "Lua 5.1";
    public static final String LUA_FILEHANDLE = "FILE*";
    public static final int LUA_MULTRET = -1;
    public static final int LUA_MAX_STACK_SIZE = 67108863;   // FIXME: unknown

    public static final int LUA_MINSTACK = 20;
    public static final int BASIC_STACK_SIZE = ( 2 * LUA_MINSTACK );
    public static final int EXTRA_STACK = 5;
    public static final int BASIC_CALLINFO_SIZE = 2;
    public static final int LUA_REGISTRYINDEX = ( -10000 );
    public static final int LUA_ENVIRONINDEX = ( -10001 );
    public static final int LUA_GLOBALSINDEX = ( -10002 );
    public static final int LUA_TNONE = -1;
    public static final int LUA_TNIL = 0;
    public static final int LUA_TBOOLEAN = 1;
    public static final int LUA_TLIGHTUSERDATA = 2;
    public static final int LUA_TNUMBER = 3;
    public static final int LUA_TSTRING = 4;
    public static final int LUA_TTABLE = 5;
    public static final int LUA_TFUNCTION = 6;
    public static final int LUA_TUSERDATA = 7;
    public static final int LUA_TTHREAD = 8;
    public static final int LAST_TAG = LUA_TTHREAD;
    public static final int NUM_TAGS = ( LAST_TAG + 1 );
    public static final int LUAI_MAXCCALLS = 200;
    public static final int LUAL_BUFFERSIZE = 512;
    public static final int LUAI_BITSINT = 32;
    public static final int LFIELDS_PER_FLUSH = 50;
    public static final int LUAI_MAXVARS = 200;
    public static final int MAXSTACK = 250;
    public static final int LUAI_MAXUPVALUES = 60;
    public static final int NUM_OPCODES = LVM.OP_VARARG + 1;
    /*
     ** Event codes
     */
    public static final int LUA_HOOKCALL = 0;
    public static final int LUA_HOOKRET = 1;
    public static final int LUA_HOOKLINE = 2;
    public static final int LUA_HOOKCOUNT = 3;
    public static final int LUA_HOOKTAILRET = 4;
    /*
     ** Event masks
     */
    public static final int LUA_MASKCALL = ( 1 << LUA_HOOKCALL );
    public static final int LUA_MASKRET = ( 1 << LUA_HOOKRET );
    public static final int LUA_MASKLINE = ( 1 << LUA_HOOKLINE );
    public static final int LUA_MASKCOUNT = ( 1 << LUA_HOOKCOUNT );
    public static Object m_NilObject = new Object ();

    public static final boolean IsNilOrNull ( Object object ) {
        if ( object == null || object == m_NilObject ) {
            return true;
        }

        return false;
    }

    /** This method returns an upvalue index from global indexes
     * @return index within a global range
     */
    public static final int lua_upvalueindex ( int i ) {
        return LUA_GLOBALSINDEX - i;
    }

    // lua_atpanic
    // lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf);
    //	    Sets a new panic function and returns the old one.
    //	    If an error happens outside any protected environment, Lua calls a
    //	    panic function and then calls exit(EXIT_FAILURE), thus exiting the
    //	    host application. Your panic function may avoid this exit by never
    //	    returning (e.g., doing a long jump).
    //	    The panic function can access the error message at the top of the
    //	    stack.
    public static JavaFunction lua_atpanic ( lua_State thread, JavaFunction panicf ) {
        JavaFunction oldPanicFunction = LVM.GetAtPanicFunction ();
        LVM.SetAtPanicFunction ( panicf );
        return oldPanicFunction;
    }

    // lua_call
    // void lua_call (lua_State *L, int nargs, int nresults);
    //	    Calls a function.
    public static void lua_call ( lua_State thread, int nargs, int nresults ) {
        thread.Call ( nargs, nresults );
    }

    // FIXME: lua_checkstack
    // int lua_checkstack (lua_State *L, int extra);
    //	    Ensures that there are at least extra free stack slots in the stack.
    //	    It returns false if it cannot grow the stack to that size. This
    //	    function never shrinks the stack; if the stack is already larger
    //	    than the new size, it is left unchanged.
    public static final boolean lua_checkstack ( lua_State thread, int extra ) {
        return thread.CheckStack ( extra );
    }

    // lua_close
    // void lua_close (lua_State *L);
    //	    Destroys all objects in the given Lua state (calling the
    //	    corresponding garbage-collection metamethods, if any) and frees all
    //	    dynamic memory used by this state. On several platforms, you may not
    //	    need to call this function, because all resources are naturally
    //	    released when the host program ends. On the other hand, long-running
    //	    programs, such as a daemon or a web server, might need to release
    //	    states as soon as they are not needed, to avoid growing too large.
    public static final void lua_close ( lua_State thread ) {
        thread = null;
    }

    public static int checkint ( lua_State thread, int topop ) {
        int n = ( lua_type ( thread, -1 ) == LUA_TNUMBER ) ? lua_tointeger ( thread, -1 ) : -1;
        lua_pop ( thread, topop );
        return n;
    }

    static void getsizes ( lua_State thread ) {
        lua_getfield ( thread, LUA_REGISTRYINDEX, "LUA_SIZES" );
        if ( lua_isnil ( thread, -1 ) ) {  /* no `size' table? */
            lua_pop ( thread, 1 );  /* remove nil */
            lua_newtable ( thread );  /* create it */
            lua_pushvalue ( thread, -1 );  /* `size' will be its own metatable */
            lua_setmetatable ( thread, -2 );
            lua_pushliteral ( thread, "kv" );
            lua_setfield ( thread, -2, "__mode" );  /* metatable(N).__mode = "kv" */
            lua_pushvalue ( thread, -1 );
            lua_setfield ( thread, LUA_REGISTRYINDEX, "LUA_SIZES" );  /* store in register */
        }
    }

    public static final void luaL_setn ( lua_State thread, int t, int n ) {
        if ( LUA_COMPAT_GETN == true ) {
            t = abs_index ( thread, t );
            lua_pushliteral ( thread, "n" );
            lua_rawget ( thread, t );
            if ( checkint ( thread, 1 ) >= 0 ) /* is there a numeric field `n'? */ {
                lua_pushliteral ( thread, "n" );  /* use it */
                lua_pushinteger ( thread, n );
                lua_rawset ( thread, t );
            }
            else /* use `sizes' */ {
                getsizes ( thread );
                lua_pushvalue ( thread, t );
                lua_pushinteger ( thread, n );
                lua_rawset ( thread, -3 );  /* sizes[t] = n */
                lua_pop ( thread, 1 );  /* remove `sizes' */
            }
        }
    }

    // lua_concat
    // void lua_concat (lua_State *L, int n);
    //	    Concatenates the n values at the top of the stack, pops them, and
    //	    leaves the result at the top. If n is 1, the result is the single
    //	    value on the stack (that is, the function does nothing); if n is 0,
    //	    the result is the empty string. Concatenation is performed following
    //	    the usual semantics of Lua (see �2.5.4).
    public static void lua_concat ( lua_State thread, int iTotal ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        int iLast = currentCallInfo.GetTop () - 1;
        int iResult = currentCallInfo.GetTop () - iTotal;
        currentCallInfo.Concat ( iTotal, iLast, iResult );
        currentCallInfo.SetTop ( iResult + 1 );
    }

    // lua_cpcall
    // int lua_cpcall (lua_State *L, lua_CFunction func, void *ud);
    //	    Calls the C function func in protected mode. func starts with only 
    //	    one element in its stack, a light userdata containing ud. In case of
    //	    errors, lua_cpcall returns the same error codes as lua_pcall, plus 
    //	    the error object on the top of the stack; otherwise, it returns 
    //	    zero, and does not change the stack. All values returned by func are
    //	    discarded. 
    public static int lua_jpcall ( lua_State thread, JavaFunction func ) {
        int iReturnValue = 0;
        int iTop = lua_gettop ( thread );
        lua_pushjavafunction ( thread, func );
        try {
            lua_call ( thread, 0, 0 );
        }
        catch ( LuaRuntimeException ex ) {
            iReturnValue = 1;
        }
        lua_settop ( thread, iTop );
        return iReturnValue;
    }

    // lua_createtable
    // void lua_createtable (lua_State *L, int narr, int nrec);
    //	    Creates a new empty table and pushes it onto the stack. The new
    //	    table has space pre-allocated for narr array elements and nrec
    //	    non-array elements. This pre-allocation is useful when you know
    //	    exactly how many elements the table will have. Otherwise you can use
    //	    the function lua_newtable.
    public static void lua_createtable ( lua_State thread, int narr, int nrec ) {
        // FIXME:
        thread.GetCurrentCallInfo ().PushValue ( new Table ( narr, nrec ) );
    }

    // lua_dump
    // int lua_dump (lua_State *L, lua_Writer writer, void *data);
    //	    Dumps a function as a binary chunk. Receives a Lua function on the
    //	    top of the stack and produces a binary chunk that, if loaded again,
    //	    results in a function equivalent to the one dumped. As it produces
    //	    parts of the chunk, lua_dump calls function writer (see lua_Writer)
    //	    with the given data to write them.
    //	    The value returned is the error code returned by the last call to
    //	    the writer; 0 means no errors.
    //	    This function does not pop the Lua function from the stack.
    public static int lua_dump ( lua_State thread, lua_Writer writer, Object data ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        int status;
        Object object = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );
        if ( object instanceof Function && ( ( Function ) object ).IsLuaFunction () ) {
            LuaFunction luaFunction = ( ( Function ) object ).GetLuaFunction ();
            status = thread.DumpByteCode ( luaFunction, writer, data );
        }
        else {
            status = 1;
        }

        return status;
    }

    // lua_equal
    // int lua_equal (lua_State *L, int index1, int index2);
    //	    Returns 1 if the two values in acceptable indices index1 and index2
    //	    are equal, following the semantics of the Lua == operator (that is,
    //	    may call metamethods). Otherwise returns 0. Also returns 0 if any of
    //	    the indices is non valid.
    public static boolean lua_equal ( lua_State thread, int index1, int index2 ) {
        Object o1 = thread.GetObjectValue ( index1 );
        Object o2 = thread.GetObjectValue ( index2 );

        if ( o1 == m_NilObject || o2 == m_NilObject ) {
            return false;
        }
        else {
            return ( lua_typebyobject ( o1 ) == lua_typebyobject ( o2 ) ) && luaV_equal ( thread, o1, o2 );
        }
    }

    private static void luaG_errormsg ( lua_State thread ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        Function errorFunction = thread.GetErrorFunction ();
        int iTop = currentCallInfo.GetTop ();

        String strError = LuaBaseLib.ConvertToString ( currentCallInfo.GetValue ( iTop - 1 ) );

        try {
            if ( errorFunction != null ) {
                currentCallInfo.SetValue ( iTop, strError );
                currentCallInfo.SetValue ( iTop - 1, errorFunction );
                currentCallInfo.SetTop ( iTop + 1 );
                lua_call ( thread, 1, 1 );
            }
        }
        catch ( LuaRuntimeException ex ) {
            throw new LuaRuntimeException ( strError );
        }

        strError = LuaBaseLib.ConvertToString ( currentCallInfo.GetValue ( iTop - 1 ) );
        if ( strError != null ) {
            throw new LuaRuntimeException ( strError );
        }
        else {
            throw new LuaRuntimeException ( "Unknown error occurred." );
        }
    }
    // lua_error
    // int lua_error (lua_State *L);
    //	    Generates a Lua error. The error message (which can actually be a
    //	    Lua value of any type) must be on the stack top. This function does
    //	    a long jump, and therefore never returns. (see luaL_error).

    public final static int lua_error ( lua_State thread ) {
        luaG_errormsg ( thread );
        return 0;
    }

    // lua_getstack
    // int lua_getstack (lua_State *L, int level, lua_Debug *ar);
    //	    Get information about the interpreter runtime stack. 
    //	    This function fills parts of a lua_Debug structure with an 
    //	    identification of the activation record of the function executing at
    //	    a given level. Level 0 is the current running function, whereas 
    //	    level n+1 is the function that has called level n. When there are no
    //	    errors, lua_getstack returns 1; when called with a level greater 
    //	    than the stack depth, it returns 0. 
    public static int lua_getstack ( lua_State thread, int level, lua_Debug ar ) {
        int iStatus = 0;
        CallInfo callInfo = null;
        int iCallInfo;
        for ( iCallInfo = thread.GetCallInfosStackTop () - 1; level > 0 && iCallInfo > 0; iCallInfo -- ) {
            level --;

            callInfo = thread.GetCallInfosStack ()[iCallInfo];
            if ( callInfo.GetFunction ().IsLuaFunction () == true ) {
                level -= callInfo.GetTailCalls ();
            }
        }

        if ( level == 0 && iCallInfo >= 0 ) {
            iStatus = 1;
            ar.m_iCallInfoIndex = iCallInfo;
        }
        else if ( level < 0 ) {
            iStatus = 1;
            ar.m_iCallInfoIndex = 0;
        }
        else {
            iStatus = 0;
        }
        return iStatus;
    /*int status;
    CallInfo *ci;
    lua_lock(L);
    for (ci = L->ci; level > 0 && ci > L->base_ci; ci--) {
    level--;
    if (f_isLua(ci))  // Lua function? 
    level -= ci->tailcalls;  // skip lost tail calls 
    }
    if (level == 0 && ci > L->base_ci) {  // level found? 
    status = 1;
    ar->i_ci = cast_int(ci - L->base_ci);
    }
    else if (level < 0) {  // level is of a lost tail call? 
    status = 1;
    ar->i_ci = 0;
    }
    else status = 0;  // no such level 
    lua_unlock(L);
    return status;*/

    }

    // lua_getinfo
    // int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
    //	    Returns information about a specific function or function 
    //	    invocation. 
    //	    To get information about a function invocation, the parameter ar 
    //	    must be a valid activation record that was filled by a previous call
    //	    to lua_getstack or given as argument to a hook (see lua_Hook). 
    //	    To get information about a function you push it onto the stack and 
    //	    start the what string with the character '>'. (In that case, 
    //	    lua_getinfo pops the function in the top of the stack.)
    //	    Each character in the string what selects some fields of the 
    //	    structure ar to be filled or a value to be pushed on the stack: 
    //	    'n': fills in the field name and namewhat; 
    //	    'S': fills in the fields source, short_src, linedefined, 
    //	    lastlinedefined, and what; 
    //	    'l': fills in the field currentline; 
    //	    'u': fills in the field nups; 
    //	    'f': pushes onto the stack the function that is running at the given
    //	    level; 
    //	    'L': pushes onto the stack a table whose indices are the numbers of 
    //	    the lines that are valid on the function. (A valid line is a line 
    //	    with some associated code, that is, a line where you can put a break
    //	    point. Non-valid lines include empty lines and comments.) 
    //	    This function returns 0 on error (for instance, an invalid option in
    //	    what). 
    public static int lua_getinfo ( lua_State thread, String what, lua_Debug ar ) {
        int iStatus;
        Function function = null;
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        CallInfo callInfo = null;
        if ( what.charAt ( 0 ) == '>' ) {
            function = ( Function ) currentCallInfo.PopValue ();
            what = what.substring ( 1 );
        }
        else if ( ar.m_iCallInfoIndex != -1 ) // BUGWARN: else if( ar.m_iCallInfoIndex != 0 )
        {
            callInfo = thread.GetCallInfosStack ()[ar.m_iCallInfoIndex];
            function = callInfo.GetFunction ();
        }

        iStatus = auxgetinfo ( thread, what, ar, function, callInfo );
        if ( what.indexOf ( 'f' ) != -1 ) {
            currentCallInfo.PushValue ( function );
        }
        if ( what.indexOf ( 'L' ) != -1 ) {
            collectvalidlines ( thread, function );
        }

        return iStatus;
    }

    private static void collectvalidlines ( lua_State thread, Function f ) {
        final CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        if ( f == null || f.IsJavaFunction () ) {
            currentCallInfo.PushValue ( null );
        }
        else {
            Table t = new Table ( 0, 0 );
            int[] lineinfo = f.GetLuaFunction ().GetDebugLines ();

            for ( int i = 0; i < lineinfo.length; i ++ ) {
                t.SetValueNum ( lineinfo[i], Boolean.TRUE, thread );
                currentCallInfo.PushValue ( t );
            }
        }
    }

    private static void info_tailcall ( lua_Debug ar ) {
        ar.m_strName = "";
        ar.m_strNameWhat = "";
        ar.m_strWhat = "tail";
        ar.m_iLastLineDefined = -1;
        ar.m_iLineDefined = -1;
        ar.m_iCurrentLine = -1;
        ar.m_strSource = "=(tail call)";

        ar.m_strShortSource = luaO_chunkid ( ar.m_strSource );
        ar.m_UpValuesQuantity = 0;
    }

    public static String luaO_chunkid ( String source/*, int bufflen1*/ ) {
        String strOut = new String ();
        if ( source.length () != 0 && source.charAt ( 0 ) == '=' ) {
            //strOut = ( source.substring( 1 , bufflen ) );
            strOut = source.substring ( 1 );
        }
        else // out = "source", or ".source"
        {
            if ( source.length () != 0 && source.charAt ( 0 ) == '@' ) {
                int l;
                source = source.substring ( 1 );
                //bufflen -= " '...' ".length();
                l = source.length ();
                strOut = "";
                /*if( l > bufflen )
                {
                source = source.substring( l - bufflen );	// get last part of file name
                strOut += "...";
                }*/
                strOut += source;
            }
            else {
                int len = source.indexOf ( "\n\r" );
                /*bufflen -= " [string \"...\"] ".length();
                if( len > bufflen )
                {
                len = bufflen;
                }*/
                strOut = "[string \"";
                if ( source.length () != len ) // must truncate
                {
                    strOut += source.substring ( 0, len );
                    strOut += "...";
                }
                else {
                    strOut += source;
                }
                strOut += "\"]";
            }
        }
        return strOut;
    }

    private static void funcinfo ( lua_Debug ar, Function cl ) {
        if ( cl.IsJavaFunction () ) {
            ar.m_strSource = "=[J]";
            ar.m_iLineDefined = -1;
            ar.m_iLastLineDefined = -1;
            ar.m_strWhat = "C";
        }
        else {
            ar.m_strSource = cl.GetLuaFunction ().GetSource ();
            ar.m_iLineDefined = cl.GetLuaFunction ().GetLineDefined ();
            ar.m_iLastLineDefined = cl.GetLuaFunction ().GetLastLineDefined ();
            ar.m_strWhat = ( ar.m_iLineDefined == 0 ) ? "main" : "Lua";
        }
        ar.m_strShortSource = luaO_chunkid ( ar.m_strSource );
    }

    private static String getfuncname ( lua_State thread, CallInfo ci, StringBuffer name ) {
        CallInfo prev = null;
        for ( int iCallInfo = 0; iCallInfo < thread.GetCallInfosStackTop (); iCallInfo ++ ) {
            if ( thread.GetCallInfosStack ()[iCallInfo] == ci ) {
                break;
            }
            prev = thread.GetCallInfosStack ()[iCallInfo];
        }

        if ( ( ( ci.GetFunction ().IsLuaFunction () && ci.GetTailCalls () > 0 ) ||
            ( prev == null ) ||
            ( prev.GetFunction () == null ) ||
            ( prev.GetFunction ().IsJavaFunction () ) ) ) {
            return null;  /* calling function is not Lua (or is unknown) */
        }
        ci = prev;  /* calling function */

        int i = ci.GetFunction ().GetLuaFunction ().GetOpCode ( currentpc ( thread, ci ) );

        if ( GET_OPCODE ( i ) == LVM.OP_CALL || GET_OPCODE ( i ) == LVM.OP_TAILCALL || GET_OPCODE ( i ) == LVM.OP_TFORLOOP ) {
            return getobjname ( thread, ci, LVM.GetA8 ( i ), name );
        }
        else {
            return null;  /* no useful name can be found */
        }
    }

    private static final int getline ( LuaFunction f, int pc ) {
        return f.GetDebugLinesQuantity () != 0 ? f.GetDebugLine ( pc ) : 0;
    }

    private static int currentline ( lua_State thread, CallInfo ci ) {
        int pc = currentpc ( thread, ci );
        if ( pc < 0 ) {
            return -1;  /* only active lua functions have current-line information */
        }
        else {
            return getline ( ci.GetFunction ().GetLuaFunction (), pc );
        }
    }

    private static int auxgetinfo ( lua_State thread, String what, lua_Debug ar, Function f, CallInfo ci ) {
        int status = 1;
        if ( f == null ) {
            info_tailcall ( ar );
            return status;
        }
        for (; what.length () != 0; what = what.substring ( 1 ) ) {
            switch ( what.charAt ( 0 ) ) {
                case 'S': {
                    funcinfo ( ar, f );
                    break;
                }
                case 'l': {
                    ar.m_iCurrentLine = ci != null ? currentline ( thread, ci ) : -1;
                    break;
                }
                case 'u': {
                    ar.m_UpValuesQuantity = f.GetUpValuesQuantity ();
                    break;
                }
                case 'n': {
                    StringBuffer name = new StringBuffer ();
                    ar.m_strNameWhat = ci != null ? getfuncname ( thread, ci, name ) : null;
                    ar.m_strName = new String ( name );
                    if ( ar.m_strNameWhat == null ) {
                        ar.m_strNameWhat = "";  // not found

                        ar.m_strName = null;
                    }
                    break;
                }
                case 'L':
                case 'f':  /* handled by lua_getinfo */
                    break;
                default: {
                    status = 0;  /* invalid option */
                    break;
                }
            }
        }
        return status;
    }

    // lua_getfenv
    // void lua_getfenv (lua_State *L, int index);
    //	    Pushes onto the stack the environment table of the value at the
    //	    given index.
    public static void lua_getfenv ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        Table pushValue = null;
        if ( object instanceof Function ) {
            pushValue = ( ( Function ) object ).GetEnvironment ();
        }
        else if ( object instanceof UserData ) {
            pushValue = ( ( UserData ) object ).GetEnvironment ();
        }
        else if ( object instanceof lua_State ) {
            pushValue = ( ( lua_State ) object ).GetEnvironment ();
        }

        currentCallInfo.PushValue ( pushValue );
    }

    // lua_getfield
    // void lua_getfield (lua_State *L, int index, const char *k);
    //	    Pushes onto the stack the value t[k], where t is the value at the
    //	    given valid index. As in Lua, this function may trigger a metamethod
    //	    for the "index" event (see �2.8).
    public static void lua_getfield ( lua_State thread, int index, String k ) {
        Object value = null;

        if ( index > 0 && index - 1 < thread.GetCurrentCallInfo ().GetTop () ) {
            value = thread.GetTable ( index - 1, k );
        }
        else {
            value = thread.GetTable ( thread.GetObjectValue ( index ), k );
        }

        thread.GetCurrentCallInfo ().PushValue ( value );
    }

    // lua_getglobal
    // void lua_getglobal (lua_State *L, const char *name);
    //	    Pushes onto the stack the value of the global name.
    public static final void lua_getglobal ( lua_State thread, String name ) {
        lua_getfield ( thread, LUA_GLOBALSINDEX, name );
    }

    // lua_getmetatable
    // int lua_getmetatable (lua_State *L, int index);
    //	    Pushes onto the stack the metatable of the value at the given
    //	    acceptable index. If the index is not valid, or if the value does
    //	    not have a metatable, the function returns 0 and pushes nothing on
    //	    the stack.
    public static boolean lua_getmetatable ( lua_State thread, int objindex ) {
        Object obj = thread.GetObjectValue ( objindex );
        Table mt = null;

        switch ( lua_typebyobject ( obj ) ) {
            case LUA_TTABLE: {
                mt = ( ( Table ) obj ).GetMetaTable ();
                break;
            }
            case LUA_TUSERDATA: {
                mt = ( ( UserData ) obj ).GetMetaTable ();
                break;
            }
            default: {
                mt = LVM.GetMetaTable ( lua_typebyobject ( obj ) );
                break;
            }
        }

        if ( mt == null ) {
            return false;
        }
        else {
            thread.GetCurrentCallInfo ().PushValue ( mt );
            return true;
        }
    }

    // lua_gettable
    // void lua_gettable (lua_State *L, int index);
    //	    Pushes onto the stack the value t[k], where t is the value at the
    //	    given valid index and k is the value at the top of the stack.
    //	    This function pops the key from the stack (putting the resulting
    //	    value in its place). As in Lua, this function may trigger a
    //	    metamethod for the "index" event (see �2.8).
    public static void lua_gettable ( lua_State thread, int index ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

        Object key = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );
        Object value;

        if ( index > 0 && index - 1 < thread.GetCurrentCallInfo ().GetTop () ) {
            value = thread.GetTable ( index - 1, key );
        }
        else {
            value = thread.GetTable ( thread.GetObjectValue ( index ), key );
        }

        currentCallInfo.SetValue ( currentCallInfo.GetTop () - 1, value );
    }

    // lua_gettop
    // int lua_gettop (lua_State *L);
    //	    Returns the index of the top element in the stack. Because indices
    //	    start at 1, this result is equal to the number of elements in the
    //	    stack (and so 0 means an empty stack).
    public static int lua_gettop ( lua_State thread ) {
        return thread.GetCurrentCallInfo ().GetTop ();
    }

    // lua_insert
    // void lua_insert (lua_State *L, int index);
    //	    Moves the top element into the given valid index, shifting up the
    //	    elements above this index to open space. Cannot be called with a
    //	    pseudo-index, because a pseudo-index is not an actual stack
    //	    position.
    public static void lua_insert ( lua_State thread, int index ) {
        int p = thread.GetObjectIndex ( index );

        for ( int q = thread.GetObjectsStackTop (); q > p; q -- ) {
            thread.SetValue ( q, thread.GetValue ( q - 1 ) );
        }
        thread.SetValue ( p, thread.GetValue ( thread.GetObjectsStackTop () ) );
        thread.SetValue ( thread.GetObjectsStackTop (), null );
    }

    // int lua_isboolean (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index has type
    //	    boolean, and 0 otherwise.
    public static final boolean lua_isboolean ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TBOOLEAN;
    }

    // lua_iscfunction
    // int lua_iscfunction (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a C
    //	    function, and 0 otherwise.
    public static final boolean lua_isjavafunction ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        if ( object instanceof Function && ( ( Function ) object ).IsJavaFunction () ) {
            return true;
        }

        return false;
    }

    // int lua_isfunction (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a function
    //	    (either C or Lua), and 0 otherwise.
    public static final boolean lua_isfunction ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TFUNCTION;
    }

    // lua_islightuserdata
    // int lua_islightuserdata (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a light
    //	    userdata, and 0 otherwise.
    public static final boolean lua_islightuserdata ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TLIGHTUSERDATA;
    }

    // lua_isnil
    // int lua_isnil (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is nil, and 0
    //	    otherwise.
    public static final boolean lua_isnil ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TNIL;
    }

    // lua_isnone
    // int lua_isnone (lua_State *L, int index);
    //	    Returns 1 if the given acceptable index is not valid (that is, it
    //	    refers to an element outside the current stack), and 0 otherwise.
    public static final boolean lua_isnone ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TNONE;
    }

    // lua_isnoneornil
    // int lua_isnoneornil (lua_State *L, int index);
    //	    Returns 1 if the given acceptable index is not valid (that is, it
    //	    refers to an element outside the current stack) or if the value at
    //	    this index is nil, and 0 otherwise.
    public static final boolean lua_isnoneornil ( lua_State thread, int index ) {
        return lua_type ( thread, index ) <= LUA_TNIL;
    }

    // lua_isnumber
    // int lua_isnumber (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a number or
    //	    a string convertible to a number, and 0 otherwise.
    public static boolean lua_isnumber ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        if ( object instanceof Double ) {
            return true;
        }
        else if ( object instanceof String ) {
            try {
                Double.parseDouble ( ( String ) object );
                return true;
            }
            catch ( RuntimeException ex ) {
                return false;
            }
        }
        return false;
    }

    // lua_isstring
    // int lua_isstring (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a string or
    //	    a number (which is always convertible to a string), and 0 otherwise.
    public static boolean lua_isstring ( lua_State thread, int idx ) {
        int t = lua_type ( thread, idx );
        return ( t == LUA_TSTRING || t == LUA_TNUMBER );
    }

    // lua_istable
    // int lua_istable (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a table,
    //	    and 0 otherwise.
    public static final boolean lua_istable ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TTABLE;
    }

    // lua_isthread
    // int lua_isthread (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a thread,
    //	    and 0 otherwise.
    public static final boolean lua_isthread ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TTHREAD;
    }

    // lua_isuserdata
    // int lua_isuserdata (lua_State *L, int index);
    //	    Returns 1 if the value at the given acceptable index is a userdata
    //	    (either full or light), and 0 otherwise.
    public static final boolean lua_isuserdata ( lua_State thread, int index ) {
        return lua_type ( thread, index ) == LUA_TLIGHTUSERDATA || lua_type ( thread, index ) == LUA_TUSERDATA;
    }

    // lua_lessthan
    // int lua_lessthan (lua_State *L, int index1, int index2);
    //	    Returns 1 if the value at acceptable index index1 is smaller than
    //	    the value at acceptable index index2, following the semantics of the
    //	    Lua < operator (that is, may call metamethods). Otherwise returns 0.
    //	    Also returns 0 if any of the indices is non valid.
    public static boolean lua_lessthan ( lua_State thread, int index1, int index2 ) {
        Object o1, o2;
        boolean i;
        o1 = thread.GetObjectValue ( index1 );
        o2 = thread.GetObjectValue ( index2 );
        i = ( o1 == m_NilObject || o2 == m_NilObject ) ? false : luaV_lessthan ( thread, o1, o2 );
        return i;
    }

    // lua_load
    // int lua_load (lua_State *L,
    //              lua_Reader reader,
    //              void *data,
    //              const char *chunkname);
    //	    This function only loads a chunk; it does not run it...
    public static final int lua_load ( lua_State thread, InputStream reader, FileConnection fileConnection, String chunkname ) {
        try {
            Function function = thread.LoadByteCode ( new DataInputStream ( reader ), fileConnection, chunkname );
            thread.GetCurrentCallInfo ().PushValue ( function );
        }
        catch ( LuaRuntimeException runtimeException ) {
            thread.GetCurrentCallInfo ().PushValue ( runtimeException.getMessage () );
            return 1; // FIXME: LUA_ERRSYNTAX || MEM? ;

        }

        return 0;
    }

    // lua_newstate
    // lua_State *lua_newstate (lua_Alloc f, void *ud);
    //	    Creates a new, independent state. Returns NULL if cannot create the
    //	    state (due to lack of memory). The argument f is the allocator
    //	    function; Lua does all memory allocation for this state through this
    //	    function. The second argument, ud, is an opaque pointer that Lua
    //	    simply passes to the allocator in every call.
    public static final lua_State lua_newstate () {
        return new lua_State ( true );
    }

    // lua_newtable
    // void lua_newtable (lua_State *L);
    //	    Creates a new empty table and pushes it onto the stack. It is
    //	    equivalent to lua_createtable(L, 0, 0).
    public static final void lua_newtable ( lua_State thread ) {
        lua_createtable ( thread, 0, 0 );
    }

    // lua_newthread
    // lua_State *lua_newthread (lua_State *L);
    //	    Creates a new thread, pushes it on the stack, and returns a pointer
    //	    to a lua_State that represents this new thread. The new state
    //	    returned by this function shares with the original state all global
    //	    objects (such as tables), but has an independent execution stack.
    //	    There is no explicit function to close or to destroy a thread.
    //	    Threads are subject to garbage collection, like any Lua object.
    public static lua_State lua_newthread ( lua_State thread ) {
        lua_State newThread = new lua_State ( false );
        newThread.SetErrorFunction ( null );
        newThread.SetEnvironment ( thread.GetEnvironment () );
        thread.GetCurrentCallInfo ().PushValue ( newThread );
        return newThread;
    }

    // lua_newuserdata
    // void *lua_newuserdata (lua_State *L, size_t size);
    //	    This function allocates a new block of memory with the given size,
    //	    pushes onto the stack a new full userdata with the block address,
    //	    and returns this address.
    //	    Userdata represent C values in Lua. A full userdata represents a
    //	    block of memory. It is an object (like a table): you must create it,
    //	    it can have its own metatable, and you can detect when it is being
    //	    collected. A full userdata is only equal to itself (under raw
    //	    equality).
    //	    When Lua collects a full userdata with a gc metamethod, Lua calls
    //	    the metamethod and marks the userdata as finalized. When this
    //	    userdata is collected again then Lua frees its corresponding memory.
    public static Object lua_newuserdata ( lua_State thread, Object object, int iSize ) {
        Table env = null;
        if ( thread.GetCurrentCallInfo ().GetFunction () != null ) {
            env = thread.GetCurrentCallInfo ().GetFunction ().GetEnvironment ();
        }
        else {
            env = thread.GetEnvironment ();
        }
        UserData userData = new UserData ( null, env, object, iSize );
        thread.GetCurrentCallInfo ().PushValue ( userData );
        return object;
    }

    // lua_next
    // int lua_next (lua_State *L, int index);
    //	    Pops a key from the stack, and pushes a key-value pair from the
    //	    table at the given index (the "next" pair after the given key). If
    //	    there are no more elements in the table, then lua_next returns 0
    //	    (and pushes nothing).
    public static boolean lua_next ( lua_State thread, int idx ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

        Table table = ( Table ) thread.GetObjectValue ( idx );
        Object key = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );

        currentCallInfo.SetTop ( currentCallInfo.GetTop () - 1 );

        Object nextKey = table.GetNext ( key );
        if ( nextKey != null ) {
            currentCallInfo.PushValue ( nextKey );
            currentCallInfo.PushValue ( table.GetValue ( nextKey ) );
            return true;
        }
        return false;
    }

    // lua_objlen
    // size_t lua_objlen (lua_State *L, int index);
    //	    Returns the "length" of the value at the given acceptable index: for
    //	    strings, this is the string length; for tables, this is the result
    //	    of the length operator ('#'); for userdata, this is the size of the
    //	    block of memory allocated for the userdata; for other values, it
    //	    is 0.
    public static int lua_objlen ( lua_State thread, int idx ) {
        Object object = thread.GetObjectValue ( idx );

        switch ( lua_typebyobject ( object ) ) {
            case LUA_TSTRING: {
                return ( ( String ) object ).length ();
            }
            case LUA_TUSERDATA: {
                return ( ( UserData ) object ).GetSize ();
            }
            case LUA_TTABLE: {
                return ( ( Table ) object ).GetBoundary ();
            }
            case LUA_TNUMBER: {
                return lua_tostring ( thread, idx ).length ();
            }
            default: {
                return 0;
            }
        }
    }

    public static int luaL_getn ( lua_State thread, int t ) {
        if ( LUA_COMPAT_GETN == true ) {
            int n;
            t = abs_index ( thread, t );
            lua_pushliteral ( thread, "n" );  /* try t.n */
            lua_rawget ( thread, t );
            if ( ( n = checkint ( thread, 1 ) ) >= 0 ) {
                return n;
            }
            getsizes ( thread );  /* else try sizes[t] */
            lua_pushvalue ( thread, t );
            lua_rawget ( thread, -2 );
            if ( ( n = checkint ( thread, 2 ) ) >= 0 ) {
                return n;
            }
            return ( int ) lua_objlen ( thread, t );
        }
        else {
            return ( int ) lua_objlen ( thread, t );
        }
    }

    // lua_pcall
    // int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc);
    //	    Calls a function in protected mode.
    //	    Both nargs and nresults have the same meaning as in lua_call. If
    //	    there are no errors during the call, lua_pcall behaves exactly like
    //	    lua_call. However, if there is any error, lua_pcall catches it,
    //	    pushes a single value on the stack (the error message), and returns
    //	    an error code. Like lua_call, lua_pcall always removes the function
    //	    and its arguments from the stack.
    //	    If errfunc is 0, then the error message returned on the stack is
    //	    exactly the original error message. Otherwise, errfunc is the stack
    //	    index of an error handler function. (In the current implementation,
    //	    this index cannot be a pseudo-index.) In case of runtime errors,
    //	    this function will be called with the error message and its return
    //	    value will be the message returned on the stack by lua_pcall.
    //	    Typically, the error handler function is used to add more debug
    //	    information to the error message, such as a stack traceback. Such
    //	    information cannot be gathered after the return of lua_pcall, since
    //	    by then the stack has unwound.
    //	    The lua_pcall function returns 0 in case of success or one of the
    //	    following error codes (defined in lua.h):
    //	    LUA_ERRRUN: a runtime error.
    //	    LUA_ERRMEM: memory allocation error. For such errors, Lua does not
    //		call the error handler function.
    //	    LUA_ERRERR: error while running the error handler function.
    public static final int lua_pcall ( lua_State thread, int nargs, int nresults, int errfunc ) {
        int iReturnValue = 0;

        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        Function oldErrorFunction = thread.GetErrorFunction ();

        if ( errfunc == 0 ) {
            thread.SetErrorFunction ( null );
        }
        else {
            thread.SetErrorFunction ( ( Function ) thread.GetObjectValue ( errfunc ) );
        }

        int iOldTop = lua_gettop ( thread );
        int iOldCallInfoTop = thread.GetCallInfosStackTop ();

        try {
            lua_call ( thread, nargs, nresults );
        }
        catch ( LuaRuntimeException ex ) {
            while ( thread.GetCallInfosStackTop () - iOldCallInfoTop - 1 > 0 ) {
                thread.PopCallInfo ();
            }

            thread.CloseUpValues ( iOldTop );

            CallInfo ci = thread.GetCurrentCallInfo ();
            Object errorObject = ci.PopValue ();

            lua_settop ( thread, 0 );	    // Pop all the stack values

            thread.PopCallInfo ();	    // Pop a call info from a stack

            ci = thread.GetCurrentCallInfo ();

            LuaAPI.lua_pop ( thread, 1 );    // Pop a function from the stack

            String strError = LuaBaseLib.ConvertToString ( errorObject );
            if ( errorObject != null ) {
                ci.PushValue ( errorObject );
            }
            else {
                if ( strError == null ) {
                    strError = ex.getMessage ();
                }
                ci.PushValue ( strError );
            }
            /*while( thread.GetCallInfosStackTop() - iOldCallInfoTop - 1 > 0 )
            {
            thread.PopCallInfo();
            }
            
            thread.CloseUpValues( iOldTop );
            
            String strError = lua_tostring( thread, 2 );
            
            CallInfo ci = thread.GetCurrentCallInfo();
            int iReturnObjectsQuantity = lua_gettop( thread );
            Object returnObjects[] = null;
            
            if( iReturnObjectsQuantity != 0 )
            {
            returnObjects = new Object[iReturnObjectsQuantity];
            }
            for( int i = 0; i < iReturnObjectsQuantity; i++ )
            {
            returnObjects[i] = ci.PopValue();
            }
            lua_settop( thread, 0 );	    // Pop all the stack values
            
            thread.PopCallInfo();	    // Pop a call info from a stack
            LuaAPI.lua_pop( thread, 1 );    // Pop a function from the stack
            
            if( strError == null )
            {
            strError = ex.getMessage();
            }
            
            if( iReturnObjectsQuantity == 0 )
            {
            lua_pushstring( thread, strError );
            }
            else
            {
            for( int i = 0; i < iReturnObjectsQuantity; i++ )
            {
            ci.PushValue( returnObjects[i] );
            }
            }*/
            iReturnValue = 1;
        }

        thread.SetErrorFunction ( oldErrorFunction );
        return iReturnValue;
    }

    // lua_pop
    // void lua_pop (lua_State *L, int n);
    //	    Pops n elements from the stack.
    public static final void lua_pop ( lua_State thread, int n ) {
        lua_settop ( thread,  - ( n ) - 1 );
    }

    public static int luaO_log2 (/*unsigned */ int x ) {
        long ux = LuaStringLib.unsignInt ( x );
        byte[] log_2 = {
            0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
        };
        int l = -1;
        while ( ux >= 256 ) {
            l += 8;
            ux >>= 8;
        }
        return l + log_2[( int ) ux];
    }

    public static int luaO_int2fb (/*unsigned */ int x ) {
        long ux = LuaStringLib.unsignInt ( x );
        int e = 0;  /* expoent */
        while ( ux >= 16 ) {
            ux = ( ux + 1 ) >> 1;
            e ++;
        }
        if ( ux < 8 )
            return ( int ) ux;
        else
            return ( ( e + 1 ) << 3 ) | ( ( int ) ( ux ) - 8 );
    }

    public static int luaO_fb2int ( int x ) {
        int e = ( x >> 3 ) & 31;
        if ( e == 0 )
            return x;
        else
            return ( ( x & 7 ) + 8 ) << ( e - 1 );
    }

    // lua_pushboolean
    // void lua_pushboolean (lua_State *L, int b);
    //	    Pushes a boolean value with value b onto the stack.
    public static final void lua_pushboolean ( lua_State thread, boolean b ) {
        thread.GetCurrentCallInfo ().PushValue ( b == false ? Boolean.FALSE : Boolean.TRUE );
    }

    // void lua_pushcfunction (lua_State *L, lua_CFunction f);
    //	    Pushes a C function onto the stack. This function receives a pointer
    //	    to a C function and pushes onto the stack a Lua value of type
    //	    function that, when called, invokes the corresponding C function.
    //	    Any function to be registered in Lua must follow the correct
    //	    protocol to receive its parameters and return its results
    //	    (see lua_CFunction).
    public static final void lua_pushjavafunction ( lua_State thread, JavaFunction f, int nup ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

        Function function = thread.GetCurrentCallInfo ().GetFunction ();

        Table environment = thread.GetEnvironment ();
        if ( function != null ) {
            environment = function.GetEnvironment ();
        }
        Function newFunction = new Function ( f, environment, nup );

        int iOldnup = nup;
        while ( ( nup -- ) != 0 ) {
            int iIndex = currentCallInfo.GetTop () - iOldnup + nup;
            Object value = currentCallInfo.GetValue ( iIndex );
            UpValue upValue = new UpValue ( null );
            upValue.SetValue ( value );
            newFunction.SetUpValue ( nup, upValue );
        }

        currentCallInfo.SetTop ( currentCallInfo.GetTop () - iOldnup );

        thread.GetCurrentCallInfo ().PushValue ( newFunction );
    }

    public static final void lua_pushjavafunction ( lua_State thread, JavaFunction f ) {
        lua_pushjavafunction ( thread, f, 0 );
    }

    // lua_pushfstring
    // const char *lua_pushfstring (lua_State *L, const char *fmt, ...);
    //	    Pushes onto the stack a formatted string and returns a pointer to
    //	    this string. It is similar to the C function sprintf, but has some
    //	    important differences:
    //	    You do not have to allocate space for the result: the result is a
    //	    Lua string and Lua takes care of memory allocation (and
    //	    deallocation, through garbage collection).
    //	    The conversion specifiers are quite restricted. There are no flags,
    //	    widths, or precisions. The conversion specifiers can only be '%%'
    //	    (inserts a '%' in the string), '%s' (inserts a zero-terminated
    //	    string, with no size restrictions), '%f' (inserts a lua_Number),
    //	    '%p' (inserts a pointer as a hexadecimal numeral), '%d' (inserts an
    //	    int), and '%c' (inserts an int as a character).
    public static final String lua_pushfstring ( lua_State thread, String msg ) {
        lua_pushstring ( thread, msg );
        return msg;
    }

    // lua_pushinteger
    // void lua_pushinteger (lua_State *L, lua_Integer n);
    //	    Pushes a number with value n onto the stack.
    public static final void lua_pushinteger ( lua_State thread, int n ) {
        thread.GetCurrentCallInfo ().PushValue ( new Double ( n ) );
    }

    // lua_pushlightuserdata
    // void lua_pushlightuserdata (lua_State *L, void *p);
    //	    Pushes a light userdata onto the stack.
    public static final void lua_pushlightuserdata ( lua_State thread, Object p ) {
        thread.GetCurrentCallInfo ().PushValue ( p );
    }

    // lua_pushliteral
    // void lua_pushliteral (lua_State *L, const char *s);
    //	    This macro is equivalent to lua_pushlstring, but can be used only
    //	    when s is a literal string. In these cases, it automatically
    //	    provides the string length.
    public static final void lua_pushliteral ( lua_State thread, String s ) {
        lua_pushlstring ( thread, s, s.length () );
    }

    // lua_pushlstring
    // void lua_pushlstring (lua_State *L, const char *s, size_t len);
    //	    Pushes the string pointed to by s with size len onto the stack. Lua
    //	    makes (or reuses) an internal copy of the given string, so the
    //	    memory at s can be freed or reused immediately after the function
    //	    returns. The string can contain embedded zeros.
    public static void lua_pushlstring ( lua_State thread, String s, int len ) {
        if ( len < 0 ) {
            len = 0;
        }

        if ( s == null ) {
            thread.GetCurrentCallInfo ().PushValue ( null );
        }
        else {
            // FIXME: 0, len ?
            thread.GetCurrentCallInfo ().PushValue ( len < s.length () ? s.substring ( 0, len ) : s );
        }
    }

    // lua_pushnil
    // void lua_pushnil (lua_State *L);
    //	    Pushes a nil value onto the stack.
    public static final void lua_pushnil ( lua_State thread ) {
        thread.GetCurrentCallInfo ().PushValue ( null );
    }

    // lua_pushinteger
    // void lua_pushnumber (lua_State *L, lua_Number n);
    //	    Pushes a number with value n onto the stack.
    public static final void lua_pushnumber ( lua_State thread, double n ) {
        thread.GetCurrentCallInfo ().PushValue ( new Double ( n ) );
    }

    // lua_pushstring
    // void lua_pushstring (lua_State *L, const char *s);
    //	    Pushes the zero-terminated string pointed to by s onto the stack.
    //	    Lua makes (or reuses) an internal copy of the given string, so the
    //	    memory at s can be freed or reused immediately after the function
    //	    returns. The string cannot contain embedded zeros; it is assumed to
    //	    end at the first zero.
    public static final void lua_pushstring ( lua_State thread, String s ) {
        thread.GetCurrentCallInfo ().PushValue ( s );
    }

    // lua_pushthread
    // int lua_pushthread (lua_State *L);
    //	    Pushes the thread represented by L onto the stack. Returns 1 if this
    //	    thread is the main thread of its state.
    public static final boolean lua_pushthread ( lua_State thread ) {
        thread.GetCurrentCallInfo ().PushValue ( thread );

        // TODO: return mainThread == thread
        return thread.GetMainThreadFlag ();
    }

    // lua_pushvalue
    // void lua_pushvalue (lua_State *L, int index);
    //	    Pushes a copy of the element at the given valid index onto the
    //	    stack.
    public static final void lua_pushvalue ( lua_State thread, int iIndex ) {
        thread.GetCurrentCallInfo ().PushValue ( thread.GetObjectValue ( iIndex ) );
    }

    // lua_rawequal
    // int lua_rawequal (lua_State *L, int index1, int index2);
    //	    Returns 1 if the two values in acceptable indices index1 and index2
    //	    are primitively equal (that is, without calling metamethods).
    //	    Otherwise returns 0. Also returns 0 if any of the indices are non
    //	    valid.
    public static boolean luaO_rawequalObj ( Object t1, Object t2 ) {
        if ( lua_typebyobject ( t1 ) != lua_typebyobject ( t2 ) ) {
            return false;
        }
        else {
            switch ( lua_typebyobject ( t1 ) ) {
                case LUA_TNIL: {
                    return true;
                }
                case LUA_TNUMBER: {
                    return ( ( Double ) t1 ).equals ( t2 );
                }
                case LUA_TBOOLEAN: {
                    return ( ( Boolean ) t1 ).equals ( t2 );
                }
                case LUA_TLIGHTUSERDATA: {
                    return t1 == t2;
                }
                default: {
                    // TODO: assert
                    return t1 == t2;
                }
            }
        }
    }

    // lua_rawequal
    // int lua_rawequal (lua_State *L, int index1, int index2);
    //	    Returns 1 if the two values in acceptable indices index1 and index2
    //	    are primitively equal (that is, without calling metamethods).
    //	    Otherwise returns 0. Also returns 0 if any of the indices are non
    //	    valid.
    public static boolean lua_rawequal ( lua_State thread, int index1, int index2 ) {
        Object o1 = thread.GetObjectValue ( index1 );
        Object o2 = thread.GetObjectValue ( index2 );

        if ( o1 == null || o2 == null ) {
            return false;
        }
        else {
            return luaO_rawequalObj ( o1, o2 );
        }
    }

    // lua_rawget
    // void lua_rawget (lua_State *L, int index);
    //	    Similar to lua_gettable, but does a raw access (i.e., without
    //	    metamethods).
    public static void lua_rawget ( lua_State thread, int index ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        Table table = ( Table ) thread.GetObjectValue ( index );
        Object key = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );
        currentCallInfo.SetValue ( currentCallInfo.GetTop () - 1, table.GetValue ( key ) );
    }

    // lua_rawgeti
    // void lua_rawgeti (lua_State *L, int index, int n);
    //	    Pushes onto the stack the value t[n], where t is the value at the
    //	    given valid index. The access is raw; that is, it does not invoke
    //	    metamethods.
    public static void lua_rawgeti ( lua_State thread, int idx, int n ) {
        Table table = ( Table ) thread.GetObjectValue ( idx );
        Object value = table.GetValueNum ( n );
        thread.GetCurrentCallInfo ().PushValue ( value );
    }

    // lua_rawset
    // void lua_rawset (lua_State *L, int index);
    //	    Similar to lua_settable, but does a raw assignment (i.e., without
    //	    metamethods).
    public static void lua_rawset ( lua_State thread, int index ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        Table table = ( Table ) thread.GetObjectValue ( index );
        Object key = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 2 );
        Object value = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );
        table.SetValue ( key, value, thread );
        currentCallInfo.SetTop ( currentCallInfo.GetTop () - 2 );
    }

    // lua_rawseti
    // void lua_rawseti (lua_State *L, int index, int n);
    //	    Does the equivalent of t[n] = v, where t is the value at the given
    //	    valid index and v is the value at the top of the stack.
    //	    This function pops the value from the stack. The assignment is raw;
    //	    that is, it does not invoke metamethods.
    public static void lua_rawseti ( lua_State thread, int index, int n ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

        Table table = ( Table ) thread.GetObjectValue ( index );

        int iTop = currentCallInfo.GetTop ();

        Object value = currentCallInfo.GetValue ( iTop - 1 );

        table.SetValueNum ( n, value, thread );

        currentCallInfo.SetTop ( iTop - 1 );
    }

    // lua_register
    // void lua_register (lua_State *L, const char *name, lua_CFunction f);
    //	    Sets the C function f as the new value of global name.
    public static void lua_register ( lua_State thread, String name, JavaFunction f ) {
        lua_register ( thread, name, f );
        lua_pushjavafunction ( thread, f );
        lua_setglobal ( thread, name );
    }

    // lua_remove
    // void lua_remove (lua_State *L, int index);
    //	    Removes the element at the given valid index, shifting down the
    //	    elements above this index to fill the gap. Cannot be called with a
    //	    pseudo-index, because a pseudo-index is not an actual stack
    //	    position.
    public static void lua_remove ( lua_State thread, int index ) {
        int p = thread.GetObjectIndex ( index );
        while (  ++ p < thread.GetObjectsStackTop () ) {
            thread.SetValue ( p - 1, thread.GetValue ( p ) );
        }

        thread.SetObjectsStackTop ( thread.GetObjectsStackTop () - 1 );
    }

    // lua_replace
    // void lua_replace (lua_State *L, int index);
    //	    Moves the top element into the given position (and pops it), without
    //	    shifting any element (therefore replacing the value at the given
    //	    position).
    public static void lua_replace ( lua_State thread, int index ) throws LuaRuntimeException {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

        // explicit test for incompatible code
        if ( index == LUA_ENVIRONINDEX && thread.GetCallInfosStackTop () == 1 ) {
            LuaAPI.luaG_runerror ( thread, "no calling environment" );
        }

        Object o = thread.GetObjectValue ( index );
        if ( index == LUA_ENVIRONINDEX ) {
            Function function = currentCallInfo.GetFunction ();
            function.SetEnvironment ( ( Table ) currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 ) );
        }
        else {
            thread.SetObjectValue ( index, currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 ) );
        //currentCallInfo.SetValue( LuaAPI.LUA_GLOBALSINDEX - index, currentCallInfo.GetValue( currentCallInfo.GetTop() - 1 ) );
        }
        currentCallInfo.SetTop ( currentCallInfo.GetTop () - 1 );

    }

    private static int luaD_rawrunprotected ( lua_State thread, JavaFunctionPlus func, int index ) {
        int status = 0;
        try {
            func.Call ( thread, index );
        }
        catch ( LuaRuntimeException ex ) {
            status = -1;
        }
        catch ( Exception ex ) {
            status = -1;
            lua_pushstring ( thread, "a java exception occurred within the coroutine" );
        }
        return status;
    }

    static class resume implements JavaFunctionPlus {

        public int Call ( lua_State thread, int index ) {
            CallInfo ci = thread.GetCurrentCallInfo ();
            Object firstArg = thread.GetValue ( ci.GetLocalObjectsStackBase () + 1 );

            if ( thread.GetStatus () == 0 ) /* start coroutine? */ {
                //lua_assert(ci == L->base_ci && firstArg > L->base);
                Object object = ci.GetValue ( 0 );
                if ( object instanceof Function ) {
                    Function function = ( Function ) object;
                    if ( function.IsLuaFunction () ) {
                        LuaFunction luaFunction = function.GetLuaFunction ();
//function, iFunctionIndexOnTheStack + 1, iFunctionIndexOnTheStack, iResultsQuantity, iArgsQuantity
                        //CallInfo callInfo = new CallInfo( this, function, iFunctionIndexOnTheStack + 1, iFunctionIndexOnTheStack, iResultsQuantity, iArgsQuantity );
                        CallInfo callInfo = new CallInfo ( thread, function, ci.GetLocalObjectsStackBase () + 1, ci.GetLocalObjectsStackBase (), LUA_MULTRET, ci.GetTop () - ci.GetLocalObjectsStackBase () - 1 );

                        thread.PushCallInfo ( callInfo );
                    }
                    else {
                        JavaFunction javaFunction = function.GetJavaFunction ();

                        CallInfo callInfo = new CallInfo ( thread, function, ci.GetLocalObjectsStackBase () + 1, ci.GetLocalObjectsStackBase (), LUA_MULTRET, ci.GetTop () - ci.GetLocalObjectsStackBase () );

                        thread.PushCallInfo ( callInfo );

                        int iReturnValuesQuantity = javaFunction.Call ( thread );

                        int iTop = ci.GetTop ();
                        int iActualReturnBase = iTop - iReturnValuesQuantity;

                        ci.CopyStack ( iActualReturnBase, -1, iReturnValuesQuantity );
                        ci.SetTop ( iReturnValuesQuantity - 1 );

                        thread.PopCallInfo ();
                        return -1;
                    }
                }
            //if (luaD_precall(L, firstArg - 1, LUA_MULTRET) != PCRLUA)
            }
            else /* resuming from previous yield */ {

//                lua_assert(L->status == LUA_YIELD);
                thread.SetStatus ( 0 );
                if ( ci.GetFunction ().IsJavaFunction () ) /* `common' yield?*/ {
                    /* finish interrupted execution of `OP_CALL'*/
                    //lua_assert(GET_OPCODE(*((ci-1)->savedpc - 1)) == OP_CALL || GET_OPCODE(*((ci-1)->savedpc - 1)) == OP_TAILCALL);

                    //int iReturnValuesQuantity = ci.GetFunction().GetJavaFunction().Call( thread );
                    //iReturnValuesQuantity = iResultsQuantity;


                    int wanted, i;
                    wanted = ci.GetResultsWanted ();
                    //L->base = (ci - 1)->base;  /* restore base */
                    int res = ci.GetReturnBase ();
                    /* move results to correct place */
                    //thread.CopyStack(thread.GetObjectsStackTop() - wanted, res, wanted);
                    //thread.SetObjectsStackTop(res + (wanted == LUA_MULTRET ? 0 : wanted));


                    for ( i = wanted; i != 0 && index < thread.GetObjectsStackTop (); i -- ) {
                        Object o1 = thread.GetValue ( index ++ );
                        thread.SetValue ( res ++, o1 );
                    //setobjs2s(L, res++, index++);
                    }
                    while ( i -- > 0 ) {
                        thread.SetValue ( res ++, null );
                    }
                    thread.SetObjectsStackTop ( res );/* and correct top if not multiple results */

                    /*if(wanted != -1)
                    {
                    thread.SetObjectsStackTop( thread.GetCurrentCallInfo().GetTop());/* and correct top if not multiple results */
                    /*}*/
                    thread.PopCallInfo ();



                    /*                      ci = L->ci--;
                    res = ci->func;  /* res == final position of 1st result
                    wanted = ci->nresults;
                    L->base = (ci - 1)->base;  /* restore base
                    L->savedpc = (ci - 1)->savedpc;  /* restore savedpc
                    /* move results to correct place
                    for (i = wanted; i != 0 && firstResult < L->top; i--)
                    setobjs2s(L, res++, firstResult++);
                    while (i-- > 0)
                    setnilvalue(res++);
                    L->top = res;
                    return (wanted - LUA_MULTRET);  /* 0 iff wanted == LUA_MULTRET
                     */


                    /*if (luaD_poscall(L, firstArg))  /* complete it... */
                    {
                    }
                }
                else /* yielded inside a hook: just continue its execution */ {
                    //L->base = L->ci->base;
                }
            }
            LVM.Execute ( thread, thread.GetCallInfosStackTop () - 1 );
            //luaV_execute(L, cast_int(L->ci - L->base_ci));

            return -1;
        }
    }

    public static int lua_resume ( lua_State thread, int nargs ) {
        int status = 0;
        if ( thread.GetStatus () != LuaBaseLib.LUA_YIELD && ( thread.GetStatus () != 0 /*|| L->ci != L->base_ci*/ ) ) {
            return -1;//resume_error(thread, "cannot resume non-suspended coroutine");

        }
        if ( thread.getNCCalls () >= LUAI_MAXCCALLS ) {
            return -1;//resume_error(thread, "C stack overflow");

        }
        //luai_userstateresume(thread, nargs);
        //lua_assert(L->errfunc == 0);
        thread.setNCCalls ( thread.getNCCalls () + 1 );
        thread.setBaseCcalls ( thread.getNCCalls () );
//        Object o = thread.GetValue(thread.GetObjectsStackTop() - nargs);
        status = luaD_rawrunprotected ( thread, new resume (), thread.GetObjectsStackTop () - nargs );
        if ( status != 0 ) /* error? */ {
            thread.SetStatus ( status );  /* mark thread as `dead' */
        //LuaAPI.lua_pushstring(thread, "coroutine.resume error");
        }
        else {
            if ( thread.getNCCalls () != thread.getBaseCcalls () ) {
                LuaAPI.luaL_error ( thread, "lua_assert(L->nCcalls == L->baseCcalls) faild" );
            }
            status = thread.GetStatus ();
        }
        thread.setNCCalls ( thread.getNCCalls () - 1 );
        return status;
    }

    // lua_setfenv
    // int lua_setfenv (lua_State *L, int index);
    //	    Pops a table from the stack and sets it as the new environment for
    //	    the value at the given index. If the value at the given index is
    //	    neither a function nor a thread nor a userdata, lua_setfenv returns
    //	    0. Otherwise it returns 1.
    public static boolean lua_setfenv ( lua_State thread, int index ) {

        boolean res = true;

        Object o = thread.GetObjectValue ( index );
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        if ( o instanceof Function ) {
            Function function = ( Function ) o;
            function.SetEnvironment ( ( Table ) currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 ) );
        }
        else if ( o instanceof UserData ) {
            UserData userData = ( UserData ) o;
            userData.SetEnvironment ( ( Table ) currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 ) );
        }
        else if ( o instanceof lua_State ) {
            lua_State threadValue = ( lua_State ) o;
            threadValue.SetEnvironment ( ( Table ) currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 ) );
        }
        else {
            res = false;
        }

        currentCallInfo.SetTop ( currentCallInfo.GetTop () - 1 );

        return res;
    }

    // lua_setfield
    // void lua_setfield (lua_State *L, int index, const char *k);
    //	    Does the equivalent to t[k] = v, where t is the value at the given
    //	    valid index and v is the value at the top of the stack.
    //	    This function pops the value from the stack. As in Lua, this
    //	    function may trigger a metamethod for the "newindex" event
    //	    (see �2.8).
    public static void lua_setfield ( lua_State thread, int index, String k ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

        Object value = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );

        if ( index > 0 && index - 1 < currentCallInfo.GetTop () ) {
            thread.SetTable ( index - 1, k, value );
        }
        else {
            final Table table = ( Table ) thread.GetObjectValue ( index );
            thread.SetTable ( table, k, value );
        }

        currentCallInfo.SetTop ( currentCallInfo.GetTop () - 1 );
    }

    // lua_setglobal
    // void lua_setglobal (lua_State *L, const char *name);
    //	    Pops a value from the stack and sets it as the new value of global
    //	    name. It is defined as a macro:
    public static final void lua_setglobal ( lua_State thread, String name ) {
        lua_setfield ( thread, LUA_GLOBALSINDEX, name );
    }

    // lua_setmetatable
    // int lua_setmetatable (lua_State *L, int index);
    //	    Pops a table from the stack and sets it as the new metatable for the
    //	    value at the given acceptable index.
    public static boolean lua_setmetatable ( lua_State thread, int index ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        Object obj = thread.GetObjectValue ( index );
        Table mt;
        if ( LuaAPI.IsNilOrNull ( currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 ) ) == true ) {
            mt = null;
        }
        else {
            mt = ( Table ) currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );
        }
        switch ( lua_typebyobject ( obj ) ) {
            case LUA_TTABLE: {
                ( ( Table ) obj ).SetMetaTable ( mt );
                break;
            }
            case LUA_TUSERDATA: {
                ( ( UserData ) obj ).SetMetaTable ( mt );
                break;
            }
            default: {
                LVM.SetMetaTable ( lua_typebyobject ( obj ), mt );
                break;
            }
        }

        currentCallInfo.SetTop ( currentCallInfo.GetTop () - 1 );

        return true;
    }

    // lua_settable
    // void lua_settable (lua_State *L, int index);
    //	    Does the equivalent to t[k] = v, where t is the value at the given
    //	    valid index, v is the value at the top of the stack, and k is the
    //	    value just below the top.
    //	    This function pops both the key and the value from the stack. As in
    //	    Lua, this function may trigger a metamethod for the "newindex" event
    //	    (see �2.8).
    public static void lua_settable ( lua_State thread, int index ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

        Object key = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 2 );
        Object value = currentCallInfo.GetValue ( currentCallInfo.GetTop () - 1 );

        if ( index > 0 && index - 1 < currentCallInfo.GetTop () ) {
            thread.SetTable ( index - 1, key, value );
        }
        else {
            final Table table = ( Table ) thread.GetObjectValue ( index );
            thread.SetTable ( table, key, value );
        }

        currentCallInfo.SetTop ( currentCallInfo.GetTop () - 2 );
    }

    // lua_settop
    // void lua_settop (lua_State *L, int index);
    //	    Accepts any acceptable index, or 0, and sets the stack top to this
    //	    index. If the new top is larger than the old one, then the new
    //	    elements are filled with nil. If index is 0, then all stack elements
    //	    are removed.
    public static final void lua_settop ( lua_State thread, int index ) {
        if ( index >= 0 ) {
            while ( thread.GetObjectsStackTop () < thread.GetCurrentCallInfo ().GetLocalObjectsStackBase () + index ) {
                thread.SetValue ( thread.GetObjectsStackTop (), null );
                thread.SetObjectsStackTop ( thread.GetObjectsStackTop () + 1 );
            }
            thread.SetObjectsStackTop ( thread.GetCurrentCallInfo ().GetLocalObjectsStackBase () + index );
        }
        else {
            thread.SetObjectsStackTop ( thread.GetObjectsStackTop () + index + 1 );  // `subtract' index (index is negative)

        }
    }

    // lua_status
    // int lua_status (lua_State *L);
    //	    Returns the status of the thread L. 
    //	    The status can be 0 for a normal thread, an error code if the thread
    //	    finished its execution with an error, or LUA_YIELD if the thread is 
    //	    suspended. 
    public static final int lua_status ( lua_State thread ) {
        return thread.GetStatus ();
    }

    // lua_toboolean
    // int lua_toboolean (lua_State *L, int index);
    //	    Converts the Lua value at the given acceptable index to a C boolean
    //	    value (0 or 1). Like all tests in Lua, lua_toboolean returns 1 for
    //	    any Lua value different from false and nil; otherwise it returns 0.
    //	    It also returns 0 when called with a non-valid index. (If you want
    //	    to accept only actual boolean values, use lua_isboolean to test the
    //	    value's type.)
    public static final boolean lua_toboolean ( lua_State thread, int index ) {
        // FIXME: ensure that a value is a boolean. Follow the text above.
        Object o = thread.GetObjectValue ( index );
        if ( o instanceof Boolean ) {
            return ( ( Boolean ) o ).booleanValue ();
        }
        else if ( o == null || o == LuaAPI.m_NilObject ) {
            return false;
        }
        else {
            return true;
        }
    }

    // lua_tocfunction
    // lua_CFunction lua_tocfunction (lua_State *L, int index);
    //	    Converts a value at the given acceptable index to a C function. That
    //	    value must be a C function; otherwise, returns NULL.
    public static final JavaFunction lua_tojavafunction ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        if ( object instanceof Function ) {
            Function function = ( Function ) object;
            if ( function.IsJavaFunction () ) {
                return function.GetJavaFunction ();
            }
        }
        return null;
    }

    // lua_tointeger
    // lua_Integer lua_tointeger (lua_State *L, int index);
    //	    Converts the Lua value at the given acceptable index to the signed
    //	    integral type lua_Integer. The Lua value must be a number or a
    //	    string convertible to a number (see �2.2.1); otherwise,
    //	    lua_tointeger returns 0.
    //	    If the number is not an integer, it is truncated in some
    //	    non-specified way.
    // COMMOENT: If the number is not an integer, function returns 0
    public static final int lua_tointeger ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        if ( object instanceof Double ) {
            return ( int ) ( ( Double ) object ).doubleValue ();
        }
        return 0;
    }

    // lua_tolstring
    // const char *lua_tolstring (lua_State *L, int index, size_t *len);
    //	    Converts the Lua value at the given acceptable index to a C string.
    //	    If len is not NULL, it also sets *len with the string length. The
    //	    Lua value must be a string or a number; otherwise, the function
    //	    returns NULL. If the value is a number, then lua_tolstring also
    //	    changes the actual value in the stack to a string. (This change
    //	    confuses lua_next when lua_tolstring is applied to keys during a
    //	    table traversal.)
    //	    lua_tolstring returns a fully aligned pointer to a string inside the
    //	    Lua state. This string always has a zero ('\0') after its last
    //	    character (as in C), but may contain other zeros in its body.
    //	    Because Lua has garbage collection, there is no guarantee that the
    //	    pointer returned by lua_tolstring will be valid after the
    //	    corresponding value is removed from the stack.
    public static String lua_tolstring ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        if (  ! ( object instanceof String ) ) {
            if (  ! luaV_tostring ( thread, index ) ) {
                return null;
            }
        }
        String string = ( String ) thread.GetObjectValue ( index );
        return string;
    }

    // lua_tonumber
    // lua_Number lua_tonumber (lua_State *L, int index);
    //	    Converts the Lua value at the given acceptable index to the C type
    //	    lua_Number (see lua_Number). The Lua value must be a number or a
    //	    string convertible to a number (see �2.2.1); otherwise, lua_tonumber
    //	    returns 0.
    public static final double lua_tonumber ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        if ( object instanceof Double ) {
            return ( ( Double ) object ).doubleValue ();
        }
        else if ( object instanceof String ) {
            try {
                return Double.parseDouble ( ( String ) object );
            }
            catch ( RuntimeException ex ) {
            }
        }
        return 0;
    }

    // lua_topointer
    // const void *lua_topointer (lua_State *L, int index);
    //	    Converts the value at the given acceptable index to a generic C
    //	    pointer (void*). The value may be a userdata, a table, a thread, or
    //	    a function; otherwise, lua_topointer returns NULL. Different objects
    //	    will give different pointers. There is no way to convert the pointer
    //	    back to its original value.
    //	    Typically this function is used only for debug information
    public static final Object lua_topointer ( lua_State thread, int index ) {
        //return 
        Object object = thread.GetObjectValue ( index );
        switch ( lua_typebyobject ( object ) ) {
            case LUA_TTABLE:
            case LUA_TFUNCTION:
            case LUA_TTHREAD: {
                return object;
            }
            case LUA_TUSERDATA:
            case LUA_TLIGHTUSERDATA: {
                return lua_touserdata ( thread, index );
            }
            default: {
                return null;
            }
        }
    }

    // lua_tostring
    // const char *lua_tostring (lua_State *L, int index);
    //	    Equivalent to lua_tolstring with len equal to NULL.
    public static final String lua_tostring ( lua_State thread, int index ) {
        return lua_tolstring ( thread, index );
    }

    // lua_tothread
    // lua_State *lua_tothread (lua_State *L, int index);
    //	    Converts the value at the given acceptable index to a Lua thread
    //	    (represented as lua_State*). This value must be a thread; otherwise,
    //	    the function returns NULL.
    public static final lua_State lua_tothread ( lua_State thread, int index ) {
        Object object = thread.GetObjectValue ( index );
        if ( object instanceof lua_State ) {
            return ( lua_State ) object;
        }
        return null;
    }

    // lua_touserdata
    // void *lua_touserdata (lua_State *L, int index);
    //	    If the value at the given acceptable index is a full userdata,
    //	    returns its block address. If the value is a light userdata, returns
    //	    its pointer. Otherwise, returns NULL.
    public static Object lua_touserdata ( lua_State thread, int idx ) {
        // BUGWARN: 
        Object object = thread.GetObjectValue ( idx );
        switch ( lua_typebyobject ( object ) ) {
            case LUA_TUSERDATA: {
                return ( ( UserData ) object ).GetUserData ();
            }
            default: {
                return object;
            }
            /*case LUA_TLIGHTUSERDATA:
            {
            return object;
            }
            default:
            {
            return null;
            }*/
        }
    }

    public static final int lua_typebyobject ( Object object ) {
        if ( object == null ) {
            return LuaAPI.LUA_TNIL;
        }
        if ( object == LuaAPI.m_NilObject ) {
            return LuaAPI.LUA_TNONE;
        }
        else if ( object instanceof Boolean ) {
            return LuaAPI.LUA_TBOOLEAN;
        }
        else if ( object instanceof Double ) {
            return LuaAPI.LUA_TNUMBER;
        }
        else if ( object instanceof String ) {
            return LuaAPI.LUA_TSTRING;
        }
        else if ( object instanceof Table ) {
            return LuaAPI.LUA_TTABLE;
        }
        else if ( object instanceof Function || object instanceof JavaFunction ) {
            return LuaAPI.LUA_TFUNCTION;
        }
        else if ( object instanceof UserData ) {
            return LuaAPI.LUA_TUSERDATA;
        }
        else if ( object instanceof lua_State ) {
            return LuaAPI.LUA_TTHREAD;
        }

        return LuaAPI.LUA_TLIGHTUSERDATA;
    }

    // lua_type
    // int lua_type (lua_State *L, int index);
    //	    Returns the type of the value in the given acceptable index, or
    //	    LUA_TNONE for a non-valid index (that is, an index to an "empty"
    //	    stack position). The types returned by lua_type are coded by the
    //	    following constants defined in lua.h: LUA_TNIL, LUA_TNUMBER,
    //	    LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA,
    //	    LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
    public static final int lua_type ( lua_State thread, int index ) {
        return lua_typebyobject ( thread.GetObjectValue ( index ) );
    }
    // lua_typename
    // const char *lua_typename  (lua_State *L, int tp);
    //	    Returns the name of the type encoded by the value tp, which must be
    //	    one the values returned by lua_type.
    private static final String[] luaT_typenames = {
        "nil",
        "boolean",
        "userdata",
        "number",
        "string",
        "table",
        "function",
        "userdata",
        "thread",
        "proto",
        "upval"
    };

    public static final String lua_typename ( lua_State thread, int t ) {
        return ( t == LuaAPI.LUA_TNONE ) ? "no value" : luaT_typenames[t];
    }

    // lua_xmove
    // void lua_xmove (lua_State *from, lua_State *to, int n);
    //	    Exchange values between different threads of the same global state.
    //	    This function pops n values from the stack from, and pushes them
    //	    onto the stack to.
    public static void lua_xmove ( lua_State from, lua_State to, int n ) {
        if ( from == to ) {
            return;
        }

        int topOriginal = from.GetObjectsStackTop ();
        int topChanged = from.GetObjectsStackTop () - n;
        from.SetObjectsStackTopWithoutClearingStack ( topChanged );
        //from.SetObjectsStackTop( from.GetObjectsStackTop() - n );
        for ( int i = 0; i < n; i ++ ) {
            to.SetValue ( to.GetObjectsStackTop (), from.GetValue ( from.GetObjectsStackTop () + i ) );
            to.SetObjectsStackTop ( to.GetObjectsStackTop () + 1 );
        }
        from.SetObjectsStackTop ( topOriginal );
        from.SetObjectsStackTop ( topChanged );
    }

    public static int lua_yield ( lua_State thread, int nresults ) {
        //luai_userstateyield(L, nresults);
        //lua_lock(L);
        if ( ( thread.getNCCalls () > thread.getBaseCcalls () ) || ( ( thread.getNCCalls () == 0 ) && ( thread.getNCCalls () == thread.getBaseCcalls () ) ) ) {
            luaG_runerror ( thread, "attempt to yield across metamethod/J-call boundary" );
        }
        /*CallInfo ci = thread.GetCurrentCallInfo();
        Object value = thread.GetValue(thread.GetObjectsStackTop() - 1);
        thread.SetValue(0, value);
        thread.SetObjectsStackTop(1);*/

        //L->base = L->top - nresults;  /* protect stack slots below */
        //ci.SetLocalObjectStackBase( ci.GetTop() - nresults);
//        thread.CopyStack(thread.GetObjectsStackTop() - 1, thread.GetObjectsStackTop() - 1 - nresults, nresults );
//        thread.SetObjectsStackTop(thread.GetObjectsStackTop() - nresults);

        thread.SetStatus ( LuaBaseLib.LUA_YIELD );
        return -1;
    /*
    luai_userstateyield(L, nresults);
    lua_lock(L);
    if (L->nCcalls > L->baseCcalls)
    luaG_runerror(L, "attempt to yield across metamethod/C-call boundary");
    L->base = L->top - nresults;  /* protect stack slots below
    L->status = LUA_YIELD;
    lua_unlock(L);
    return -1;
     */
    }

    /*
     * AUXILARY LIBRARY
     **/
    // luaL_addchar
    // void luaL_addchar (luaL_Buffer *B, char c);
    //	    Adds the character c to the buffer B (see luaL_Buffer).
    public static void luaL_addchar ( luaL_Buffer B, char c ) {
        B.m_strValue += c;
    }

    // luaL_addlstring
    // void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l);
    //	    Adds the string pointed to by s with length l to the buffer B (see
    //	    luaL_Buffer). The string may contain embedded zeros.
    public static void luaL_addlstring ( luaL_Buffer B, String s, int l ) {
        for ( int i = 0; i < l; i ++ ) {
            luaL_addchar ( B, s.charAt ( i ) );
        }
    }

    // luaL_addsize
    // void luaL_addsize (luaL_Buffer *B, size_t n);
    //	    Adds to the buffer B (see luaL_Buffer) a string of length n
    //	    previously copied to the buffer area (see luaL_prepbuffer).
    /*public static void luaL_addsize( luaL_Buffer B, int n )
    {
     * // FIXME:
    B = B.substring( n );
    }*/
    // luaL_addstring
    // void luaL_addstring (luaL_Buffer *B, const char *s);
    //	    Adds the zero-terminated string pointed to by s to the buffer B (see
    //	    luaL_Buffer). The string may not contain embedded zeros.
    public static void luaL_addstring ( luaL_Buffer B, String s ) {
        luaL_addlstring ( B, s, s.length () );
    }

    // luaL_addvalue
    // void luaL_addvalue (luaL_Buffer *B);
    //	    Adds the value at the top of the stack to the buffer B (see
    //	    luaL_Buffer). Pops the value.
    //	    This is the only function on string buffers that can (and must) be
    //	    called with an extra element on the stack, which is the value to be
    //	    added to the buffer.
    public static void luaL_addvalue ( lua_State thread, luaL_Buffer B ) {
        B.m_strValue += lua_tostring ( thread, -1 );
        lua_pop ( thread, 1 ); /* remove from stack */
    }

    // luaL_argcheck
    // void luaL_argcheck (lua_State *L, int cond, int narg, const char *extramsg);
    //	    Checks whether cond is true. If not, raises an error with the
    //	    following message, where func is retrieved from the call stack:
    //		bad argument #<narg> to <func> (<extramsg>)
    public static void luaL_argcheck ( lua_State thread, boolean cond, int narg, String extramsg ) {
        if ( cond == false ) {
            luaL_argerror ( thread, narg, extramsg );
        }
    }

    // luaL_argerror
    // int luaL_argerror (lua_State *L, int narg, const char *extramsg);
    //	    Raises an error with the following message, where func is retrieved
    //	    from the call stack:
    //		bad argument #<narg> to <func> (<extramsg>)
    //	    This function never returns, but it is an idiom to use it in C
    //	    functions as return luaL_argerror(args).
    public static int luaL_argerror ( lua_State thread, int narg, String extramsg ) {
        // FIXME:
        return luaL_error ( thread, "bad argument " + narg + "(\"" + extramsg + "\")" );
    }

    // luaL_buffinit
    // void luaL_buffinit (lua_State *L, luaL_Buffer *B);
    //	    Initializes a buffer B. This function does not allocate any space;
    //	    the buffer must be declared as a variable (see luaL_Buffer).
    public static final void luaL_buffinit ( lua_State thread, luaL_Buffer s ) {
        s.SetThread ( thread );
        s.m_strValue = "";
    }

    public static final int abs_index ( lua_State thread, int i ) {
        return ( ( i ) > 0 || ( i ) <= LUA_REGISTRYINDEX ? ( i ) : lua_gettop ( thread ) + ( i ) + 1 );
    }

    // luaL_callmeta
    // int luaL_callmeta (lua_State *L, int obj, const char *e);
    //	    Calls a metamethod.
    //	    If the object at index obj has a metatable and this metatable has a
    //	    field e, this function calls this field and passes the object as its
    //	    only argument. In this case this function returns 1 and pushes onto
    //	    the stack the value returned by the call. If there is no metatable
    //	    or no metamethod, this function returns 0 (without pushing any value
    //	    on the stack).
    public static boolean luaL_callmeta ( lua_State thread, int obj, String event ) {
        obj = abs_index ( thread, obj );
        if (  ! luaL_getmetafield ( thread, obj, event ) ) /* no metafield? */ {
            return false;
        }
        lua_pushvalue ( thread, obj );
        lua_call ( thread, 1, 1 );
        return true;
    }

    // luaL_checkany
    // void luaL_checkany (lua_State *L, int narg);
    //	    Checks whether the function has an argument of any type (including
    //	    nil) at position narg.
    public static void luaL_checkany ( lua_State thread, int narg ) {
        if ( lua_type ( thread, narg ) == LUA_TNONE ) {
            luaL_argerror ( thread, narg, "value expected" );
        }
    }

    // luaL_checkinteger
    // lua_Integer luaL_checkinteger (lua_State *L, int narg);
    //	    Checks whether the function argument narg is a number and returns
    //	    this number cast to a lua_Integer.
    public static int luaL_checkinteger ( lua_State thread, int narg ) {
        int d = lua_tointeger ( thread, narg );
        if ( d == 0 &&  ! lua_isnumber ( thread, narg ) ) /* avoid extra test when d is not 0 */ {
            luaL_typerror ( thread, narg, lua_typename ( thread, LUA_TNUMBER ) );
        }
        return d;
    }

    public static final int luaL_opt ( lua_State thread, JavaFunctionPlus function, int n, int d ) {
        return lua_isnoneornil ( thread, n ) ? d : function.Call ( thread, n );
    }

    // luaL_checkint
    // int luaL_checkint (lua_State *L, int narg);
    //	    Checks whether the function argument narg is a number and returns
    //	    this number cast to an int.
    public static final class luaL_checkint implements JavaFunctionPlus {

        public int Call ( lua_State thread, int narg ) {
            return luaL_checkint ( thread, narg );
        }
    }

    public static final int luaL_checkint ( lua_State thread, int narg ) {
        return luaL_checkinteger ( thread, narg );
    }

    // luaL_checklong
    // long luaL_checklong (lua_State *L, int narg);
    //	    Checks whether the function argument narg is a number and returns
    //	    this number cast to an int.
    public static final long luaL_checklong ( lua_State thread, int narg ) {
        return luaL_checkinteger ( thread, narg );
    }

    // luaL_checklstring
    // const char *luaL_checklstring (lua_State *L, int narg, size_t *l);
    //	    Checks whether the function argument narg is a string and returns
    //	    this string; if l is not NULL fills *l with the string's length.
    //	    This function uses lua_tolstring to get its result, so all
    //	    conversions and caveats of that function apply here.
    public static final String luaL_checkstring ( lua_State thread, int narg ) {
        return luaL_checklstring ( thread, narg );
    }

    // luaL_checklstring
    // const char *luaL_checklstring (lua_State *L, int narg, size_t *l);
    //	    Checks whether the function argument narg is a string and returns
    //	    this string; if l is not NULL fills *l with the string's length.
    //	    This function uses lua_tolstring to get its result, so all
    //	    conversions and caveats of that function apply here.
    public static String luaL_checklstring ( lua_State thread, int narg ) {
        String s = lua_tolstring ( thread, narg );
        if ( s == null ) {
            luaL_typerror ( thread, narg, lua_typename ( thread, LUA_TSTRING ) );
        }
        return s;
    }

    // luaL_checknumber
    // lua_Number luaL_checknumber (lua_State *L, int narg);
    //	    Checks whether the function argument narg is a number and returns
    //	    this number.
    public static double luaL_checknumber ( lua_State thread, int narg ) {
        double d = lua_tonumber ( thread, narg );
        if ( d == 0 &&  ! lua_isnumber ( thread, narg ) ) /* avoid extra test when d is not 0 */ {
            luaL_typerror ( thread, narg, lua_typename ( thread, LUA_TNUMBER ) );
        }
        return d;
    }

    // luaL_checkoption
    // int luaL_checkoption (lua_State *L, int narg, const char *def, const char *const lst[]);
    //	    Checks whether the function argument narg is a string and searches
    //	    for this string in the array lst (which must be NULL-terminated).
    //	    Returns the index in the array where the string was found. Raises an
    //	    error if the argument is not a string or if the string cannot be
    //	    found.
    //	    If def is not NULL, the function uses def as a default value when
    //	    there is no argument narg or if this argument is nil.
    //	    This is a useful function for mapping strings to C enums. (The usual
    //	    convention in Lua libraries is to use strings instead of numbers to
    //	    select options.)
    public static int luaL_checkoption ( lua_State thread, int narg, String def, String[] lst ) {
        String name = ( def != null ) ? LuaAPI.luaL_optstring ( thread, narg, def ) : LuaAPI.luaL_checkstring ( thread, narg );

        for ( int i = 0; i < lst.length; i ++ ) {
            if ( lst[i].equals ( name ) ) {
                return i;
            }
        }
        return luaL_argerror ( thread, narg, LuaAPI.lua_pushfstring ( thread, "invalid option \" + name + \"" ) );
    }

    // luaL_checkstack
    // void luaL_checkstack (lua_State *L, int sz, const char *msg);
    //	    Grows the stack size to top + sz elements, raising an error if the
    //	    stack cannot grow to that size. msg is an additional text to go into
    //	    the error message.
    public static void luaL_checkstack ( lua_State thread, int space, String mes ) {
        if ( lua_checkstack ( thread, space ) == false ) {
            luaL_error ( thread, "stack overflow " + mes );
        }
    }

    // luaL_checktype
    // void luaL_checktype (lua_State *L, int narg, int t);
    //	    Checks whether the function argument narg has type t. See lua_type
    //	    for the encoding of types for t.
    public static void luaL_checktype ( lua_State thread, int narg, int t ) {
        if ( lua_type ( thread, narg ) != t ) {
            luaL_typerror ( thread, narg, LuaAPI.lua_typename ( thread, t ) );
        }
    }

    // luaL_checkudata
    // void *luaL_checkudata (lua_State *L, int narg, const char *tname);
    //	    Checks whether the function argument narg is a userdata of the type
    //	    tname (see luaL_newmetatable).
    public static Object luaL_checkudata ( lua_State thread, int ud, String tname ) {
        Object p = lua_touserdata ( thread, ud );
        if ( p != null ) {
            if ( lua_getmetatable ( thread, ud ) ) {
                lua_getfield ( thread, LUA_REGISTRYINDEX, tname );
                if ( lua_rawequal ( thread, -1, -2 ) ) {
                    lua_pop ( thread, 2 );
                    return p;
                }
            }
        }
        luaL_typerror ( thread, ud, tname );
        return null;
    }

    // luaL_dofile
    // int luaL_dofile (lua_State *L, const char *filename);
    //	    Loads and runs the given file. It is defined as the following macro:
    //	    (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0))
    //	    It returns 0 if there are no errors or 1 in case of errors.
    public static final boolean luaL_dofile ( lua_State thread, String filename ) {
        return luaL_loadfile ( thread, filename ) != 0 || LuaAPI.lua_pcall ( thread, 0, LUA_MULTRET, 0 ) != 0;
    }

    // luaL_dostring
    // int luaL_dostring (lua_State *L, const char *str);
    //	    Loads and runs the given string. It is defined as the following
    //	    macro:
    //	    (luaL_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0))
    //	    It returns 0 if there are no errors or 1 in case of errors.
    public static boolean luaL_dostring ( lua_State thread, String str ) {
        return luaL_loadstring ( thread, str ) != 0 || lua_pcall ( thread, 0, LUA_MULTRET, 0 ) != 0;
    }

    // luaL_error
    // int luaL_error (lua_State *L, const char *fmt, ...);
    //	    Raises an error. The error message format is given by fmt plus any
    //	    extra arguments, following the same rules of lua_pushfstring. It
    //	    also adds at the beginning of the message the file name and the line
    //	    number where the error occurred, if this information is available.
    //	    This function never returns, but it is an idiom to use it in C
    //	    functions as return luaL_error(args).
    public static int luaL_error ( lua_State thread, String msg ) {
        CallInfo callInfo = thread.GetCurrentCallInfo ();
        luaL_where ( thread, 1 );
        callInfo.PushValue ( ( String ) callInfo.PopValue () + msg );
        lua_error ( thread );
        return 0;
    }

    // luaL_getmetafield
    // int luaL_getmetafield (lua_State *L, int obj, const char *e);
    //	    Pushes onto the stack the field e from the metatable of the object
    //	    at index obj. If the object does not have a metatable, or if the
    //	    metatable does not have this field, returns 0 and pushes nothing.
    public static boolean luaL_getmetafield ( lua_State thread, int obj, String event ) {
        if (  ! LuaAPI.lua_getmetatable ( thread, obj ) ) /* no metatable? */ {
            return false;
        }
        LuaAPI.lua_pushstring ( thread, event );
        LuaAPI.lua_rawget ( thread, -2 );
        if ( LuaAPI.lua_isnil ( thread, -1 ) ) {
            LuaAPI.lua_pop ( thread, 2 );  /* remove metatable and metafield */
            return false;
        }
        else {
            LuaAPI.lua_remove ( thread, -2 );  /* remove only metatable */
            return true;
        }
    }

    // luaL_getmetatable
    // void luaL_getmetatable (lua_State *L, const char *tname);
    //	    Pushes onto the stack the metatable associated with name tname in
    //	    the registry (see luaL_newmetatable).
    public static final void luaL_getmetatable ( lua_State thread, String tname ) {
        lua_getfield ( thread, LUA_REGISTRYINDEX, tname );
    }

    // luaL_gsub
    // const char *luaL_gsub (lua_State *L, const char *s, const char *p, const char *r);
    //	    Creates a copy of string s by replacing any occurrence of the string
    //	    p with the string r. Pushes the resulting string on the stack and
    //	    returns it.
    public static String luaL_gsub ( lua_State thread, String s, String p, String r ) {
        luaL_Buffer b = new luaL_Buffer ();
        luaL_buffinit ( thread, b );

        while ( s.indexOf ( p ) != -1 ) {
            luaL_addlstring ( b, s, s.indexOf ( p ) );
            luaL_addstring ( b, r );
            s = s.substring ( s.indexOf ( p ) + p.length () );
        }
        luaL_addstring ( b, s );	// push last suffix

        luaL_pushresult ( b );
        return lua_tostring ( thread, -1 );
    }

    public static int lua_ref ( lua_State thread, int lock ) {
        if ( lock > 0 ) {
            return luaL_ref ( thread, LUA_REGISTRYINDEX );
        }
        else {
            luaG_runerror ( thread, "unlocked references are obsolete" );
            return 0;
        }
    }

    public static void lua_getref ( lua_State thread, int ref ) {
        lua_rawgeti ( thread, LUA_REGISTRYINDEX, ref );
    }

    public static void lua_unref ( lua_State thread, int ref ) {
        luaL_unref ( thread, LUA_REGISTRYINDEX, ref );
    }

    public static String lua_getupvalue ( lua_State thread, int funcindex, int n ) {
        Function function;
        try {
            function = ( Function ) thread.GetObjectValue ( funcindex );
        }
        catch ( LuaRuntimeException ex ) {
            return null;
        }

        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        if ( function.IsJavaFunction () ) {
            if ( ( 1 <= n && n <= function.GetUpValuesQuantity () ) ) {
                currentCallInfo.PushValue ( function.GetUpValue ( n - 1 ).GetValue () );
                return "";
            }
        }
        else {
            if ( ( 1 <= n && n <= function.GetLuaFunction ().GetUpValuesQuantity () ) ) {
                currentCallInfo.PushValue ( function.GetUpValue ( n - 1 ).GetValue () );
                return function.GetLuaFunction ().GetUpValueName ( n - 1 );
            }
        }
        return null;
    }

    public static String lua_setupvalue ( lua_State thread, int funcindex, int n ) {
        Function function;
        try {
            function = ( Function ) thread.GetObjectValue ( funcindex );
        }
        catch ( LuaRuntimeException ex ) {
            return null;
        }

        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();


        if ( function.IsJavaFunction () ) {
            if ( ( 1 <= n && n <= function.GetUpValuesQuantity () ) ) {
                function.GetUpValue ( n - 1 ).SetValue ( currentCallInfo.PopValue () );
                return "";
            }
        }
        else {
            if ( ( 1 <= n && n <= function.GetLuaFunction ().GetUpValuesQuantity () ) ) {
                function.GetUpValue ( n - 1 ).SetValue ( currentCallInfo.PopValue () );
                return function.GetLuaFunction ().GetUpValueName ( n - 1 );
            }
        }
        return null;
    }

    // luaL_loadbuffer
    // int luaL_loadbuffer (lua_State *L, const char *buff, size_t sz, const char *name);
    //	    Loads a buffer as a Lua chunk. This function uses lua_load to load
    //	    the chunk in the buffer pointed to by buff with size sz.
    //	    This function returns the same results as lua_load. name is the
    //	    chunk name, used for debug information and error messages.
    public static int luaL_loadbuffer ( lua_State thread, String buff, String name ) {
        //ByteArrayInputStream bais = new ByteArrayInputStream( buff.getBytes() );
        char[] chars = buff.toCharArray ();
        byte[] bytes = new byte[ chars.length ];
        for ( int i = 0; i < chars.length; i ++ ) {
            bytes[i] = ( byte ) ( chars[i] & 0xFF );
        }

        ByteArrayInputStream bais = new ByteArrayInputStream ( bytes );
        return lua_load ( thread, bais, null, name );
    }

    // luaL_loadfile
    // int luaL_loadfile (lua_State *L, const char *filename);
    //	    Loads a file as a Lua chunk. This function uses lua_load to load the
    //	    chunk in the file named filename. If filename is NULL, then it loads
    //	    from the standard input. The first line in the file is ignored if it
    //	    starts with a #.
    //	    This function returns the same results as lua_load, but it has an
    //	    extra error code LUA_ERRFILE if it cannot open/read the file.
    //	    As lua_load, this function only loads the chunk; it does not run it.
    public static int luaL_loadfile ( lua_State thread, String filename ) {
        InputStream inputStream = null;
        FileConnection fileConnection = null;
        FCISContainer container = checkFileOnStorage ( filename );
        if ( container != null ) {
            inputStream = container.is;
            fileConnection = container.fc;
        }
        if ( inputStream == null || fileConnection == null ) {
            inputStream = thread.getClass ().getResourceAsStream ( filename );
        }

        if ( inputStream == null && filename.length () != 0 && filename.charAt ( 0 ) != '/' ) {
            inputStream = thread.getClass ().getResourceAsStream ( "/" + filename );
        }

        if ( inputStream != null ) {
            return lua_load ( thread, inputStream, fileConnection, filename );

        }
        thread.GetCurrentCallInfo ().PushValue ( "cannot open " + filename + " No such file or directory" );
        return 1; // FIXME: LUA_ERRSYNTAX || MEM? ;

    }

    private final static class FCISContainer {

        public FileConnection fc = null;
        public InputStream is = null;
    }

    private static FCISContainer checkFileOnStorage ( String filename ) {
        FileConnection fileConnection = null;
        InputStream is = null;
        if (  ! filename.startsWith ( "file://" ) )
            filename = "file://" + filename;

        try {
            fileConnection = ( FileConnection ) Connector.open ( filename );
            if ( fileConnection == null )
                return null;
            if (  ! fileConnection.exists () ) {
                try {
                    fileConnection.close ();
                }
                catch ( Exception e ) {
                }
                finally {
                    fileConnection = null;
                }
                return null;
            }

            is = fileConnection.openInputStream ();
            if ( is == null ) {
                try {
                    fileConnection.close ();
                }
                catch ( Exception e ) {
                }
                finally {
                    fileConnection = null;
                }
                return null;
            }

        }
        catch ( Exception ex ) {
            if ( is != null ) {
                try {
                    is.close ();
                }
                catch ( Exception e ) {
                }
                finally {
                    is = null;
                }
            }
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
        FCISContainer container = new FCISContainer ();
        container.fc = fileConnection;
        container.is = is;
        return container;
    }

    // luaL_loadstring
    // int luaL_loadstring (lua_State *L, const char *s);
    //	    Loads a string as a Lua chunk. This function uses lua_load to load
    //	    the chunk in the zero-terminated string s.
    //	    This function returns the same results as lua_load.
    //	    Also as lua_load, this function only loads the chunk; it does not
    //	    run it.
    public static int luaL_loadstring ( lua_State thread, String s ) {
        return luaL_loadbuffer ( thread, s, s );
    }

    // luaL_newmetatable
    // int luaL_newmetatable (lua_State *L, const char *tname);
    //	    If the registry already has the key tname, returns 0. Otherwise,
    //	    creates a new table to be used as a metatable for userdata, adds it
    //	    to the registry with key tname, and returns 1.
    //	    In both cases pushes onto the stack the final value associated with
    //	    tname in the registry.
    public static boolean luaL_newmetatable ( lua_State thread, String tname ) {
        lua_getfield ( thread, LUA_REGISTRYINDEX, tname );  /* get registry.name */
        if (  ! lua_isnil ( thread, -1 ) ) /* name already in use? */ {
            return false;  /* leave previous value on top, but return 0 */
        }
        lua_pop ( thread, 1 );
        lua_newtable ( thread );  /* create metatable */
        lua_pushvalue ( thread, -1 );
        lua_setfield ( thread, LUA_REGISTRYINDEX, tname );  /* registry.name = metatable */
        return true;
    }

    // luaL_newstate
    // lua_State *luaL_newstate (void);
    //	    Creates a new Lua state. It calls lua_newstate with an allocator
    //	    based on the standard C realloc function and then sets a panic
    //	    function (see lua_atpanic) that prints an error message to the
    //	    standard error output in case of fatal errors.
    //	    Returns the new state, or NULL if there is a memory allocation
    //	    error.
    public static final class panic implements JavaFunction {

        public int Call ( lua_State thread ) throws LuaRuntimeException {
            throw new LuaRuntimeException ( "PANIC: unprotected error in call to Lua API (" + lua_tostring ( thread, -1 ) + ")" );
        }
    }

    public static lua_State luaL_newstate () {
        lua_State thread = lua_newstate ();
        if ( thread != null ) {
            lua_atpanic ( thread, new panic () );
        }
        return thread;
    }

    public static final lua_State lua_open () {
        return luaL_newstate ();
    }

    // luaL_openlibs
    // void luaL_openlibs (lua_State *L);
    //	    Opens all standard Lua libraries into the given state.
    public static void luaL_openlibs ( lua_State thread ) {
        luaL_Reg[] luaReg = new luaL_Reg[] {
            new luaL_Reg ( "", new LuaBaseLib.luaopen_base () ),
            new luaL_Reg ( LuaDebugLib.LUA_DBLIBNAME, new LuaDebugLib.luaopen_debug () ),
            new luaL_Reg ( LuaPackageLib.LUA_LOADLIBNAME, new LuaPackageLib.luaopen_package () ),
            new luaL_Reg ( LuaTableLib.LUA_TABLIBNAME, new LuaTableLib.luaopen_table () ),
            new luaL_Reg ( LuaIOLib.LUA_IOLIBNAME, new LuaIOLib.luaopen_io () ),
            new luaL_Reg ( LuaOSLib.LUA_OSLIBNAME, new LuaOSLib.luaopen_os () ),
            new luaL_Reg ( LuaStringLib.LUA_STRLIBNAME, new LuaStringLib.luaopen_string () ),
            new luaL_Reg ( LuaMathLib.LUA_MATHLIBNAME, new LuaMathLib.luaopen_math () ),
        };

        for ( int iLib = 0; iLib < luaReg.length; iLib ++ ) {
            lua_pushjavafunction ( thread, luaReg[iLib].GetJavaFunction () );
            lua_pushstring ( thread, luaReg[iLib].GetFunctionName () );
            lua_call ( thread, 1, 0 );
        }
    }

    // luaL_optint
    // int luaL_optint (lua_State *L, int narg, int d);
    //	    If the function argument narg is a number, returns this number cast
    //	    to an int. If this argument is absent or is nil, returns d.
    //	    Otherwise, raises an error.
    public static int luaL_optint ( lua_State thread, int narg, int def ) {
        return luaL_optinteger ( thread, narg, def );
    }

    // luaL_optinteger
    // lua_Integer luaL_optinteger (lua_State *L, int narg, lua_Integer d);
    //	    If the function argument narg is a number, returns this number cast
    //	    to a lua_Integer. If this argument is absent or is nil, returns d.
    //	    Otherwise, raises an error.
    public static int luaL_optinteger ( lua_State thread, int narg, int def ) {
        return lua_isnoneornil ( thread, narg ) ? def : luaL_checkinteger ( thread, narg );
    }

    // luaL_optlong
    // long luaL_optlong (lua_State *L, int narg, long d);
    //	    If the function argument narg is a number, returns this number cast
    //	    to a long. If this argument is absent or is nil, returns d.
    //	    Otherwise, raises an error.
    public static long luaL_optlong ( lua_State thread, int narg, long d ) {
        return ( long ) luaL_optinteger ( thread, narg, ( int ) d );
    }

    // luaL_optstring
    // const char *luaL_optstring (lua_State *L, int narg, const char *d);
    //	    If the function argument narg is a string, returns this string. If
    //	    this argument is absent or is nil, returns d. Otherwise, raises an
    //	    error.
    public static String luaL_optstring ( lua_State thread, int narg, String d ) {
        return luaL_optlstring ( thread, narg, d );
    }

    // luaL_optnumber
    // lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number d);
    //	    If the function argument narg is a number, returns this number. If
    //	    this argument is absent or is nil, returns d. Otherwise, raises an error.
    public static double luaL_optnumber ( lua_State thread, int narg, double def ) {
        return LuaAPI.lua_isnoneornil ( thread, narg ) ? def : luaL_checknumber ( thread, narg );
    }

    // luaL_optlstring
    // const char *luaL_optlstring (lua_State *L, int narg, const char *d);
    //	    If the function argument narg is a string, returns this string. If
    //	    this argument is absent or is nil, returns d. Otherwise, raises an
    //	    error.
    public static String luaL_optlstring ( lua_State thread, int narg, String def ) {
        if ( LuaAPI.lua_isnoneornil ( thread, narg ) ) {
            return def;
        }
        else {
            return LuaAPI.luaL_checklstring ( thread, narg );
        }
    }

    // luaL_prepbuffer
    // char *luaL_prepbuffer (luaL_Buffer *B);
    //	    Returns an address to a space of size LUAL_BUFFERSIZE where you can 
    //	    copy a string to be added to buffer B (see luaL_Buffer). After 
    //	    copying the string into this space you must call luaL_addsize with 
    //	    the size of the string to actually add it to the buffer. 
    public static String luaL_perpbuffer ( luaL_Buffer B ) {
        return B.m_strValue;
    }

    // luaL_pushresult
    // void luaL_pushresult (luaL_Buffer *B);
    //	    Finishes the use of buffer B leaving the final string on the top of
    //	    the stack.
    public static final void luaL_pushresult ( luaL_Buffer B ) {
        lua_pushstring ( B.GetThread (), B.m_strValue );
    }
    // luaL_ref
    // int luaL_ref (lua_State *L, int t);
    //	    Creates and returns a reference, in the table at index t, for the
    //	    object at the top of the stack (and pops the object).
    //	    A reference is a unique integer key. As long as you do not manually
    //	    add integer keys into table t, luaL_ref ensures the uniqueness of
    //	    the key it returns. You can retrieve an object referred by reference
    //	    r by calling lua_rawgeti(L, t, r).
    //	    Function luaL_unref frees a reference and its associated object.
    //	    If the object at the top of the stack is nil, luaL_ref returns the
    //	    constant LUA_REFNIL. The constant LUA_NOREF is guaranteed to be
    //	    different from any reference returned by luaL_ref.
    public static final int LUA_NOREF = ( -2 );
    public static final int LUA_REFNIL = ( -1 );

    public static int luaL_ref ( lua_State thread, int t ) {
        int ref;
        t = abs_index ( thread, t );
        if ( lua_isnil ( thread, -1 ) ) {
            lua_pop ( thread, 1 );  /* remove from stack */
            return LUA_REFNIL;  /* `nil' has a unique fixed reference */
        }
        lua_rawgeti ( thread, t, FREELIST_REF );  /* get first free element */
        ref = ( int ) lua_tointeger ( thread, -1 );  /* ref = t[FREELIST_REF] */
        lua_pop ( thread, 1 );  /* remove it from stack */
        if ( ref != 0 ) {  /* any free element? */
            lua_rawgeti ( thread, t, ref );  /* remove it from list */
            lua_rawseti ( thread, t, FREELIST_REF );  /* (t[FREELIST_REF] = t[ref]) */
        }
        else {  /* no free elements */
            ref = ( int ) lua_objlen ( thread, t );
            ref ++;  /* create new reference */
        }
        lua_rawseti ( thread, t, ref );
        return ref;
    }

    // luaL_typename
    // const char *luaL_typename (lua_State *L, int index);
    //	    Returns the name of the type of the value at the given index.
    public static String luaL_typename ( lua_State thread, int index ) {
        return lua_typename ( thread, lua_type ( thread, index ) );
    }

    // luaL_typerror
    // int luaL_typerror (lua_State *L, int narg, const char *tname);
    //	    Generates an error with a message like the following:
    //	    location: bad argument narg to 'func' (tname expected, got rt)
    //	    where location is produced by luaL_where, func is the name of the
    //	    current function, and rt is the type name of the actual argument.
    public static int luaL_typerror ( lua_State thread, int narg, String tname ) {
        String msg = lua_pushfstring ( thread, tname + " expected, got " + luaL_typename ( thread, narg ) );
        return luaL_argerror ( thread, narg, msg );
    }
    // luaL_unref
    // void luaL_unref (lua_State *L, int t, int ref);
    //	    Releases reference ref from the table at index t (see luaL_ref). The
    //	    entry is removed from the table, so that the referred object can be
    //	    collected. The reference ref is also freed to be used again.
    //	    If ref is LUA_NOREF or LUA_REFNIL, luaL_unref does nothing.
    public static final int FREELIST_REF = 0;	/* free list of references */


    public static void luaL_unref ( lua_State thread, int t, int ref ) {
        if ( ref >= 0 ) {
            t = abs_index ( thread, t );
            lua_rawgeti ( thread, t, FREELIST_REF );
            lua_rawseti ( thread, t, ref );  /* t[ref] = t[FREELIST_REF] */
            lua_pushinteger ( thread, ref );
            lua_rawseti ( thread, t, FREELIST_REF );  /* t[FREELIST_REF] = ref */
        }
    }

    // luaL_where
    // void luaL_where (lua_State *L, int lvl);
    //	    Pushes onto the stack a string identifying the current position of
    //	    the control at level lvl in the call stack. Typically this string
    //	    has the following format:
    //	    chunkname:currentline:
    //	    Level 0 is the running function, level 1 is the function that called
    //	    the running function, etc.
    //	    This function is used to build a prefix for error messages.
    public static void luaL_where ( lua_State thread, int level ) {
        lua_Debug ar = new lua_Debug ();
        if ( lua_getstack ( thread, level, ar ) != 0 ) /* check function at level */ {
            lua_getinfo ( thread, "Sl", ar );  /* get info about it */
            if ( ar.m_iCurrentLine > 0 ) /* is there info? */ {
                lua_pushstring ( thread, ar.m_strShortSource + ":" + ar.m_iCurrentLine + ": " );
                return;
            }
        }
        lua_pushliteral ( thread, "" );  /* else, no information available... */
    }
    // lua_gc
    // int lua_gc (lua_State *L, int what, int data);
    //	    Controls the garbage collector. 
    //	    This function performs several tasks, according to the value of the
    //	    parameter.
    public static final int LUA_GCSTOP = 0;
    public static final int LUA_GCRESTART = 1;
    public static final int LUA_GCCOLLECT = 2;
    public static final int LUA_GCCOUNT = 3;
    public static final int LUA_GCCOUNTB = 4;
    public static final int LUA_GCSTEP = 5;
    public static final int LUA_GCSETPAUSE = 6;
    public static final int LUA_GCSETSTEPMUL = 7;

    public static int lua_gc ( lua_State thread, int what, int data ) {
        int res = 0;

        switch ( what ) {
            /*case LUA_GCSTOP: 
            {
            g->GCthreshold = MAX_LUMEM;
            break;
            }*/
            case LUA_GCRESTART:
            case LUA_GCCOLLECT: {
                System.gc ();
                System.gc ();
                System.gc ();
                break;
            }
            case LUA_GCCOUNT: {
                /* GC values are expressed in Kbytes: #bytes/2^10 */
                res = ( int ) ( ( Runtime.getRuntime ().totalMemory () - Runtime.getRuntime ().freeMemory () ) >> 10 );
                break;
            }
            case LUA_GCCOUNTB: {
                res = ( int ) ( ( Runtime.getRuntime ().totalMemory () - Runtime.getRuntime ().freeMemory () ) & 0x3ff );
                break;
            }
            /*case LUA_GCSTEP: 
            {
            lu_mem a = (cast(lu_mem, data) << 10);
            if (a <= g->totalbytes)
            g->GCthreshold = g->totalbytes - a;
            else
            g->GCthreshold = 0;
            while (g->GCthreshold <= g->totalbytes)
            luaC_step(L);
            if (g->gcstate == GCSpause)  // end of cycle?
            res = 1;  // signal it 
            break;
            }
            case LUA_GCSETPAUSE: 
            {
            res = g->gcpause;
            g->gcpause = data;
            break;
            }
            case LUA_GCSETSTEPMUL: 
            {
            res = g->gcstepmul;
            g->gcstepmul = data;
            break;
            }*/
            default: {
                res = -1;  /* invalid option */
            }
        }
        return res;
    }

    // luaL_register
    // void luaL_register (lua_State *L, const char *libname, const luaL_Reg *l);
    //	    Opens a library.
    //	    When called with libname equal to NULL, it simply registers all
    //	    functions in the list l (see luaL_Reg) into the table on the top of
    //	    the stack.
    //	    When called with a non-null libname, luaL_register creates a new
    //	    table t, sets it as the value of the global variable libname, sets
    //	    it as the value of package.loaded[libname], and registers on it all
    //	    functions in the list l. If there is a table in
    //	    package.loaded[libname] or in variable libname, reuses this table
    //	    instead of creating a new one.
    //	    In any case the function leaves the table on the top of the stack.
    public static void luaL_register ( lua_State thread, String libname, luaL_Reg[] luaReg ) {
        luaI_openlib ( thread, libname, luaReg, 0 );
    }

    public static String luaL_findtable ( lua_State thread, int idx, String fname, int szhint ) {
        String e = null;
        lua_pushvalue ( thread, idx );

        do {
            int i = fname.indexOf ( '.' );
            int iLen = 0;
            if ( i == -1 ) {
                e = fname.substring ( fname.length () );
                iLen = fname.length ();
            }
            else {
                e = fname.substring ( i );
                iLen = fname.length () - e.length ();
            }

            lua_pushlstring ( thread, fname, iLen );   // FIXME: is it right?

            lua_rawget ( thread, -2 );
            if ( lua_isnil ( thread, -1 ) ) {
                lua_pop ( thread, 1 );

                int iRecs = szhint;
                if ( e.length () == 0 || e.charAt ( 0 ) != '.' ) {
                    iRecs = 1;
                }

                lua_createtable ( thread, 0, iRecs );

                lua_pushlstring ( thread, fname, iLen );   // FIXME: is it right?

                lua_pushvalue ( thread, -2 );
                lua_settable ( thread, -4 );
            }
            else if (  ! lua_istable ( thread, -1 ) ) {
                lua_pop ( thread, 2 );
                return fname;
            }
            lua_remove ( thread, -2 );

            if ( e.length () == 0 ) {
                e = "End";
            }
            else {
                fname = e.substring ( 1 );
            }
        } while ( e.charAt ( 0 ) == '.' ); // FIXME:

        return null;
    }

    private static void luaI_openlib ( lua_State thread, String libname, luaL_Reg[] luaReg, int nup ) {

        if ( libname != null ) {
            int size = luaReg.length;

            luaL_findtable ( thread, LUA_REGISTRYINDEX, "_LOADED", 1 );
            lua_getfield ( thread, -1, libname );  // get _LOADED[libname]

            if (  ! lua_istable ( thread, -1 ) ) {
                lua_pop ( thread, 1 );
                if ( luaL_findtable ( thread, LUA_GLOBALSINDEX, libname, size ) != null ) {
                    luaL_error ( thread, "name conflict for module " + libname );
                }
                lua_pushvalue ( thread, -1 );
                lua_setfield ( thread, -3, libname );
            }
            lua_remove ( thread, -2 );
            lua_insert ( thread,  - ( nup + 1 ) );
        }

        for ( int k = 0; k < luaReg.length; k ++ ) {
            for ( int i = 0; i < nup; i ++ ) {
                lua_pushvalue ( thread,  - nup );
            }
            lua_pushjavafunction ( thread, luaReg[k].GetJavaFunction (), nup );
            lua_setfield ( thread,  - ( nup + 2 ), luaReg[k].GetFunctionName () );
        }
        lua_pop ( thread, nup );
    }

    public static int luaV_strcmp ( String ls, String rs ) {
        int ll = ls.length ();
        int lr = rs.length ();
        int min = Math.min ( ll, lr );

        for ( int i = 0; i < min; i ++ ) {
            if ( ls.charAt ( i ) > rs.charAt ( i ) ) {
                return 1;
            }
            else if ( ls.charAt ( i ) < rs.charAt ( i ) ) {
                return -1;
            }
        }

        if ( ll > lr ) {
            return 1;
        }
        else if ( ll < lr ) {
            return -1;
        }
        else {
            return 0;
        }
    }

    public static boolean luaV_lessthan ( lua_State thread, Object objectL, Object objectR ) throws LuaRuntimeException {
        if ( lua_typebyobject ( objectL ) != lua_typebyobject ( objectR ) ) {
            return luaG_ordererror ( thread, objectL, objectR );
        }
        else if ( lua_typebyobject ( objectL ) == LUA_TNUMBER ) {
            return ( ( Double ) objectL ).doubleValue () < ( ( Double ) objectR ).doubleValue ();
        }
        else if ( lua_typebyobject ( objectL ) == LUA_TSTRING ) {
            return luaV_strcmp ( ( String ) objectL, ( String ) objectR ) < 0;
        }
        else {
            Function function = ( Function ) lua_State.GetEqualMetaTableObjectByObjects ( objectL, objectR, LVM.TM_LT );
            if ( function != null ) {
                Boolean result = ( Boolean ) thread.CallMetaTable ( function, objectL, objectR );
                return result.booleanValue ();
            }
        }
        String strTypeL = LuaAPI.lua_typename ( thread, LuaAPI.lua_typebyobject ( objectL ) );
        String strTypeR = LuaAPI.lua_typename ( thread, LuaAPI.lua_typebyobject ( objectR ) );

        return luaG_ordererror ( thread, objectL, objectR );
    }

    public static boolean luaV_lessequal ( lua_State thread, Object objectL, Object objectR ) throws LuaRuntimeException {
        if ( lua_typebyobject ( objectL ) != lua_typebyobject ( objectR ) ) {
            return luaG_ordererror ( thread, objectL, objectR );
        }
        else if ( lua_typebyobject ( objectL ) == LUA_TNUMBER ) {
            return ( ( Double ) objectL ).doubleValue () <= ( ( Double ) objectR ).doubleValue ();
        }
        else if ( lua_typebyobject ( objectL ) == LUA_TSTRING ) {
            return luaV_strcmp ( ( String ) objectL, ( String ) objectR ) <= 0;
        }
        else {
            Function function = ( Function ) lua_State.GetEqualMetaTableObjectByObjects ( objectL, objectR, LVM.TM_LE );
            if ( function != null ) {
                Boolean result = ( Boolean ) thread.CallMetaTable ( function, objectL, objectR );
                return result.booleanValue ();
            }
            function = ( Function ) lua_State.GetEqualMetaTableObjectByObjects ( objectR, objectL, LVM.TM_LT );
            if ( function != null ) {
                Boolean result = ( Boolean ) thread.CallMetaTable ( function, objectR, objectL );
                return  ! result.booleanValue ();
            }
        }

        return luaG_ordererror ( thread, objectL, objectR );
    }

    public static boolean luaV_equal ( lua_State thread, Object objectL, Object objectR ) throws LuaRuntimeException {
        if ( lua_typebyobject ( objectL ) != lua_typebyobject ( objectR ) ) {
            return luaG_ordererror ( thread, objectL, objectR );
        }
        switch ( lua_typebyobject ( objectL ) ) {
            case LUA_TNIL: {
                return true;
            }
            case LUA_TBOOLEAN: {
                return ( ( Boolean ) objectL ).booleanValue () == ( ( Boolean ) objectR ).booleanValue ();
            }
            case LUA_TNUMBER: {
                return ( ( Double ) objectL ).doubleValue () == ( ( Double ) objectR ).doubleValue ();
            }
            case LUA_TSTRING: {
                return ( ( String ) objectL ).equals ( ( String ) objectR );
            }
            case LUA_TTABLE:
            case LUA_TUSERDATA: {
                if ( objectL == objectR ) {
                    return true;
                }
                break;
            }
            default: {
                return objectL == objectR;
            }

        }

        Function function = ( Function ) lua_State.GetEqualMetaTableObjectByObjects ( objectL, objectR, LVM.TM_EQ );
        if ( function == null ) {
            return false;
        }

        Boolean result = ( Boolean ) thread.CallMetaTable ( function, objectL, objectR );
        return result.booleanValue ();
    }

    public static boolean luaV_tostring ( lua_State thread, int objIndex ) {
        Object object = thread.GetObjectValue ( objIndex );
        if ( lua_type ( thread, objIndex ) != LUA_TNUMBER ) {
            return false;
        }

        Double doubleValue = ( ( Double ) object );
        if ( doubleValue.doubleValue () - doubleValue.intValue () == 0 ) {
            thread.SetObjectValue ( objIndex, String.valueOf ( ( ( Double ) object ).intValue () ) );
        }
        else {
            thread.SetObjectValue ( objIndex, String.valueOf ( ( ( Double ) object ).doubleValue () ) );
        }
        return true;
    }

    private static String luaF_getlocalname ( LuaFunction luaFunction, int local_number, int iIP ) {
        for ( int i = 0; i < luaFunction.GetLocalVairablesSize () && luaFunction.GetLocalVariable ( i ).GetStartIP () <= iIP; i ++ ) {
            if ( iIP < luaFunction.GetLocalVariable ( i ).GetEndIP () ) /* is variable active? */ {
                local_number --;
                if ( local_number == 0 ) {
                    return luaFunction.GetLocalVariable ( i ).GetName ();
                }
            }
        }
        return null;  /* not found */
    }

    private final static char opmode ( int t, int a, int b, int c, int m ) {
        return ( char ) ( ( ( t ) << 7 ) | ( ( a ) << 6 ) | ( ( b ) << 4 ) | ( ( c ) << 2 ) | ( m ) );
    }
    private static final int OpArgN = 0;  /* argument is not used */

    private static final int OpArgU = 1;  /* argument is used */

    private static final int OpArgR = 2;  /* argument is a register or a jump offset */

    private static final int OpArgK = 3;	/* argument is a constant or register/constant */

    private static final int iABC = 0;
    private static final int iABx = 1;
    private static final int iAsBx = 2;
    private static char luaP_opmodes[] = new char[] {
        /*       T  A    B       C     mode		   opcode	*/
        opmode ( 0, 1, OpArgR, OpArgN, iABC ) /* OP_MOVE */, opmode ( 0, 1, OpArgK, OpArgN, iABx ) /* OP_LOADK */, opmode ( 0, 1, OpArgU, OpArgU, iABC ) /* OP_LOADBOOL */, opmode ( 0, 1, OpArgR, OpArgN, iABC ) /* OP_LOADNIL */, opmode ( 0, 1, OpArgU, OpArgN, iABC ) /* OP_GETUPVAL */, opmode ( 0, 1, OpArgK, OpArgN, iABx ) /* OP_GETGLOBAL */, opmode ( 0, 1, OpArgR, OpArgK, iABC ) /* OP_GETTABLE */, opmode ( 0, 0, OpArgK, OpArgN, iABx ) /* OP_SETGLOBAL */, opmode ( 0, 0, OpArgU, OpArgN, iABC ) /* OP_SETUPVAL */, opmode ( 0, 0, OpArgK, OpArgK, iABC ) /* OP_SETTABLE */, opmode ( 0, 1, OpArgU, OpArgU, iABC ) /* OP_NEWTABLE */, opmode ( 0, 1, OpArgR, OpArgK, iABC ) /* OP_SELF */, opmode ( 0, 1, OpArgK, OpArgK, iABC ) /* OP_ADD */, opmode ( 0, 1, OpArgK, OpArgK, iABC ) /* OP_SUB */, opmode ( 0, 1, OpArgK, OpArgK, iABC ) /* OP_MUL */, opmode ( 0, 1, OpArgK, OpArgK, iABC ) /* OP_DIV */, opmode ( 0, 1, OpArgK, OpArgK, iABC ) /* OP_MOD */, opmode ( 0, 1, OpArgK, OpArgK, iABC ) /* OP_POW */, opmode ( 0, 1, OpArgR, OpArgN, iABC ) /* OP_UNM */, opmode ( 0, 1, OpArgR, OpArgN, iABC ) /* OP_NOT */, opmode ( 0, 1, OpArgR, OpArgN, iABC ) /* OP_LEN */, opmode ( 0, 1, OpArgR, OpArgR, iABC ) /* OP_CONCAT */, opmode ( 0, 0, OpArgR, OpArgN, iAsBx ) /* OP_JMP */, opmode ( 1, 0, OpArgK, OpArgK, iABC ) /* OP_EQ */, opmode ( 1, 0, OpArgK, OpArgK, iABC ) /* OP_LT */, opmode ( 1, 0, OpArgK, OpArgK, iABC ) /* OP_LE */, opmode ( 1, 1, OpArgR, OpArgU, iABC ) /* OP_TEST */, opmode ( 1, 1, OpArgR, OpArgU, iABC ) /* OP_TESTSET */, opmode ( 0, 1, OpArgU, OpArgU, iABC ) /* OP_CALL */, opmode ( 0, 1, OpArgU, OpArgU, iABC ) /* OP_TAILCALL */, opmode ( 0, 0, OpArgU, OpArgN, iABC ) /* OP_RETURN */, opmode ( 0, 1, OpArgR, OpArgN, iAsBx ) /* OP_FORLOOP */, opmode ( 0, 1, OpArgR, OpArgN, iAsBx ) /* OP_FORPREP */, opmode ( 1, 0, OpArgN, OpArgU, iABC ) /* OP_TFORLOOP */, opmode ( 0, 0, OpArgU, OpArgU, iABC ) /* OP_SETLIST */, opmode ( 0, 0, OpArgN, OpArgN, iABC ) /* OP_CLOSE */, opmode ( 0, 1, OpArgU, OpArgN, iABx ) /* OP_CLOSURE */, opmode ( 0, 1, OpArgU, OpArgN, iABC )		/* OP_VARARG */

    };

    private static final int getOpMode ( int m ) {
        return luaP_opmodes[m] & 3;
    }

    private static final int getBMode ( int m ) {
        return ( luaP_opmodes[m] >> 4 ) & 3;
    }

    private static final int getCMode ( int m ) {
        return ( luaP_opmodes[m] >> 2 ) & 3;
    }

    private static final int testAMode ( int m ) {
        return luaP_opmodes[m] & ( 1 << 6 );
    }

    private static final int testTMode ( int m ) {
        return luaP_opmodes[m] & ( 1 << 7 );
    }

    private static final int GET_OPCODE ( int i ) {
        return LVM.GetInstruction ( i );
    }

    private static boolean precheck ( LuaFunction luaFunction ) {
        if (  ! ( luaFunction.GetMaxStackSize () <= MAXSTACK ) )
            return false;
        if (  ! ( luaFunction.GetUpValuesSize () <= luaFunction.GetUpValuesQuantity () ) )
            return false;
        if (  ! ( luaFunction.GetDebugLinesQuantity () == luaFunction.GetSizeOpCode () || luaFunction.GetDebugLinesQuantity () == 0 ) )
            return false;
        if (  ! ( GET_OPCODE ( luaFunction.GetOpCode ( luaFunction.GetSizeOpCode () - 1 ) ) == LVM.OP_RETURN ) )
            return false;

        return true;
    }

// #define checkopenop(pt,pc)	luaG_checkopenop((pt)->code[(pc)+1])
    private static int luaG_checkopenop ( int i ) {
        switch ( GET_OPCODE ( i ) ) {
            case LVM.OP_CALL:
            case LVM.OP_TAILCALL:
            case LVM.OP_RETURN:
            case LVM.OP_SETLIST: {
                if (  ! ( LVM.GetB9 ( i ) == 0 ) )
                    return 0;
                return 1;
            }
            default:
                return 0;  // invalid instruction after an open call 

        }
    }

    private static int checkArgMode ( LuaFunction luaFunction, int r, int mode ) {
        switch ( mode ) {
            case OpArgN:
                if (  ! ( r == 0 ) )
                    return 0;
                break;
            case OpArgU:
                break;
            case OpArgR:
                if (  ! ( ( r ) < luaFunction.GetMaxStackSize () ) )
                    return 0;
                break;
            case OpArgK:
                //check(LVM.IsConstant( r ) ? LVM.GetConstantIndex( r ) < luaFunction.GetConstantsQuantity() : r < luaFunction.GetMaxStackSize());
                if (  ! ( LVM.IsConstant ( r ) ? LVM.GetConstantIndex ( r ) < luaFunction.GetConstantsQuantity () : r < luaFunction.GetMaxStackSize () ) )
                    return 0;
                break;
        }
        return 1;
    }

    private static int symbexec ( LuaFunction luaFunction, int lastpc, int reg ) {
        int pc;
        int last;  // stores position of last instruction that changed `reg' /

        last = luaFunction.GetSizeOpCode () - 1;  // points to final return (a `neutral' instruction) 

        if (  ! ( precheck ( luaFunction ) ) ) {
            return 0;
        }

        for ( pc = 0; pc < lastpc; pc ++ ) {
            int i = luaFunction.GetOpCode ( pc );
            int op = GET_OPCODE ( i );
            int a = LVM.GetA8 ( i );
            int b = 0;
            int c = 0;
            if (  ! ( op < NUM_OPCODES ) )
                return 0;
            if (  ! ( ( a ) < luaFunction.GetMaxStackSize () ) )
                return 0;
            switch ( getOpMode ( op ) ) {
                case iABC: {
                    b = LVM.GetB9 ( i );
                    c = LVM.GetC9 ( i );
                    if ( ( checkArgMode ( luaFunction, b, getBMode ( op ) ) ) == 0 )
                        return 0; //check(checkArgMode(pt, b, getBMode(op)));

                    if ( ( checkArgMode ( luaFunction, c, getCMode ( op ) ) ) == 0 )
                        return 0; //check(checkArgMode(pt, c, getCMode(op)));		    

                    break;
                }
                case iABx: {
                    b = LVM.GetBx ( i );
                    if ( getBMode ( op ) == OpArgK ) {
                        if (  ! ( b < luaFunction.GetConstantsQuantity () ) )
                            return 0;
                    }
                    break;
                }
                case iAsBx: {
                    b = LVM.GetSBx ( i );
                    if ( getBMode ( op ) == OpArgR ) {
                        int dest = pc + 1 + b;
                        if (  ! ( 0 <= dest && dest < luaFunction.GetSizeOpCode () ) )
                            return 0;
                        if ( dest > 0 ) {
                            // cannot jump to a setlist count 
                            int d = luaFunction.GetOpCode ( dest - 1 );
                            if (  ! (  ! ( GET_OPCODE ( d ) == LVM.OP_SETLIST && LVM.GetC9 ( d ) == 0 ) ) )
                                return 0; //check(!(GET_OPCODE(d) == OP_SETLIST && GETARG_C(d) == 0)); 

                        }
                    }
                    break;
                }
            }
            if ( testAMode ( op ) != 0 ) {
                if ( a == reg ) {
                    last = pc;  // change register `a' 

                }
            }
            if ( testTMode ( op ) != 0 ) {
                if (  ! ( pc + 2 < luaFunction.GetSizeOpCode () ) )
                    return 0; //check();  // check skip 

                if (  ! ( GET_OPCODE ( luaFunction.GetOpCode ( pc + 1 ) ) == LVM.OP_JMP ) )
                    return 0; //check();		

            }
            switch ( op ) {
                case LVM.OP_LOADBOOL: {
                    if (  ! ( c == 0 || pc + 2 < luaFunction.GetSizeOpCode () ) )
                        return 0;//check(c == 0 || pc + 2 < pt ->  sizecode);  // check its jump 

                    break;
                }
                case LVM.OP_LOADNIL: {
                    if ( a <= reg && reg <= b ) {
                        last = pc;  // set registers from `a' to `b' 

                    }
                    break;
                }
                case LVM.OP_GETUPVAL:
                case LVM.OP_SETUPVAL: {
                    if (  ! ( b < luaFunction.GetUpValuesQuantity () ) )
                        return 0;
                    break;
                }
                case LVM.OP_GETGLOBAL:
                case LVM.OP_SETGLOBAL: {
                    if (  ! ( luaFunction.GetConstant ( b ) instanceof String ) )
                        return 0;//check(ttisstring(&  pt ->  k[b]));

                    break;
                }
                case LVM.OP_SELF: {
                    if (  ! ( ( a + 1 ) < luaFunction.GetMaxStackSize () ) )
                        return 0;//checkreg(pt, a + 1);

                    if ( reg == a + 1 ) {
                        last = pc;
                    }
                    break;
                }
                case LVM.OP_CONCAT: {
                    if (  ! ( b < c ) )
                        return 0; // at least two operands 

                    break;
                }
                case LVM.OP_TFORLOOP: {
                    if (  ! ( c >= 1 ) )
                        return 0; // at least one result (control variable)  

                    if (  ! ( ( a + 2 + c ) < luaFunction.GetMaxStackSize () ) )
                        return 0;//checkreg(pt, );  // space for results 

                    if ( reg >= a + 2 ) {
                        last = pc;  // affect all regs above its base 

                    }
                    break;
                }
                case LVM.OP_FORLOOP:
                case LVM.OP_FORPREP:
                    if (  ! ( ( a + 3 ) < luaFunction.GetMaxStackSize () ) )
                        return 0; //checkreg(pt, a + 3);
                // go through

                case LVM.OP_JMP: {
                    int dest = pc + 1 + b;
                    // not full check and jump is forward and do not skip `lastpc'? 
                    if ( reg != ( ( 1 << 8 ) - 1 ) && pc < dest && dest <= lastpc ) {
                        pc += b;  // do the jump 

                    }
                    break;
                }
                case LVM.OP_CALL:
                case LVM.OP_TAILCALL: {
                    if ( b != 0 ) {
                        if (  ! ( ( a + b - 1 ) < luaFunction.GetMaxStackSize () ) )
                            return 0;
                    }
                    c --;  // c = num. returns 

                    if ( c == LUA_MULTRET ) {
                        //check(checkopenop(pt, pc));
                        // TODO: FIXME:
                    }
                    else if ( c != 0 ) {
                        if (  ! ( ( a + c - 1 ) < luaFunction.GetMaxStackSize () ) )
                            return 0;
                    }
                    if ( reg >= a ) {
                        last = pc;  // affect all registers above base 

                    }
                    break;
                }
                case LVM.OP_RETURN: {
                    b --;  // b = num. returns 

                    if ( b > 0 ) {
                        if (  ! ( ( a + b - 1 ) < luaFunction.GetMaxStackSize () ) )
                            return 0;
                    }
                    break;
                }
                case LVM.OP_SETLIST: {
                    if ( b > 0 ) {
                        if (  ! ( ( a + b ) < luaFunction.GetMaxStackSize () ) )
                            return 0;
                    }
                    if ( c == 0 ) {
                        pc ++;
                    }
                    break;
                }
                case LVM.OP_CLOSURE: {
                    int nup, j;
                    if (  ! ( b < luaFunction.GetLuaFunctionsQuantity () ) )
                        return 0;
                    nup = luaFunction.GetLuaFunction ( b ).GetUpValuesQuantity ();
                    if (  ! ( pc + nup < luaFunction.GetSizeOpCode () ) )
                        return 0;
                    for ( j = 1; j <= nup; j ++ ) {
                        int op1 = GET_OPCODE ( luaFunction.GetOpCode ( pc + j ) );
                        if (  ! ( op1 == LVM.OP_GETUPVAL || op1 == LVM.OP_MOVE ) )
                            return 0;
                    }
                    if ( reg != ( ( 1 << 8 ) - 1 ) ) // tracing? 
                    {
                        pc += nup;  // do not 'execute' these pseudo-instructions

                    }
                    break;
                }
                case LVM.OP_VARARG: {
                    //check((pt ->  is_vararg & VARARG_ISVARARG) && !(pt ->  is_vararg & VARARG_NEEDSARG));
                    if (  ! ( luaFunction.GetIsVararg () &&  ! luaFunction.GetNeedsArg () ) )
                        return 0;
                    b --;
                    if ( b == LUA_MULTRET ) {
                        // TODO: FIXME:
                        //check(checkopenop(pt, pc));
                    }
                    // FIXME:
                    if (  ! ( ( a + b - 1 ) < luaFunction.GetMaxStackSize () ) )
                        return 0; //checkreg(pt, a + b - 1);

                    break;
                }
                default:
                    break;
            }
        }
        return luaFunction.GetOpCode ( last );
    }

    private static String kname ( LuaFunction luaFunction, int c ) {
        if ( LVM.IsConstant ( c ) && luaFunction.GetConstant ( LVM.GetConstantIndex ( c ) ) instanceof String ) {
            return ( String ) luaFunction.GetConstant ( LVM.GetConstantIndex ( c ) );
        }
        else {
            return "?";
        }
    }

    private static final int currentpc ( lua_State thread, CallInfo currentCallInfo ) {
        if ( currentCallInfo.GetFunction ().IsJavaFunction () ) {
            return -1;
        }
        return currentCallInfo.GetIP () - 1;
    }

    private static String getobjname ( lua_State thread, CallInfo ci, int stackpos, StringBuffer name ) {
        if ( ci.GetFunction ().IsLuaFunction () ) /* a Lua function? */ {
            LuaFunction luaFunction = ci.GetFunction ().GetLuaFunction ();
            int iIP = currentpc ( thread, ci );

            String localTempString = luaF_getlocalname ( luaFunction, stackpos + 1, iIP );
            if ( localTempString != null ) {
                name.append ( localTempString );
            }
            if ( name.length () != 0 ) /* is a local? */ {
                return "local";
            }
            int i = symbexec ( luaFunction, iIP, stackpos );  /* try symbolic execution */
            switch ( GET_OPCODE ( i ) ) {
                case LVM.OP_GETGLOBAL: {
                    int g = LVM.GetBx ( i );
                    //lua_assert(ttisstring(&p->k[g]));
                    name.append ( LuaBaseLib.ConvertToString ( luaFunction.GetConstant ( g ) ) );
                    return "global";
                }
                case LVM.OP_MOVE: {
                    int a = LVM.GetA8 ( i );
                    int b = LVM.GetB9 ( i );  /* move from `b' to `a' */
                    if ( b < a ) {
                        return getobjname ( thread, ci, b, name );  /* get name for `b' */
                    }
                    break;
                }
                case LVM.OP_GETTABLE: {
                    int k = LVM.GetC9 ( i );  /* key index */
                    name.append ( kname ( luaFunction, k ) );
                    return "field";
                }
                case LVM.OP_GETUPVAL: {
                    int u = LVM.GetB9 ( i );  /* upvalue index */
                    name.append ( luaFunction.GetUpValuesQuantity () > 0 ? LuaBaseLib.ConvertToString ( luaFunction.GetUpValueName ( u ) )
                        : "?" );
                    return "upvalue";
                }
                case LVM.OP_SELF: {
                    int k = LVM.GetC9 ( i );  /* key index */
                    name.append ( kname ( luaFunction, k ) );
                    return "method";
                }
                default: {
                    break;
                }
            }
        }
        return null;  /* no useful name found */
    }

    public static void luaG_typeerror ( lua_State thread, int iIndexValue, Object objectValue, String op ) {
        String name = null;
        String kind = null;
        if ( iIndexValue != -1 ) {
            // Value is in the stack. Get the value name
            objectValue = thread.GetCurrentCallInfo ().GetValue ( iIndexValue );
            StringBuffer stringBuffer = new StringBuffer ();
            kind = getobjname ( thread, thread.GetCurrentCallInfo (), iIndexValue, stringBuffer );
            name = new String ( stringBuffer );
        }

        String t = lua_typename ( thread, lua_typebyobject ( objectValue ) );

        if ( kind != null ) {
            luaG_runerror ( thread, "attempt to " + op + " " + kind + " '" + name + "' (a " + t + " value)" );
        }
        else {
            luaG_runerror ( thread, "attempt to " + op + " a " + t + " value" );
        }
    }

    public static void luaG_runerror ( lua_State thread, String strMessage ) {
        CallInfo ci = thread.GetCurrentCallInfo ();
        LuaFunction luaFunction = getluaproto ( ci );
        if ( luaFunction != null ) {
            strMessage = luaO_chunkid ( luaFunction.GetSource () ) + ":" + currentline ( thread, ci ) + ": " + strMessage;
        }

        lua_pushstring ( thread, strMessage );
        luaG_errormsg ( thread );
    }

    public static boolean luaG_ordererror ( lua_State thread, Object p1, Object p2 ) {
        String t1 = lua_typename ( thread, lua_typebyobject ( p1 ) );
        String t2 = lua_typename ( thread, lua_typebyobject ( p2 ) );

        if ( t1.equals ( t2 ) ) {
            luaG_runerror ( thread, "attempt to compare two " + t1 + " values" );
        }
        else {
            luaG_runerror ( thread, "attempt to compare " + t1 + " with " + t2 );
        }

        return false;
    }

    private static final LuaFunction getluaproto ( CallInfo ci ) {
        return ci != null && ci.GetFunction () != null && ci.GetFunction ().IsLuaFunction ()
            ? ci.GetFunction ().GetLuaFunction ()
            : null;
    }

    public static final lua_Hook lua_gethook ( lua_State thread ) {
        return thread.GetHook ();
    }

    public static final int lua_gethookmask ( lua_State thread ) {
        return thread.GetHookMask ();
    }

    public static final int lua_gethookcount ( lua_State thread ) {
        return thread.GetBaseHookCount ();
    }

    public static int lua_sethook ( lua_State thread, lua_Hook func, int mask, int count ) {
        if ( func == null || mask == 0 ) {  /* turn off hooks? */
            mask = 0;
            func = null;
        }
        thread.SetHook ( func );
        thread.SetBaseHookCount ( count );
        thread.ResetHookCount ();
        thread.SetHookMask ( mask );
        return 1;
    }

    private static String findlocal ( lua_State thread, CallInfo ci, int n ) {
        String name = null;
        LuaFunction fp = getluaproto ( ci );
        if ( ( fp != null ) && ( name = luaF_getlocalname ( fp, n, currentpc ( thread, ci ) ) ) != null ) {
            return name;  // is a local variable in a Lua function 

        }
        else {
            int limit = 0;
            if ( ci == thread.GetCurrentCallInfo () ) {
                limit = thread.GetObjectsStackTop ();
            }
            else {
                for ( int iCallInfo = 0; iCallInfo < thread.GetCallInfosStackTop (); iCallInfo ++ ) {
                    if ( ci == thread.GetCallInfosStack ()[iCallInfo] ) {
                        limit = thread.GetCallInfosStack ()[iCallInfo + 1].GetLocalObjectsStackBase ();
                        break;
                    }
                }
            }

            if ( limit - ci.GetLocalObjectsStackBase () >= n && n > 0 ) // is 'n' inside 'ci' stack? 
            {
                return "(*temporary)";
            }
            else {
                return null;
            }
        }
    }

    public static String lua_getlocal ( lua_State thread, lua_Debug ar, int n ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        CallInfo ci = thread.GetCallInfosStack ()[ar.m_iCallInfoIndex];
        String name = findlocal ( thread, ci, n );
        if ( name != null ) {
            currentCallInfo.PushValue ( currentCallInfo.GetValue ( n - 1 ) );
        }
        return name;
    }

    public static String lua_setlocal ( lua_State thread, lua_Debug ar, int n ) {
        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        CallInfo ci = thread.GetCallInfosStack ()[ar.m_iCallInfoIndex];
        String name = findlocal ( thread, ci, n );
        if ( name != null ) {
            currentCallInfo.SetValue ( n - 1, currentCallInfo.PopValue () );
        }
        return name;
    }
    public static final boolean DEBUG_PRINT = false;
    public static final boolean DEBUG_STACK = false;

    public static final void println ( String s ) {
        if ( DEBUG_PRINT == true ) {
            System.out.println ( s );
        }
    }
}

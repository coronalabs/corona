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
class CallInfo {

    private lua_State m_Thread;
    private int m_iIP;	// Instruction pointer

    private int m_iLocalObjectsStackBase;
    private int m_iResultsWanted;
    private int m_iArgsQuantity;
    private int m_iReturnBase;
    private int m_iTopToRestore;
    private Function m_Function;
    private int m_iTailCalls;

    public final int GetTailCalls () {
        return m_iTailCalls;
    }

    public final void SetTailCalls ( int iTailCalls ) {
        m_iTailCalls = iTailCalls;
    }

    public final void SaveTop () {
        m_iTopToRestore = GetTop ();
    }

    public final void RestoreTop () {
        SetTop ( m_iTopToRestore );
    }

    public final int GetReturnBase () {
        return m_iReturnBase;
    }

    public CallInfo ( lua_State thread, Function function, int iLocalStackBase, int iReturnBase, int iResultsWanted, int iArgsQuantity ) {
        this.m_iArgsQuantity = iArgsQuantity;
        this.m_iResultsWanted = iResultsWanted;
        this.m_iReturnBase = iReturnBase;
        this.m_Function = function;
        this.m_Thread = thread;
        this.m_iIP = 0;
        this.m_iTailCalls = 0;
        this.m_iLocalObjectsStackBase = iLocalStackBase;

        if ( m_Function == null ) {
        }
        else if ( this.m_Function instanceof Function ) {
            if ( m_Function.IsLuaFunction () ) {
                LuaFunction luaFunction = function.GetLuaFunction ();
                if ( function.GetLuaFunction ().GetIsVararg () == false ) {
                    SetTop ( luaFunction.GetParamsQuantity () ); // Clear unneeded arguments

                    SetTop ( luaFunction.GetMaxStackSize () );
                }
                else {
                    CallInfo currentCallInfo = thread.GetCurrentCallInfo ();

                    Table tableOfArgs = null;

                    int iFixedArgs = luaFunction.GetParamsQuantity ();
                    int iExtraArgs = m_iArgsQuantity - iFixedArgs;  // Math.abs

                    if ( iExtraArgs < 0 ) {
                        iExtraArgs = 0;
                    }

                    // In case if FixedArgs < ActualArgs// FIXME: no currentCallInfo, it's silly
                    for (; m_iArgsQuantity < iFixedArgs; m_iArgsQuantity ++ ) {
                        currentCallInfo.PushValue ( null );
                    }

                    this.m_iLocalObjectsStackBase += m_iArgsQuantity;

                    SetTop ( luaFunction.GetMaxStackSize () );

                    // Prepare a table
                    if ( LuaAPI.LUA_COMPAT_VARARG == true ) {
                        if ( luaFunction.GetNeedsArg () ) {

                            tableOfArgs = new Table ( iExtraArgs, 1 );

                            for ( int iArg = 0; iArg < iExtraArgs; iArg ++ ) {
                                tableOfArgs.SetValueNum ( iArg + 1, GetValue ( iFixedArgs - m_iArgsQuantity + iArg ), thread );
                            }

                            tableOfArgs.SetValue ( "n", new Double ( iExtraArgs ), thread );
                        }
                    }

                    CopyStack (  - m_iArgsQuantity, 0, iFixedArgs );
                    ClearStack (  - m_iArgsQuantity, -1 - iExtraArgs );

                    if ( tableOfArgs != null ) {
                        SetValue ( iFixedArgs, tableOfArgs );
                    }
                }
            }
            else {
                JavaFunction javaFunction = function.GetJavaFunction ();
                this.m_iLocalObjectsStackBase = iLocalStackBase;
            //SetTop( LuaAPI.LUA_MIN_STACK_SIZE );
            }
        }
        else {
            // FIXME: may be try to call a metatable? NOT FROM HERE!
            throw new LuaRuntimeException ( "attemp to call a nil value" );
        }
    }

    public void AdjustResults ( int iReturnValuesQuantity ) {
        int iFirstResult = m_Thread.GetObjectsStackTop () - iReturnValuesQuantity;

        if ( this.GetResultsWanted () != LuaAPI.LUA_MULTRET ) {
            for (; iReturnValuesQuantity < this.GetResultsWanted (); iReturnValuesQuantity ++ ) {
                PushValue ( null );
            }

            iReturnValuesQuantity = this.GetResultsWanted ();
        }

        m_Thread.CopyStack ( iFirstResult, GetReturnBase (), iReturnValuesQuantity );
        m_Thread.SetObjectsStackTop ( GetReturnBase () + iReturnValuesQuantity );
    }

    public final int GetResultsWanted () {
        return this.m_iResultsWanted;
    }

    public final Function GetFunction () {
        return ( Function ) this.m_Function;
    }

    public final void SetLocalObjectStackBase ( int iLocalStackBase ) {
        this.m_iLocalObjectsStackBase = iLocalStackBase;
    }

    public final int GetLocalObjectsStackBase () {
        return this.m_iLocalObjectsStackBase;
    }

    public final int GetIP () {
        return this.m_iIP;
    }

    public final void SetIP ( int iIP ) {
        this.m_iIP = iIP;
    }

    private final void IncrementTop () {
        this.m_Thread.SetObjectsStackTop ( this.m_Thread.GetObjectsStackTop () + 1 );
    }

    private final void DecrementTop () {
        this.m_Thread.SetObjectsStackTop ( this.m_Thread.GetObjectsStackTop () - 1 );
    }

    public final void SetTop ( int iIndex ) {
        this.m_Thread.SetObjectsStackTop ( GetLocalObjectsStackBase () + iIndex );
    }

    public final int GetTop () {
        return this.m_Thread.GetObjectsStackTop () - GetLocalObjectsStackBase ();
    }

    public final void SetValue ( int iIndex, Object value ) {
        m_Thread.SetValue ( GetLocalObjectsStackBase () + iIndex, value );
    }

    public final Object GetValue ( int iIndex ) {
        return this.m_Thread.GetValue ( GetLocalObjectsStackBase () + iIndex );
    }

    public void PushValue ( Object value ) {
        int iTop = GetTop ();
        SetValue ( iTop, value );
        IncrementTop ();
    }

    public Object PopValue () {
        int iTop = GetTop ();
        Object value = GetValue ( iTop - 1 );
        SetTop ( iTop - 1 );
        return value;
    }

    public final void ClearStack ( int iFrom, int iTo ) {
        for ( int iIndex = iFrom; iIndex <= iTo; iIndex ++ ) {
            SetValue ( iIndex, null );
        }
    }

    public final void CopyStack ( int iFrom, int iTo, int iLength ) {
        for ( int iIndex = 0; iIndex < iLength; iIndex ++ ) {
            SetValue ( iTo + iIndex, GetValue ( iFrom + iIndex ) );
        }
    }

    public UpValue FindUpValue ( int iIndex ) {
        return this.m_Thread.FindUpValue ( this.GetLocalObjectsStackBase () + iIndex );
    }

    public final void CloseUpValues ( int iIndex ) {
        this.m_Thread.CloseUpValues ( this.GetLocalObjectsStackBase () + iIndex );
    }

    public void Concat ( int iTotal, int iLast, int iResult ) {
        int iFirst = iLast - iTotal + 1;

        Object res = GetValue ( iLast );
        iLast --;
        while ( iFirst <= iLast ) {
            // Optimize for multi string concats
            {
                String resStr = LuaBaseLib.ConvertToString ( res );
                if ( res != null && resStr != null ) {

                    int nStrings = 0;
                    int pos = iLast;
                    while ( iFirst <= pos ) {
                        Object o = GetValue ( pos );
                        pos --;
                        if ( LuaBaseLib.ConvertToString ( o ) == null ) {
                            break;
                        }
                        nStrings ++;
                    }
                    if ( nStrings > 0 ) {
                        StringBuffer concatBuffer = new StringBuffer ();

                        int firstString = iLast - nStrings + 1;
                        while ( firstString <= iLast ) {
                            concatBuffer.append ( LuaBaseLib.ConvertToString ( GetValue ( firstString ) ) );
                            firstString ++;
                        }
                        concatBuffer.append ( resStr );

                        res = concatBuffer.toString ().intern ();

                        iLast = iLast - nStrings;
                    }
                }
            }

            if ( iFirst <= iLast ) {
                Object leftConcat = GetValue ( iLast );
                Function function = ( Function ) lua_State.GetMetaTableObjectByObject ( leftConcat, LVM.TM_CONCAT );
                if ( function == null ) {
                    function = ( Function ) lua_State.GetMetaTableObjectByObject ( res, LVM.TM_CONCAT );
                }
                if ( function == null ) {
                    //throw new RuntimeException( "missing __concat for " + leftConcat + " and " + res );
                    LuaAPI.luaG_typeerror ( m_Thread, -1, leftConcat, "concatenate" );
                }
                res = this.m_Thread.CallMetaTable ( function, leftConcat, res );
                iLast --;
            }
        }
        SetValue ( iResult, res );
    }

    public void PushVarArgs ( int index, int n ) {
        int nParams = m_Function.GetLuaFunction ().GetParamsQuantity ();
        int nVarargs = this.m_iArgsQuantity - nParams;
        if ( nVarargs < 0 )
            nVarargs = 0;
        if ( n == -1 )
            n = nVarargs;
        if ( nVarargs > n )
            nVarargs = n;

        SetTop ( index + n );

        CopyStack (  - m_iArgsQuantity + nParams, index, nVarargs );

        int numNils = n - nVarargs;
        if ( numNils > 0 ) {
            ClearStack ( index + nVarargs, index + n - 1 );
        }
    }
    //public void luaV_concat (lua_State *L, int total, int last) {
 /* do {
    StkId top = L->base + last + 1;
    int n = 2;   number of elements handled in this pass (at least 2)
    if (!(ttisstring(top-2) || ttisnumber(top-2)) || !tostring(L, top-1)) {
    if (!call_binTM(L, top-2, top-1, top-2, TM_CONCAT))
    luaG_concaterror(L, top-2, top-1);
    } else if (tsvalue(top-1)->len == 0)   second op is empty?
    (void)tostring(L, top - 2);  /* result is first op (as string)
    else {
    /* at least two string values; get as many as possible
    size_t tl = tsvalue(top-1)->len;
    char *buffer;
    int i;
    /* collect total length
    for (n = 1; n < total && tostring(L, top-n-1); n++) {
    size_t l = tsvalue(top-n-1)->len;
    if (l >= MAX_SIZET - tl) luaG_runerror(L, "string length overflow");
    tl += l;
    }
    buffer = luaZ_openspace(L, &G(L)->buff, tl);
    tl = 0;
    for (i=n; i>0; i--) {  /* concat all strings
    size_t l = tsvalue(top-i)->len;
    memcpy(buffer+tl, svalue(top-i), l);
    tl += l;
    }
    setsvalue2s(L, top-n, luaS_newlstr(L, buffer, tl));
    }
    total -= n-1;  /* got `n' strings to create 1 new
    last -= n-1;
    } while (total > 1);  /* repeat until only 1 result left
    }*/
}

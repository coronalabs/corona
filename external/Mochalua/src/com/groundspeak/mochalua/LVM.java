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
class LVM {

    // OpCodes
    public static final int OP_MOVE = 0;  /*	A B	R(A) := R(B)					*/

    public static final int OP_LOADK = 1;  /*	A Bx	R(A) := Kst(Bx)					*/

    public static final int OP_LOADBOOL = 2;  /*	A B C	R(A) := (Bool)B; if (C) pc++			*/

    public static final int OP_LOADNIL = 3;  /*	A B	R(A) := ... := R(B) := nil			*/

    public static final int OP_GETUPVAL = 4;  /*	A B	R(A) := UpValue[B]				*/

    public static final int OP_GETGLOBAL = 5;  /*	A Bx	R(A) := Gbl[Kst(Bx)]				*/

    public static final int OP_GETTABLE = 6;  /*	A B C	R(A) := R(B)[RK(C)]				*/

    public static final int OP_SETGLOBAL = 7;  /*	A Bx	Gbl[Kst(Bx)] := R(A)				*/

    public static final int OP_SETUPVAL = 8;  /*	A B	UpValue[B] := R(A)				*/

    public static final int OP_SETTABLE = 9;  /*	A B C	R(A)[RK(B)] := RK(C)				*/

    public static final int OP_NEWTABLE = 10;  /*	A B C	R(A) := {} (size = B,C)				*/

    public static final int OP_SELF = 11;  /*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/

    public static final int OP_ADD = 12;  /*	A B C	R(A) := RK(B) + RK(C)				*/

    public static final int OP_SUB = 13;  /*	A B C	R(A) := RK(B) - RK(C)				*/

    public static final int OP_MUL = 14;  /*	A B C	R(A) := RK(B) * RK(C)				*/

    public static final int OP_DIV = 15;  /*	A B C	R(A) := RK(B) / RK(C)				*/

    public static final int OP_MOD = 16;  /*	A B C	R(A) := RK(B) % RK(C)				*/

    public static final int OP_POW = 17;  /*	A B C	R(A) := RK(B) ^ RK(C)				*/

    public static final int OP_UNM = 18;  /*	A B	R(A) := -R(B)					*/

    public static final int OP_NOT = 19;  /*	A B	R(A) := not R(B)				*/

    public static final int OP_LEN = 20;  /*	A B	R(A) := length of R(B)				*/

    public static final int OP_CONCAT = 21;  /*	A B C	R(A) := R(B).. ... ..R(C)			*/

    public static final int OP_JMP = 22;  /*	sBx	pc+=sBx					*/

    public static final int OP_EQ = 23;  /*	A B C	if ((RK(B) == RK(C)) ~= A) then pc++		*/

    public static final int OP_LT = 24;  /*	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++  		*/

    public static final int OP_LE = 25;  /*	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++  		*/

    public static final int OP_TEST = 26;  /*	A C	if not (R(A) <=> C) then pc++			*/

    public static final int OP_TESTSET = 27;  /*	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++	*/

    public static final int OP_CALL = 28;  /*	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */

    public static final int OP_TAILCALL = 29;  /*	A B C	return R(A)(R(A+1), ... ,R(A+B-1))		*/

    public static final int OP_RETURN = 30;  /*	A B	return R(A), ... ,R(A+B-2)	(see note)	*/

    public static final int OP_FORLOOP = 31;  /*	A sBx	R(A)+=R(A+2); if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/

    public static final int OP_FORPREP = 32;  /*	A sBx	R(A)-=R(A+2); pc+=sBx				*/

    public static final int OP_TFORLOOP = 33;  /*	A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2)); if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++	*/

    public static final int OP_SETLIST = 34;  /*	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B	*/

    public static final int OP_CLOSE = 35;  /*	A 	close all variables in the stack up to (>=) R(A)*/

    public static final int OP_CLOSURE = 36;  /*	A Bx	R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))	*/

    public static final int OP_VARARG = 37;  /*	A B	R(A), R(A+1), ..., R(A+B-1) = vararg		*/

    public static final int TM_INDEX = 0;
    public static final int TM_NEWINDEX = 1;
    public static final int TM_GC = 2;
    public static final int TM_MODE = 3;
    public static final int TM_EQ = 4;  /* last tag method with `fast' access */

    public static final int TM_ADD = 5;
    public static final int TM_SUB = 6;
    public static final int TM_MUL = 7;
    public static final int TM_DIV = 8;
    public static final int TM_MOD = 9;
    public static final int TM_POW = 10;
    public static final int TM_UNM = 11;
    public static final int TM_LEN = 12;
    public static final int TM_LT = 13;
    public static final int TM_LE = 14;
    public static final int TM_CONCAT = 15;
    public static final int TM_CALL = 16;
    public static final int TM_N = 17;  /* number of elements in the enum */

    private static final int FIELDS_PER_FLUSH = 50;
    //private static Table m_GlobalTable;
    private static Table m_Registry = new Table ( 0, 2 );
    private static Table[] m_MetaTable = new Table[ LuaAPI.NUM_TAGS ];
    private static JavaFunction m_AtPanicFunction;

    public static void Reinit () {
        m_Registry = new Table ( 0, 2 );
        m_MetaTable = new Table[ LuaAPI.NUM_TAGS ];
        m_AtPanicFunction = null;
    }

    public static void exit () {
    }

    public static final String GetEventNameByEvent ( int iEvent ) {
        return m_strLuaEventsName[iEvent];
    }

    public static final void SetAtPanicFunction ( JavaFunction javaFunction ) {
        m_AtPanicFunction = javaFunction;
    }

    public static final JavaFunction GetAtPanicFunction () {
        return m_AtPanicFunction;
    }

    public static final Table GetRegistry () {
        return m_Registry;
    }

    public static final Table GetMetaTable ( int iType ) {
        if ( LuaAPI.LUA_TNIL > iType || iType >= LuaAPI.NUM_TAGS ) {
            return null;
        }
        return m_MetaTable[iType];
    }

    public static final void SetMetaTable ( int iType, Table newTable ) {
        m_MetaTable[iType] = newTable;
    }
    private static final String[] m_strLuaEventsName = {
        "__index",
        "__newindex",
        "__gc",
        "__mode",
        "__eq",
        "__add",
        "__sub",
        "__mul",
        "__div",
        "__mod",
        "__pow",
        "__unm",
        "__len",
        "__lt",
        "__le",
        "__concat",
        "__call"
    };

    public static final int GetInstruction ( int iOpCode ) {
        final int INSTR_MASK = 0x3F; // 00111111b

        return iOpCode & INSTR_MASK;
    }

    public static final int GetA8 ( int iOpCode ) {
        return ( iOpCode >>> 6 ) & 255;
    }

    public static final int GetC9 ( int iOpCode ) {
        return ( iOpCode >>> 14 ) & 511;
    }

    public static final int GetB9 ( int iOpCode ) {
        return ( iOpCode >>> 23 ) & 511;
    }

    public static final int GetBx ( int iOpCode ) {
        return ( iOpCode >>> 14 );
    }

    public static final int GetSBx ( int iOpCode ) {
        return ( iOpCode >>> 14 ) - 131071;
    }

    public static final boolean IsConstant ( int iOpCode ) {
        int a = ( iOpCode ) & BITRK;
        int b = BITRK;
        return ( ( iOpCode ) & BITRK ) != 0;
    }

    public static final int GetConstantIndex ( int i ) {
        return ( ( int ) ( i ) &  ~ BITRK );
    }

    public static final Object GetRCB ( CallInfo callInfo, LuaFunction luaFunction, int iOpCode ) {
        int B = GetB9 ( iOpCode );

        if ( IsConstant ( B ) ) {
            return luaFunction.GetConstant ( GetConstantIndex ( B ) );
        }
        else {
            return callInfo.GetValue ( B );
        }
    }

    public static final Object GetRCC ( CallInfo callInfo, LuaFunction luaFunction, int iOpCode ) {
        int C = GetC9 ( iOpCode );

        if ( IsConstant ( C ) ) {
            return luaFunction.GetConstant ( GetConstantIndex ( C ) );
        }
        else {
            return callInfo.GetValue ( C );
        }
    }
    /*
     ** size and position of opcode arguments.
     */
    private static final int SIZE_C = 9;
    private static final int SIZE_B = 9;
    private static final int SIZE_Bx = ( SIZE_C + SIZE_B );
    private static final int SIZE_A = 8;
    private static final int SIZE_OP = 6;
    private static final int POS_OP = 0;
    private static final int POS_A = ( POS_OP + SIZE_OP );
    private static final int POS_C = ( POS_A + SIZE_A );
    private static final int POS_B = ( POS_C + SIZE_C );
    private static final int POS_Bx = POS_C;
    private static final int MAXARG_Bx = Integer.MAX_VALUE;
    private static final int MAXARG_sBx = Integer.MAX_VALUE;
    private static final int MAXARG_A = ( ( 1 << SIZE_A ) - 1 );
    private static final int MAXARG_B = ( ( 1 << SIZE_B ) - 1 );
    private static final int MAXARG_C = ( ( 1 << SIZE_C ) - 1 );
    private static final int BITRK = ( 1 << ( SIZE_B - 1 ) );

    public static int OperationFromInstruction ( int iInstruction ) {
        switch ( iInstruction ) {
            case OP_ADD:
                return TM_ADD;
            case OP_SUB:
                return TM_SUB;
            case OP_MUL:
                return TM_MUL;
            case OP_DIV:
                return TM_DIV;
            case OP_MOD:
                return TM_MOD;
            case OP_POW:
                return TM_POW;
            default:
                return -1;
        }
    }

    public static final int luaO_fb2int ( int x ) {
        int e = ( x >> 3 ) & 31;
        if ( e == 0 ) {
            return x;
        }
        else {
            return ( ( x & 7 ) + 8 ) << ( e - 1 );
        }
    }

    public static void Execute ( lua_State thread, int iExecutedCalls ) {

        CallInfo currentCallInfo = thread.GetCurrentCallInfo ();
        Function currentFunction = currentCallInfo.GetFunction ();
        LuaFunction currentLuaFunction = currentFunction.GetLuaFunction ();

        while ( true ) {
            try {

                int iOpCode = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );   // Increment IP

                int iInstruction = GetInstruction ( iOpCode );

                int A = GetA8 ( iOpCode );

                switch ( iInstruction ) {
                    case OP_MOVE: {
                        final Object value = currentCallInfo.GetValue ( GetB9 ( iOpCode ) );
                        currentCallInfo.SetValue ( A, value );
                        break;
                    }
                    case OP_LOADK: {
                        final int indexBx = GetBx ( iOpCode );
                        final Object value = currentLuaFunction.GetConstant ( indexBx );
                        currentCallInfo.SetValue ( A, value );
                        break;
                    }
                    case OP_LOADBOOL: {
                        final Boolean value = GetB9 ( iOpCode ) == 0 ? Boolean.FALSE : Boolean.TRUE;
                        currentCallInfo.SetValue ( A, value );
                        if ( GetC9 ( iOpCode ) != 0 ) {
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        }
                        break;
                    }
                    case OP_LOADNIL: {
                        final int indexB9 = GetB9 ( iOpCode );
                        currentCallInfo.ClearStack ( A, indexB9 );
                        break;
                    }

                    case OP_GETUPVAL: {
                        final int indexB9 = GetB9 ( iOpCode );
                        final Object value = currentFunction.GetUpValue ( indexB9 ).GetValue ();
                        currentCallInfo.SetValue ( A, value );
                        break;
                    }

                    case OP_GETGLOBAL: {
                        final int indexBx = GetBx ( iOpCode );
                        final Object key = currentLuaFunction.GetConstant ( indexBx );
                        final Object value = thread.GetTable ( currentFunction.GetEnvironment (), key );
                        currentCallInfo.SetValue ( A, value );
                        break;
                    }
                    case OP_GETTABLE: {
                        final int indexB9 = GetB9 ( iOpCode );
                        final Object key = GetRCC ( currentCallInfo, currentLuaFunction, iOpCode );
                        final Object value = thread.GetTable ( indexB9, key );
                        currentCallInfo.SetValue ( A, value );
                        break;
                    }
                    case OP_SETGLOBAL: {
                        final int indexBx = GetBx ( iOpCode );
                        final Table table = currentFunction.GetEnvironment ();
                        final Object key = currentLuaFunction.GetConstant ( indexBx );
                        final Object value = currentCallInfo.GetValue ( A );
                        thread.SetTable ( table, key, value );
                        break;
                    }

                    case OP_SETUPVAL: {
                        final int indexB9 = GetB9 ( iOpCode );
                        final Object value = currentCallInfo.GetValue ( A );
                        currentFunction.GetUpValue ( indexB9 ).SetValue ( value );
                        break;
                    }
                    case OP_SETTABLE: {
                        final Object table = currentCallInfo.GetValue ( A );
                        final Object key = GetRCB ( currentCallInfo, currentLuaFunction, iOpCode );
                        final Object value = GetRCC ( currentCallInfo, currentLuaFunction, iOpCode );
                        thread.SetTable ( A, key, value );
                        break;
                    }
                    case OP_NEWTABLE: {
                        int iArraySize = luaO_fb2int ( GetB9 ( iOpCode ) );
                        int iHashSize = luaO_fb2int ( GetC9 ( iOpCode ) );
                        currentCallInfo.SetValue ( A, new Table ( iArraySize, iHashSize ) );
                        break;
                    }
                    case OP_SELF: {
                        final int B = GetB9 ( iOpCode );
                        final Object object = currentCallInfo.GetValue ( B );
                        currentCallInfo.SetValue ( A + 1, object );

                        final Object value = thread.GetTable ( B, GetRCC ( currentCallInfo, currentLuaFunction, iOpCode ) );
                        currentCallInfo.SetValue ( A, value );
                        break;
                    }
                    case OP_ADD:
                    case OP_SUB:
                    case OP_MUL:
                    case OP_DIV:
                    case OP_MOD:
                    case OP_POW: {
                        Object B = GetRCB ( currentCallInfo, currentLuaFunction, iOpCode );
                        Object C = GetRCC ( currentCallInfo, currentLuaFunction, iOpCode );

                        int iIndexB = IsConstant ( GetB9 ( iOpCode ) ) ? -1 : GetB9 ( iOpCode );
                        int iIndexC = IsConstant ( GetB9 ( iOpCode ) ) ? -1 : GetC9 ( iOpCode );

                        Object result = null;
                        Double v1 = LuaBaseLib.ConvertToDouble ( B );
                        Double v2 = LuaBaseLib.ConvertToDouble ( C );

                        if ( v1 != null && v2 != null ) {
                            double doubleB = v1.doubleValue ();
                            double doubleC = v2.doubleValue ();
                            double doubleD = 0;

                            switch ( iInstruction ) {
                                case OP_ADD:
                                     {
                                        doubleD = doubleB + doubleC;
                                    }
                                    break;
                                case OP_SUB:
                                     {
                                        doubleD = doubleB - doubleC;
                                    }
                                    break;
                                case OP_MUL:
                                     {
                                        doubleD = doubleB * doubleC;
                                    }
                                    break;
                                case OP_DIV:
                                     {
                                        doubleD = doubleB / doubleC;
                                    }
                                    break;
                                case OP_MOD:
                                     {
                                        doubleD = ( doubleB ) - Math.floor ( ( doubleB ) / ( doubleC ) ) * ( doubleC );
                                    }
                                    break;
                                case OP_POW:
                                     {
                                        doubleD = LuaMathLib.pow ( doubleB, doubleC );
                                    }
                                    break;
                            }
                            result = new Double ( doubleD );
                        }
                        else {
                            int iOperation = OperationFromInstruction ( iInstruction );
                            Function function = ( Function ) lua_State.GetMetaTableObjectByObjects ( B, C, iOperation );

                            if ( function == null ) {
                                if ( LuaBaseLib.ConvertToDouble ( B ) == null ) {
                                    C = B;
                                    iIndexC = iIndexB;
                                }
                                LuaAPI.luaG_typeerror ( thread, iIndexC, C, "perform arithmetic on" );
                            }

                            result = thread.CallMetaTable ( function, B, C );

                        }

                        currentCallInfo.SetValue ( A, result );
                        break;
                    }
                    case OP_UNM: {
                        final int indexB9 = GetB9 ( iOpCode );
                        Object value = currentCallInfo.GetValue ( indexB9 );
                        Double doubleValue = LuaBaseLib.ConvertToDouble ( value );
                        Object result = null;
                        if ( doubleValue == null ) {
                            Function function = ( Function ) lua_State.GetMetaTableObjectByObject ( value, TM_UNM );
                            result = thread.CallMetaTable ( function, value, null );
                        }
                        else {
                            result = new Double (  - doubleValue.doubleValue () );
                        }

                        currentCallInfo.SetValue ( A, result );
                        break;
                    }
                    case OP_NOT: {
                        final int indexB9 = GetB9 ( iOpCode );
                        Object value = currentCallInfo.GetValue ( indexB9 );
                        boolean b = value == null | ( value instanceof Boolean && ( ( Boolean ) value ).equals ( Boolean.FALSE ) );
                        currentCallInfo.SetValue ( A, new Boolean ( b ) );
                        break;
                    }
                    case OP_LEN: {
                        final int indexB9 = GetB9 ( iOpCode );
                        Object value = currentCallInfo.GetValue ( indexB9 );
                        Object result;
                        if ( value instanceof Table ) {
                            result = new Double ( ( ( Table ) value ).GetBoundary () );
                        }
                        else if ( value instanceof String ) {

                            result = new Double ( ( ( String ) value ).length () );
                        }
                        else {
                            Function function = ( Function ) lua_State.GetMetaTableObjectByObject ( value, TM_LEN );
                            result = thread.CallMetaTable ( function, value, null );
                        }
                        currentCallInfo.SetValue ( A, result );
                        break;
                    }
                    case OP_CONCAT: {
                        int B = GetB9 ( iOpCode );
                        int C = GetC9 ( iOpCode );
                        currentCallInfo.Concat ( C - B + 1, C, A );
                        break;
                    }
                    case OP_JMP: {
                        currentCallInfo.SetIP ( currentCallInfo.GetIP () + GetSBx ( iOpCode ) );
                        break;
                    }
                    case OP_EQ: {
                        final Object objectL = GetRCB ( currentCallInfo, currentLuaFunction, iOpCode );
                        final Object objectR = GetRCC ( currentCallInfo, currentLuaFunction, iOpCode );
                        boolean bA = A != 0 ? true : false;

                        if ( ( LuaAPI.lua_typebyobject ( objectL ) == LuaAPI.lua_typebyobject ( objectR ) &&
                            LuaAPI.luaV_equal ( thread, objectL, objectR ) ) == bA ) {
                            iOpCode = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + GetSBx ( iOpCode ) );
                        }
                        currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        break;
                    }
                    case OP_LT: {
                        final Object objectL = GetRCB ( currentCallInfo, currentLuaFunction, iOpCode );
                        final Object objectR = GetRCC ( currentCallInfo, currentLuaFunction, iOpCode );
                        boolean bA = A != 0 ? true : false;

                        if ( LuaAPI.luaV_lessthan ( thread, objectL, objectR ) == bA ) {
                            iOpCode = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + GetSBx ( iOpCode ) );
                        }
                        currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        break;
                    }
                    case OP_LE: {
                        final Object objectL = GetRCB ( currentCallInfo, currentLuaFunction, iOpCode );
                        final Object objectR = GetRCC ( currentCallInfo, currentLuaFunction, iOpCode );
                        boolean bA = A != 0 ? true : false;

                        if ( LuaAPI.luaV_lessequal ( thread, objectL, objectR ) == bA ) {
                            iOpCode = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + GetSBx ( iOpCode ) );
                        }
                        currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        break;
                    }
                    case OP_TEST: {
                        boolean bC = GetC9 ( iOpCode ) != 0 ? true : false;

                        Object value = currentCallInfo.GetValue ( A );

                        if ( ( ( value == null ) || ( ( value instanceof Boolean ) && ( ( Boolean ) value ).equals ( Boolean.FALSE ) ) ) != bC ) {
                            iOpCode = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + GetSBx ( iOpCode ) );
                        }

                        currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        break;
                    }
                    case OP_TESTSET: {
                        boolean bC = GetC9 ( iOpCode ) != 0 ? true : false;
                        Object value = currentCallInfo.GetValue ( GetB9 ( iOpCode ) );
                        if ( ( value == null || ( value instanceof Boolean && ( ( Boolean ) value ).equals ( Boolean.FALSE ) ) ) != bC ) {
                            iOpCode = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + GetSBx ( iOpCode ) );
                            currentCallInfo.SetValue ( A, value );
                        }
                        currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        break;
                    }
                    case OP_CALL:
                    case OP_TAILCALL: {
                        int B = GetB9 ( iOpCode );
                        int C = GetC9 ( iOpCode );
                        int iResultsQuantity = GetC9 ( iOpCode ) - 1;

                        boolean bRestoreTop = C != 0;

                        int iArgsQuantity = B - 1;
                        if ( iArgsQuantity != -1 ) {
                            currentCallInfo.SetTop ( A + iArgsQuantity + 1 );
                        }
                        else {
                            iArgsQuantity = currentCallInfo.GetTop () - A - 1;
                        }

                        Object object = currentCallInfo.GetValue ( A );
                        if (  ! ( object instanceof Function ) ) {
                            Object tmFunction = lua_State.GetMetaTableObjectByObject ( object, TM_CALL );
                            if ( tmFunction != null ) {
                                object = tmFunction;
                            }

                            currentCallInfo.PushValue ( null );	// new extra argumnet

                            iArgsQuantity ++;
                            for ( int iArg = currentCallInfo.GetTop () - 1; iArg > A; iArg -- ) {
                                currentCallInfo.SetValue ( iArg, currentCallInfo.GetValue ( iArg - 1 ) );
                            }

                            currentCallInfo.SetValue ( A, object );
                        }

                        if ( object instanceof Function ) {
                            Function function = ( Function ) object;

                            CallInfo callInfo = new CallInfo (
                                thread,
                                function,
                                currentCallInfo.GetLocalObjectsStackBase () + A + 1,
                                currentCallInfo.GetLocalObjectsStackBase () + A,
                                iResultsQuantity,
                                iArgsQuantity );

                            thread.PushCallInfo ( callInfo );

                            currentCallInfo = callInfo;
                            currentFunction = function;

                            if ( function.IsLuaFunction () ) {
                                LuaFunction luaFunction = function.GetLuaFunction ();

                                currentLuaFunction = luaFunction;

                                iExecutedCalls ++;
                            }
                            else {
                                JavaFunction javaFunction = function.GetJavaFunction ();

                                currentLuaFunction = null;

                                int iReturnValuesQuantity = javaFunction.Call ( thread );
                                if ( iReturnValuesQuantity < 0 ) //yielding?
                                {
                                    return;
                                }

                                currentCallInfo.AdjustResults ( iReturnValuesQuantity );

                                thread.PopCallInfo ();

                                currentCallInfo = thread.GetCurrentCallInfo ();
                                currentFunction = currentCallInfo.GetFunction ();
                                currentLuaFunction = currentFunction.GetLuaFunction ();

                                if ( bRestoreTop == true ) {
                                    currentCallInfo.SetTop ( currentLuaFunction.GetMaxStackSize () );
                                }
                            }
                        }
                        else {
                            LuaAPI.luaG_typeerror ( thread, A, null, "call" );
                        }
                        break;
                    }

                    case 99://OP_TAILCALL:
                    {
                        int B = GetB9 ( iOpCode );

                        thread.CloseUpValues ( currentCallInfo.GetLocalObjectsStackBase () );

                        int iArgsQuantity = B - 1;
                        if ( iArgsQuantity == -1 ) {
                            iArgsQuantity = currentCallInfo.GetTop () - A - 1;
                        }

                        Object object = currentCallInfo.GetValue ( A );
                        if (  ! ( object instanceof Function ) ) {
                            object = lua_State.GetMetaTableObjectByObject ( object, TM_CALL );

                            currentCallInfo.PushValue ( null );	// new extra argumnet

                            iArgsQuantity ++;
                            for ( int iArg = currentCallInfo.GetTop () - 1; iArg > A; iArg -- ) {
                                currentCallInfo.SetValue ( iArg, currentCallInfo.GetValue ( iArg - 1 ) );
                            }

                            currentCallInfo.SetValue ( A, object );
                        }

                        thread.CopyStack ( currentCallInfo.GetLocalObjectsStackBase () + A, currentCallInfo.GetReturnBase (), iArgsQuantity + 1 ); // B - args count + 1 func

                        thread.SetObjectsStackTop ( currentCallInfo.GetReturnBase () + iArgsQuantity + 1 );

                        if ( object instanceof Function ) {
                            Function function = ( Function ) object;

                            CallInfo callInfo = new CallInfo (
                                thread,
                                function,
                                currentCallInfo.GetReturnBase () + 1,
                                currentCallInfo.GetReturnBase (),
                                currentCallInfo.GetResultsWanted (),
                                iArgsQuantity );

                            callInfo.SetTailCalls ( currentCallInfo.GetTailCalls () + 1 );

                            thread.GetCallInfosStack ()[thread.GetCallInfosStackTop () - 1] = callInfo;

                            if ( function.IsLuaFunction () ) {
                                LuaFunction luaFunction = function.GetLuaFunction ();

                                currentCallInfo = callInfo;
                                currentFunction = function;
                                currentLuaFunction = luaFunction;
                            }
                            else {
                                JavaFunction javaFunction = function.GetJavaFunction ();

                                currentCallInfo = callInfo;
                                currentFunction = function;
                                currentLuaFunction = null;

                                int iReturnValuesQuantity = javaFunction.Call ( thread );

                                if ( iReturnValuesQuantity < 0 ) //yielding?
                                {
                                    return;
                                }

                                currentCallInfo.AdjustResults ( iReturnValuesQuantity );

                                thread.PopCallInfo ();

                                currentCallInfo = thread.GetCurrentCallInfo ();
                                currentFunction = currentCallInfo.GetFunction ();
                                if ( currentFunction != null ) {
                                    currentLuaFunction = currentFunction.GetLuaFunction ();
                                }
                                else {
                                    currentLuaFunction = null;
                                }

                                if (  -- iExecutedCalls <= 0 ) {
                                    return;
                                }
                            }

                        }
                        else {
                            LuaAPI.luaG_typeerror ( thread, A, null, "call" );
                        }
                        break;
                    }
                    case OP_RETURN: {

                        int iFirstResult = A;
                        int iResultsQuantity = GetB9 ( iOpCode ) - 1;

                        thread.CloseUpValues ( currentCallInfo.GetLocalObjectsStackBase () );

                        if ( iResultsQuantity == LuaAPI.LUA_MULTRET ) {
                            iResultsQuantity = currentCallInfo.GetTop () - iFirstResult;
                        }

                        thread.CopyStack ( currentCallInfo.GetLocalObjectsStackBase () + iFirstResult, currentCallInfo.GetReturnBase (), iResultsQuantity );
                        thread.SetObjectsStackTop ( currentCallInfo.GetReturnBase () + iResultsQuantity );

                        int iAddResults = currentCallInfo.GetResultsWanted () - iResultsQuantity;
                        if ( currentCallInfo.GetResultsWanted () != -1 && iResultsQuantity != -1 ) {
                            thread.SetObjectsStackTop ( currentCallInfo.GetReturnBase () + iResultsQuantity + iAddResults );
                        }

                        thread.PopCallInfo ();

                        if (  -- iExecutedCalls <= 0 ) {
                            return;
                        }
                        else {
                            iResultsQuantity = currentCallInfo.GetResultsWanted () - LuaAPI.LUA_MULTRET;
                            currentCallInfo = thread.GetCurrentCallInfo ();
                            currentFunction = currentCallInfo.GetFunction ();
                            currentLuaFunction = currentFunction.GetLuaFunction ();

                            if ( iResultsQuantity != 0 ) {
                                if ( currentFunction.IsLuaFunction () ) {
                                    currentCallInfo.SetTop ( currentCallInfo.GetFunction ().GetLuaFunction ().GetMaxStackSize () );
                                }
                            }
                            break;
                        }
                    }
                    case OP_FORLOOP: {
                        Double doubleIter = LuaBaseLib.ConvertToDouble ( currentCallInfo.GetValue ( A ) );
                        if ( doubleIter == null ) {
                            LuaAPI.luaG_runerror ( thread, "\"for\" initial value must be a number" );
                        }
                        double iter = doubleIter.doubleValue ();

                        Double doubleEnd = LuaBaseLib.ConvertToDouble ( currentCallInfo.GetValue ( A + 1 ) );
                        if ( doubleEnd == null ) {
                            LuaAPI.luaG_runerror ( thread, "\"for\" end value must be a number" );
                        }
                        double end = doubleEnd.doubleValue ();

                        Double doubleStep = LuaBaseLib.ConvertToDouble ( currentCallInfo.GetValue ( A + 2 ) );
                        if ( doubleStep == null ) {
                            LuaAPI.luaG_runerror ( thread, "\"for\" step value must be a number" );
                        }
                        double step = doubleStep.doubleValue ();

                        iter += step;
                        Double iterDouble = new Double ( iter );

                        if ( ( step > 0 ) ? iter <= end : iter >= end ) {
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + GetSBx ( iOpCode ) );
                            currentCallInfo.SetValue ( A, iterDouble );
                            currentCallInfo.SetValue ( A + 3, iterDouble );
                        }
                        break;
                    }
                    case OP_FORPREP: {
                        // FIXME:
                        int B = GetSBx ( iOpCode );

                        Double doubleIter = LuaBaseLib.ConvertToDouble ( currentCallInfo.GetValue ( A ) );
                        if ( doubleIter == null ) {
                            LuaAPI.luaG_runerror ( thread, "\"for\" initial value must be a number" );
                        }
                        double iter = doubleIter.doubleValue ();

                        Double doubleStep = LuaBaseLib.ConvertToDouble ( currentCallInfo.GetValue ( A + 2 ) );
                        if ( doubleStep == null ) {
                            LuaAPI.luaG_runerror ( thread, "\"for\" step value must be a number" );
                        }
                        double step = doubleStep.doubleValue ();

                        currentCallInfo.SetValue ( A, new Double ( iter - step ) );
                        currentCallInfo.SetIP ( currentCallInfo.GetIP () + B );
                        break;
                    }

                    case OP_TFORLOOP: {
                        final int C = GetC9 ( iOpCode );

                        currentCallInfo.SetTop ( A + 6 );
                        currentCallInfo.CopyStack ( A, A + 3, 3 );

                        thread.Call ( 2, GetC9 ( iOpCode ) );

                        currentCallInfo.SetTop ( A + 3 + C );

                        Object aObj3 = currentCallInfo.GetValue ( A + 3 );
                        if ( LuaAPI.IsNilOrNull ( aObj3 ) == false ) {
                            currentCallInfo.SetValue ( A + 2, aObj3 );
                        }
                        else {
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        }
                        break;
                    }

                    case OP_SETLIST: {
                        int B = GetB9 ( iOpCode );
                        int C = GetC9 ( iOpCode );

                        if ( B == 0 ) {
                            B = currentCallInfo.GetTop () - A - 1;
                        }
                        if ( C == 0 ) {
                            C = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                        }

                        int iOffset = ( C - 1 ) * FIELDS_PER_FLUSH;

                        Table table = ( Table ) currentCallInfo.GetValue ( A );

                        if ( iOffset + B > table.GetArraySize () ) {
                            table.ResizeArray ( iOffset + B );
                        }

                        for ( int i = 1; i <= B; i ++ ) {
                            final Object key = new Double ( iOffset + i );
                            final Object value = currentCallInfo.GetValue ( A + i );
                            table.SetValue ( key, value, thread );
                        }
                        break;
                    }
                    case OP_CLOSE: {
                        currentCallInfo.CloseUpValues ( A );
                        break;
                    }
                    case OP_CLOSURE: {
                        final int indexBx = GetBx ( iOpCode );
                        LuaFunction luaFunction = currentLuaFunction.GetLuaFunction ( indexBx );
                        Function function = new Function ( luaFunction, currentFunction.GetEnvironment () );

                        final int iUpValuesQuantity = luaFunction.GetUpValuesQuantity ();
                        for ( int iUpValue = 0; iUpValue < iUpValuesQuantity; iUpValue ++ ) {
                            final int iNextOpCode = currentLuaFunction.GetOpCode ( currentCallInfo.GetIP () );
                            currentCallInfo.SetIP ( currentCallInfo.GetIP () + 1 );
                            final int iNextInstruction = GetInstruction ( iNextOpCode );

                            final int iNextB = GetB9 ( iNextOpCode );

                            if ( iNextInstruction == OP_GETUPVAL ) {
                                function.SetUpValue ( iUpValue, currentFunction.GetUpValue ( iNextB ) );
                            }
                            else // OP_MOVE
                            {
                                function.SetUpValue ( iUpValue, currentCallInfo.FindUpValue ( iNextB ) );
                            }
                        }
                        currentCallInfo.SetValue ( A, function );
                        break;
                    }
                    case OP_VARARG: {
                        int B = GetB9 ( iOpCode ) - 1;
                        currentCallInfo.PushVarArgs ( A, B );
                        break;
                    }

                    default: {
                        throw new LuaRuntimeException ( "invalid operation code '" + iInstruction + "'" );
                    }
                }
            }
            catch ( LuaRuntimeException e ) {
                throw e;
            }
        }
    }
}

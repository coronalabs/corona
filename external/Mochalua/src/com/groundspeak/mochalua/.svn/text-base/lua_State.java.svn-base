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
import java.util.Vector;
import java.io.ByteArrayOutputStream;
import javax.microedition.io.file.FileConnection;

/**
 *
 * @author a.fornwald
 */
public class lua_State {

    public static final int MAXTAGLOOP = 100;
    private static final int LUA_SIGNATURE = 0x1B4C7561;
    private static final int LUAC_VERSION = 0x51;
    private static final int LUAC_FORMAT = 0;
    private static final int LUA_SIZEOFINT = 4;
    private static final int LUA_SIZEOFSIZET4 = 4;
    private static final int LUA_SIZEOFSIZET8 = 4;
    private static final int LUA_SIZEOFINSTRUCTION = 4;
    private static final int LUA_SIZEOFNUMBER = 8;
    private static final boolean LUA_ISINTEGRAL = false;
    private static final boolean LUA_ISLITTLEENDIAN = true;
    // Stack
    private Object[] m_ObjectsStack;
    private int m_iObjectStackTop;
    private int status = 0;
    private int nCcalls;  /* number of nested C calls */

    private int baseCcalls;  /* nested C calls when resuming coroutine */

    private boolean isMainThread = false;
    // Call info
    private CallInfo[] m_CallInfosStack;
    private int m_iCallInfosStackTop;
    private Table m_Environment;
    private Vector m_LiveUpValues;
    private Function m_ErrorFunction;
    private lua_Hook m_Hook;
    private int m_iHookMask;
    private int m_iHookCount;
    private int m_iBaseHookCount;

    private static final class pmain implements JavaFunction {

        public int Call ( lua_State thread ) {
            return 0;
        }
    }

    /**
     * Creates a new instance of lua_State
     */
    public lua_State ( boolean isMainThread ) {
        this.nCcalls = this.baseCcalls = 0;
        // Init stack
        this.m_ObjectsStack = new Object[ LuaAPI.BASIC_STACK_SIZE + LuaAPI.EXTRA_STACK ];
        this.m_iObjectStackTop = 0;

        // Init call info
        this.m_CallInfosStack = new CallInfo[ LuaAPI.BASIC_CALLINFO_SIZE ];
        this.m_iCallInfosStackTop = 0;

        this.m_Environment = new Table ( 0, 2 );

        this.m_LiveUpValues = new Vector ();

        this.isMainThread = isMainThread;

        this.m_iHookCount = 0;

        SetErrorFunction ( new Function ( new LuaDebugLib.traceback (), GetEnvironment (), 0 ) );

        // base callinfo that shouldn't be ever deleted
        PushCallInfo ( new CallInfo ( this, new Function ( new pmain (), GetEnvironment (), 0 ), 0, 0, 0, 0 ) );
    }
    public static final int PCRLUA = 0;
    public static final int PCRJAVA = 1;

    public final lua_Hook GetHook () {
        return this.m_Hook;
    }

    public final void ResetHookCount () {
        this.m_iHookCount = this.m_iBaseHookCount;
    }

    public final void SetHook ( lua_Hook hook ) {
        this.m_Hook = hook;
    }

    public final int GetHookMask () {
        return this.m_iHookMask;
    }

    public final void SetHookMask ( int iHookMask ) {
        this.m_iHookMask = iHookMask;
    }

    public final int GetBaseHookCount () {
        return this.m_iBaseHookCount;
    }

    public final void SetBaseHookCount ( int iBaseHookCount ) {
        this.m_iBaseHookCount = iBaseHookCount;
    }

    public final Function GetErrorFunction () {
        return this.m_ErrorFunction;
    }

    public final void SetErrorFunction ( Function errorFunction ) {
        this.m_ErrorFunction = errorFunction;
    }

    public final int GetStatus () {
        return this.status;
    }

    public final void SetStatus ( int newStatus ) {
        this.status = newStatus;
    }

    public final void SetEnvironment ( Table environment ) {
        this.m_Environment = environment;
    }

    public final Table GetEnvironment () {
        return this.m_Environment;
    }

    public void Call ( int iArgsQuantity, int iResultsQuantity ) throws LuaRuntimeException {
        if (  ++ this.nCcalls >= LuaAPI.LUAI_MAXCCALLS ) {
            if ( this.nCcalls == LuaAPI.LUAI_MAXCCALLS ) {
                LuaAPI.luaG_runerror ( this, "J stack overflow" );
            }
            else if ( this.nCcalls >= ( LuaAPI.LUAI_MAXCCALLS + ( LuaAPI.LUAI_MAXCCALLS >> 3 ) ) ) {
                throw new LuaRuntimeException ( "error while handing stack error" );
            }
        }

        int iFunctionIndexOnTheStack = GetObjectsStackTop () - ( iArgsQuantity + 1 );
        Object object = m_ObjectsStack[iFunctionIndexOnTheStack];

        if ( object instanceof Function ) {
            Function function = ( Function ) object;

            CallInfo callInfo = new CallInfo ( this, function, iFunctionIndexOnTheStack + 1, iFunctionIndexOnTheStack, iResultsQuantity, iArgsQuantity );

            PushCallInfo ( callInfo );

            if ( function.IsLuaFunction () ) {
                LVM.Execute ( this, 1 );
            }
            else {
                int iReturnValuesQuantity = function.GetJavaFunction ().Call ( this );
                if ( iResultsQuantity == LuaAPI.LUA_MULTRET ) {
                    iResultsQuantity = iReturnValuesQuantity;
                }

                int i = iResultsQuantity - iReturnValuesQuantity;

                int iTop = callInfo.GetTop ();
                int iActualReturnBase = iTop - iReturnValuesQuantity;

                if ( i > 0 ) {
                    callInfo.SetTop ( callInfo.GetTop () + i );
                }

                callInfo.CopyStack ( iActualReturnBase, -1, iResultsQuantity );
                callInfo.SetTop ( iResultsQuantity - 1 );

                PopCallInfo ();
            }
        }
        else {

            //CallInfo callInfo = new CallInfo( this, null, iFunctionIndexOnTheStack + 1, iFunctionIndexOnTheStack, iResultsQuantity, iArgsQuantity );

            //PushCallInfo( callInfo );

            LuaAPI.luaG_typeerror ( this, iFunctionIndexOnTheStack, null, "call" );
        }

        this.nCcalls --;
    }

    public final void SetValue ( int iIndex, Object value ) {
        Object oldObject = this.m_ObjectsStack[iIndex];

        Collectable.Increment ( value );
        Collectable.Decrement ( this, oldObject );

        this.m_ObjectsStack[iIndex] = value;
    }

    public final Object GetValue ( int iIndex ) {
        return this.m_ObjectsStack[iIndex];
    }

    public int GetObjectIndex ( int iIndex ) {
        CallInfo currentCallInfo = GetCurrentCallInfo ();
        if ( iIndex > 0 ) {
            int p = currentCallInfo.GetLocalObjectsStackBase () + iIndex - 1;
            return p;
        }
        else if ( iIndex > LuaAPI.LUA_REGISTRYINDEX ) {
            // FIXME: ensure in a correct return value
            return m_iObjectStackTop + iIndex;
        }
        else {
            LuaAPI.luaG_runerror ( this, "attempt to get an invalid index from \"" + iIndex + "\"" );
            return 0;
        }
    }

    public void SetObjectValue ( int iIndex, Object object ) {
        CallInfo currentCallInfo = GetCurrentCallInfo ();
        if ( iIndex > 0 ) {
            currentCallInfo.SetValue ( iIndex - 1, object );
        }
        else if ( iIndex > LuaAPI.LUA_REGISTRYINDEX ) {
            // FIXME: ensure in a correct return value
            //m_ObjectsStack[m_iObjectStackTop + iIndex] = object;
            SetValue ( m_iObjectStackTop + iIndex, object );
        }
        else {
            switch ( iIndex ) {
                case LuaAPI.LUA_REGISTRYINDEX: {
                    throw new LuaRuntimeException ( "Not implemented yet" );
                }
                case LuaAPI.LUA_ENVIRONINDEX: {
                    throw new LuaRuntimeException ( "Not implemented yet" );
                }
                case LuaAPI.LUA_GLOBALSINDEX: {
                    SetEnvironment ( ( Table ) object );
                }

                default: {
                    Function function = GetCurrentCallInfo ().GetFunction ();
                    iIndex = LuaAPI.LUA_GLOBALSINDEX - iIndex;
                    if ( iIndex <= function.GetUpValuesQuantity () ) {
                        function.GetUpValue ( iIndex - 1 ).SetValue ( object );
                    }
                }
            }
        }
    }

    public Object GetObjectValue ( int iIndex ) {
        CallInfo currentCallInfo = GetCurrentCallInfo ();
        if ( iIndex > 0 ) {
            if ( iIndex - 1 >= currentCallInfo.GetTop () ) {
                return LuaAPI.m_NilObject;
            }
            else {
                return currentCallInfo.GetValue ( iIndex - 1 );
            }
        }
        else if ( iIndex > LuaAPI.LUA_REGISTRYINDEX ) {
            // FIXME: ensure in a correct return value
            return m_ObjectsStack[m_iObjectStackTop + iIndex];
        }
        else {
            switch ( iIndex ) {
                case LuaAPI.LUA_REGISTRYINDEX: {
                    return LVM.GetRegistry ();
                }
                case LuaAPI.LUA_ENVIRONINDEX: {
                    return currentCallInfo.GetFunction ().GetEnvironment ();
                }
                case LuaAPI.LUA_GLOBALSINDEX: {
                    return GetEnvironment ();
                }

                default: {
                    Function function = GetCurrentCallInfo ().GetFunction ();
                    iIndex = LuaAPI.LUA_GLOBALSINDEX - iIndex;
                    if ( iIndex <= function.GetUpValuesQuantity () ) {
                        return function.GetUpValue ( iIndex - 1 ).GetValue ();
                    }
                    else {
                        return LuaAPI.m_NilObject;
                    }
                }
            }
        }
    }

    public void SetObjectsStackTopWithoutClearingStack ( int iTop ) {
        this.m_iObjectStackTop = iTop;
    }

    public void SetObjectsStackTop ( int iTop ) {
        if ( this.m_iObjectStackTop < iTop ) {
            GrowStack ( iTop );
        }
        else {
            // Clear all objects in range [new Top value, old Top value - 1]
            ClearObjectsStack ( iTop, this.m_iObjectStackTop - 1 );
        }

        this.m_iObjectStackTop = iTop;
    }

    public final int GetObjectsStackTop () {
        return this.m_iObjectStackTop;
    }

    public final void ClearObjectsStack ( int iFrom, int iTo ) {
        for ( int iIndex = iFrom; iIndex <= iTo; iIndex ++ ) {
            SetValue ( iIndex, null );
        }
    }

    public final void CopyStack ( int iFrom, int iTo, int iLength ) {
        for ( int iIndex = 0; iIndex < iLength; iIndex ++ ) {
            SetValue ( iIndex + iTo, GetValue ( iIndex + iFrom ) );
        }
    }

    public final boolean CheckStack ( int iIndex ) {
        if ( m_ObjectsStack.length + iIndex > LuaAPI.LUA_MAX_STACK_SIZE ) {
            return false;
        }
        return true;
    }
    private static boolean bErrorGenerated = false;

    public void GrowStack ( int iIndex ) {
        if ( CheckStack ( iIndex ) == false ) {
            if ( bErrorGenerated == false ) {
                bErrorGenerated = true;
                LuaAPI.luaG_runerror ( this, "stack overflow" );
            }
        }

        int iOldSize = this.m_ObjectsStack.length;
        int iNewSize = iOldSize;

        while ( iNewSize <= iIndex ) {
            iNewSize = 2 * iNewSize;
        }

        if ( iNewSize > iOldSize ) {
            Object[] newObjectsStack = new Object[ iNewSize ];
            System.arraycopy ( this.m_ObjectsStack, 0, newObjectsStack, 0, iOldSize );
            this.m_ObjectsStack = newObjectsStack;
        }
    }

    public final int GetObjectsStackSize () {
        return this.m_ObjectsStack.length;
    }

    public final int GetCallInfosStackTop () {
        return this.m_iCallInfosStackTop;
    }

    public final CallInfo[] GetCallInfosStack () {
        return this.m_CallInfosStack;
    }

    public final void SetCallInfosStackTop ( int iNewCallInfosStackTop ) {
        if ( iNewCallInfosStackTop + 1 > this.m_CallInfosStack.length ) {
            CallInfo[] newCallInfoStack = new CallInfo[ iNewCallInfosStackTop + 1 ];
            System.arraycopy ( this.m_CallInfosStack, 0, newCallInfoStack, 0, this.m_CallInfosStack.length );
            this.m_CallInfosStack = newCallInfoStack;
        }
        this.m_iCallInfosStackTop = iNewCallInfosStackTop;
    }

    public final CallInfo GetCurrentCallInfo () {
        if ( this.m_iCallInfosStackTop > 0 ) {
            return this.m_CallInfosStack[this.m_iCallInfosStackTop - 1];
        }
        return null;
    }

    public void PushCallInfo ( CallInfo callInfo ) {
        // Save top
        if ( GetCurrentCallInfo () != null ) {
            GetCurrentCallInfo ().SaveTop ();
        }

        this.m_CallInfosStack[this.m_iCallInfosStackTop] = callInfo;
        SetCallInfosStackTop ( this.GetCallInfosStackTop () + 1 );
    }

    public void PopCallInfo () {
        m_CallInfosStack[m_iCallInfosStackTop - 1] = null;
        SetCallInfosStackTop ( GetCallInfosStackTop () - 1 );
    }

    public Object GetTable ( int tableIndex, Object key ) {
        Object t = this.GetCurrentCallInfo ().GetValue ( tableIndex );

        for ( int iLoop = 0; iLoop < MAXTAGLOOP; iLoop ++ ) {
            Object metaTable = null;

            if ( t instanceof Table ) {
                Table table = ( Table ) t;
                Object result = table.GetValue ( key );
                if ( LuaAPI.IsNilOrNull ( result ) == false || ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_INDEX ) ) == null ) {
                    return result;
                }
            }
            else if ( LuaAPI.IsNilOrNull ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_INDEX ) ) ) {
                LuaAPI.luaG_typeerror ( this, tableIndex, null, "index" );
            }

            if ( metaTable instanceof Function ) {
                return CallMetaTable ( ( Function ) metaTable, t, key );
            }

            t = metaTable;
        }
        LuaAPI.luaG_runerror ( this, "loop in gettable" );
        return null;
    }

    public Object GetTable ( Object t, Object key ) {
        for ( int iLoop = 0; iLoop < MAXTAGLOOP; iLoop ++ ) {
            Object metaTable = null;

            if ( t instanceof Table ) {
                Table table = ( Table ) t;
                Object result = table.GetValue ( key );
                if ( LuaAPI.IsNilOrNull ( result ) == false || ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_INDEX ) ) == null ) {
                    return result;
                }
            }
            else if ( LuaAPI.IsNilOrNull ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_INDEX ) ) ) {
                LuaAPI.luaG_typeerror ( this, -1, t, "index" );
            }

            if ( metaTable instanceof Function ) {
                return CallMetaTable ( ( Function ) metaTable, t, key );
            }

            t = metaTable;
        }
        LuaAPI.luaG_runerror ( this, "loop in gettable" );
        return null;
    }

    public Object CallMetaTable ( Function function, Object p1 ) {
        CallInfo currentCallInfo = this.GetCurrentCallInfo ();
        int iOldTop = currentCallInfo.GetTop ();

        currentCallInfo.PushValue ( function );
        currentCallInfo.PushValue ( p1 );
        this.Call ( 1, 1 );

        Object res = currentCallInfo.GetValue ( iOldTop );
        currentCallInfo.SetTop ( iOldTop );

        return res;
    }

    public Object CallMetaTable ( Function function, Object p1, Object p2 ) {
        CallInfo currentCallInfo = this.GetCurrentCallInfo ();
        int iOldTop = currentCallInfo.GetTop ();

        currentCallInfo.PushValue ( function );
        currentCallInfo.PushValue ( p1 );
        currentCallInfo.PushValue ( p2 );
        this.Call ( 2, 1 );

        Object res = currentCallInfo.GetValue ( iOldTop );
        currentCallInfo.SetTop ( iOldTop );

        return res;
    }

    public void CallMetaTable ( Function function, Object p1, Object p2, Object p3 ) {
        CallInfo currentCallInfo = this.GetCurrentCallInfo ();
        int iOldTop = currentCallInfo.GetTop ();

        currentCallInfo.PushValue ( function );
        currentCallInfo.PushValue ( p1 );
        currentCallInfo.PushValue ( p2 );
        currentCallInfo.PushValue ( p3 );
        this.Call ( 3, 0 );

        currentCallInfo.SetTop ( iOldTop );
    }

    public static Object GetMetaTableObjectByObject ( Object object, int iEvent ) {
        Table metaTable;
        if ( object instanceof Table ) {
            metaTable = ( ( Table ) object ).GetMetaTable ();
        }
        else if ( object instanceof UserData ) {
            metaTable = ( ( UserData ) object ).GetMetaTable ();
        }
        else {
            metaTable = LVM.GetMetaTable ( LuaAPI.lua_typebyobject ( object ) );
        }

        if ( metaTable != null ) {
            return metaTable.GetValue ( LVM.GetEventNameByEvent ( iEvent ) );
        }

        return null;
    }

    public static Object GetMetaTableObjectByObjects ( Object o1, Object o2, int iEvent ) {
        Object object = ( Function ) GetMetaTableObjectByObject ( o1, iEvent );
        if ( object == null ) {
            object = GetMetaTableObjectByObject ( o2, iEvent );
        }

        return object;
    }

    public static Function GetEqualMetaTableObjectByObjects ( Object o1, Object o2, int iEvent ) {
        Function function1 = ( Function ) GetMetaTableObjectByObject ( o1, iEvent );
        Function function2 = ( Function ) GetMetaTableObjectByObject ( o2, iEvent );

        if ( function1 == null || function1 != function2 ) {
            return null;
        }
        else {
            return function1;
        }
    }

    public void SetTable ( int tableIndex, Object key, Object value ) {
        Object t = this.GetCurrentCallInfo ().GetValue ( tableIndex );

        for ( int iLoop = 0; iLoop < MAXTAGLOOP; iLoop ++ ) {
            Object metaTable = null;

            if ( t instanceof Table ) {
                Table table = ( Table ) t;

                if ( LuaAPI.IsNilOrNull ( table.GetValue ( key ) ) == false || ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_NEWINDEX ) ) == null ) {
                    table.SetValue ( key, value, this );
                    return;
                }
            }
            else if ( LuaAPI.IsNilOrNull ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_NEWINDEX ) ) ) {
                LuaAPI.luaG_typeerror ( this, tableIndex, null, "index" );
            }

            if ( metaTable != null && metaTable instanceof Function ) {
                CallMetaTable ( ( Function ) metaTable, t, key, value );
                return;
            }
            else {
                t = metaTable;
            }
        }
        LuaAPI.luaG_runerror ( this, "loop in gettable" );
    }

    public void SetTable ( Object t, Object key, Object value ) {
        for ( int iLoop = 0; iLoop < MAXTAGLOOP; iLoop ++ ) {
            Object metaTable = null;

            if ( t instanceof Table ) {
                Table table = ( Table ) t;

                if ( LuaAPI.IsNilOrNull ( table.GetValue ( key ) ) == false || ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_NEWINDEX ) ) == null ) {
                    table.SetValue ( key, value, this );
                    return;
                }
            }
            else if ( LuaAPI.IsNilOrNull ( metaTable = GetMetaTableObjectByObject ( t, LVM.TM_NEWINDEX ) ) ) {
                LuaAPI.luaG_typeerror ( this, -1, t, "index" );
            }

            if ( metaTable != null && metaTable instanceof Function ) {
                CallMetaTable ( ( Function ) metaTable, t, key, value );
                return;
            }
            else {
                t = metaTable;
            }
        }
        LuaAPI.luaG_runerror ( this, "loop in gettable" );
    }

    public int DumpByteCode ( LuaFunction luaFunction, lua_Writer writer, Object userData ) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream ();

        final boolean bIsLittleEndian = LUA_ISLITTLEENDIAN;
        final int iSizeOfsize_t = LUA_SIZEOFSIZET4;

        LuaFunction.WriteLuaInt ( baos, false, LUA_SIGNATURE );

        baos.write ( LUAC_VERSION );
        baos.write ( LUAC_FORMAT );
        baos.write ( bIsLittleEndian == true ? 1 : 0 );
        baos.write ( LUA_SIZEOFINT );
        baos.write ( iSizeOfsize_t );
        baos.write ( LUA_SIZEOFINSTRUCTION );
        baos.write ( LUA_SIZEOFNUMBER );
        baos.write ( LUA_ISINTEGRAL == false ? 0 : 1 );

        luaFunction.Dump ( baos, iSizeOfsize_t, bIsLittleEndian, writer, null, userData );

        String toWriter = null;//baos.toString();

        try {
            toWriter = new String ( baos.toByteArray (), "US_ASCII" );
        }
        catch ( Exception ex ) {
        }

        return writer.Call ( this, toWriter, toWriter.length (), userData );
    }

    public Function LoadByteCode ( DataInputStream dis, FileConnection fileConnection, String chunkname ) throws LuaRuntimeException {
        try {
            boolean bIsLittleEndian;
            int iSizeOfsize_t;

            if ( dis.readInt () != LUA_SIGNATURE ) {
                throw new LuaRuntimeException ( "Not a LUAs bytecode file" );
            }

            if ( dis.read () != LUAC_VERSION ) {
                throw new LuaRuntimeException ( "Wrong version of LUAs bytecode. Expected 5.1." );
            }

            if ( dis.read () != LUAC_FORMAT ) {
                throw new LuaRuntimeException ( "Wrong format of LUAs bytecode." );
            }

            bIsLittleEndian = ( dis.read () == 1 );

            if ( bIsLittleEndian != LUA_ISLITTLEENDIAN ) {
                throw new LuaRuntimeException ( "BigEndian is not supported for now." );
            }

            if ( dis.read () != LUA_SIZEOFINT ) {
                throw new LuaRuntimeException ( "Only sizeof( int ) = 4 is supported." );
            }

            iSizeOfsize_t = dis.read ();
            if ( iSizeOfsize_t != LUA_SIZEOFSIZET4 && iSizeOfsize_t != LUA_SIZEOFSIZET8 ) {
                throw new LuaRuntimeException ( "Only sizeof( size_t ) = 4 is supported." );
            }

            if ( dis.read () != LUA_SIZEOFINSTRUCTION ) {
                throw new LuaRuntimeException ( "Only sizeof( Instruction ) = 4 is supported." );
            }

            if ( dis.read () != LUA_SIZEOFNUMBER ) {
                throw new LuaRuntimeException ( "Only sizeof( Number ) = 4 is supported." );
            }

            if ( dis.read () == 0 ? false : true != LUA_ISINTEGRAL ) {
                throw new LuaRuntimeException ( "Only sizeof( Integral ) = 4 is supported." );
            }

            LuaFunction luaFunction = new LuaFunction ( dis, bIsLittleEndian, iSizeOfsize_t, "=?" );
            Function function = new Function ( luaFunction, GetEnvironment () );

            return function;
        }
        catch ( IOException ex ) {
            throw new LuaRuntimeException ( ex.getMessage () == null ? "Not a LUAs bytecode file" : ex.getMessage () );
        }
        finally {
            if ( dis != null ) {
                try {
                    dis.close ();
                }
                catch ( IOException e ) {
                    e.printStackTrace ();
                }
                finally {
                    dis = null;
                }
            }

            if ( fileConnection != null ) {
                try {
                    fileConnection.close ();
                }
                catch ( IOException e ) {
                    e.printStackTrace ();
                }
                finally {
                    fileConnection = null;
                }
            }
        }
    }

    public void CloseUpValues ( int iCloseIndex ) {
        int iLoopIndex = m_LiveUpValues.size ();
        while (  -- iLoopIndex >= 0 ) {
            UpValue uv = ( UpValue ) m_LiveUpValues.elementAt ( iLoopIndex );
            if ( uv.GetIndex () < iCloseIndex ) {
                return;
            }

            int iIndex = uv.GetIndex ();
            uv.Reset ();
            uv.SetValue ( this.m_ObjectsStack[iIndex] );

            m_LiveUpValues.removeElementAt ( iLoopIndex );
        }
    }

    public UpValue FindUpValue ( int iScanIndex ) {
        int iLoopIndex = m_LiveUpValues.size ();
        while (  -- iLoopIndex >= 0 ) {
            UpValue uv = ( UpValue ) m_LiveUpValues.elementAt ( iLoopIndex );
            if ( uv.GetIndex () == iScanIndex ) {
                return uv;
            }
            if ( uv.GetIndex () < iScanIndex ) {
                break;
            }
        }
        UpValue uv = new UpValue ( this, iScanIndex );

        m_LiveUpValues.insertElementAt ( uv, iLoopIndex + 1 );
        return uv;
    }

    public int getNCCalls () {
        return this.nCcalls;
    }

    public void setNCCalls ( int newNCCalls ) {
        this.nCcalls = newNCCalls;
    }

    public int getBaseCcalls () {
        return this.baseCcalls;
    }

    public void setBaseCcalls ( int newBaseCcalls ) {
        this.baseCcalls = newBaseCcalls;
    }

    public boolean GetMainThreadFlag () {
        return isMainThread;
    }
}

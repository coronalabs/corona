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

import java.io.DataInputStream;
import java.io.IOException;
import java.io.ByteArrayOutputStream;

/**
 *
 * @author a.fornwald
 */
class LuaFunction {

    public class LocalVariable {

        private String m_strName;
        private int m_iStartIP;
        private int m_iEndIP;

        public LocalVariable ( String strName, int iStartIP, int iEndIP ) {
            this.m_strName = strName;
            this.m_iStartIP = iStartIP;
            this.m_iEndIP = iEndIP;
        }

        public final String GetName () {
            return this.m_strName;
        }

        public final int GetStartIP () {
            return this.m_iStartIP;
        }

        public final int GetEndIP () {
            return this.m_iEndIP;
        }
    }
    private static final int VARARG_HASARG = 1;
    private static final int VARARG_ISVARARG = 2;
    private static final int VARARG_NEEDSARG = 4;
    private int[] m_aOpcodes;
    private Object[] m_aConstants;
    private String m_strSource;
    private int m_iLineDefined;
    private int m_iLastLineDefined;
    private int m_iUpValuesQuantity;
    private int m_iParamsQuantity;
    private boolean m_bIsVararg;
    private boolean m_bNeedsArg;
    private int m_iMaxStackSize;
    private LuaFunction[] m_LuaFunctions;
    private int[] m_aDebugLines;
    private String[] m_strUpValuesNames;
    private LocalVariable[] m_LocalVariables;

    public int GetLocalVairablesSize () {
        return m_LocalVariables != null ? m_LocalVariables.length : 0;
    }

    public LocalVariable GetLocalVariable ( int iIndex ) {
        return m_LocalVariables[iIndex];
    }

    public final int GetDebugLinesQuantity () {
        return this.m_aDebugLines != null ? this.m_aDebugLines.length : 0;
    }

    public final int GetDebugLine ( int iIndex ) {
        return this.m_aDebugLines[iIndex];
    }

    public final int GetConstantsQuantity () {
        return this.m_aConstants != null ? this.m_aConstants.length : 0;
    }

    public final String GetUpValueName ( int iIndex ) {
        return m_strUpValuesNames[iIndex];
    }

    public final int GetLineDefined () {
        return m_iLineDefined;
    }

    public final int GetLastLineDefined () {
        return m_iLastLineDefined;
    }

    public final String GetSource () {
        return m_strSource;
    }

    public final boolean GetIsVararg () {
        return this.m_bIsVararg;
    }

    public final boolean GetNeedsArg () {
        return this.m_bNeedsArg;
    }

    public final int GetParamsQuantity () {
        return m_iParamsQuantity;
    }

    public final int GetMaxStackSize () {
        return this.m_iMaxStackSize;
    }

    public final int GetUpValuesSize () {
        return this.m_strUpValuesNames.length;
    }

    public static final int ReverseBytes ( int intToReverse ) {
        int a = ( intToReverse >> 24 ) & 0x000000FF;
        int b = ( intToReverse >> 8 ) & 0x0000FF00;
        int c = ( intToReverse << 8 ) & 0x00FF0000;
        int d = intToReverse << 24;

        return d | c | b | a;

    }

    public static final long ReverseBytes ( long v ) {
        long a1 = ReverseBytes ( ( int ) ( v >> 32 ) ) & 0xFFFFFFFFL;
        long b1 = ReverseBytes ( ( int ) v ) & 0xFFFFFFFFL;

        return ( b1 << 32 ) | a1;
    }

    public static final int ReadLuaInt ( DataInputStream dis, boolean bIsLittleEndian ) throws IOException {
        if ( bIsLittleEndian == true ) {
            return ReverseBytes ( dis.readInt () );
        }
        else {
            return dis.readInt ();
        }
    }

    public static final long ReadLuaLong ( DataInputStream dis, boolean bIsLittleEndian ) throws IOException {
        if ( bIsLittleEndian == true ) {
            return ReverseBytes ( dis.readLong () );
        }
        else {
            return dis.readLong ();
        }
    }

    public static String ReadLuaString ( DataInputStream dis, boolean bIsLittleEndian, int iSizeOfsize_t ) throws IOException {
        int iStringLength = 0;

        if ( iSizeOfsize_t == 4 ) {
            iStringLength = ReadLuaInt ( dis, bIsLittleEndian );
        }
        else // 8
        {
            iStringLength = ( int ) ReadLuaLong ( dis, bIsLittleEndian );
        }

        if ( iStringLength == 0 ) {
            return null;
        }

        iStringLength = iStringLength - 1;  // Null char is not needed

        byte[] aStringData = new byte[ iStringLength ];

        int iBytesRead = dis.read ( aStringData, 0, iStringLength );

        dis.read (); // Read the null char

        String returnString = new String ( aStringData );

        aStringData = null;

        return returnString;
    }

    public LuaFunction ( DataInputStream dis, boolean bIsLittleEndian, int iSizeOfsize_t, String strSource ) throws IOException {
        this.m_strSource = ReadLuaString ( dis, bIsLittleEndian, iSizeOfsize_t );
        if ( m_strSource == null ) {
            this.m_strSource = strSource;
        }

        this.m_iLineDefined = ReadLuaInt ( dis, bIsLittleEndian );
        this.m_iLastLineDefined = ReadLuaInt ( dis, bIsLittleEndian );

        this.m_iUpValuesQuantity = dis.read ();
        this.m_iParamsQuantity = dis.read ();

        int bVararg = dis.read ();
        this.m_bIsVararg = ( bVararg & VARARG_ISVARARG ) != 0;
        this.m_bNeedsArg = ( bVararg & VARARG_NEEDSARG ) != 0;

        this.m_iMaxStackSize = dis.read ();

        // Load code
        int iCodeSize = ReadLuaInt ( dis, bIsLittleEndian );

        this.m_aOpcodes = new int[ iCodeSize ];
        for ( int iOpCode = 0; iOpCode < iCodeSize; iOpCode ++ ) {
            m_aOpcodes[iOpCode] = ReadLuaInt ( dis, bIsLittleEndian );
        }

        // Load constants
        int iConstantsQuantity = ReadLuaInt ( dis, bIsLittleEndian );
        this.m_aConstants = new Object[ iConstantsQuantity ];
        for ( int iConstant = 0; iConstant < iConstantsQuantity; iConstant ++ ) {
            switch ( dis.read () ) {
                case LuaAPI.LUA_TNIL:
                     {
                        this.m_aConstants[iConstant] = null;
                    }
                    break;
                case LuaAPI.LUA_TBOOLEAN:
                     {
                        this.m_aConstants[iConstant] = new Boolean ( dis.read () != 0 );
                    }
                    break;
                case LuaAPI.LUA_TNUMBER:
                     {
                        this.m_aConstants[iConstant] = new Double ( Double.longBitsToDouble ( ReadLuaLong ( dis, bIsLittleEndian ) ) );
                    }
                    break;
                case LuaAPI.LUA_TSTRING:
                     {
                        this.m_aConstants[iConstant] = ReadLuaString ( dis, bIsLittleEndian, iSizeOfsize_t );
                    }
                    break;
                default: {
                    throw new LuaRuntimeException ( "Constant value has a wrong type." );
                }
            }
        }

        // Load prototypes
        int iLuaFunctionsQuantity = ReadLuaInt ( dis, bIsLittleEndian );
        this.m_LuaFunctions = new LuaFunction[ iLuaFunctionsQuantity ];
        for ( int iLuaFunction = 0; iLuaFunction < iLuaFunctionsQuantity; iLuaFunction ++ ) {
            this.m_LuaFunctions[iLuaFunction] = new LuaFunction ( dis, bIsLittleEndian, iSizeOfsize_t, this.m_strSource );
        }

        // Load debug
        int iDebugLines = ReadLuaInt ( dis, bIsLittleEndian );
        this.m_aDebugLines = new int[ iDebugLines ];
        for ( int iLine = 0; iLine < iDebugLines; iLine ++ ) {
            this.m_aDebugLines[iLine] = ReadLuaInt ( dis, bIsLittleEndian );
        }

        // Read local vairables
        int iLocalVairablesQuantity = ReadLuaInt ( dis, bIsLittleEndian );
        this.m_LocalVariables = new LocalVariable[ iLocalVairablesQuantity ];
        for ( int iLocVar = 0; iLocVar < iLocalVairablesQuantity; iLocVar ++ ) {
            this.m_LocalVariables[iLocVar] = new LocalVariable (
                ReadLuaString ( dis, bIsLittleEndian, iSizeOfsize_t ),
                ReadLuaInt ( dis, bIsLittleEndian ),
                ReadLuaInt ( dis, bIsLittleEndian ) );
        }

        // Read upvalues
        int iUpValuesNames = ReadLuaInt ( dis, bIsLittleEndian );
        this.m_strUpValuesNames = new String[ iUpValuesNames ];
        for ( int iUpValuesName = 0; iUpValuesName < iUpValuesNames; iUpValuesName ++ ) {
            this.m_strUpValuesNames[iUpValuesName] = ReadLuaString ( dis, bIsLittleEndian, iSizeOfsize_t );
        }
    }

    public final int GetOpCode ( int iIndex ) {
        return this.m_aOpcodes[iIndex];
    }

    public final int GetSizeOpCode () {
        return this.m_aOpcodes != null ? this.m_aOpcodes.length : 0;
    }

    public final int GetLuaFunctionsQuantity () {
        return this.m_LuaFunctions.length;
    }

    public final Object GetConstant ( int iIndex ) {
        return this.m_aConstants[iIndex];
    }

    public final int[] GetDebugLines () {
        return this.m_aDebugLines;
    }

    public final LuaFunction GetLuaFunction ( int iIndex ) {
        return this.m_LuaFunctions[iIndex];
    }

    public final int GetUpValuesQuantity () {
        return this.m_iUpValuesQuantity;
    }

    public static void WriteLuaString ( ByteArrayOutputStream baos, boolean bIsLittleEndian, int iSizeOfSize_t, String strString ) {
        if ( strString == null ) {
            WriteLuaInt ( baos, bIsLittleEndian, 0 );
            return;
        }

        if ( iSizeOfSize_t == 4 ) {
            WriteLuaInt ( baos, bIsLittleEndian, strString.length () + 1 );
        }
        else // 8
        {
            WriteLuaLong ( baos, bIsLittleEndian, strString.length () + 1 );
        }

        baos.write ( strString.getBytes (), 0, strString.length () );
        baos.write ( 0 );
    }

    public static void WriteLuaInt ( ByteArrayOutputStream baos, boolean bIsLittleEndian, int iValue ) {
        if ( bIsLittleEndian == true ) {
            baos.write ( ( iValue ) & 0xFF );
            baos.write ( ( iValue >> 8 ) & 0xFF );
            baos.write ( ( iValue >> 16 ) & 0xFF );
            baos.write ( ( iValue >> 24 ) & 0xFF );
        /*baos.write( 0x80 );
        baos.write( 0x81 );
        baos.write( 0x82 );
        baos.write( 0x83 );*/
        }
        else {
            baos.write ( ( iValue >> 24 ) & 0xFF );
            baos.write ( ( iValue >> 16 ) & 0xFF );
            baos.write ( ( iValue >> 8 ) & 0xFF );
            baos.write ( ( iValue ) & 0xFF );
        }
    }

    public static void WriteLuaLong ( ByteArrayOutputStream baos, boolean bIsLittleEndian, long lValue ) {
        if ( bIsLittleEndian == true ) {
            WriteLuaInt ( baos, bIsLittleEndian, ( int ) ( lValue ) );
            WriteLuaInt ( baos, bIsLittleEndian, ( int ) ( lValue >> 32 ) );
        }
        else {
            WriteLuaInt ( baos, bIsLittleEndian, ( int ) ( lValue >> 32 ) );
            WriteLuaInt ( baos, bIsLittleEndian, ( int ) ( lValue ) );
        }
    }

    public void Dump ( ByteArrayOutputStream baos, int iSizeOfsize_t, boolean bIsLittleEndian, lua_Writer writer, String strSource, Object userData ) {
        String strSourceToWrite = this.m_strSource.equals ( strSource ) == true ? null : this.m_strSource;
        WriteLuaString ( baos, bIsLittleEndian, iSizeOfsize_t, strSourceToWrite );

        WriteLuaInt ( baos, bIsLittleEndian, this.m_iLineDefined );
        WriteLuaInt ( baos, bIsLittleEndian, this.m_iLastLineDefined );

        baos.write ( this.m_iUpValuesQuantity );
        baos.write ( this.m_iParamsQuantity );

        int bVarArg = 0;
        if ( this.m_bIsVararg == true ) {
            bVarArg = bVarArg | VARARG_ISVARARG;
        }
        if ( this.m_bNeedsArg == true ) {
            bVarArg = bVarArg | VARARG_NEEDSARG;
        }
        baos.write ( bVarArg );
        baos.write ( this.m_iMaxStackSize );

        // Save code
        WriteLuaInt ( baos, bIsLittleEndian, this.m_aOpcodes.length );

        for ( int iOpCode = 0; iOpCode < m_aOpcodes.length; iOpCode ++ ) {
            WriteLuaInt ( baos, bIsLittleEndian, this.m_aOpcodes[iOpCode] );
        }

        // Save constants
        WriteLuaInt ( baos, bIsLittleEndian, this.m_aConstants.length );
        for ( int iConstant = 0; iConstant < m_aConstants.length; iConstant ++ ) {
            if ( this.m_aConstants[iConstant] == null ) {
                baos.write ( LuaAPI.LUA_TNIL );
            }
            else if ( this.m_aConstants[iConstant] instanceof Boolean ) {
                baos.write ( LuaAPI.LUA_TBOOLEAN );
                baos.write ( ( ( Boolean ) this.m_aConstants[iConstant] ).booleanValue () == false ? 0 : 1 ); // new Boolean( dis.read() != 0 );

            }
            else if ( this.m_aConstants[iConstant] instanceof Double ) {
                baos.write ( LuaAPI.LUA_TNUMBER );
                WriteLuaLong ( baos, bIsLittleEndian, Double.doubleToLongBits ( ( ( Double ) this.m_aConstants[iConstant] ).doubleValue () ) );
            }
            else if ( this.m_aConstants[iConstant] instanceof String ) {
                baos.write ( LuaAPI.LUA_TSTRING );
                WriteLuaString ( baos, bIsLittleEndian, iSizeOfsize_t, ( String ) this.m_aConstants[iConstant] );
            }
            else {
                throw new LuaRuntimeException ( "Constant value has a wrong type." );
            }
        }

        // Save prototypes
        WriteLuaInt ( baos, bIsLittleEndian, this.m_LuaFunctions.length );
        for ( int iLuaFunction = 0; iLuaFunction < this.m_LuaFunctions.length; iLuaFunction ++ ) {
            this.m_LuaFunctions[iLuaFunction].Dump ( baos, iSizeOfsize_t, bIsLittleEndian, writer, this.m_strSource, userData );
        }

        // Save debug
        WriteLuaInt ( baos, bIsLittleEndian, this.m_aDebugLines.length );
        for ( int iLine = 0; iLine < this.m_aDebugLines.length; iLine ++ ) {
            WriteLuaInt ( baos, bIsLittleEndian, this.m_aDebugLines[iLine] );
        }

        // Save debug
        WriteLuaInt ( baos, bIsLittleEndian, this.m_LocalVariables.length );
        for ( int iLocVar = 0; iLocVar < this.m_LocalVariables.length; iLocVar ++ ) {
            WriteLuaString ( baos, bIsLittleEndian, iSizeOfsize_t, this.m_LocalVariables[iLocVar].GetName () );
            WriteLuaInt ( baos, bIsLittleEndian, this.m_LocalVariables[iLocVar].GetStartIP () );
            WriteLuaInt ( baos, bIsLittleEndian, this.m_LocalVariables[iLocVar].GetEndIP () );
        }

        // Save upvalues
        WriteLuaInt ( baos, bIsLittleEndian, this.m_strUpValuesNames.length );
        for ( int iUpValue = 0; iUpValue < this.m_strUpValuesNames.length; iUpValue ++ ) {
            WriteLuaString ( baos, bIsLittleEndian, iSizeOfsize_t, this.m_strUpValuesNames[iUpValue] );
        }
    }
}

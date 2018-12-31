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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Vector;
import javax.microedition.io.Connector;
import javax.microedition.io.file.FileConnection;
import javax.microedition.io.file.FileSystemRegistry;

/**
 *
 * @author p.pavelko
 */
class LuaIOLib {

    public final static int IO_INPUT = 1;
    public final static int IO_OUTPUT = 2;
    public final static int SEEK_CUR = 1;
    public final static int SEEK_END = 2;
    public final static int SEEK_SET = 0;
    public final static int _IOFBF = 0x0000;
    public final static int _IOLBF = 0x0040;
    public final static int _IONBF = 0x0004;
    //public final static int LIMIT = 10;
    public final static String[] fnames = { "input", "output" };

    static class FileStruct {

        public static final int ACCESS_MODE_READ = 0;
        public static final int ACCESS_MODE_WRITE = 1;
        public static final int ACCESS_MODE_APPEND = 2;
        public static final int ACCESS_MODE_READ_PLUS = 3;
        public static final int ACCESS_MODE_WRITE_PLUS = 4;
        public static final int ACCESS_MODE_APPEND_PLUS = 5;
        private static final int OP_NONE = 0;
        private static final int OP_READ = 1;
        private static final int OP_WRITE = 2;
        public StringBuffer buffer = null;
        public int bufferPosition = 0;
        public FileConnection connection = null;
        public InputStream inputStream = null;
        public OutputStream outputStream = null;
        public boolean binaryMode = false;
        public int accessMode = ACCESS_MODE_READ;
        public boolean isTmpFile = false;
        public int bufferingType = _IOFBF;
        public int bufferSize = LuaAPI.LUAL_BUFFERSIZE;
        public int bufferCounter = 0;
        public boolean isClosed = false;
        private boolean isFlushed = true;
        private int lastOp = OP_NONE;
        public boolean bothRWMode = false;

        public int fread ( StringBuffer buf, int size, int count ) {
            if ( bothRWMode && lastOp == OP_WRITE ) {
                return -1;
            }
            try {
                switch ( accessMode ) {
                    case ACCESS_MODE_WRITE:
                    case ACCESS_MODE_APPEND: {
                        return -1;
                    }
                    case ACCESS_MODE_READ: {
                        if ( inputStream == null )
                            inputStream = connection.openInputStream ();
                        if ( inputStream == null )
                            return -1;

                        int k = 0;
                        int i = 0;
                        lastOp = OP_READ;
                        for (; i < count * size; i ++ ) {
                            k = inputStream.read ();
                            if ( k == -1 )
                                return -1;
                            buf.append ( ( char ) k );
                            bufferPosition ++;
                        }
                        return i;
                    }
                    case ACCESS_MODE_READ_PLUS:
                    case ACCESS_MODE_WRITE_PLUS:
                    case ACCESS_MODE_APPEND_PLUS: {
                        int i = 0;
                        lastOp = OP_READ;
                        for (; i < count * size; i ++ ) {
                            if ( bufferPosition >= buffer.length () )
                                return -1;
                            buf.append ( buffer.charAt ( bufferPosition ++ ) );
                        }
                        return i;
                    }
                }
            }
            catch ( Exception ex ) {
                return -1;
            }
            return -1;
        }

        public int fwrite ( String buf, int size, int count ) {
            if ( bothRWMode && lastOp == OP_READ ) {
                return -1;
            }

            switch ( accessMode ) {
                case ACCESS_MODE_READ: {
                    return -1;
                }
                case ACCESS_MODE_READ_PLUS:
                case ACCESS_MODE_WRITE:
                case ACCESS_MODE_WRITE_PLUS: {
                    int i = 0;
                    lastOp = OP_WRITE;
                    isFlushed = false;
                    for (; i < count * size; i ++ ) {
                        bufferCounter ++;
                        if ( bufferPosition >= buffer.length () ) {
                            buffer.append ( buf.charAt ( i ) );
                            bufferPosition ++;
                        }
                        else {
                            buffer.setCharAt ( bufferPosition ++, buf.charAt ( i ) );
                        }
                    }

                    switch ( bufferingType ) {
                        case _IOFBF:
                             {
                                if ( bufferCounter > bufferSize ) {
                                    try {
                                        fflush ();
                                        bufferCounter = 0;
                                    }
                                    catch ( Exception ex ) {
                                        return -1;
                                    }
                                }
                            }
                            break;
                        case _IOLBF:
                             {
                                if ( buf.toString ().indexOf ( "\n" ) != -1 || bufferCounter > bufferSize ) {
                                    try {
                                        fflush ();
                                        if ( bufferCounter > bufferSize )
                                            bufferCounter = 0;
                                    }
                                    catch ( Exception ex ) {
                                        return -1;
                                    }
                                }
                            }
                            break;
                        case _IONBF:
                             {
                                try {
                                    fflush ();
                                }
                                catch ( Exception ex ) {
                                    return -1;
                                }
                            }
                            break;
                    }
                    return i;
                }
                case ACCESS_MODE_APPEND:
                case ACCESS_MODE_APPEND_PLUS: {
                    int i = 0;
                    isFlushed = false;
                    lastOp = OP_WRITE;
                    for (; i < count * size; i ++ ) {
                        bufferCounter ++;
                        buffer.append ( buf.charAt ( i ) );
                    }
                    bufferPosition = buffer.length ();
                    switch ( bufferingType ) {
                        case _IOFBF:
                             {
                                if ( bufferCounter > bufferSize ) {
                                    try {
                                        fflush ();
                                        bufferCounter = 0;
                                    }
                                    catch ( Exception ex ) {
                                        return -1;
                                    }
                                }
                            }
                            break;
                        case _IOLBF:
                             {
                                if ( buf.toString ().indexOf ( "\n" ) != -1 || bufferCounter > bufferSize ) {
                                    try {
                                        fflush ();
                                        if ( bufferCounter > bufferSize )
                                            bufferCounter = 0;
                                    }
                                    catch ( Exception ex ) {
                                        return -1;
                                    }
                                }
                            }
                            break;
                        case _IONBF:
                             {
                                try {
                                    fflush ();
                                }
                                catch ( Exception ex ) {
                                    return -1;
                                }
                            }
                            break;
                    }
                    return i;
                }
            }
            return -1;
        }

        public double readNumber () throws Exception {
            if ( bothRWMode && lastOp == OP_WRITE ) {
                return -1;
            }
            switch ( accessMode ) {
                case ACCESS_MODE_WRITE:
                case ACCESS_MODE_APPEND: {
                    return -1;
                }
                case ACCESS_MODE_READ: {
                    if ( inputStream == null )
                        inputStream = connection.openInputStream ();
                    if ( inputStream == null )
                        throw new Exception ();

                    int k = 0;
                    lastOp = OP_READ;
                    StringBuffer sb = new StringBuffer ();
                    boolean isNonWSCharOccurred = false;

                    for (;;) {
                        k = inputStream.read ();
                        bufferPosition ++;
                        switch ( k ) {
                            case ' ':
                            case '\n':
                            case '\t': {
                                if ( isNonWSCharOccurred == false ) {
                                    continue;
                                }
                                bufferPosition --;
                                inputStream.reset ();
                                inputStream.skip ( bufferPosition );

                                Double res = new Double ( ( ( Double ) Double.valueOf ( sb.toString () ) ).doubleValue () );
                                if ( res != null ) {
                                    return res.doubleValue ();
                                }
                                else {
                                    throw new Exception ( "readNumber: String is not number" );
                                }
                            }
                            case -1: {
                                Double res = new Double ( ( ( Double ) Double.valueOf ( sb.toString () ) ).doubleValue () );
                                if ( res != null ) {
                                    return res.doubleValue ();
                                }
                                else {
                                    throw new Exception ( "readNumber: String is not number" );
                                }

                            }
                            default:
                                sb.append ( ( char ) k );
                                isNonWSCharOccurred = true;
                                break;
                        }
                    }
                }
                case ACCESS_MODE_READ_PLUS:
                case ACCESS_MODE_WRITE_PLUS:
                case ACCESS_MODE_APPEND_PLUS: {
                    lastOp = OP_READ;
                    StringBuffer buf = new StringBuffer ();
                    int k = 0;
                    boolean isNonWSCharOccurred = false;
                    for (;;) {
                        k = buffer.charAt ( bufferPosition ++ );
                        switch ( k ) {
                            case ' ':
                            case '\n':
                            case '\t': {
                                if ( isNonWSCharOccurred == false ) {
                                    continue;
                                }
                                bufferPosition --;

                                Double res = new Double ( ( ( Double ) Double.valueOf ( buf.toString () ) ).doubleValue () );
                                if ( res != null ) {
                                    return res.doubleValue ();
                                }
                                else {
                                    throw new Exception ( "readNumber: String is not number" );
                                }
                            }
                            case -1: {
                                Double res = new Double ( ( ( Double ) Double.valueOf ( buf.toString () ) ).doubleValue () );
                                if ( res != null ) {
                                    return res.doubleValue ();
                                }
                                else {
                                    throw new Exception ( "readNumber: String is not number" );
                                }

                            }
                            default:
                                buf.append ( ( char ) k );
                                isNonWSCharOccurred = true;
                                break;
                        }
                    }
                }
            }
            return -1;
        }

        public StringBuffer fgets ( StringBuffer str, int num ) {
            if ( bothRWMode && lastOp == OP_WRITE ) {
                return null;
            }
            try {
                switch ( accessMode ) {
                    case ACCESS_MODE_WRITE:
                    case ACCESS_MODE_APPEND: {
                        return null;
                    }
                    case ACCESS_MODE_READ: {
                        lastOp = OP_READ;
                        if ( inputStream == null )
                            inputStream = connection.openInputStream ();
                        if ( inputStream == null )
                            return null;

                        int k = 0;
                        for ( int i = 0; i < num; i ++ ) {
                            k = inputStream.read ();
                            if ( k == -1 ) {
                                return null;
                            }
                            str.append ( ( char ) k );
                            bufferPosition ++;
                            if ( ( ( char ) k ) == '\n' )
                                return str;
                        }
                        return str;
                    }
                    case ACCESS_MODE_READ_PLUS:
                    case ACCESS_MODE_WRITE_PLUS:
                    case ACCESS_MODE_APPEND_PLUS: {
                        lastOp = OP_READ;
                        for ( int i = 0; i < num; i ++ ) {
                            if ( bufferPosition >= buffer.length () ) {
                                if ( i == 0 )
                                    return null;
                                else
                                    return str;
                            }
                            else {
                                str.append ( buffer.charAt ( bufferPosition ) );
                                if ( buffer.charAt ( bufferPosition ++ ) == '\n' )
                                    return str;

                            }
                        }
                        return str;
                    }
                }
                return null;
            }
            catch ( Exception ex ) {
                return null;
            }
        }

        public int ftell () {
            return bufferPosition;
        }

        public int fseek ( long offset, int origin ) throws Exception {
            fflush ();
            switch ( origin ) {
                case SEEK_SET:
                     {
                        if ( offset < 0 )
                            return 1;
                        switch ( accessMode ) {
                            case ACCESS_MODE_READ: {
                                if ( connection == null )
                                    return 1;
                                if ( inputStream == null )
                                    inputStream = connection.openInputStream ();
                                if ( inputStream == null )
                                    return 1;
                                inputStream.reset ();
                                inputStream.skip ( offset );
                                bufferPosition = ( int ) ( offset );
                                return 0;
                            }
                            case ACCESS_MODE_WRITE:
                            case ACCESS_MODE_APPEND:
                            case ACCESS_MODE_READ_PLUS:
                            case ACCESS_MODE_WRITE_PLUS:
                            case ACCESS_MODE_APPEND_PLUS: {
                                bufferPosition = ( int ) ( offset );
                                return 0;
                            }
                        }
                    }
                    break;
                case SEEK_CUR:
                     {
                        switch ( accessMode ) {
                            case ACCESS_MODE_READ: {
                                if ( connection == null )
                                    return 1;
                                if ( inputStream == null )
                                    inputStream = connection.openInputStream ();
                                if ( inputStream == null )
                                    return 1;

                                bufferPosition += ( int ) ( offset );
                                if ( bufferPosition < 0 )
                                    bufferPosition = 0;
                                if ( offset > 0 ) {
                                    inputStream.skip ( offset );
                                    return 0;
                                }
                                else {
                                    inputStream.reset ();
                                    inputStream.skip ( bufferPosition );
                                    return 0;
                                }
                            }
                            case ACCESS_MODE_WRITE:
                            case ACCESS_MODE_APPEND:
                            case ACCESS_MODE_READ_PLUS:
                            case ACCESS_MODE_WRITE_PLUS:
                            case ACCESS_MODE_APPEND_PLUS: {
                                bufferPosition += ( int ) ( offset );
                                if ( bufferPosition < 0 )
                                    bufferPosition = 0;
                                return 0;
                            }
                        }
                    }
                    break;
                case SEEK_END:
                     {
                        if ( offset < 0 )
                            return 1;

                        switch ( accessMode ) {
                            case ACCESS_MODE_READ: {
                                if ( connection == null )
                                    return 1;
                                if ( inputStream == null )
                                    inputStream = connection.openInputStream ();
                                if ( inputStream == null )
                                    return 1;

                                int fileSize = 0;
                                byte c;
                                int j = 0;
                                inputStream.reset ();
                                while ( ( j = inputStream.read () ) != -1 ) {
                                    fileSize ++;
                                }
                                inputStream.reset ();

                                bufferPosition = fileSize - ( int ) offset;
                                if ( bufferPosition < 0 )
                                    bufferPosition = 0;
                                inputStream.skip ( bufferPosition );
                                return 0;
                            }
                            case ACCESS_MODE_WRITE:
                            case ACCESS_MODE_APPEND:
                            case ACCESS_MODE_READ_PLUS:
                            case ACCESS_MODE_WRITE_PLUS:
                            case ACCESS_MODE_APPEND_PLUS: {
                                int fileSize = buffer.length ();
                                bufferPosition = fileSize - ( int ) offset;
                                if ( bufferPosition < 0 )
                                    bufferPosition = 0;
                                return 0;
                            }
                        }
                    }
                    break;
            }
            return 1;
        }

        public int fflush () throws Exception {
            lastOp = OP_NONE;
            switch ( accessMode ) {
                case ACCESS_MODE_READ: {
                    return 0;
                }
                case ACCESS_MODE_WRITE:
                case ACCESS_MODE_APPEND:
                case ACCESS_MODE_READ_PLUS:
                case ACCESS_MODE_WRITE_PLUS:
                case ACCESS_MODE_APPEND_PLUS: {
                    if ( connection == null )
                        return 1;
                    if ( outputStream != null ) {
                        outputStream.close ();
                        outputStream = null;
                    }
                    outputStream = connection.openOutputStream ();
                    if ( outputStream == null )
                        return 1;
                    if ( isFlushed ) {
                        return 0;
                    }
                    else {
                        outputStream.write ( buffer.toString ().getBytes () );
                        isFlushed = true;
                        return 0;
                    }
                }
            }
            return 0;
        }

        public int fclose () {
            if ( isClosed ) {
                return -1;
            }
            try {
                if ( fflush () != 0 )
                    return -1;
                if ( inputStream != null ) {
                    inputStream.close ();
                    inputStream = null;
                }
                if ( outputStream != null ) {
                    outputStream.close ();
                    outputStream = null;
                }

                if ( connection != null ) {
                    if ( isTmpFile && connection.exists () ) {
                        connection.delete ();
                    }

                    connection.close ();
                    connection = null;
                }
                if ( buffer != null ) {
                    buffer = null;
                }
                if ( tempFiles != null && tempFiles.contains ( this ) ) {
                    tempFiles.removeElement ( this );
                }
                System.gc ();
                isClosed = true;
                return 0;
            }
            catch ( Exception ex ) {
                return -1;
            }
        }

        public int setvbuf ( int mode, int buffSize ) {
            if ( buffSize <= 0 && mode != _IONBF )
                return 1;
            bufferingType = mode;
            bufferSize = buffSize;
            return 0;
        }
    }
    public static final String LUA_IOLIBNAME = "io";
    private static Vector tempFiles = null;

    public static void clearTempFilesVector () {
        if ( tempFiles != null ) {
            int size = tempFiles.size ();
            for ( int i = 0; i < size; i ++ ) {
                FileStruct file = ( FileStruct ) tempFiles.elementAt ( i );
                file.fclose ();
            }
            tempFiles.removeAllElements ();
            tempFiles = null;
        }
    }

    public static FileStruct fopen ( FileStruct file, String filename ) {
        return fopen ( file, filename, "r" );
    }

    public static FileStruct tmpfile ( lua_State thread, FileStruct file ) {
        file.isTmpFile = true;
        Enumeration roots = FileSystemRegistry.listRoots ();
        String root = null;
        while ( roots.hasMoreElements () ) {
            root = ( String ) roots.nextElement ();
            if ( root != null ) {
                root = "/" + root;
                break;
            }
        }
        if ( root == null ) {
            LuaAPI.luaL_error ( thread, "There are no available FileSystem root for temp file creation" );
        }

        root += System.currentTimeMillis () + ".txt";
        file = fopen ( file, root, "w+" );
        if ( file != null ) {
            if ( tempFiles == null ) {
                tempFiles = new Vector ();
            }
            tempFiles.addElement ( file );
        }

        return file;
    }

    public static FileStruct fopen ( FileStruct file, String filename, String mode ) {
        if ( filename == null || mode == null )
            return null;

        if (  ! filename.startsWith ( "file://" ) )
            filename = "file://" + filename;

        file.buffer = new StringBuffer ();
        if ( mode.indexOf ( "b" ) != -1 )
            file.binaryMode = true;

        file.bothRWMode = false;
        if ( mode.indexOf ( "+" ) != -1 )
            file.bothRWMode = true;
        if ( mode.indexOf ( "r" ) != -1 ) {
            if ( file.bothRWMode )
                file.accessMode = FileStruct.ACCESS_MODE_READ_PLUS;
            else
                file.accessMode = FileStruct.ACCESS_MODE_READ;
        }
        if ( mode.indexOf ( "w" ) != -1 ) {
            if ( file.bothRWMode )
                file.accessMode = FileStruct.ACCESS_MODE_WRITE_PLUS;
            else
                file.accessMode = FileStruct.ACCESS_MODE_WRITE;
        }
        if ( mode.indexOf ( "a" ) != -1 ) {
            if ( file.bothRWMode )
                file.accessMode = FileStruct.ACCESS_MODE_APPEND_PLUS;
            else
                file.accessMode = FileStruct.ACCESS_MODE_APPEND;
        }

        try {
            //int mode1 = 0;
//            if(file.accessMode == FileStruct.ACCESS_MODE_WRITE || file.accessMode == FileStruct.ACCESS_MODE_WRITE_PLUS)
            //          {
            file.connection = ( FileConnection ) Connector.open ( filename );
            //       }
            //       else
            //     {
            //       mode1 = Connector.READ_WRITE;
            // }


            if ( file.connection == null )
                return null;

            if (  ! file.connection.exists () ) {
                if ( file.accessMode == FileStruct.ACCESS_MODE_READ || file.accessMode == FileStruct.ACCESS_MODE_READ_PLUS ) {
                    return null;
                }
                else if ( file.accessMode == FileStruct.ACCESS_MODE_APPEND || file.accessMode == FileStruct.ACCESS_MODE_APPEND_PLUS || file.accessMode == FileStruct.ACCESS_MODE_WRITE || file.accessMode == FileStruct.ACCESS_MODE_WRITE_PLUS )
                    file.connection.create ();
            }
            else if ( file.accessMode == FileStruct.ACCESS_MODE_WRITE || file.accessMode == FileStruct.ACCESS_MODE_WRITE_PLUS ) {
                file.connection.delete ();
                file.connection.close ();
                file.connection = null;
                file.connection = ( FileConnection ) Connector.open ( filename );
                if (  ! file.connection.exists () ) {
                    file.connection.create ();
                }
            }

            if ( file.accessMode == FileStruct.ACCESS_MODE_APPEND || file.accessMode == FileStruct.ACCESS_MODE_APPEND_PLUS || file.accessMode == FileStruct.ACCESS_MODE_READ_PLUS ) {
                file.inputStream = file.connection.openInputStream ();
                if ( file.inputStream == null )
                    return null;

                int j = 0;
                while ( ( j = file.inputStream.read () ) != -1 ) {
                    file.buffer.append ( ( char ) j );
                }
                if ( file.accessMode == FileStruct.ACCESS_MODE_READ_PLUS )
                    file.bufferPosition = 0;
                else
                    file.bufferPosition = file.buffer.length ();
            }
            else {
                file.bufferPosition = 0;
            }

            if ( file.accessMode == FileStruct.ACCESS_MODE_READ ) {
                file.inputStream = file.connection.openInputStream ();
                if ( file.inputStream == null )
                    return null;
                else
                    file.inputStream.mark ( Integer.MAX_VALUE );
            }


        }
        catch ( Exception ex ) {
            if ( file != null ) {
                if ( file.inputStream != null ) {
                    try {
                        file.inputStream.close ();
                    }
                    catch ( IOException e ) {
                    }
                    finally {
                        file.inputStream = null;
                    }
                }
                if ( file.connection != null ) {
                    try {
                        file.connection.close ();
                    }
                    catch ( IOException e ) {
                    }
                    finally {
                        file.connection = null;
                    }
                }
            }
            return null;
        }
        finally {
        }
        return file;
    }

    static int test_eof ( lua_State thread, FileStruct f ) {
        int c = f.fread ( new StringBuffer (), 1, 1 );
        if ( c != -1 ) {
            try {
                f.fseek ( -1, SEEK_CUR );
            }
            catch ( Exception ex ) {
                return 0;
            }
        }
        LuaAPI.lua_pushlstring ( thread, "", 0 );
        return c != -1 ? 1 : 0;
    }

    static int read_chars ( lua_State thread, FileStruct file, int n ) {
        int rlen;  /* how much to read */
        int nr;  /* number of chars actually read */
        StringBuffer p = new StringBuffer ();
        rlen = LuaAPI.LUAL_BUFFERSIZE;  /* try to read that much each time */
        if ( rlen > n ) /* cannot read more than asked */

            rlen = n;
        do {
            nr = file.fread ( p, 1, rlen );
            n -= nr;  /* still have to read `n' chars */
        } while ( n > 0 && nr == rlen );  /* until end of count or eof */
        LuaAPI.lua_pushstring ( thread, p.toString () );
        int res = 0;
        if ( n == 0 || LuaAPI.lua_objlen ( thread, -1 ) > 0 )
            res = 1;

        return res;
    }

    static void read_number ( lua_State thread, FileStruct file ) throws Exception {
        //Double d = Scanf.readNumber(file.inputStream);
        double d = file.readNumber ();
        LuaAPI.lua_pushnumber ( thread, d );
    }

    static int read_line ( lua_State thread, FileStruct file ) {
        StringBuffer p = new StringBuffer ();
        for (;;) {
            int l;
            if ( file.fgets ( p, LuaAPI.LUAL_BUFFERSIZE ) == null )/* eof? */ {
                LuaAPI.lua_pushstring ( thread, p.toString () );// luaL_pushresult(&b);  /* close buffer */

                int res = 0;
                if ( LuaAPI.lua_objlen ( thread, -1 ) > 0 ) /* check whether read something */

                    res = 1;
                return res;
            }
            //String c = p.toString();
            //System.out.println(c);
            l = p.length ();

            if ( l == 0 || p.charAt ( l - 1 ) != '\n' ) {
                //luaL_addsize(&b, l);
            }
            else {
                //luaL_addsize(&b, l - 1);  /* do not include `eol' */
                String str = p.toString ();
                int len = str.length ();
                len -= 1;
                if ( len < 0 )
                    len = 0;
                LuaAPI.lua_pushstring ( thread, str.substring ( 0, len ) );/* do not include `eol' */
                //luaL_pushresult(&b);  /* close buffer */
                return 1;  /* read at least an `eol' */
            }
        }
    }

    static int g_read ( lua_State thread, FileStruct file, int first ) {
        int nargs = LuaAPI.lua_gettop ( thread ) - 1;
        int success;
        int n;
        if ( nargs == 0 ) /* no arguments? */ {
            success = read_line ( thread, file );
            n = first + 1;  /* to return 1 result */
        }
        else /* ensure stack space for all results and for auxlib's buffer */ {
            LuaAPI.luaL_checkstack ( thread, nargs + LuaAPI.LUA_MINSTACK, "too many arguments" );
            success = 1;
            for ( n = first; ( nargs -- ) != 0 && success != 0; n ++ ) {
                if ( LuaAPI.lua_type ( thread, n ) == /*LUA_TNUMBER*/ 3 ) {
                    int l = ( int ) LuaAPI.lua_tointeger ( thread, n );
                    success = ( l == 0 ) ? test_eof ( thread, file ) : read_chars ( thread, file, l );
                }
                else {
                    String p = LuaAPI.lua_tostring ( thread, n );
                    LuaAPI.luaL_argcheck ( thread, p != null && p.charAt ( 0 ) == '*', n, "invalid option" );
                    switch ( p.charAt ( 1 ) ) {
                        case 'n':  /* number */
                            success = 1;
                            try {
                                read_number ( thread, file );
                            }
                            catch ( Exception ex ) {
                                success = 0;
                            }

                            break;
                        case 'l':  /* line */
                            success = read_line ( thread, file );
                            break;
                        case 'a':  /* file */
                            read_chars ( thread, file, Integer.MAX_VALUE );  /* read MAX_SIZE_T chars */
                            success = 1; /* always success */
                            break;
                        default:
                            return LuaAPI.luaL_argerror ( thread, n, "invalid format" );
                    }
                }
            }
        }
        //if (ferror(f))
        //    return pushresult(thread, 0, null);
        if ( success != 1 ) {
            LuaAPI.lua_pop ( thread, 1 );  /* remove last result */
            LuaAPI.lua_pushnil ( thread );  /* push nil instead */
        }
        return n - first;
    }

    public static final class io_read implements JavaFunction {

        public int Call ( lua_State thread ) {
            return g_read ( thread, getiofile ( thread, IO_INPUT ), 1 );
        }
    }

    static FileStruct getiofile ( lua_State thread, int findex ) {
        FileStruct f = null;
        LuaAPI.lua_rawgeti ( thread, LuaAPI.LUA_ENVIRONINDEX, findex );
        f = ( FileStruct ) LuaAPI.lua_touserdata ( thread, -1 );
        if ( f == null || f.isClosed == true )
            LuaAPI.luaL_error ( thread, "standard " + fnames[findex - 1] + " file is closed" );
        return f;
    }

    public static final class f_read implements JavaFunction {

        public int Call ( lua_State thread ) {
            return g_read ( thread, tofile ( thread ), 2 );
        }
    }

    static int g_iofile ( lua_State thread, int f, String mode ) {
        if (  ! LuaAPI.lua_isnoneornil ( thread, 1 ) ) {
            String filename = LuaAPI.lua_tostring ( thread, 1 );
            if ( filename != null ) {
                FileStruct pf = newfile ( thread );
                pf = fopen ( pf, filename, mode );
                if ( pf == null )
                    fileerror ( thread, 1, filename );
            }
            else {
                tofile ( thread );  /* check that it's a valid file handle */
                LuaAPI.lua_pushvalue ( thread, 1 );
            }
            LuaAPI.lua_rawseti ( thread, LuaAPI.LUA_ENVIRONINDEX, f );
        }
        /* return current value */
        LuaAPI.lua_rawgeti ( thread, LuaAPI.LUA_ENVIRONINDEX, f );
        return 1;
    }

    public static final class io_input implements JavaFunction {

        public int Call ( lua_State thread ) {
            return g_iofile ( thread, IO_INPUT, "r" );
        }
    }

    public static final class io_output implements JavaFunction {

        public int Call ( lua_State thread ) {
            return g_iofile ( thread, IO_OUTPUT, "w" );
        }
    }

    static int g_write ( lua_State thread, FileStruct f, int arg ) {
        int nargs = LuaAPI.lua_gettop ( thread ) - 1;
        int status = 1;
        for (; ( nargs -- ) > 0; arg ++ ) {
            if ( LuaAPI.lua_type ( thread, arg ) == LuaAPI.LUA_TNUMBER ) {
                /* optimization: could be done exactly as for strings */
                /*		String str = "" + LuaAPI.lua_tonumber(thread, arg);
                int g = str.substring(str.indexOf(".") + 1, str.length()).length();
                if(g > 14)
                str = str.substring(0, str.length() - (g - 14));
                StringBuffer sb = new StringBuffer();
                sb.append(str);
                if(status == 1 && f.fwrite(sb, 1, sb.length()) > 0)
                status = 1;
                else
                status = 0;
                 */
                String str = "";
                try {
                    str = new Printf ( "%.14g" ).sprintf ( LuaAPI.lua_tonumber ( thread, arg ) );
                }
                catch ( Exception ex ) {
                    status = 0;
                }
                status = status == 1 && f.fwrite ( str, 1, str.length () ) > 0 ? 1 : 0;
            }
            else {
                String s = LuaAPI.luaL_checklstring ( thread, arg );
                int iStrLen = LuaStringLib.StringLength ( s );
                status = status == 1 && ( f.fwrite ( s, 1, iStrLen ) == iStrLen ) ? 1 : 0;
            }
        }
        return pushresult ( thread, status, null );
    }

    public static final class io_write implements JavaFunction {

        public int Call ( lua_State thread ) {
            return g_write ( thread, getiofile ( thread, IO_OUTPUT ), 1 );
        }
    }

    public static final class f_write implements JavaFunction {

        public int Call ( lua_State thread ) {
            return g_write ( thread, tofile ( thread ), 2 );
        }
    }

    public static final class f_seek implements JavaFunction {

        public int Call ( lua_State thread ) {
            final int[] mode = { SEEK_SET, SEEK_CUR, SEEK_END };
            final String[] modenames = { "set", "cur", "end" };
            FileStruct f = tofile ( thread );
            int op = LuaAPI.luaL_checkoption ( thread, 2, "cur", modenames );
            long offset = LuaAPI.luaL_optlong ( thread, 3, 0 );
            try {
                op = f.fseek ( offset, mode[op] );
            }
            catch ( Exception ex ) {
                op = 1;
            }
            if ( op != 0 )
                return pushresult ( thread, 0, null );  /* error */

            else {
                LuaAPI.lua_pushinteger ( thread, f.ftell () );
                return 1;
            }
        }
    }

    public static final class io_flush implements JavaFunction {

        public int Call ( lua_State thread ) {
            int res = 0;
            try {
                if ( getiofile ( thread, IO_OUTPUT ).fflush () == 0 )
                    res = 1;
            }
            catch ( Exception ex ) {
                res = 0;
            }
            return pushresult ( thread, res, null );
        }
    }

    public static final class f_flush implements JavaFunction {

        public int Call ( lua_State thread ) {
            int res = 0;
            try {
                if ( tofile ( thread ).fflush () == 0 )
                    res = 1;
            }
            catch ( Exception ex ) {
                res = 0;
            }
            return pushresult ( thread, res, null );
        }
    }

    static FileStruct tofile ( lua_State thread ) {
        FileStruct f = ( FileStruct ) LuaAPI.luaL_checkudata ( thread, 1, LuaAPI.LUA_FILEHANDLE );
        if ( f == null || f.isClosed == true )
            LuaAPI.luaL_error ( thread, "attempt to use a closed file" );
        return f;
    }

    public static final class io_readline implements JavaFunction {

        public int Call ( lua_State thread ) {
            FileStruct f = ( FileStruct ) LuaAPI.lua_touserdata ( thread, LuaAPI.lua_upvalueindex ( 1 ) );
            int sucess;
            if ( f == null || f.isClosed == true )  /* file is already closed? */

                LuaAPI.luaL_error ( thread, "file is already closed" );
            sucess = read_line ( thread, f );
            if ( sucess == 1 )
                return 1;
            else /* EOF */ {
                if ( LuaAPI.lua_toboolean ( thread, LuaAPI.lua_upvalueindex ( 2 ) ) ) /* generator created file? */ {
                    LuaAPI.lua_settop ( thread, 0 );
                    LuaAPI.lua_pushvalue ( thread, LuaAPI.lua_upvalueindex ( 1 ) );
                    aux_close ( thread );  /* close it */
                }
                return 0;
            }
        }
    }

    public static final class io_close implements JavaFunction {

        public int Call ( lua_State thread ) {
            if ( LuaAPI.lua_gettop ( thread ) < 2 )
                LuaAPI.lua_rawgeti ( thread, LuaAPI.LUA_ENVIRONINDEX, IO_OUTPUT );
            tofile ( thread );  /* make sure argument is a file */
            return aux_close ( thread );
        }
    }

    public static final class io_gc implements JavaFunction {

        public int Call ( lua_State thread ) {
            FileStruct f = ( FileStruct ) LuaAPI.luaL_checkudata ( thread, 1, LuaAPI.LUA_FILEHANDLE );
            /* ignore closed files */
            if ( f != null )
                aux_close ( thread );
            return 0;
        }
    }

    public static final class io_tostring implements JavaFunction {

        public int Call ( lua_State thread ) {
            FileStruct f = ( FileStruct ) LuaAPI.luaL_checkudata ( thread, 1, LuaAPI.LUA_FILEHANDLE );
            if ( f == null || f.isClosed == true )
                LuaAPI.lua_pushliteral ( thread, "file (closed)" );
            else
                LuaAPI.lua_pushfstring ( thread, "file (" + f.toString () + ")" );
            return 1;
        }
    }

    public static final class io_open implements JavaFunction {

        public int Call ( lua_State thread ) {
            String filename = LuaAPI.luaL_checkstring ( thread, 1 );
            String mode = LuaAPI.luaL_optstring ( thread, 2, "r" );
            FileStruct pf = newfile ( thread );
            pf = fopen ( pf, filename, mode );
            return ( pf == null ) ? pushresult ( thread, 0, filename ) : 1;
        }
    }

    /*
     ** this function has a separated environment, which defines the
     ** correct __close for 'popen' files
     */
    public static final class io_popen implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_error ( thread, "Not implemented yet" );
            return -1;
        /*            String filename = LuaAPI.luaL_checkstring(thread, 1);
        String mode = LuaAPI.luaL_optstring(thread, 2, "r");
        FileStruct pf = newfile(thread);
        //pf = lua_popen(thread, filename, mode);
        return (pf == null) ? pushresult(thread, 0, filename) : 1;
         */
        }
    }

    public static final class io_tmpfile implements JavaFunction {

        public int Call ( lua_State thread ) {
            FileStruct pf = newfile ( thread );
            pf = tmpfile ( thread, pf );
            return ( pf == null ) ? pushresult ( thread, 0, null ) : 1;
        }
    }

    public static final class io_fclose implements JavaFunction {

        public int Call ( lua_State thread ) {
            FileStruct p = ( FileStruct ) LuaAPI.luaL_checkudata ( thread, 1, LuaAPI.LUA_FILEHANDLE );
            int ok = p.fclose () == 0 ? 1 : 0;
            p = null;
            return pushresult ( thread, ok, null );
        }
    }

    public static final class io_noclose implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.lua_pushnil ( thread );
            LuaAPI.lua_pushliteral ( thread, "cannot close standard file" );
            return 2;
        }
    }

    public static final class io_pclose implements JavaFunction {

        public int Call ( lua_State thread ) {
            LuaAPI.luaL_error ( thread, "Not implemented yet" );
            return -1;
        /*            FileStruct p = (FileStruct)LuaAPI.luaL_checkudata(thread, 1, LuaAPI.LUA_FILEHANDLE);
        int ok = 0;//lua_pclose(thread, p);
        p = null;
        return pushresult(thread, ok, null);
         */
        }
    }

    public static final class f_lines implements JavaFunction {

        public int Call ( lua_State thread ) {
            tofile ( thread );  /* check that it's a valid file handle */
            aux_lines ( thread, 1, 0 );
            return 1;
        }
    }

    static void aux_lines ( lua_State thread, int idx, int toclose ) {
        LuaAPI.lua_pushvalue ( thread, idx );
        boolean res;
        if ( toclose == 1 )
            res = true;
        else
            res = false;
        LuaAPI.lua_pushboolean ( thread, res );  /* close/not close file when finished */
        LuaAPI.lua_pushjavafunction ( thread, new io_readline (), 2 );

    }

    public static final class io_lines implements JavaFunction {

        public int Call ( lua_State thread ) {
            if ( LuaAPI.lua_isnoneornil ( thread, 1 ) ) /* no arguments? */ {
                /* will iterate over default input */
                LuaAPI.lua_rawgeti ( thread, LuaAPI.LUA_ENVIRONINDEX, IO_INPUT );
                return new f_lines ().Call ( thread );
            }
            else {
                String filename = LuaAPI.luaL_checkstring ( thread, 1 );
                FileStruct pf = newfile ( thread );
                pf = fopen ( pf, filename, "r" );
                if ( pf == null )
                    fileerror ( thread, 1, filename );
                aux_lines ( thread, LuaAPI.lua_gettop ( thread ), 1 );
                return 1;
            }
        }
    }

    static int pushresult ( lua_State thread, int i, String filename ) {
        int en = 0;//errno;  /* calls to Lua API may change this value */

        if ( i != 0 ) {
            LuaAPI.lua_pushboolean ( thread, true );
            return 1;
        }
        else {
            LuaAPI.lua_pushnil ( thread );
            if ( filename != null )
                LuaAPI.lua_pushfstring ( thread, filename + ": " + "error"/*FIXME: error msg must be correct "%s: %s", filename, strerror(en)*/ );
            else
                LuaAPI.lua_pushfstring ( thread, "error"/*FIXME: error msg must be correct "%s", strerror(en)*/ );
            LuaAPI.lua_pushinteger ( thread, en );
            return 3;
        }
    }

    static void fileerror ( lua_State thread, int arg, String filename ) {
        LuaAPI.lua_pushfstring ( thread, filename + ":" + "error"/*FIXME: error string must be correct "%s: %s", filename, strerror(errno)*/ );
        LuaAPI.luaL_argerror ( thread, arg, LuaAPI.lua_tostring ( thread, -1 ) );
    }

    public static final class io_type implements JavaFunction {

        public int Call ( lua_State thread ) {
            Object ud = null;
            LuaAPI.luaL_checkany ( thread, 1 );
            ud = LuaAPI.lua_touserdata ( thread, 1 );
            LuaAPI.lua_getfield ( thread, LuaAPI.LUA_REGISTRYINDEX, LuaAPI.LUA_FILEHANDLE );
            if ( ud == null ||  ! LuaAPI.lua_getmetatable ( thread, 1 ) ||  ! LuaAPI.lua_rawequal ( thread, -2, -1 ) )
                LuaAPI.lua_pushnil ( thread );  /* not a file */

            else if ( ( ( FileStruct ) ud ).isClosed == true )
                LuaAPI.lua_pushliteral ( thread, "closed file" );
            else
                LuaAPI.lua_pushliteral ( thread, "file" );

            return 1;
        }
    }

    static FileStruct newfile ( lua_State thread ) {
        FileStruct pf = ( FileStruct ) LuaAPI.lua_newuserdata ( thread, new FileStruct (), 0 );
        LuaAPI.luaL_getmetatable ( thread, LuaAPI.LUA_FILEHANDLE );
        LuaAPI.lua_setmetatable ( thread, -2 );
        return pf;
    }

    static FileStruct newfile ( lua_State thread, FileStruct file ) {
        file = ( FileStruct ) LuaAPI.lua_newuserdata ( thread, file, 0 );
        LuaAPI.luaL_getmetatable ( thread, LuaAPI.LUA_FILEHANDLE );
        LuaAPI.lua_setmetatable ( thread, -2 );
        return file;
    }

    static int aux_close ( lua_State thread ) {
        LuaAPI.lua_getfenv ( thread, 1 );
        LuaAPI.lua_getfield ( thread, -1, "__close" );
        return LuaAPI.lua_tojavafunction ( thread, -1 ).Call ( thread );
    }

    public static final class f_setvbuf implements JavaFunction {

        public int Call ( lua_State thread ) {
            final int[] mode = { _IONBF, _IOFBF, _IOLBF };
            final String[] modenames = { "no", "full", "line" };
            FileStruct f = tofile ( thread );
            int op = LuaAPI.luaL_checkoption ( thread, 2, null, modenames );
            int sz = LuaAPI.luaL_optinteger ( thread, 3, LuaAPI.LUAL_BUFFERSIZE );
            int res = f.setvbuf ( mode[op], sz );
            int res2 = 0;
            if ( res == 0 )
                res2 = 1;
            return pushresult ( thread, res2, null );
        }
    }

    static void createmeta ( lua_State thread ) {

        luaL_Reg[] luaReg = new luaL_Reg[] {
            new luaL_Reg ( "close", new io_close () ),
            new luaL_Reg ( "flush", new f_flush () ),
            new luaL_Reg ( "lines", new f_lines () ),
            new luaL_Reg ( "read", new f_read () ),
            new luaL_Reg ( "seek", new f_seek () ),
            new luaL_Reg ( "setvbuf", new f_setvbuf () ),
            new luaL_Reg ( "write", new f_write () ),
            new luaL_Reg ( "__gc", new io_gc () ),
            new luaL_Reg ( "__tostring", new io_tostring () ),
        };

        LuaAPI.luaL_newmetatable ( thread, LuaAPI.LUA_FILEHANDLE );  /* create metatable for file handles */
        LuaAPI.lua_pushvalue ( thread, -1 );  /* push metatable */
        LuaAPI.lua_setfield ( thread, -2, "__index" );  /* metatable.__index = metatable */
        LuaAPI.luaL_register ( thread, null, luaReg );
    }

    static void createstdfile ( lua_State thread, FileStruct f, int k, String fname ) {
        newfile ( thread, f );

        if ( k > 0 ) {
            LuaAPI.lua_pushvalue ( thread, -1 );
            LuaAPI.lua_rawseti ( thread, LuaAPI.LUA_ENVIRONINDEX, k );
        }

        LuaAPI.lua_pushvalue ( thread, -2 );  /* copy environment */
        LuaAPI.lua_setfenv ( thread, -2 );  /* set it */
        LuaAPI.lua_setfield ( thread, -3, fname );
    }

    static void newfenv ( lua_State thread, JavaFunction cls ) {
        LuaAPI.lua_createtable ( thread, 0, 1 );
        LuaAPI.lua_pushjavafunction ( thread, cls );
        LuaAPI.lua_setfield ( thread, -2, "__close" );
    }

    public static final class luaopen_io implements JavaFunction {

        public int Call ( lua_State thread ) {
            createmeta ( thread );

            luaL_Reg[] luaReg = new luaL_Reg[] {
                new luaL_Reg ( "close", new io_close () ),
                new luaL_Reg ( "flush", new io_flush () ),
                new luaL_Reg ( "input", new io_input () ),
                new luaL_Reg ( "lines", new io_lines () ),
                new luaL_Reg ( "open", new io_open () ),
                new luaL_Reg ( "output", new io_output () ),
                new luaL_Reg ( "popen", new io_popen () ),
                new luaL_Reg ( "read", new io_read () ),
                new luaL_Reg ( "tmpfile", new io_tmpfile () ),
                new luaL_Reg ( "type", new io_type () ),
                new luaL_Reg ( "write", new io_write () ),
            };

            /* create (private) environment (with fields IO_INPUT, IO_OUTPUT, __close) */
            newfenv ( thread, new io_fclose () );
            LuaAPI.lua_replace ( thread, LuaAPI.LUA_ENVIRONINDEX );
            /* open library */
            LuaAPI.luaL_register ( thread, LUA_IOLIBNAME, luaReg );
            /* create (and set) default files */
            newfenv ( thread, new io_noclose () );  /* close function for default files */
            createstdfile ( thread, new FileStruct (), IO_INPUT, "stdin" );
            createstdfile ( thread, new FileStruct (), IO_OUTPUT, "stdout" );
            createstdfile ( thread, new FileStruct (), 0, "stderr" );
            LuaAPI.lua_pop ( thread, 1 );  /* pop environment for default files */
            LuaAPI.lua_getfield ( thread, -1, "popen" );
            newfenv ( thread, new io_pclose () );  /* create environment for 'popen' */
            LuaAPI.lua_setfenv ( thread, -2 );  /* set fenv for 'popen' */
            LuaAPI.lua_pop ( thread, 1 );  /* pop 'popen' */
            return 1;
        }
    }

    public int Call ( lua_State thread ) {
        return 0;
    }
}

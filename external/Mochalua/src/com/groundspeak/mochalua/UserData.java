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
class UserData extends Collectable {

    private Table m_MetaTable;
    private Table m_Environment;
    private Object m_UserData;
    private int m_iSize;

    public UserData ( Table metaTable, Table environment, Object userData, int iSize ) {
        super ();

        SetMetaTable ( metaTable );
        SetEnvironment ( environment );
        SetUserData ( userData );
        SetSize ( iSize );
    }

    public final void SetSize ( int iSize ) {
        this.m_iSize = iSize;
    }

    public final int GetSize () {
        return this.m_iSize;
    }

    public final void SetUserData ( Object userData ) {
        this.m_UserData = userData;
    }

    public final Object GetUserData () {
        return this.m_UserData;
    }

    public final void SetMetaTable ( Table metaTable ) {
        this.m_MetaTable = metaTable;
    }

    public final Table GetMetaTable () {
        return this.m_MetaTable;
    }

    public final void SetEnvironment ( Table environment ) {
        this.m_Environment = environment;
    }

    public final Table GetEnvironment () {
        return this.m_Environment;
    }
}

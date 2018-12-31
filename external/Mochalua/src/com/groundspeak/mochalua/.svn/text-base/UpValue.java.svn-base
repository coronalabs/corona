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
class UpValue {

    private Object m_Value;
    private int m_iIndex;
    private lua_State m_Thread;

    public UpValue ( lua_State thread ) {
        this.m_Thread = thread;
    }

    public UpValue ( lua_State thread, int iIndex ) {
        this.m_Thread = thread;
        SetIndex ( iIndex );
    }

    public final boolean IsValueInTheStack () {
        return this.m_Thread != null;
    }

    public final void SetIndex ( int iIndex ) {
        this.m_iIndex = iIndex;
    }

    public final int GetIndex () {
        return this.m_iIndex;
    }

    public final void SetValue ( Object value ) {
        if ( this.m_Thread == null ) {
            m_Value = value;
            Collectable.Increment ( value );
            return;
        }
        m_Thread.SetValue ( m_iIndex, value );
    }

    public final Object GetValue () {
        if ( this.m_Thread == null ) {
            return this.m_Value;
        }

        return m_Thread.GetValue ( m_iIndex );
    }

    public final void Reset () {
        this.m_Thread = null;
        SetIndex ( 0 );
    }
}

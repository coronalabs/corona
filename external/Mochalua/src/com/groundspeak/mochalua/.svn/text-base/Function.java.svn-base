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
class Function extends Collectable {

    private LuaFunction m_LuaFunction;  // LuaFunction

    private JavaFunction m_JavaFunction;    // JavaFunction

    private Table m_Environment;
    private UpValue[] m_UpValues;

    public Function ( LuaFunction luaFunction, Table environment ) {
        super ();

        this.m_Environment = environment;
        this.m_LuaFunction = luaFunction;
        this.m_JavaFunction = null;
        this.m_UpValues = new UpValue[ luaFunction.GetUpValuesQuantity () ];
    }

    public Function ( JavaFunction javaFunction, Table environment, int iUpValuesQuantity ) {
        this.m_Environment = environment;
        this.m_JavaFunction = javaFunction;
        this.m_LuaFunction = null;
        if ( iUpValuesQuantity > 0 ) {
            this.m_UpValues = new UpValue[ iUpValuesQuantity ];
        }
    }

    public boolean IsLuaFunction () {
        return this.m_JavaFunction == null && this.m_LuaFunction != null;
    }

    public boolean IsJavaFunction () {
        return this.m_LuaFunction == null && this.m_JavaFunction != null;
    }

    public final LuaFunction GetLuaFunction () {
        return this.m_LuaFunction;
    }

    public final JavaFunction GetJavaFunction () {
        return this.m_JavaFunction;
    }

    public final void SetEnvironment ( Table environment ) {
        this.m_Environment = environment;
    }

    public final Table GetEnvironment () {
        return this.m_Environment;
    }

    public final UpValue GetUpValue ( int iIndex ) {
        return this.m_UpValues[iIndex];
    }

    public final void SetUpValue ( int iIndex, UpValue value ) {
        this.m_UpValues[iIndex] = value;
    }

    public final int GetUpValuesQuantity () {
        if ( this.m_UpValues == null ) {
            return 0;
        }
        else {
            return m_UpValues.length;
        }
    }
}

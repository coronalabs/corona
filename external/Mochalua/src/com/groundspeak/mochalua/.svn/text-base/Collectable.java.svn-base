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
class Collectable {

    private int m_iUsageCounter;
    private boolean m_bDestroying;

    public Collectable () {
        this.m_iUsageCounter = 0;
        this.m_bDestroying = false;
    }

    public static boolean Increment ( Object object ) {
        if ( object instanceof Collectable ) {
            ( ( Collectable ) object ).IncrementUseage ();
            return true;
        }

        return false;
    }

    // If object has called __gc function then result is true, elsewhy false
    public static boolean Decrement ( lua_State thread, Object object ) {
        if ( object instanceof Collectable ) {
            // Call __gc function if it exists
            Collectable collectable = ( Collectable ) object;
            collectable.DecrementUsage ();
            if ( collectable.GetUsageCounter () == 0 && collectable.GetDestroying () == false ) {
                collectable.SetDestroying ( true );

                if ( object instanceof Function ) {
                    Function function = ( Function ) object;
                    for ( int iUpValue = 0; iUpValue < function.GetUpValuesQuantity (); iUpValue ++ ) {
                        UpValue upValue = function.GetUpValue ( iUpValue );
                        if ( upValue != null && upValue.IsValueInTheStack () == false ) {
                            Collectable.Decrement ( thread, upValue.GetValue () );
                        }
                    }
                }
                else if ( object instanceof Table ) {
                    Table table = ( Table ) object;
                    Object key = table.GetNext ( null );
                    while ( key != null ) {
                        Collectable.Decrement ( thread, key );
                        Collectable.Decrement ( thread, table.GetValue ( key ) );

                        key = table.GetNext ( key );
                    }
                }

                Function function = ( Function ) lua_State.GetMetaTableObjectByObject ( object, LVM.TM_GC );
                if ( function != null ) {
                    thread.CallMetaTable ( function, object );
                    return true;
                }
            }
        }
        return false;
    }

    private final void IncrementUseage () {
        if ( this.m_bDestroying == false ) {
            this.m_iUsageCounter ++;
        }
    }

    private final void DecrementUsage () {
        if ( this.m_bDestroying == false ) {
            this.m_iUsageCounter --;
        }
    }

    private final int GetUsageCounter () {
        return this.m_iUsageCounter;
    }

    private final void SetDestroying ( boolean bDestroying ) {
        this.m_bDestroying = bDestroying;
    }

    private final boolean GetDestroying () {
        return this.m_bDestroying;
    }
}

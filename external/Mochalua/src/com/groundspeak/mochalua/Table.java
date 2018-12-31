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

import java.lang.ref.WeakReference;

/**
 *
 * @author a.fornwald
 */
class Table extends Collectable {

    public class Pair {

        private Object m_Key;
        private Object m_Value;
        private Pair m_NextPair;
        private int m_iHash;
        private Table m_Table;
        // For GetNext
        private Pair m_NextPairForNext;
        private Pair m_PrevPairForNext;

        public Pair ( Table table, int iHash, Object key, Object value, Pair nextPair ) {
            this.m_Table = table;
            this.m_iHash = iHash;
            this.SetKey ( key );
            this.SetValue ( value );
            this.m_NextPair = nextPair;

            PutPairToSequence ();
        }

        /*
         * FOR DEBUG USE ONLY: Don't remove
        private int SearchInSequence( Object key )
        {
        int iCount = 0;
        for( Pair pair = this.m_Table.m_NextSequence; pair != null; pair = pair.GetNextPairForNext() )
        {
        if( pair.KeyEquals( key ) == true )
        {
        iCount++;
        }
        }
        
        if( iCount > 1 )
        {
        throw new RuntimeException( "Error: key is already in the table." );
        }
        
        return iCount;
        }*/
        private final void PutPairToSequence () {
            Object key = GetKey ();
            if ( m_Table.CanBeIndexToArray ( key ) == true ) {
                int iKey = ( ( Double ) key ).intValue ();

                // Check it a new element can be last
                Pair m_LastKeyNumberInSequence = this.m_Table.m_LastKeyNumberInSequence;
                if ( m_LastKeyNumberInSequence != null ) {
                    int iLastKey = ( ( Double ) m_LastKeyNumberInSequence.GetKey () ).intValue ();
                    if ( iKey > iLastKey ) {
                        this.SetNextPairForNext ( m_LastKeyNumberInSequence.GetNextPairForNext () );
                        this.SetPrevPairForNext ( m_LastKeyNumberInSequence );
                        m_LastKeyNumberInSequence.SetNextPairForNext ( this );
                        if ( this.GetNextPairForNext () == null ) {
                            this.m_Table.m_SequenceTail = this;
                        }
                        if ( this.GetPrevPairForNext () == null ) {
                            this.m_Table.m_SequenceHead = this;
                        }
                        this.m_Table.m_LastKeyNumberInSequence = this;
                        return;
                    }
                }

                for ( Pair pair = this.m_Table.m_SequenceHead, prev = null; pair != null; prev = pair, pair = pair.GetNextPairForNext () ) {
                    boolean bCanBeIndexToArray = CanBeIndexToArray ( pair.GetKey () );
                    if ( ( bCanBeIndexToArray == true && ( ( Double ) pair.GetKey () ).intValue () > iKey ) ||
                        bCanBeIndexToArray == false ) {
                        if ( prev == null ) {
                            m_Table.m_SequenceHead = this;
                            this.SetNextPairForNext ( pair );
                            pair.SetPrevPairForNext ( this );
                        }
                        else {
                            pair.SetPrevPairForNext ( this );
                            prev.SetNextPairForNext ( this );
                            this.SetNextPairForNext ( pair );
                            this.SetPrevPairForNext ( prev );
                        }

                        if ( bCanBeIndexToArray == false ) // is this number is last
                        {
                            this.m_Table.m_LastKeyNumberInSequence = this;
                        }
                        return;
                    }
                }

                this.m_Table.m_LastKeyNumberInSequence = this;
            }

            if ( m_Table.m_SequenceTail != null ) {
                m_Table.m_SequenceTail.SetNextPairForNext ( this );
                this.SetPrevPairForNext ( m_Table.m_SequenceTail );
            }
            else {
                this.SetPrevPairForNext ( null );
                m_Table.m_SequenceHead = this;
            }

            this.SetNextPairForNext ( null );
            m_Table.m_SequenceTail = this;
        }

        public final void SetNextPairForNext ( Pair nextPairForNext ) {
            this.m_NextPairForNext = nextPairForNext;
        }

        public final Pair GetNextPairForNext () {
            return this.m_NextPairForNext;
        }

        public final void SetPrevPairForNext ( Pair prevPairForNext ) {
            this.m_PrevPairForNext = prevPairForNext;
        }

        public final Pair GetPrevPairForNext () {
            return this.m_PrevPairForNext;
        }

        public final boolean KeyEquals ( Object key ) {
            final int iHash = key.hashCode ();
            if ( GetHash () == iHash && GetKey ().equals ( key ) ) {
                return true;
            }

            return false;
        }

        private final int GetHash () {
            return this.m_iHash;
        }

        private final void SetKey ( Object key ) {
            if ( m_Table.GetIsWeakKeysMode () == true ) {
                key = Refer ( key );
            }
            this.m_Key = key;
        }

        private final Object GetKey () {
            Object key = m_Key;

            if ( m_Table.GetIsWeakKeysMode () == true ) {
                key = UnRefer ( key );
            }

            return key;
        }

        private final void SetValue ( Object value ) {
            if ( m_Table.GetIsWeakValuesMode () == true ) {
                value = Refer ( value );
            }
            this.m_Value = value;
        }

        private final Object GetValue () {
            Object value = m_Value;

            if ( m_Table.GetIsWeakValuesMode () == true ) {
                value = UnRefer ( value );
            }

            return value;
        }

        public final void SetNextPair ( Pair pair ) {
            this.m_NextPair = pair;
        }

        public final Pair GetNextPair () {
            return this.m_NextPair;
        }

        private final Object Refer ( Object object ) {
            if ( CanBeWeakReference ( object ) == false ) {
                return object;
            }

            return new WeakReference ( object );
        }

        private final Object UnRefer ( Object object ) {
            if ( CanBeWeakReference ( object ) == false ) {
                return object;
            }

            return ( ( WeakReference ) object ).get ();
        }
    }
    private int m_iArraySize;
    private int m_iHashSize;
    private Pair m_Pairs[];
    private boolean m_bIsWeakKeysMode;
    private boolean m_bIsWeakValuesMode;
    private Table m_MetaTable;
    private int m_iCount;
    private int m_iThreshold;
    // Sequence
    private Pair m_SequenceHead;
    private Pair m_SequenceTail;
    // Acceleration vars
    private Pair m_LastKeyNumberInSequence;
    // Change this value to get faster tables
    private static final float LOAD_FACTOR = 0.75f;

    public Table ( int iArraySize, int iHashSize ) {
        this.m_iArraySize = iArraySize;
        this.m_iHashSize = iHashSize;

        this.m_bIsWeakKeysMode = false;
        this.m_bIsWeakValuesMode = false;

        if ( iArraySize + iHashSize == 0 ) {
            iHashSize = 1;
        }

        int iInitialSize = iArraySize + iHashSize;

        this.m_Pairs = new Pair[ iInitialSize ];

        this.m_iThreshold = ( int ) ( iInitialSize * LOAD_FACTOR );

        this.m_SequenceHead = null;
        this.m_SequenceTail = null;

        this.m_iCount = 0;
    }

    public final boolean GetIsWeakKeysMode () {
        return this.m_bIsWeakKeysMode;
    }

    public final boolean GetIsWeakValuesMode () {
        return this.m_bIsWeakValuesMode;
    }

    private void UpdateWeakness ( boolean bIsWeakKeysMode, boolean bIsWeakValuesMode ) {
        CollectGarbage ();

        for ( int iIndex = this.m_Pairs.length; iIndex -- > 0;) {
            for ( Pair pair = this.m_Pairs[iIndex], prev = null; pair != null; prev = pair, pair = pair.GetNextPair () ) {
                if ( m_bIsWeakKeysMode != bIsWeakKeysMode ) {
                    if ( GetIsWeakKeysMode () == true ) {
                        pair.m_Key = pair.UnRefer ( pair.m_Key );
                    }
                    else {
                        pair.m_Key = pair.Refer ( pair.m_Key );
                    }
                }

                if ( m_bIsWeakValuesMode != bIsWeakValuesMode ) {
                    if ( GetIsWeakValuesMode () == true ) {
                        pair.m_Value = pair.UnRefer ( pair.m_Value );
                    }
                    else {
                        pair.m_Value = pair.Refer ( pair.m_Value );
                    }
                }
            }
        }

        this.m_bIsWeakKeysMode = bIsWeakKeysMode;
        this.m_bIsWeakValuesMode = bIsWeakValuesMode;

        CollectGarbage ();
    }

    public final int GetArraySize () {
        return this.m_iArraySize;
    }

    private final void SetArraySize ( int iNewArraySize ) {
        this.m_iArraySize = iNewArraySize;
    }

    public final int GetHashSize () {
        return this.m_iHashSize;
    }

    private final void SetHashSize ( int iNewHashSize ) {
        this.m_iHashSize = iNewHashSize;
    }

    public final boolean CanBeWeakReference ( Object object ) {
        return  ! ( object == null ||
            object instanceof String ||
            object instanceof Double ||
            object instanceof Boolean );
    }

    public final void IsVaildKey ( Object key ) {
        if ( key == null ) {
            throw new LuaRuntimeException ( "table index is nil" );
        }
        if ( key instanceof Double && ( ( Double ) key ).isNaN () == true ) {
            throw new LuaRuntimeException ( "table index is NaN" );
        }
    }

    public final Pair GetPair ( Object key ) {
        CollectGarbage ();

        try {
            IsVaildKey ( key );
        }
        catch ( LuaRuntimeException ex ) {
            return null;
        }

        final int iHash = key.hashCode ();
        final int iIndex = ( iHash & 0x7FFFFFFF ) % m_Pairs.length;
        for ( Pair pair = this.m_Pairs[iIndex]; pair != null; pair = pair.GetNextPair () ) {
            if ( pair.KeyEquals ( key ) == true ) {
                return pair;
            }
        }
        return null;
    }

    public final Object GetValue ( Object key ) {
        Pair pair = GetPair ( key );
        if ( pair != null ) {
            return pair.GetValue ();
        }
        return null;
    }

    public final void Remove ( Object key ) {
        int iHash = key.hashCode ();
        int iIndex = ( iHash & 0x7FFFFFFF ) % m_Pairs.length;
        for ( Pair pair = this.m_Pairs[iIndex], prev = null; pair != null; prev = pair, pair = pair.GetNextPair () ) {
            if ( pair.KeyEquals ( key ) == true ) {
                // this.m_Modifications++;
                RemoveFromSequence ( pair );

                if ( prev != null ) {
                    prev.SetNextPair ( pair.GetNextPair () );
                }
                else {
                    this.m_Pairs[iIndex] = pair.GetNextPair ();
                }
                this.m_iCount --;
                return;
            }
        }
    }

    public final void SetValue ( Object key, Object value, lua_State thread ) {
        IsVaildKey ( key );

        int iHash = key.hashCode ();
        int iIndex = ( iHash & 0x7FFFFFFF ) % this.m_Pairs.length;

        for ( Pair pair = this.m_Pairs[iIndex], prev = null; pair != null; prev = pair, pair = pair.GetNextPair () ) {
            if ( pair.KeyEquals ( key ) == true ) {
                Collectable.Increment ( value );

                Collectable.Decrement ( thread, pair.GetValue () );

                if ( pair.GetValue () == null ) {
                    if ( value != null ) {
                        pair.SetValue ( value );
                        pair.PutPairToSequence ();
                    }
                    return;
                }
                pair.SetValue ( value );

                if ( value == null ) {
                    Collectable.Decrement ( thread, pair.GetKey () );
                    Collectable.Decrement ( thread, pair.GetValue () );
                    RemoveFromSequence ( pair );
                }
                return;
            }
        }

        // If it's a new key, just don't add it if it's null. But I'm not sure.
        if ( value == null ) {
            return;
        }

        //this.m_Modifications++;    
        if ( this.m_iCount >= this.m_iThreshold ) {
            Rehash ();

            iIndex = ( iHash & 0x7FFFFFFF ) % this.m_Pairs.length;
        }


        Pair pair = this.m_Pairs[iIndex];

        Collectable.Increment ( key );
        Collectable.Increment ( value );
        this.m_Pairs[iIndex] = new Pair ( this, iHash, key, value, pair );

        m_iCount ++;
    }

    public final Object GetValueNum ( int iKey ) {
        return GetValue ( new Double ( iKey ) );
    }

    public final void SetValueNum ( int iKey, Object value, lua_State thread ) {
        SetValue ( new Double ( iKey ), value, thread );
    }

    private final void CollectGarbage () {
        if ( GetIsWeakKeysMode () == false && GetIsWeakValuesMode () == false ) {
            return;
        }

        for ( int iIndex = this.m_Pairs.length; iIndex -- > 0;) {
            for ( Pair pair = this.m_Pairs[iIndex], prev = null; pair != null; pair = pair.GetNextPair () ) {
                if ( pair.GetKey () == null || pair.GetValue () == null ) {
                    if ( GetIsWeakKeysMode () == true ) {
                        RemoveFromSequence ( pair );
                    }

                    if ( prev != null ) {
                        prev.SetNextPair ( pair.GetNextPair () );
                        prev = pair;
                    }
                    else {
                        this.m_Pairs[iIndex] = pair.GetNextPair ();
                        prev = null;
                    }
                    this.m_iCount --;
                }
            }
        }
    }

    public final void Rehash () {
        int iOldSize = this.m_Pairs.length;
        Pair oldPairs[] = this.m_Pairs;

        int iNewSize = iOldSize * 2 + 1;
        this.m_Pairs = new Pair[ iNewSize ];

        //this.m_iModifications++;
        this.m_iThreshold = ( int ) ( iNewSize * LOAD_FACTOR );

        for ( int i = iOldSize; i -- > 0;) {
            for ( Pair old = oldPairs[i]; old != null;) {
                Pair pair = old;
                old = old.GetNextPair ();

                int iIndex = ( pair.GetHash () & 0x7FFFFFFF ) % iNewSize;
                pair.SetNextPair ( this.m_Pairs[iIndex] );
                this.m_Pairs[iIndex] = pair;
            }
        }
    }

    public final boolean CanBeIndexToArray ( Object key ) {
        if ( key instanceof Double ) {
            int iIndex = ( ( Double ) key ).intValue ();
            if ( ( double ) iIndex == ( ( Double ) key ).doubleValue () && iIndex > 0 && iIndex <= this.m_iArraySize ) {
                return true;
            }
        }
        return false;
    }

    public final void RemoveFromSequence ( Pair pair ) {
        if ( m_LastKeyNumberInSequence == pair ) {
            m_LastKeyNumberInSequence = pair.m_PrevPairForNext;
        }

        Pair prev = pair.GetPrevPairForNext ();
        Pair next = pair.GetNextPairForNext ();
        if ( prev != null ) {
            prev.SetNextPairForNext ( next );
        }
        else {
            this.m_SequenceHead = pair.GetNextPairForNext ();
        }

        if ( next != null ) {
            next.SetPrevPairForNext ( prev );
        }
        else {
            this.m_SequenceTail = prev;
        }
    }

    public final Object GetNext ( Object key ) {
        if ( key == null ) {
            if ( this.m_SequenceHead != null ) {
                return this.m_SequenceHead.GetKey ();
            }
            else {
                return null;
            }
        }

        Pair pair = GetPair ( key );
        if ( pair != null && pair.GetNextPairForNext () != null ) {
            return pair.GetNextPairForNext ().GetKey ();
        }

        return null;
    }

    public final void ResizeArray ( int iNewArraySize ) {
        SetArraySize ( iNewArraySize );
    }

    public final Table GetMetaTable () {
        return this.m_MetaTable;
    }

    public final int GetBoundary () {
        int iMaxIndex = 0;
        Object key = GetNext ( null );

        while ( key != null ) {
            if ( key instanceof Double ) {
                int iIndex = ( ( Double ) key ).intValue ();
                if ( ( double ) iIndex == ( ( Double ) key ).doubleValue () && iIndex > 0 ) {
                    iMaxIndex = iIndex;
                }
            }
            key = GetNext ( key );
        }

        return iMaxIndex;
    }

    public final void SetMetaTable ( Table newMetaTable ) {
        this.m_MetaTable = newMetaTable;

        if ( newMetaTable == null ) {
            return;
        }

        String mode = ( String ) newMetaTable.GetValue ( "__mode" );
        if ( mode != null ) {
            UpdateWeakness ( mode.indexOf ( 'k' ) != -1, mode.indexOf ( 'v' ) != -1 );
        }
    }
}

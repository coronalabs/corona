//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_Profiling.h"
#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Time.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void Profiling::Entry::SetShortName( ShortName out, const char* name )
{
    strncpy( out, name, sizeof( ShortName ) - 1 );
}

void Profiling::Entry::SetName( const char* name )
{
    SetShortName( fName, name );
}

Profiling::Profiling( Rtt_Allocator* allocator, const char* name )
:   fNext(NULL),
    fBelow(NULL)
{
    fArray1 = Rtt_NEW( allocator, Array<Entry>( allocator ) );
    fArray2 = Rtt_NEW( allocator, Array<Entry>( allocator ) );

    Entry::SetShortName( fName, name );
}

Profiling::~Profiling()
{
    Rtt_DELETE( fArray1 );
    Rtt_DELETE( fArray2 );
}

void Profiling::Commit()
{
    Array<Entry> * temp = fArray2;

    fArray2 = fArray1;
    fArray1 = temp;

    fArray1->Clear();

    if ( fBelow )
    {
        fBelow->AddEntry( fName, true );
    }

    fTopList = fBelow;
    fBelow = NULL;
}

void Profiling::Push()
{
    fBelow = fTopList;
    fTopList = this;
}

void Profiling::AddEntry( const char* name, bool isListName )
{
    Entry entry;

    entry.SetName( name );

    entry.fTime = !isListName ? Rtt_GetAbsoluteTime() : Entry::SublistTime();

    fArray1->Append( entry );
}

int Profiling::VisitEntries( lua_State* L ) const
{
    if ( lua_istable( L, 1 ) )
    {
        int index = 1;
        const Entry* entries = fArray2->ReadAccess();
        S32 length = fArray2->Length();

        for ( S32 i = 0; i < length; ++i, index += 2 )
        {
            lua_pushstring( L, entries[i].fName );
            lua_rawseti( L, 1, index );

            if ( Entry::SublistTime() != entries[i].fTime )
            {
                lua_pushnumber( L, Rtt_AbsoluteToMicroseconds( entries[i].fTime - entries[0].fTime ) );
            }

            else
            {
                lua_pushboolean( L, 0 );
            }

            lua_rawseti( L, 1, index + 1 );
        }

        lua_pushinteger( L, length * 2 );
    }

    else
    {
        lua_pushinteger( L, 0 );
    }

    return 1;
}

Profiling* Profiling::Open( Rtt_Allocator* allocator, const char* name )
{
	Profiling* profiling = Profiling::GetOrCreate( allocator, name );

    Rtt_ASSERT( profiling );

    if ( !Profiling::Find( profiling ) )
    {
        profiling->Push();
	
	    return profiling;
    }
    
    else
    {
        return NULL;
    }
}

void Profiling::AddEntry( void* profiling, const char* name )
{
    if ( NULL != profiling && fTopList == profiling )
    {
        fTopList->AddEntry( name );
    }
}

void Profiling::Close( void* profiling )
{
    if ( NULL != profiling && fTopList == profiling )
    {
        fTopList->Commit();
    }
}

bool Profiling::Find( const Profiling* profiling )
{
    for ( const Profiling* cur = fTopList; cur; cur = cur->fBelow )
    {
        if ( profiling == cur )
        {
            return true;
        }
    }

    return false;
}

int Profiling::DestroyAll( struct lua_State* L )
{
    Profiling* cur = fFirstList;

    while ( cur )
    {
		Profiling* next = cur->fNext;

        Rtt_DELETE( cur );

        cur = next;
    }

    fFirstList = NULL;
    fTopList = NULL;

    // n.b. leave sums list intact

    return 0;
}

Profiling* Profiling::GetOrCreate( Rtt_Allocator* allocator, const char* name )
{
	Profiling* profiling = Get( name );

    if ( NULL == profiling )
    {
        Entry::ShortName shorter;

        Entry::SetShortName( shorter, name );

        profiling = Get( shorter );

        if ( NULL != profiling )
        {
            Rtt_Log( "Found profiling list %s, but with shorter name %s", name, shorter );

            return profiling;
        }

        profiling = Rtt_NEW( allocator, Profiling( allocator, name ) );

        if ( 0 != Rtt_StringCompare( name, profiling->fName ) )
        {
            Rtt_Log( "Profiling list created, but name %s shortened to %s", name, shorter );
        }

        profiling->fNext = fFirstList;
        fFirstList = profiling;
    }

    return profiling;
}

Profiling* Profiling::Get( const char* name )
{
    for ( Profiling* cur = fFirstList; cur; cur = cur->fNext )
    {
        if ( 0 == Rtt_StringCompare( name, cur->fName ) )
        {
            return cur;
        }
    }

    return NULL;
}

void Profiling::ResetSums()
{
    for ( Sum* sum = fFirstSum; sum; sum = sum->fNext )
    {
        sum->Reset();
    }
}

int Profiling::VisitSums( lua_State* L)
{
    if ( lua_istable( L, 1 ) )
    {
        int index = 1;

        for ( const Sum* sum = fFirstSum; sum; sum = sum->fNext )
        {
            if ( 0 == sum->fTimingCount )
            {
                continue;
            }

            lua_pushstring( L, sum->fName );
            lua_rawseti( L, 1, index );
            lua_pushinteger( L, Rtt_AbsoluteToMicroseconds( sum->fTotalTime ) );
            lua_rawseti( L, 1, index + 1 );
            lua_pushinteger( L, sum->fTimingCount );
            lua_rawseti( L, 1, index + 2 );

            index += 3;
        }

        lua_pushinteger( L, index - 1 );
    }

    else
    {
        lua_pushinteger( L, 0 );
    }
    
    return 1;
}

Profiling* Profiling::fFirstList;
Profiling* Profiling::fTopList;
Profiling::Sum* Profiling::fFirstSum;

Profiling::EntryRAII::EntryRAII( Rtt_Allocator* allocator, const char* name )
{
    fProfiling = Profiling::GetOrCreate( allocator, name );

    Rtt_ASSERT( fProfiling );

    if ( !Profiling::Find( fProfiling ) )
    {
        fProfiling->Push();
    }

    else
    {
        Rtt_ASSERT_NOT_REACHED(); // possible, but indicates misuse
    }
}

Profiling::EntryRAII::~EntryRAII()
{
    Rtt_ASSERT( fProfiling );

    fProfiling->Commit();
}

void Profiling::EntryRAII::Add( const char* name ) const
{
    fProfiling->AddEntry( name );
}

Profiling::Sum::Sum( const char* name )
{
    Entry::SetShortName( fName, name );

    fNext = fFirstSum;
    fFirstSum = this;
}
			
void Profiling::Sum::AddTiming( U64 diff )
{
    fTotalTime += diff;

    ++fTimingCount;
}

bool Profiling::Sum::Acquire()
{
    if ( fEnabled )
    {
        bool acquired = 0 == fRefCount;

        ++fRefCount;

        return acquired;
    }

    else
    {
        return false;
    }
}

bool Profiling::Sum::Release()
{
    if ( fEnabled )
    {
        Rtt_ASSERT( fRefCount > 0 );

        --fRefCount;

        return 0 == fRefCount;
    }

    else
    {
        return false;
    }
}
			
void Profiling::Sum::EnableSums( bool newValue )
{
    fEnabled = newValue;
}
			
void Profiling::Sum::Reset()
{
    Rtt_ASSERT( 0 == fRefCount );

    fTotalTime = 0;
    fTimingCount = 0;
}
			
bool Profiling::Sum::fEnabled;

Profiling::SumRAII::SumRAII( Sum& sum )
:   fSum( sum )
{
    if ( fSum.Acquire() )
    {
        fBegan = Rtt_GetAbsoluteTime();
    }
}

Profiling::SumRAII::~SumRAII()
{
    if ( fSum.Release() )
    {
        fSum.AddTiming( Rtt_GetAbsoluteTime() - fBegan );
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

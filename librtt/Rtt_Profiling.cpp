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

    fTop = fBelow;
    fBelow = NULL;
}

void Profiling::Push()
{
    fBelow = fTop;
    fTop = this;
}

void Profiling::AddEntry( const char* name, bool isListName )
{
    Entry entry;

    entry.SetName( name );

    entry.fTime = !isListName ? Rtt_GetAbsoluteTime() : Entry::SublistTime();

    fArray1->Append( entry );
}

int Profiling::Visit( lua_State* L ) const
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

        return 1;
    }

    else
    {
        return 0;
    }
}

Profiling* Profiling::Open( Rtt_Allocator* allocator, const char* name )
{
	Profiling* profiling = Profiling::GetOrCreate( allocator, name );

    Rtt_ASSERT( profiling );

    if ( Profiling::Find( profiling ) )
    {
        CoronaLog( "List %s is already in use!", name ); // TODO: use some other logging...

        return NULL;
    }

    profiling->Push();
}

void Profiling::AddEntry( void* list, const char* name )
{
    if ( NULL != list )
    {
        if ( fTop == list )
        {
            fTop->AddEntry( name );
        }

        else
        {
            CoronaLog( "List %s is invalid or not active", name ); // TODO?
        }
    }
}

void Profiling::Close( void* list )
{
    if ( NULL != list )
    {
        if ( fTop == list )
        {
            fTop->Commit();
        }

        else
        {
            CoronaLog( "List is invalid or not active" ); // TODO?
        }
    }
}

bool Profiling::Find( const Profiling* profiling )
{
    for ( const Profiling* cur = fTop; cur; cur = cur->fBelow )
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
    Profiling* cur = fFirst;

    while ( cur )
    {
		Profiling* next = cur->fNext;

        Rtt_DELETE( cur );

        cur = next;
    }

    fFirst = fTop = NULL;

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
            CoronaLog( "Found list %s, but with shorter name %s", name, shorter ); // TODO

            return profiling;
        }

        profiling = Rtt_NEW( allocator, Profiling( allocator, name ) );

        if ( 0 != Rtt_StringCompare( name, profiling->fName ) )
        {
            CoronaLog( "List created, but name %s shortened to %s", name, shorter ); // TODO
        }

        profiling->fNext = fFirst;
        fFirst = profiling;
    }

    return profiling;
}

Profiling* Profiling::Get( const char* name )
{
    for ( Profiling* cur = fFirst; cur; cur = cur->fNext )
    {
        if ( 0 == Rtt_StringCompare( name, cur->fName ) )
        {
            return cur;
        }
    }

    return NULL;
}

Profiling* Profiling::fFirst;
Profiling* Profiling::fTop;

Profiling::RAII::RAII( Rtt_Allocator* allocator, const char* name )
{
    fProfiling = Profiling::GetOrCreate( allocator, name );

    Rtt_ASSERT( fProfiling );

    if ( Profiling::Find( fProfiling ) )
    {
        CoronaLog( "List %s is already in use!", name ); // TODO
    }

    fProfiling->Push();
}

Profiling::RAII::~RAII()
{
    Rtt_ASSERT( fProfiling );

    fProfiling->Commit();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

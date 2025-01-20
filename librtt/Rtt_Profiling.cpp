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

void Profiling::Payload::SetString( const char* str )
{
	fValue = str;
	fType = kString;
}

void Profiling::Payload::SetPointer( const void* ptr, bool isTable )
{
	fValue = ptr;
	fType = isTable ? kTable : kFunction;
}

const char* Profiling::Payload::GetString() const
{
    if ( kString == fType )
    {
        return static_cast<const char*>( fValue );
    }

    else
    {
        return NULL;
    }
}

const void* Profiling::Payload::GetPointer() const
{
    if ( kTable == fType || kFunction == fType )
    {
        return fValue;
    }

    else
    {
        return NULL;
    }
}

Profiling::Profiling( Rtt_Allocator* allocator, const char* name )
:   fBelow( NULL )
{
    fArray1 = Rtt_NEW( allocator, Array<Entry>( allocator ) );
    fArray2 = Rtt_NEW( allocator, Array<Entry>( allocator ) );

    fName.SetString( name );
}

Profiling::~Profiling()
{
    Rtt_DELETE( fArray1 );
    Rtt_DELETE( fArray2 );
}

void Profiling::Commit( ProfilingState& state )
{
    Array<Entry> * temp = fArray2;

    fArray2 = fArray1;
    fArray1 = temp;

    fArray1->Clear();

    if ( fBelow )
    {
        fBelow->AddEntry( fName.GetString(), true );
    }

    state.SetTopOfList( fBelow );

    fBelow = NULL;
}

void Profiling::Push( ProfilingState& state )
{
    fBelow = state.GetTopOfList();
    
    state.SetTopOfList( this );
}

void Profiling::AddEntry( const Payload& payload, bool isListName )
{
    Entry entry;

    entry.fPayload = payload;
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
        int curType = -1;

        for ( S32 i = 0; i < length; ++i, index += 2 )
        {
            const Payload& payload = entries[i].fPayload;
            const void* ptr = payload.GetPointer();

            if ( ptr )
            {
                int type = payload.HasTablePointer();

                if ( curType != type )
                {
                    curType = type;

                    if ( payload.HasTablePointer() )
                    {
                        lua_pushliteral( L, "@tableListeners" );
                    }
                    else
                    {
                        lua_pushliteral( L, "@functionListeners" );
                    }

                    lua_rawseti( L, 1, index );
                    lua_pushboolean( L, 0 );
                    lua_rawseti( L, 1, index + 1 );

                    index += 2;
                }

                lua_pushlightuserdata( L, (void*)ptr );
            }

            else
            {
                const char* str = payload.GetString();

                Rtt_ASSERT( str );

                lua_pushstring( L, str );
            }

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

        lua_pushinteger( L, index - 1 );
    }

    else
    {
        lua_pushinteger( L, 0 );
    }

    return 1;
}

void Profiling::ResetSums()
{
    for ( Sum* sum = sFirstSum; sum; sum = sum->fNext )
    {
        sum->Reset();
    }
}

int Profiling::VisitSums( lua_State* L)
{
    if ( lua_istable( L, 1 ) )
    {
        int index = 1;

        for ( const Sum* sum = sFirstSum; sum; sum = sum->fNext )
        {
            if ( 0 == sum->fTimingCount )
            {
                continue;
            }

            lua_pushstring( L, sum->fName.GetString()/*fName*/ );
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

Profiling::Sum* Profiling::sFirstSum;

Profiling::Sum::Sum( const char* name )
{
    fName.SetString( name );

    fNext = sFirstSum;
    sFirstSum = this;
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

ProfilingState::ProfilingState( Rtt_Allocator* allocator )
:   fLists( allocator ),
    fTopList( NULL )
{
	fUpdateID = Create( "update" );
	fRenderID = Create( "render" );
}

bool ProfilingState::Find( const Profiling* profiling )
{
    for ( const Profiling* cur = fTopList; cur; cur = cur->GetBelow() )
    {
        if ( profiling == cur )
        {
            return true;
        }
    }

    return false;
}

int ProfilingState::Create( const char* name )
{
    Profiling* profiling = Rtt_NEW( fLists.Allocator(), Profiling( fLists.Allocator(), name ) );

    fLists.Append( profiling );

    return fLists.Length() - 1;
}

Profiling* ProfilingState::GetByID( int id )
{
    if ( id >= 0 && id < fLists.Length() )
    {
        return fLists[id];
    }

    else
    {
        return NULL;
    }
}

Profiling* ProfilingState::Get( const char* name )
{
    for ( S32 i = 0, length = fLists.Length(); i < length; i++ )
    {
        Profiling* cur = fLists[i];

        if ( 0 == Rtt_StringCompare( name, cur->GetName() ) && name )
        {
            return cur;
        }
    }

    return NULL;
}

Profiling* ProfilingState::Open( int id )
{
	Profiling* profiling = GetByID( id - 1 );

    if ( NULL == profiling )
    {
        return NULL;
    }

    if ( !Find( profiling ) )
    {
        profiling->Push( *this );
	
	    return profiling;
    }
    
    else
    {
        return NULL;
    }
}

void ProfilingState::AddEntry( void* profiling, const Profiling::Payload& payload )
{
    if ( NULL != profiling && fTopList == profiling )
    {
        fTopList->AddEntry( payload );
    }
}

void ProfilingState::Close( void* profiling )
{
    if ( NULL != profiling && fTopList == profiling )
    {
        fTopList->Commit( *this );
    }
}

void ProfilingState::Bookmark( short mark, int push, void* ud )
{
    Rtt_ASSERT( ud );

    ProfilingState* state = static_cast< ProfilingState* >( ud );

    int id = (int)mark;

    if ( push )
    {
        state->Open( id );
    }

    else
    {
        Profiling* profiling = state->GetByID( id - 1 );

        if ( profiling )
        {
            state->Close( profiling );
        }
    }
}

ProfilingEntryRAII::ProfilingEntryRAII( ProfilingState& state, int id )
:   fState( state )
{
    fProfiling = state.GetByID( id );

    Rtt_ASSERT( fProfiling );

    if ( !state.Find( fProfiling ) )
    {
        fProfiling->Push( state );
    }

    else
    {
        Rtt_ASSERT_NOT_REACHED(); // possible, but indicates misuse
    }
}

ProfilingEntryRAII::~ProfilingEntryRAII()
{
    Rtt_ASSERT( fProfiling );

    fProfiling->Commit( fState );
}

void ProfilingEntryRAII::Add( const Profiling::Payload& payload ) const
{
    fProfiling->AddEntry( payload );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

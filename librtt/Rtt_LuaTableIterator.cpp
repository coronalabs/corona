//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaTableIterator.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

LuaTableIterator::LuaTableIterator( lua_State *L, int index )
:	fL( L ),
	fIndex( index ),
	fTop( lua_gettop( L ) ),
	fPreviousValueOnStack( false )
{
	// Cannot accept negative indices b/c we'll be pushing/popping values off the stack,
	// in other words, we need a stable index that points to the same table on the stack
	Rtt_ASSERT( index > 0 );
	Rtt_ASSERT( lua_istable( L, index ) );

	lua_pushnil( L ); // first key
}

LuaTableIterator::~LuaTableIterator()
{
	lua_settop( fL, fTop );
}

bool
LuaTableIterator::Next() const
{
	PopPreviousValue();

	Rtt_ASSERT( ! fPreviousValueOnStack );

	// If successful, lua_next pushes 'key' (at index -2) and 'value' (at index -1)
	bool result = ( 0 != lua_next( fL, fIndex ) );
	fPreviousValueOnStack = result;
	return result;
}

const char*
LuaTableIterator::Key() const
{
	return GetString( -2 );
}

const char*
LuaTableIterator::Value() const
{
	return GetString( -1 );
}

const char*
LuaTableIterator::GetString( int index ) const
{
	lua_State *L = fL;

	const char *result = NULL;
	if ( Rtt_VERIFY( lua_isstring( L, index ) ) )
	{
		result = lua_tostring( L, index );
	}
	return result;
}

void
LuaTableIterator::PopPreviousValue() const
{
	if ( fPreviousValueOnStack )
	{
		// Pop previous value; keep 'key' for next iteration
		lua_pop( fL, 1 );
		fPreviousValueOnStack = false;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaTableIterator_H__
#define _Rtt_LuaTableIterator_H__

#include "Rtt_MKeyValueIterable.h"

#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class LuaTableIterator : public MKeyValueIterable
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		LuaTableIterator( lua_State *L, int index );
		~LuaTableIterator();

	public:
		virtual bool Next() const;
		virtual const char* Key() const;
		virtual const char* Value() const;

	private:
		const char* GetString( int index ) const;
		void PopPreviousValue() const;

	private:
		lua_State *fL;
		int fIndex;
		int fTop;
		mutable bool fPreviousValueOnStack;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaTableIterator_H__

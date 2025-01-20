//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_PaintAdapter__
#define __Rtt_PaintAdapter__

#include "Rtt_LuaUserdataProxy.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class PaintAdapter : public MLuaUserdataAdapter
{
	public:
		static const PaintAdapter& Constant();

	protected:
		PaintAdapter();

	public:
		virtual int ValueForKey(
			const LuaUserdataProxy& sender,
			lua_State *L,
			const char *key ) const;

		virtual bool SetValueForKey(
			LuaUserdataProxy& sender,
			lua_State *L,
			const char *key,
			int valueIndex ) const;

		virtual void WillFinalize( LuaUserdataProxy& sender ) const;

        // Return the hash table containing the adapter's property keys
        virtual StringHash *GetHash( lua_State *L ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_PaintAdapter__

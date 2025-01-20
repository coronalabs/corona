//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_ShapeAdapterCircle__
#define __Rtt_ShapeAdapterCircle__

#include "Display/Rtt_ShapeAdapter.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class ShapeAdapterCircle : public ShapeAdapter
{
	public:
		typedef ShapeAdapter Super;

	public:
		static const ShapeAdapterCircle& Constant();

	protected:
		ShapeAdapterCircle();

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

        // Return the hash table containing the adapter's property keys
        virtual StringHash *GetHash( lua_State *L ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ShapeAdapterCircle__

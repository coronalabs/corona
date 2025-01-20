//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_ShapeAdapterPolygon__
#define __Rtt_ShapeAdapterPolygon__

#include "Display/Rtt_ShapeAdapter.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class TesselatorPolygon;

// ----------------------------------------------------------------------------

class ShapeAdapterPolygon : public ShapeAdapter
{
	public:
		typedef ShapeAdapter Super;

	public:
		static const ShapeAdapterPolygon& Constant();

		static bool InitializeContour(
			lua_State *L, int index, TesselatorPolygon& tesselator, bool hasZ );

	protected:
		ShapeAdapterPolygon();

	public:
// No properties (except inherited ones), so disabling for now.
#if 0
		virtual int ValueForKey(
			const LuaUserdataProxy& sender,
			lua_State *L,
			const char *key ) const;

		virtual bool SetValueForKey(
			LuaUserdataProxy& sender,
			lua_State *L,
			const char *key,
			int valueIndex ) const;
#endif

        virtual StringHash *GetHash( lua_State *L ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ShapeAdapterPolygon__

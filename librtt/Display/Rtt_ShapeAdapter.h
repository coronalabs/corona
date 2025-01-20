//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_ShapeAdapter__
#define __Rtt_ShapeAdapter__

#include "Rtt_LuaUserdataProxy.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ShapeObject;

// ----------------------------------------------------------------------------

class ShapeAdapter : public MLuaUserdataAdapter
{
	public:
		typedef enum _Type
		{
			kCircleType,
			kPolygonType,
			kRectType,
			kRoundedRectType,
			kMeshType,
		}
		Type;

		static const char *StringForType( Type t );
		// static Type TypeForString( const char * s );

		// static const ShapeAdapter& Constant();

	protected:
		ShapeAdapter( Type t );

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

        virtual StringHash *GetHash( lua_State *L ) const = 0;

	private:
		const char *fTypeString;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ShapeAdapter__

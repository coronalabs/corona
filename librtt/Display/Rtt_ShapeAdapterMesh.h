//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_ShapeAdapterMesh__
#define __Rtt_ShapeAdapterMesh__

#include "Display/Rtt_ShapeAdapter.h"
#include "Display/Rtt_TesselatorMesh.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class ShapeAdapterMesh : public ShapeAdapter
{
	public:
		typedef ShapeAdapter Super;

	public:
		static const ShapeAdapterMesh& Constant();
	
		static bool InitializeMesh(lua_State *L, int index, TesselatorMesh& tesselator, bool hasZ );
		static Geometry::PrimitiveType GetMeshMode(lua_State *L, int index);

	protected:
		ShapeAdapterMesh();

	public:
		virtual int ValueForKey(
			const LuaUserdataProxy& sender,
			lua_State *L,
			const char *key ) const;


        // Return the hash table containing the adapter's property keys
        virtual StringHash *GetHash( lua_State *L ) const;
	
	private:
		static int setVertex( lua_State *L );
		static int getVertex( lua_State *L );
		static int setUV( lua_State *L );
		static int getUV( lua_State *L );
		static int getVertexOffset( lua_State *L );
		static int update(lua_State *L);
		static int getLowestIndex( lua_State *L );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ShapeAdapterMesh__

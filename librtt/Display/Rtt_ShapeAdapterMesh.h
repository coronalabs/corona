//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
	
		static bool InitializeMesh(lua_State *L, int index, TesselatorMesh& tesselator );
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

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ShapeAdapterMesh__

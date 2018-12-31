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
			lua_State *L, int index, TesselatorPolygon& tesselator );

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

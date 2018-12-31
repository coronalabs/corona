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

#ifndef __Rtt_LuaLibPhysics__
#define __Rtt_LuaLibPhysics__

#ifdef Rtt_PHYSICS	

#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

class b2Draw;
class b2Fixture;
class b2Joint;
class b2World;
struct b2Transform;
struct b2Color;

namespace Rtt
{

class PhysicsWorld;

// ----------------------------------------------------------------------------

class LuaLibPhysics
{
	public:
		typedef LuaLibPhysics Self;

	public:
		typedef enum _PhysicsUnitType
		{
			kUnknownUnitType = 0,
			kLengthUnitType,
			kVelocityUnitType,
			kAngularVelocityUnitType,
		//	kForceUnitType, // ???
		//	kTorqueUnitType, // ???

			kNumUnitTypes
		}
		PhysicsUnitType;

	public:
		static bool IsWorldLocked( lua_State *L, const char errorMsg[] );
		static bool IsWorldValid( lua_State *L, const char errorMsg[] );

	public:
		static const void* GetGroundBodyUserdata();

	public:
		static float ToMKS( PhysicsUnitType unitType, const PhysicsWorld& physics, float value );
		static float FromMKS( PhysicsUnitType unitType, const PhysicsWorld& physics, float value );

	public:
		static int Open( lua_State *L );

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS	

#endif // __Rtt_LuaLibPhysics__

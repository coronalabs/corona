//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

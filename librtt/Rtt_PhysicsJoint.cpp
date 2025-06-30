//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#ifdef Rtt_PHYSICS	

#include "Rtt_PhysicsJoint.h"

// TODO: see which of these are actually needed here
#include "Rtt_Lua.h"
#include "Rtt_LuaLibPhysics.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaContext.h"

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char PhysicsJoint::kMetatableName[] = "physics.joint"; // unique identifier for this userdata type

b2Joint*
PhysicsJoint::GetJoint( lua_State *L, int index )
{
	b2Joint *result = NULL;

	UserdataWrapper **ud = (UserdataWrapper **)luaL_checkudata( L, index, Self::kMetatableName );
	if ( ud )
	{
		UserdataWrapper *wrapper = *ud;

		result = (b2Joint*)wrapper->Dereference();
	}

	return result;
}

int
PhysicsJoint::getAnchorA( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is common to all joint types
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();
		
		Rtt_Real px = Rtt_RealMul( Rtt_FloatToReal( baseJoint->GetAnchorA().x ), scale );
		Rtt_Real py = Rtt_RealMul( Rtt_FloatToReal( baseJoint->GetAnchorA().y ), scale );
		
		lua_pushnumber( L, px );
		lua_pushnumber( L, py );
	}
	
	return 2;
}

int
PhysicsJoint::getAnchorB( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is common to all joint types
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();
		
		Rtt_Real px = Rtt_RealMul( Rtt_FloatToReal( baseJoint->GetAnchorB().x ), scale );
		Rtt_Real py = Rtt_RealMul( Rtt_FloatToReal( baseJoint->GetAnchorB().y ), scale );
		
		lua_pushnumber( L, px );
		lua_pushnumber( L, py );
	}
	
	return 2;
}

// ----------------------------------------------------------------------------

static bool
ShouldGetLocalAnchor( b2JointType jointType )
{
	bool result = false;

	switch ( jointType )
	{
		case e_distanceJoint:
		case e_revoluteJoint:
		case e_prismaticJoint:
		case e_frictionJoint:
		case e_wheelJoint:
		case e_weldJoint:
		case e_ropeJoint:
			result = true;
			break;

		default:
			break;
	}

	return result;
}

template < typename T >
static const b2Vec2& GetLocalAnchorA( b2Joint *baseJoint )
{
	T *joint = static_cast< T * >( baseJoint ); Rtt_ASSERT( baseJoint );
	return joint->GetLocalAnchorA();
}

template < typename T >
static const b2Vec2& GetLocalAnchorB( b2Joint *baseJoint )
{
	T *joint = static_cast< T * >( baseJoint ); Rtt_ASSERT( baseJoint );
	return joint->GetLocalAnchorB();
}

typedef const b2Vec2& ( *GetLocalAnchorCallback )( b2Joint * );

static GetLocalAnchorCallback
GetLocalAnchorACallback( b2JointType jointType )
{
	Rtt_ASSERT( ShouldGetLocalAnchor( jointType ) );
	GetLocalAnchorCallback result = NULL;
	switch ( jointType )
	{
		case e_distanceJoint:
			result = & GetLocalAnchorA< b2DistanceJoint >;
			break;
		case e_revoluteJoint:
			result = & GetLocalAnchorA< b2RevoluteJoint >;
			break;
		case e_prismaticJoint:
			result = & GetLocalAnchorA< b2PrismaticJoint >;
			break;
		case e_frictionJoint:
			result = & GetLocalAnchorA< b2FrictionJoint >;
			break;
		case e_wheelJoint:
			result = & GetLocalAnchorA< b2WheelJoint >;
			break;
		case e_weldJoint:
			result = & GetLocalAnchorA< b2WeldJoint >;
			break;
		case e_ropeJoint:
			result = & GetLocalAnchorA< b2RopeJoint >;
			break;

		default:
			break;
	}

	return result;
}

static GetLocalAnchorCallback
GetLocalAnchorBCallback( b2JointType jointType )
{
	Rtt_ASSERT( ShouldGetLocalAnchor( jointType ) );
	GetLocalAnchorCallback result = NULL;
	switch ( jointType )
	{
		case e_distanceJoint:
			result = & GetLocalAnchorB< b2DistanceJoint >;
			break;
		case e_revoluteJoint:
			result = & GetLocalAnchorB< b2RevoluteJoint >;
			break;
		case e_prismaticJoint:
			result = & GetLocalAnchorB< b2PrismaticJoint >;
			break;
		case e_frictionJoint:
			result = & GetLocalAnchorB< b2FrictionJoint >;
			break;
		case e_wheelJoint:
			result = & GetLocalAnchorB< b2WheelJoint >;
			break;
		case e_weldJoint:
			result = & GetLocalAnchorB< b2WeldJoint >;
			break;
		case e_ropeJoint:
			result = & GetLocalAnchorB< b2RopeJoint >;
			break;

		default:
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

bool
PhysicsJoint::HasLocalAnchor( b2Joint& joint )
{
	return ShouldGetLocalAnchor( joint.GetType() );
}

b2Vec2
PhysicsJoint::GetLocalAnchorA( b2Joint& joint )
{
	b2JointType jointType = joint.GetType();
	if ( ShouldGetLocalAnchor( jointType ) )
	{
		GetLocalAnchorCallback Callback = GetLocalAnchorACallback( jointType );
		return Callback( & joint );
	}
	
	return b2Vec2_zero;
}

b2Vec2
PhysicsJoint::GetLocalAnchorB( b2Joint& joint )
{
	b2JointType jointType = joint.GetType();
	if ( ShouldGetLocalAnchor( jointType ) )
	{
		GetLocalAnchorCallback Callback = GetLocalAnchorBCallback( jointType );
		return Callback( & joint );
	}
	
	return b2Vec2_zero;
}

// ----------------------------------------------------------------------------

int
PhysicsJoint::getLocalAnchor( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is common to all joint types
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		b2Vec2 anchor;
		GetLocalAnchorCallback callback =
			(GetLocalAnchorCallback)lua_touserdata( L, lua_upvalueindex( 1 ) ); Rtt_ASSERT( callback );
		anchor = ( * callback )( baseJoint );

		Rtt_Real px = Rtt_RealMul( Rtt_FloatToReal( anchor.x ), scale );
		Rtt_Real py = Rtt_RealMul( Rtt_FloatToReal( anchor.y ), scale );
		
		lua_pushnumber( L, px );
		lua_pushnumber( L, py );
	}
	
	return 2;
}

// ----------------------------------------------------------------------------

#ifdef Rtt_DEBUG
static bool
ShouldGetLocalAxis( b2JointType jointType )
{
	bool result = false;

	switch ( jointType )
	{
		case e_prismaticJoint:
		case e_wheelJoint:
			result = true;
			break;

		default:
			break;
	}

	return result;
}
#endif // Rtt_DEBUG

template < typename T >
static const b2Vec2& GetLocalAxisA( b2Joint *baseJoint )
{
	T *joint = static_cast< T * >( baseJoint ); Rtt_ASSERT( baseJoint );
	return joint->GetLocalAxisA();
}

typedef const b2Vec2& ( *GetLocalAxisCallback )( b2Joint * );

static GetLocalAxisCallback
GetLocalAxisACallback( b2JointType jointType )
{
	Rtt_ASSERT( ShouldGetLocalAxis( jointType ) );
	GetLocalAxisCallback result = NULL;
	switch ( jointType )
	{
		case e_prismaticJoint:
			result = & GetLocalAxisA< b2PrismaticJoint >;
			break;
		case e_wheelJoint:
			result = & GetLocalAxisA< b2WheelJoint >;
			break;

		default:
			break;
	}

	return result;
}

int
PhysicsJoint::getLocalAxis( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is common to all joint types
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();
		
		Rtt_Real px = Rtt_RealMul( Rtt_FloatToReal( baseJoint->GetAnchorB().x ), scale );
		Rtt_Real py = Rtt_RealMul( Rtt_FloatToReal( baseJoint->GetAnchorB().y ), scale );
		
		lua_pushnumber( L, px );
		lua_pushnumber( L, py );
	}
	
	return 2;
}

// ----------------------------------------------------------------------------

int
PhysicsJoint::getReactionForce( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is common to all joint types
	if ( baseJoint )
	{
		Runtime& runtime = * LuaContext::GetRuntime( L );
		float32 inverseDeltaTime = (float)runtime.GetFPS();
		Rtt_Real px = Rtt_FloatToReal( baseJoint->GetReactionForce( inverseDeltaTime ).x );
		Rtt_Real py = Rtt_FloatToReal( baseJoint->GetReactionForce( inverseDeltaTime ).y );
		
		lua_pushnumber( L, px );
		lua_pushnumber( L, py );
	}
	
	return 2;
}


int
PhysicsJoint::setRotationLimits( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is for revolute ("pivot") joints only
	if ( baseJoint )
	{
		Rtt_Real lowerLimit = Rtt_RealDegreesToRadians( lua_tonumber( L, 2 ) );
		Rtt_Real upperLimit = Rtt_RealDegreesToRadians( lua_tonumber( L, 3 ) );
		
		b2RevoluteJoint *joint = (b2RevoluteJoint*)baseJoint;
		
		joint->SetLimits( Rtt_RealToFloat( lowerLimit ), Rtt_RealToFloat( upperLimit ) );
	}
	
	return 0;
}


int
PhysicsJoint::getRotationLimits( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );

	// This is for revolute ("pivot") joints only
	if ( baseJoint )
	{
		b2RevoluteJoint *joint = (b2RevoluteJoint*)baseJoint;
		
		Rtt_Real lowerLimit = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetLowerLimit() ) );
		Rtt_Real upperLimit = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetUpperLimit() ) );
		
		lua_pushnumber( L, lowerLimit );
		lua_pushnumber( L, upperLimit );
	}
	
	return 2;
}

int
PhysicsJoint::setLimits( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is for prismatic ("piston") joints only
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		Rtt_Real lowerLimit = Rtt_RealDiv( luaL_toreal( L, 2 ), scale );
		Rtt_Real upperLimit = Rtt_RealDiv( luaL_toreal( L, 3 ), scale );

		if ( upperLimit < lowerLimit )
		{
			Swap( lowerLimit, upperLimit );
			Rtt_TRACE_SIM( ( "WARNING: In the call to pistonJoint:setLimits(), it looks like you reversed "
							 "the lower and upper limits. We assume you meant to set the lower limit as "
							 "%g and the upper limit as %g.\n", lowerLimit, upperLimit ) );
		}

		b2PrismaticJoint *joint = (b2PrismaticJoint*)baseJoint;		
		joint->SetLimits( Rtt_RealToFloat( lowerLimit ), Rtt_RealToFloat( upperLimit ) );
	}
	
	return 0;
}

int
PhysicsJoint::getLimits( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is for prismatic ("piston") joints only
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		b2PrismaticJoint *joint = (b2PrismaticJoint*)baseJoint; // assumption: this cast is OK for both cases (otherwise check type)

		Rtt_Real lowerLimit = Rtt_RealMul( Rtt_FloatToReal( joint->GetLowerLimit() ), scale );
		Rtt_Real upperLimit = Rtt_RealMul( Rtt_FloatToReal( joint->GetUpperLimit() ), scale );
		
		lua_pushnumber( L, lowerLimit );
		lua_pushnumber( L, upperLimit );
	}
	
	return 2;
}

int
PhysicsJoint::getGroundAnchorA( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is for pulley joints only
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();
		
		b2PulleyJoint *joint = (b2PulleyJoint*)baseJoint;
		
		Rtt_Real px = Rtt_RealMul( Rtt_FloatToReal( joint->GetGroundAnchorA().x ), scale );
		Rtt_Real py = Rtt_RealMul( Rtt_FloatToReal( joint->GetGroundAnchorA().y ), scale );
		
		lua_pushnumber( L, px );
		lua_pushnumber( L, py );
	}
	
	return 2;
}

int
PhysicsJoint::getGroundAnchorB( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is for pulley joints only
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();
		
		b2PulleyJoint *joint = (b2PulleyJoint*)baseJoint;
		
		Rtt_Real px = Rtt_RealMul( Rtt_FloatToReal( joint->GetGroundAnchorB().x ), scale );
		Rtt_Real py = Rtt_RealMul( Rtt_FloatToReal( joint->GetGroundAnchorB().y ), scale );
		
		lua_pushnumber( L, px );
		lua_pushnumber( L, py );
	}
	
	return 2;
}

int
PhysicsJoint::getTarget( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );

	// This is for mouse ("touch") joints only
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

		b2MouseJoint *joint = (b2MouseJoint*)baseJoint;

		b2Vec2 v( joint->GetTarget() * physics.GetPixelsPerMeter() );

		lua_pushnumber( L, v.x );
		lua_pushnumber( L, v.y );
	}

	return 2;
}

int
PhysicsJoint::setTarget( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	// This is for mouse ("touch") joints only
	if ( baseJoint )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

		b2Vec2 v( lua_tonumber( L, 2 ),
					lua_tonumber( L, 3 ) );

		v *= physics.GetMetersPerPixel();

		b2MouseJoint *joint = (b2MouseJoint*)baseJoint;
		joint->SetTarget( v );
	}
	
	return 0;
}

int
PhysicsJoint::removeSelf( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	if ( baseJoint )
	{
		// Disconnect joint and wrapper from each other

		// (1) Remove wrapper's ref to joint
		UserdataWrapper *wrapper = (UserdataWrapper*)baseJoint->GetUserData();
		if ( wrapper && UserdataWrapper::GetFinalizedValue() != wrapper )
		{
			wrapper->Invalidate();
		}
		
		// (2) Remove joint's ref to wrapper.
		// Any joint with a finalized userdata value to signal it should be 
		// destroyed after the world step (similar to body destruction cycle)
		baseJoint->SetUserData( UserdataWrapper::GetFinalizedValue() );
	}

	return 0;
}

static const char *
StringForLimitState( b2LimitState value )
{
	static const char kInactiveLimit[] = "inactive";
	static const char kLowerLimit[] = "lower";
	static const char kUpperLimit[] = "upper";
	static const char kEqualLimit[] = "equal";

	const char *result = kInactiveLimit;

	switch ( value )
	{
		case e_atLowerLimit:
			result = kLowerLimit;
			break;
		case e_atUpperLimit:
			result = kUpperLimit;
			break;
		case e_equalLimits:
			result = kEqualLimit;
			break;
		default:
			break;
	}

	return result;
}

int
PhysicsJoint::ValueForKey( lua_State *L )
{
	int result = 0;    // number of args pushed on the stack
	
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	if ( baseJoint )
	{
		const char *key = luaL_checkstring( L, 2 );		
		result = 1;

		b2JointType jointType = baseJoint->GetType();

		//////////////////////////////////////////////
		// These methods are common to all joint types
		
		if ( strcmp( "getAnchorA", key ) == 0 )
		{
			lua_pushcfunction( L, Self::getAnchorA );
		}
		else if ( strcmp( "getAnchorB", key ) == 0 )
		{
			lua_pushcfunction( L, Self::getAnchorB );
		}
		else if ( strcmp( "getReactionForce", key ) == 0 )
		{
			lua_pushcfunction( L, Self::getReactionForce );
		}
		else if ( 0 == strcmp( "reactionTorque", key ) )  // read-only
		{
			Runtime& runtime = * LuaContext::GetRuntime( L );
			float32 inverseDeltaTime = (float)runtime.GetFPS();
			lua_pushnumber( L, baseJoint->GetReactionTorque( inverseDeltaTime ) );
		}
		else if ( 0 == strcmp( "removeSelf", key ) )
		{
			lua_pushcfunction( L, Self::removeSelf );
		}
		else if ( 0 == strcmp( "isActive", key ) )
		{
			lua_pushboolean( L, baseJoint->IsActive() );
		}
		else if ( 0 == strcmp( "isCollideConnected", key ) )
		{
			lua_pushboolean( L, baseJoint->GetCollideConnected() );
		}
		else if ( 0 == strcmp( "getLocalAnchorA", key ) && ShouldGetLocalAnchor( jointType ) )
		{
			lua_pushlightuserdata( L, (void*)GetLocalAnchorACallback( jointType ) );
			lua_pushcclosure( L, Self::getLocalAnchor, 1 );
		}
		else if ( 0 == strcmp( "getLocalAnchorB", key ) && ShouldGetLocalAnchor( jointType ) )
		{
			lua_pushlightuserdata( L, (void*)GetLocalAnchorBCallback( jointType ) );
			lua_pushcclosure( L, Self::getLocalAnchor, 1 );
		}
		else
		{
			// No common property found. Look at specific properties depending on joint type

			if ( jointType == e_distanceJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				// This is exposed as a "distance" joint in Corona
				
				b2DistanceJoint *joint = (b2DistanceJoint*)baseJoint;
				
				if ( 0 == strcmp( "length", key ) )
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetLength() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "frequency", key ) )
				{
					lua_pushnumber( L, joint->GetFrequency() );
				}
				else if ( 0 == strcmp( "dampingRatio", key ) )
				{
					lua_pushnumber( L, joint->GetDampingRatio() );
				}
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_revoluteJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				// This is exposed as a "pivot" joint in Corona (aka "revolute" in Box2D terms)			

				b2RevoluteJoint *joint = (b2RevoluteJoint*)baseJoint;
				
				if ( 0 == strcmp( "isMotorEnabled", key ) )
				{
					lua_pushboolean( L, joint->IsMotorEnabled() );
				}
				else if ( 0 == strcmp( "motorSpeed", key ) )
				{
					Rtt_Real valueDegrees = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetMotorSpeed() ) );
					lua_pushnumber( L, valueDegrees );
				}
				else if ( 0 == strcmp( "motorTorque", key ) )  // read-only
				{
					Runtime& runtime = * LuaContext::GetRuntime( L );
					float32 inverseDeltaTime = (float)runtime.GetFPS();

					lua_pushnumber( L, joint->GetMotorTorque( inverseDeltaTime ) );
				}
				else if ( 0 == strcmp( "maxMotorTorque", key ) )
				{
					lua_pushnumber( L, joint->GetMaxMotorTorque() );
				}
				else if ( 0 == strcmp( "referenceAngle", key ) )  // read-only
				{
					Rtt_Real valueDegrees = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetReferenceAngle() ) );
					lua_pushnumber( L, valueDegrees );
				}
				else if ( 0 == strcmp( "jointAngle", key ) )  // read-only
				{
					Rtt_Real valueDegrees = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetJointAngle() ) );
					lua_pushnumber( L, valueDegrees );
				}
				else if ( 0 == strcmp( "jointSpeed", key ) )  // read-only
				{
					Rtt_Real valueDegrees = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetJointSpeed() ) );
					lua_pushnumber( L, valueDegrees );
				}
				else if ( 0 == strcmp( "isLimitEnabled", key ) )
				{
					lua_pushboolean( L, joint->IsLimitEnabled() );
				}
				else if ( strcmp( "setRotationLimits", key ) == 0 )
				{
					lua_pushcfunction( L, Self::setRotationLimits );
				}
				else if ( strcmp( "getRotationLimits", key ) == 0 )
				{
					lua_pushcfunction( L, Self::getRotationLimits );
				}		
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_prismaticJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				// This is exposed as a "piston" joint in Corona (aka "prismatic" in Box2D terms)
				
				b2PrismaticJoint *joint = (b2PrismaticJoint*)baseJoint;
				
				if ( 0 == strcmp( "isMotorEnabled", key ) )
				{
					lua_pushboolean( L, joint->IsMotorEnabled() );
				}
				else if ( 0 == strcmp( "motorSpeed", key ) )
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetMotorSpeed() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "motorForce", key ) ) // read-only
				{
					Runtime& runtime = * LuaContext::GetRuntime( L );
					float32 inverseDeltaTime = (float)runtime.GetFPS();

					lua_pushnumber( L, joint->GetMotorForce( inverseDeltaTime ) );
				}
				else if ( 0 == strcmp( "maxMotorForce", key ) )
				{
					lua_pushnumber( L, joint->GetMaxMotorForce() );
				}
				else if ( strcmp( "getLocalAxisA", key ) == 0 )
				{
					lua_pushlightuserdata( L, (void *)GetLocalAxisACallback( e_prismaticJoint ) );
					lua_pushcclosure( L, Self::getLocalAxis, 1 );
				}
				else if ( 0 == strcmp( "referenceAngle", key ) )  // read-only
				{
					Rtt_Real valueDegrees = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetReferenceAngle() ) );
					lua_pushnumber( L, valueDegrees );
				}
				else if ( 0 == strcmp( "jointTranslation", key ) )  // read-only
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetJointTranslation() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "jointSpeed", key ) )  // read-only
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetJointSpeed() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "isLimitEnabled", key ) )
				{
					lua_pushboolean( L, joint->IsLimitEnabled() );
				}
				else if ( strcmp( "setLimits", key ) == 0 )
				{
					lua_pushcfunction( L, Self::setLimits );
				}
				else if ( strcmp( "getLimits", key ) == 0 )
				{
					lua_pushcfunction( L, Self::getLimits );
				}
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_frictionJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				// This is exposed as a "friction" joint in Corona (also "friction" in Box2D terms)
				// A friction joint is a pivot joint that resists motion, and is therefore "sticky"	

				b2FrictionJoint *joint = (b2FrictionJoint*)baseJoint;
				
				if ( 0 == strcmp( "maxForce", key ) )
				{
					lua_pushnumber( L, joint->GetMaxForce() );
				}
				else if ( 0 == strcmp( "maxTorque", key ) )
				{
					lua_pushnumber( L, joint->GetMaxTorque() );
				}
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_wheelJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				// This is exposed as a "wheel" joint in Corona (aka "line" in Box2D terms)			

				b2WheelJoint *joint = (b2WheelJoint*)baseJoint;
				
				if ( 0 == strcmp( "isMotorEnabled", key ) )
				{
					lua_pushboolean( L, joint->IsMotorEnabled() );
				}
				else if ( 0 == strcmp( "motorSpeed", key ) )
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetMotorSpeed() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "motorTorque", key ) )  // read-only
				{
					lua_pushnumber( L, joint->GetMaxMotorTorque() );
				}
				else if ( 0 == strcmp( "maxMotorTorque", key ) )
				{
					lua_pushnumber( L, joint->GetMaxMotorTorque() );
				}
				else if ( strcmp( "getLocalAxisA", key ) == 0 )
				{
					lua_pushlightuserdata( L, (void *)GetLocalAxisACallback( e_wheelJoint ) );
					lua_pushcclosure( L, Self::getLocalAxis, 1 );
				}
				else if ( 0 == strcmp( "jointTranslation", key ) )  // read-only
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetJointTranslation() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "jointSpeed", key ) )  // read-only
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetJointSpeed() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( strcmp( "springFrequency", key ) == 0 )
				{
					lua_pushnumber( L, joint->GetSpringFrequencyHz() );
				}		
				else if ( strcmp( "springDampingRatio", key ) == 0 )
				{
					lua_pushnumber( L, joint->GetSpringDampingRatio() );
				}		
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_pulleyJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				// This is exposed as a "pulley" joint in Corona
				
				b2PulleyJoint *joint = (b2PulleyJoint*)baseJoint;
				
				if ( 0 == strcmp( "getGroundAnchorA", key ) ) // read-only
				{
					lua_pushcfunction( L, Self::getGroundAnchorA );
				}
				else if ( 0 == strcmp( "getGroundAnchorB", key ) ) // read-only
				{
					lua_pushcfunction( L, Self::getGroundAnchorB );
				}
				else if ( 0 == strcmp( "length1", key ) ) // read-only
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetLengthA() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "length2", key ) ) // read-only
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valuePixels = Rtt_RealMul( Rtt_FloatToReal( joint->GetLengthB() ), scale );
					lua_pushnumber( L, valuePixels );
				}
				else if ( 0 == strcmp( "ratio", key ) ) // read-only
				{
					lua_pushnumber( L, joint->GetRatio() );
				}
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_mouseJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				// This is exposed as a "touch" joint in Corona (aka "mouse joint" in Box2D terms)
				// A touch joint is used for dragging objects without overriding the simulation; 
				// it creates an elastic link between the current touch point -- or any point submitted 
				// by the end developer using SetTarget() -- and the specified body

				b2MouseJoint *joint = (b2MouseJoint*)baseJoint;
				
				if ( 0 == strcmp( "maxForce", key ) )
				{
					lua_pushnumber( L, joint->GetMaxForce() );
				}
				else if ( 0 == strcmp( "frequency", key ) )
				{
					lua_pushnumber( L, joint->GetFrequency() );
				}
				else if ( 0 == strcmp( "dampingRatio", key ) )
				{
					lua_pushnumber( L, joint->GetDampingRatio() );
				}
				else if ( strcmp( "setTarget", key ) == 0 )
				{
					lua_pushcfunction( L, Self::setTarget );
				}
				else if ( strcmp( "getTarget", key ) == 0 )
				{
					lua_pushcfunction( L, Self::getTarget );
				}		
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_gearJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				b2GearJoint *joint = (b2GearJoint*)baseJoint;
				
				if ( 0 == strcmp( "ratio", key ) )
				{
					lua_pushnumber( L, joint->GetRatio() );
				}
				else if ( 0 == strcmp( "joint1", key ) )
				{
					UserdataWrapper *wrapper = (UserdataWrapper *)joint->GetJoint1()->GetUserData();
					wrapper->Push();
				}
				else if ( 0 == strcmp( "joint2", key ) )
				{
					UserdataWrapper *wrapper = (UserdataWrapper *)joint->GetJoint2()->GetUserData();
					wrapper->Push();
				}
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_weldJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				b2WeldJoint *joint = (b2WeldJoint*)baseJoint;
				
				if ( 0 == strcmp( "referenceAngle", key ) )  // read-only
				{
					Rtt_Real valueDegrees = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( joint->GetReferenceAngle() ) );
					lua_pushnumber( L, valueDegrees );
				}
				else if ( 0 == strcmp( "frequency", key ) )
				{
					lua_pushnumber( L, joint->GetFrequency() );
				}
				else if ( 0 == strcmp( "dampingRatio", key ) )
				{
					lua_pushnumber( L, joint->GetDampingRatio() );
				}
				else
				{
					result = 0;
				}
			}
			else if ( jointType == e_ropeJoint ) 
			{
				//////////////////////////////////////////////////////////////////////////////
				b2RopeJoint *joint = (b2RopeJoint*)baseJoint;
				
				if ( 0 == strcmp( "maxLength", key ) )  // read-only
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

					float value = LuaLibPhysics::FromMKS( LuaLibPhysics::kLengthUnitType, physics, joint->GetMaxLength() );
					lua_pushnumber( L, value );
				}
				else if ( 0 == strcmp( "limitState", key ) )
				{
					lua_pushstring( L, StringForLimitState( joint->GetLimitState() ) );
				}
				else
				{
					result = 0;
				}
			}
			else
			{
				result = 0;
			}
		}
	}
	
	return result;
}

int
PhysicsJoint::SetValueForKey( lua_State *L )
{
	b2Joint *baseJoint = GetJoint( L, 1 );
	
	if ( baseJoint )
	{		
		const char *key = luaL_checkstring( L, 2 );
				
		b2JointType jointType = baseJoint->GetType();

		//////////////////////////////////////////////
		// These methods are common to all joint types
		
		if ( 0 == strcmp( "reactionTorque", key ) )
		{
			// No-op for read-only property
		}

			
		if (jointType == e_distanceJoint) 
		{
			//////////////////////////////////////////////////////////////////////////////
 			// This is exposed as a "distance" joint in Corona

			b2DistanceJoint *joint = (b2DistanceJoint*)baseJoint;
			
			if ( 0 == strcmp( "length", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Real valueMeters = Rtt_RealDiv( luaL_toreal( L, 3 ), scale );
					joint->SetLength( Rtt_RealToFloat( valueMeters ) );
				}
			}
			else if ( 0 == strcmp( "frequency", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					Rtt_Real newValue = luaL_toreal( L, 3 );
					joint->SetFrequency( newValue );
				}
			}
			else if ( 0 == strcmp( "dampingRatio", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					Rtt_Real newValue = luaL_toreal( L, 3 );
					joint->SetDampingRatio( newValue );
				}
			}
		}
		
		else if (jointType == e_revoluteJoint) 
		{
			//////////////////////////////////////////////////////////////////////////////
			// This is exposed as a "pivot" joint in Corona (aka "revolute" in Box2D terms)

			b2RevoluteJoint *joint = (b2RevoluteJoint*)baseJoint;
			
			if ( 0 == strcmp( "isMotorEnabled", key ) )
			{
				if ( lua_isboolean( L, 3 ) )
				{
					joint->EnableMotor( lua_toboolean( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "motorSpeed", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					Rtt_Real valueRadians = Rtt_RealDegreesToRadians( luaL_toreal( L, 3 ) );
					joint->SetMotorSpeed( Rtt_RealToFloat( valueRadians ) );
				}
			}
			else if ( 0 == strcmp( "motorTorque", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "maxMotorTorque", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetMaxMotorTorque( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "isLimitEnabled", key ) )
			{
				if ( lua_isboolean( L, 3 ) )
				{
					joint->EnableLimit( lua_toboolean( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "jointAngle", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "jointSpeed", key ) )
			{
				// No-op for read-only property
			}		
		}

		else if (jointType == e_prismaticJoint) 
		{
			//////////////////////////////////////////////////////////////////////////////
			// This is exposed as a "piston" joint in Corona (aka "prismatic" in Box2D terms)

			b2PrismaticJoint *joint = (b2PrismaticJoint*)baseJoint;
			
			if ( 0 == strcmp( "isMotorEnabled", key ) )
			{
				if ( lua_isboolean( L, 3 ) )
				{
					joint->EnableMotor( lua_toboolean( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "motorSpeed", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Real valueMeters = Rtt_RealDiv( luaL_toreal( L, 3 ), scale );
					joint->SetMotorSpeed( Rtt_RealToFloat( valueMeters ) );
				}
			}
			else if ( 0 == strcmp( "motorForce", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "maxMotorForce", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetMaxMotorForce( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "isLimitEnabled", key ) )
			{
				if ( lua_isboolean( L, 3 ) )
				{
					joint->EnableLimit( lua_toboolean( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "jointTranslation", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "jointSpeed", key ) )
			{
				// No-op for read-only property
			}
		
		}
		
		else if ( jointType == e_frictionJoint ) 
		{
			//////////////////////////////////////////////////////////////////////////////
			// This is exposed as a "friction" joint in Corona (also "friction" in Box2D terms)
			// A friction joint is a pivot joint that resists motion, and is therefore "sticky"	

			b2FrictionJoint *joint = (b2FrictionJoint*)baseJoint;
			
			if ( 0 == strcmp( "maxForce", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetMaxForce( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "maxTorque", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetMaxTorque( lua_tonumber( L, 3 ) );
				}
			}
		}
		
		else if (jointType == e_wheelJoint) 
		{
			//////////////////////////////////////////////////////////////////////////////
			// This is exposed as a "wheel" joint in Corona (aka "line" in Box2D terms)

			b2WheelJoint *joint = (b2WheelJoint*)baseJoint;
			
			if ( 0 == strcmp( "isMotorEnabled", key ) )
			{
				if ( lua_isboolean( L, 3 ) )
				{
					joint->EnableMotor( lua_toboolean( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "motorSpeed", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					Real scale = physics.GetPixelsPerMeter();
					Rtt_Real valueMeters = Rtt_RealDiv( Rtt_FloatToReal( lua_tonumber( L, 3 ) ), scale );	
					joint->SetMotorSpeed( Rtt_RealToFloat( valueMeters ) );
				}
			}
			else if ( 0 == strcmp( "motorTorque", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "maxMotorTorque", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetMaxMotorTorque( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "jointTranslation", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "jointSpeed", key ) )
			{
				// No-op for read-only property
			}
			else if ( strcmp( "springFrequency", key ) == 0 )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetSpringFrequencyHz( lua_tonumber( L, 3 ) );
				}
			}		
			else if ( strcmp( "springDampingRatio", key ) == 0 )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetSpringDampingRatio( lua_tonumber( L, 3 ) );
				}
			}		
		}

		else if ( jointType == e_pulleyJoint ) 
		{
			//////////////////////////////////////////////////////////////////////////////
 			// This is exposed as a "pulley" joint in Corona (type-specific methods are read-only)
			
			// TODO: maxLength1 and maxLength2 settable properties? (check docs)
			
			// b2PulleyJoint *joint = (b2PulleyJoint*)baseJoint;
			
			if ( 0 == strcmp( "length1", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "length2", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "ratio", key ) )
			{
				// No-op for read-only property
			}
			
		}
		
		else if ( jointType == e_mouseJoint ) 
		{
			//////////////////////////////////////////////////////////////////////////////
			// This is exposed as a "touch" joint in Corona (aka "mouse joint" in Box2D terms)
			// A touch joint is used for dragging objects without overriding the simulation; 
			// it creates an elastic link between the current touch point -- or any point submitted 
			// by the end developer using SetTarget() -- and the specified body

			b2MouseJoint *joint = (b2MouseJoint*)baseJoint;
			
			if ( 0 == strcmp( "maxForce", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetMaxForce( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "frequency", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetFrequency( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "dampingRatio", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetDampingRatio( lua_tonumber( L, 3 ) );
				}
			}
		} 

		else if ( jointType == e_gearJoint ) 
		{
			//////////////////////////////////////////////////////////////////////////////
			b2GearJoint *joint = (b2GearJoint*)baseJoint;
			
			if ( 0 == strcmp( "ratio", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetRatio( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "joint1", key ) )
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "joint2", key ) )
			{
				// No-op for read-only property
			}
		}

		else if ( jointType == e_weldJoint ) 
		{
			//////////////////////////////////////////////////////////////////////////////
			b2WeldJoint *joint = (b2WeldJoint*)baseJoint;
			
			if ( 0 == strcmp( "referenceAngle", key ) )  // read-only
			{
				// No-op for read-only property
			}
			else if ( 0 == strcmp( "frequency", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetFrequency( lua_tonumber( L, 3 ) );
				}
			}
			else if ( 0 == strcmp( "dampingRatio", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					joint->SetDampingRatio( lua_tonumber( L, 3 ) );
				}
			}
		}

		else if ( jointType == e_ropeJoint ) 
		{
			//////////////////////////////////////////////////////////////////////////////
			b2RopeJoint *joint = (b2RopeJoint*)baseJoint;
			
			if ( 0 == strcmp( "maxLength", key ) )
			{
				if ( lua_isnumber( L, 3 ) )
				{
					float value = lua_tonumber( L, 3 );
					const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
					value = LuaLibPhysics::ToMKS( LuaLibPhysics::kLengthUnitType, physics, value );
					joint->SetMaxLength( value );
				}
			}
			else if ( 0 == strcmp( "limitState", key ) )
			{
				// No-op for read-only property
			}
		}

	}

	return 0;
}


int
PhysicsJoint::Finalizer( lua_State *L )
{
	UserdataWrapper **ud = (UserdataWrapper **)luaL_checkudata( L, 1, Self::kMetatableName );
	if ( ud )
	{
		UserdataWrapper *wrapper = *ud;

		b2Joint *joint = (b2Joint*)wrapper->Dereference();
		if ( joint )
		{
			// Make sure joint no longer points to this wrapper that we're about to destroy
			joint->SetUserData( NULL );
		}

		Rtt_DELETE( wrapper );
	}

	return 0;
}

// Call this to init metatable
void
PhysicsJoint::Initialize( lua_State *L )
{
	Rtt_LUA_STACK_GUARD( L );

	const luaL_Reg kVTable[] =
	{
		{ "__index", Self::ValueForKey },
		{ "__newindex", Self::SetValueForKey },
		{ "__gc", Self::Finalizer },
		{ NULL, NULL }
	};
		
	Lua::InitializeMetatable( L, Self::kMetatableName, kVTable );
}

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS	

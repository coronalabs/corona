//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_DisplayObjectExtensions.h"

#include "Core/Rtt_StringHash.h"
#include "Display/Rtt_GroupObject.h"
#include "Display/Rtt_DisplayObject.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibPhysics.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_Runtime.h"

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

DisplayObjectExtensions::DisplayObjectExtensions( DisplayObject& owner )
:	fOwner( owner )
#ifdef Rtt_PHYSICS
	, fBody( NULL )
#endif
{
}

DisplayObjectExtensions::~DisplayObjectExtensions()
{
#ifdef Rtt_PHYSICS
	if ( fBody )
	{
		GroupObject *parent = fOwner.GetParent();
		if ( Rtt_VERIFY( parent ) )
		{
			fBody->SetUserData( NULL );

			// Do NOT DestroyBody here.  Instead, at end of StepWorld(), we lazily
			// detect if the body's userdata is NULL. If it is, we know to destroy
			// the body.
			/*
			Runtime *runtime = static_cast< Runtime* >( Rtt_AllocatorGetUserdata( parent->Allocator() ) );
			b2World *world = runtime->GetWorld();
			if ( Rtt_VERIFY( world ) )
			{
				world->DestroyBody( fBody );
			}
			*/
		}
	}
#endif // Rtt_PHYSICS
}

#ifdef Rtt_PHYSICS

int
DisplayObjectExtensions::setLinearVelocity( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();

		Real vx = Rtt_RealDiv( lua_tonumber( L, 2 ), scale );
		Real vy = Rtt_RealDiv( lua_tonumber( L, 3 ), scale );

		b2Vec2 velocity = b2Vec2( Rtt_RealToFloat( vx ), Rtt_RealToFloat( vy ) );

		fBody->SetLinearVelocity( velocity );
	}

	return 0;
}


int
DisplayObjectExtensions::getLinearVelocity( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		b2Vec2 velocityInPixelsPerSecond = ( fBody->GetLinearVelocity() * physics.GetPixelsPerMeter() );

		lua_pushnumber( L, velocityInPixelsPerSecond.x );
		lua_pushnumber( L, velocityInPixelsPerSecond.y );
	}

	return 2;
}

int
DisplayObjectExtensions::getMassWorldCenter( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		b2Vec2 massWorldCenterInPixels = ( fBody->GetWorldCenter() * physics.GetPixelsPerMeter() );

		lua_pushnumber( L, massWorldCenterInPixels.x );
		lua_pushnumber( L, massWorldCenterInPixels.y );
	}

	return 2;
}

int
DisplayObjectExtensions::getMassLocalCenter( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		b2Vec2 massLocalCenterInPixels = ( fBody->GetLocalCenter() * physics.GetPixelsPerMeter() );

		lua_pushnumber( L, massLocalCenterInPixels.x );
		lua_pushnumber( L, massLocalCenterInPixels.y );
	}

	return 2;
}

int
DisplayObjectExtensions::applyForce( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	//Runtime& runtime = * LuaContext::GetRuntime( L );
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	Real scale = physics.GetPixelsPerMeter();

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		b2Vec2 force = b2Vec2( lua_tonumber( L, 2 ), lua_tonumber( L, 3 ) );

		Real px = Rtt_FloatToReal( lua_tonumber( L, 4 ) );
		Real py = Rtt_FloatToReal( lua_tonumber( L, 5 ) );
		px = Rtt_RealDiv( px, scale );
		py = Rtt_RealDiv( py, scale );
		b2Vec2 point = b2Vec2( Rtt_RealToFloat( px ), Rtt_RealToFloat( py ) );

		fBody->ApplyForce( force, point, true );
	}

	return 0;
}


int
DisplayObjectExtensions::applyTorque( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		fBody->ApplyTorque( lua_tonumber( L, 2 ), true );
	}

	return 0;
}


int
DisplayObjectExtensions::applyLinearImpulse( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	Real scale = physics.GetPixelsPerMeter();

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		b2Vec2 impulse = b2Vec2( lua_tonumber( L, 2 ), lua_tonumber( L, 3 ) );

		Real px = Rtt_FloatToReal( lua_tonumber( L, 4 ) );
		Real py = Rtt_FloatToReal( lua_tonumber( L, 5 ) );
		px = Rtt_RealDiv( px, scale );
		py = Rtt_RealDiv( py, scale );
		b2Vec2 point = b2Vec2( Rtt_RealToFloat( px ), Rtt_RealToFloat( py ) );

		fBody->ApplyLinearImpulse( impulse, point, true );
	}

	return 0;
}


int
DisplayObjectExtensions::applyAngularImpulse( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		fBody->ApplyTorque( lua_tonumber( L, 2 ), true );
	}

	return 0;
}


int
DisplayObjectExtensions::resetMassData( lua_State *L )
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if ( o )
	{
		Self *extensions = o->GetExtensions();
		b2Body *fBody = extensions->GetBody();
		fBody->ResetMassData();
	}

	return 0;
}

int
DisplayObjectExtensions::getWorldVector(lua_State* L)
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject(L, 1);

	Rtt_WARN_SIM_PROXY_TYPE(L, 1, DisplayObject);

	if (o)
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime(L)->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		Self* extensions = o->GetExtensions();
		b2Body* fBody = extensions->GetBody();

		Real lx = Rtt_RealDiv(lua_tonumber(L, 2), scale);
		Real ly = Rtt_RealDiv(lua_tonumber(L, 3), scale);

		b2Vec2 localVector = b2Vec2(Rtt_RealToFloat(lx), Rtt_RealToFloat(ly));

		b2Vec2 worldVector = fBody->GetWorldVector(localVector);

		lua_pushnumber(L, worldVector.x);
		lua_pushnumber(L, worldVector.y);
		
		return 2;
	}

	return 0;
}

int
DisplayObjectExtensions::getInertia(lua_State* L)
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject(L, 1);

	Rtt_WARN_SIM_PROXY_TYPE(L, 1, DisplayObject);

	if (o)
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime(L)->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		Self* extensions = o->GetExtensions();
		b2Body* fBody = extensions->GetBody();
	
		float32 inertia = fBody->GetInertia() * scale;

		lua_pushnumber(L, inertia);
		
		return 1;
	}

	return 0;
}

int
DisplayObjectExtensions::getLinearVelocityFromWorldPoint(lua_State* L)
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject(L, 1);

	Rtt_WARN_SIM_PROXY_TYPE(L, 1, DisplayObject);

	if (o)
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime(L)->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		Self* extensions = o->GetExtensions();
		b2Body* fBody = extensions->GetBody();

		Real wx = Rtt_RealDiv(lua_tonumber(L, 2), scale);
		Real wy = Rtt_RealDiv(lua_tonumber(L, 3), scale);

		b2Vec2 worldPoint = b2Vec2(Rtt_RealToFloat(wx), Rtt_RealToFloat(wy));

		b2Vec2 velocity = fBody->GetLinearVelocityFromWorldPoint(worldPoint);

		lua_pushnumber(L, velocity.x);
		lua_pushnumber(L, velocity.y);
		
		return 2;
	}

	return 0;
}

int
DisplayObjectExtensions::getLinearVelocityFromLocalPoint(lua_State* L)
{
	DisplayObject* o = (DisplayObject*)LuaProxy::GetProxyableObject(L, 1);

	Rtt_WARN_SIM_PROXY_TYPE(L, 1, DisplayObject);

	if (o)
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime(L)->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		Self* extensions = o->GetExtensions();
		b2Body* fBody = extensions->GetBody();

		Real wx = Rtt_RealDiv(lua_tonumber(L, 2), scale);
		Real ly = Rtt_RealDiv(lua_tonumber(L, 3), scale);

		b2Vec2 localPoint = b2Vec2(Rtt_RealToFloat(wx), Rtt_RealToFloat(ly));

		b2Vec2 velocity = fBody->GetLinearVelocityFromLocalPoint(localPoint);

		lua_pushnumber(L, velocity.x);
		lua_pushnumber(L, velocity.y);
		
		return 2;
	}

	return 0;
}

#endif // Rtt_PHYSICS


static const char kStaticBodyType[] = "static";
static const char kKinematicBodyType[] = "kinematic";
static const char kDynamicBodyType[] = "dynamic";

int
DisplayObjectExtensions::ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction /* = false */ ) const
{
	if ( ! key || lua_isnumber( L, 2 ) ) { return 0; }

	int result = 0;

#ifdef Rtt_PHYSICS
	if ( fBody )
	{
		result = 1;

		static const char * keys[] =
		{
			"isAwake",							// 0
			"isBodyActive",						// 1
			"isBullet",							// 2
			"isSleepingAllowed",				// 3
			"isFixedRotation",					// 4
			"angularVelocity",					// 5
			"linearDamping",					// 6
			"angularDamping",					// 7
			"bodyType",							// 8
			"setLinearVelocity",				// 9
			"getLinearVelocity",				// 10
			"applyForce",						// 11
			"applyTorque",						// 12
			"applyLinearImpulse",				// 13
			"applyAngularImpulse",				// 14
			"resetMassData",					// 15
			"isSensor",							// 16
			"mass",								// 17
			"gravityScale",						// 18
			"getMassWorldCenter",				// 19
			"getMassLocalCenter",				// 20
			"getWorldVector",					// 21
			"getInertia",						// 22
			"getLinearVelocityFromWorldPoint",	// 23
			"getLinearVelocityFromLocalPoint",  // 24
		};
		static const int numKeys = sizeof( keys ) / sizeof( const char * );
		static StringHash sHash( *LuaContext::GetAllocator( L ), keys, numKeys, 25, 19, 14, __FILE__, __LINE__ );
		StringHash *hash = &sHash;

		int index = hash->Lookup( key );
		switch ( index )
		{
		case 0:
			{
				lua_pushboolean( L, fBody->IsAwake() );
			}
			break;
		case 1:
			{
				lua_pushboolean( L, fBody->IsActive() );
			}
			break;
		case 2:
			{
				lua_pushboolean( L, fBody->IsBullet() );
			}
			break;
		case 3:
			{
				lua_pushboolean( L, fBody->IsSleepingAllowed() );
			}
			break;
		case 4:
			{
				lua_pushboolean( L, fBody->IsFixedRotation() );
			}
			break;
		case 5:
			{
				Real va = Rtt_RealRadiansToDegrees( Rtt_FloatToReal( fBody->GetAngularVelocity() ) );
				lua_pushnumber( L, va );
			}
			break;
		case 6:
			{
				lua_pushnumber( L, fBody->GetLinearDamping() );
			}
			break;
		case 7:
			{
				lua_pushnumber( L, fBody->GetAngularDamping() );
			}
			break;
		case 8:
			{
				switch ( fBody->GetType() ) {
				case b2_staticBody:
					lua_pushstring( L, kStaticBodyType );
					break;
				case b2_kinematicBody:
					lua_pushstring(L, kKinematicBodyType );
					break;
				default:
					lua_pushstring( L, kDynamicBodyType );
					break;
				}
			}
			break;
		case 9:
			{
				lua_pushcfunction( L, Self::setLinearVelocity );
			}
			break;
		case 10:
			{
				lua_pushcfunction( L, Self::getLinearVelocity );
			}
			break;
		case 11:
			{
				lua_pushcfunction( L, Self::applyForce );
			}
			break;
		case 12:
			{
				lua_pushcfunction( L, Self::applyTorque );
			}
			break;
		case 13:
			{
				lua_pushcfunction( L, Self::applyLinearImpulse );
			}
			break;
		case 14:
			{
				lua_pushcfunction( L, Self::applyAngularImpulse );
			}
			break;
		case 15:
			{
				lua_pushcfunction( L, Self::resetMassData );
			}
			break;
		case 16:
			{
				// no-op for write only
				lua_pushnil( L );
			}
			break;
		case 17:
			{
				lua_pushnumber( L, fBody->GetMass() );
			}
			break;
		case 18:
			{
				lua_pushnumber( L, fBody->GetGravityScale() );
			}
			break;
		case 19:
			{
				lua_pushcfunction( L, Self::getMassWorldCenter );
			}
			break;
		case 20:
			{
				lua_pushcfunction( L, Self::getMassLocalCenter );
			}
			break;
		case 21:
			{
				lua_pushcfunction(L, Self::getWorldVector );
			}
			break;
		case 22:
			{
				lua_pushcfunction(L, Self::getInertia);
			}
			break;
		case 23:
			{
				lua_pushcfunction(L, Self::getLinearVelocityFromWorldPoint);
			}
			break;
		case 24:
			{
				lua_pushcfunction(L, Self::getLinearVelocityFromLocalPoint);
			}
			break;
		default:
			{
				result = 0;
			}
        }

        if (result == 0 && strcmp(key,"_properties") == 0)
        {
            String extensionProperties(LuaContext::GetRuntime( L )->Allocator());

            DumpObjectProperties( L, object, keys, numKeys, extensionProperties );

            lua_pushstring( L, extensionProperties.GetString() );

            result = 1;
        }
	}
#endif // Rtt_PHYSICS

	return result;
}

bool
DisplayObjectExtensions::SetValueForKey( lua_State *L, MLuaProxyable &, const char key[], int valueIndex ) const
{
	bool result = false;

	if ( ! key ) { return false; }

#ifdef Rtt_PHYSICS
	if ( fBody )
	{
		result = true;

		static const char * keys[] =
		{
			"isAwake",					// 0
			"isBodyActive",				// 1
			"isBullet",					// 2
			"isSleepingAllowed",		// 3
			"isFixedRotation",			// 4
			"angularVelocity",			// 5
			"linearDamping",			// 6
			"angularDamping",			// 7
			"bodyType",					// 8
			"isSensor",					// 9
			"gravityScale"				// 10
		};
		static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 11, 21, 2, __FILE__, __LINE__ );
		StringHash *hash = &sHash;

		int index = hash->Lookup( key );
		switch ( index )
		{
		case 0:
			{
				fBody->SetAwake( lua_toboolean( L, valueIndex ) );
			}
			break;
		case 1:
			{
				if ( ! LuaLibPhysics::IsWorldLocked( L, "display object property isBodyActive cannot be set" ) )
				{
					fBody->SetActive( lua_toboolean( L, valueIndex ) );
				}
			}
			break;
		case 2:
			{
				fBody->SetBullet( lua_toboolean( L, valueIndex ) );
			}
			break;
		case 3:
			{
				fBody->SetSleepingAllowed( lua_toboolean( L, valueIndex ) );
			}
			break;
		case 4:
			{
				fBody->SetFixedRotation( lua_toboolean( L, valueIndex ) );
			}
			break;
		case 5:
			{
				Real va = Rtt_RealDegreesToRadians( lua_tonumber( L, valueIndex ) );
				fBody->SetAngularVelocity( Rtt_RealToFloat( va ) );
			}
			break;
		case 6:
			{
				fBody->SetLinearDamping( lua_tonumber( L, valueIndex ) );
			}
			break;
		case 7:
			{
				fBody->SetAngularDamping( lua_tonumber( L, valueIndex ) );
			}
			break;
		case 8:
			{
				const char *bodyType = lua_tostring( L, valueIndex );

				if ( bodyType )
				{
					if ( strcmp( kStaticBodyType, bodyType ) == 0 )
					{
						fBody->SetType( b2_staticBody );
					}
					else if ( strcmp( kDynamicBodyType, bodyType ) == 0 )
					{
						fBody->SetType( b2_dynamicBody );
					}
					else if ( strcmp( kKinematicBodyType, bodyType ) == 0 )
					{
						fBody->SetType( b2_kinematicBody );
					}
					else
					{
						fBody->SetType( b2_dynamicBody );
					}
				}
			}
			break;
		case 9:
			{
				// Set all fixtures in the body (we call these "body elements") to the desired sensor state
				bool sensorState = lua_toboolean( L, valueIndex );
				for ( b2Fixture *f = fBody->GetFixtureList(); f; f = f->GetNext() )
				{
					f->SetSensor( sensorState );
				}
			}
			break;
		case 10:
			{
				fBody->SetGravityScale( lua_tonumber( L, valueIndex ) );
			}
			break;
		default:
			{
				result = false;
			}
		}
	}
#endif // Rtt_PHYSICS

	return result;
}

#ifdef Rtt_PHYSICS
void
DisplayObjectExtensions::SetBody( b2Body *body, b2World& world )
{
	if ( body == fBody )
	{
		// Nothing to do.
		return;
	}

	if ( fBody )
	{
		// Get rid of the previous body.
		world.DestroyBody( fBody );
	}

	fBody = body;
}

#endif // Rtt_PHYSICS

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

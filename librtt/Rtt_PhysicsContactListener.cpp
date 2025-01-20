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

#include "Rtt_PhysicsContactListener.h"

#include "Display/Rtt_DisplayObject.h"
#include "Rtt_Runtime.h"
#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PhysicsContact.h"
#include "Rtt_PhysicsWorld.h"

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PhysicsContactListener::PhysicsContactListener( Runtime& runtime )
:	fRuntime( runtime )
{
}

void
PhysicsContactListener::BeginContact(b2Contact* contact)
{
	const PhysicsWorld& physics = fRuntime.GetPhysicsWorld();

	if ( ! physics.IsProperty( PhysicsWorld::kCollisionListenerExists ) )
	{
		// Nothing to do.
		return;
	}

	const char phase[] = "began";
	
	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	
	size_t fixtureIndex1 = (size_t)fixtureA->GetUserData();
	size_t fixtureIndex2 = (size_t)fixtureB->GetUserData();
	
	b2Body *bodyA = fixtureA->GetBody();
	b2Body *bodyB = fixtureB->GetBody();
	
	DisplayObject *object1 = static_cast< DisplayObject* >( bodyA->GetUserData() );
	DisplayObject *object2 = static_cast< DisplayObject* >( bodyB->GetUserData() );

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	//// Lots of redundant code here.
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	////
	////
	// Get the out_position.
	b2Vec2 position( b2Vec2_zero );

	// It's possible for manifold->pointCount to be 0 (in the case of sensors).
	b2Manifold *manifold = contact->GetManifold();
	if( manifold->pointCount )
	{
		Real scale = physics.GetPixelsPerMeter();

		// "1": If we don't average all positions, then we only return the first one.
		int32 point_count = ( physics.GetAverageCollisionPositions() ? manifold->pointCount : 1 );

		if( physics.GetReportCollisionsInContentCoordinates() )
		{
			// Get the contact points in content-space.
			b2WorldManifold worldManifold;
			contact->GetWorldManifold( &worldManifold );

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += worldManifold.points[ i ];
			}
		}
		else
		{
			// Get the contact points in local-space.

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += manifold->points[ i ].localPoint;
			}
		}

		// Average.
		position *= ( 1.0f / (Real)point_count );

		// Scale.
		position *= scale;
	}
	////
	////
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	if ( object1 && ! object1->IsOrphan()
		 && object2 && ! object2->IsOrphan() )
	{
		UserdataWrapper *contactWrapper = PhysicsContact::CreateWrapper( fRuntime.VMContext().LuaState(), contact );
		{
			CollisionEvent e( * object1, * object2, position.x, position.y, (int) fixtureIndex1, (int) fixtureIndex2, phase );
			e.SetContact( contactWrapper );

			fRuntime.DispatchEvent( e );
		}
		contactWrapper->Invalidate();
	}
}

void
PhysicsContactListener::EndContact(b2Contact* contact)
{
	const PhysicsWorld& physics = fRuntime.GetPhysicsWorld();

	if ( ! physics.IsProperty( PhysicsWorld::kCollisionListenerExists ) )
	{
		// Nothing to do.
		return;
	}
	
	const char phase[] = "ended";

	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	
	size_t fixtureIndex1 = (size_t)fixtureA->GetUserData();
	size_t fixtureIndex2 = (size_t)fixtureB->GetUserData();
	
	b2Body *bodyA = fixtureA->GetBody();
	b2Body *bodyB = fixtureB->GetBody();
	
	DisplayObject *object1 = static_cast< DisplayObject* >( bodyA->GetUserData() );
	DisplayObject *object2 = static_cast< DisplayObject* >( bodyB->GetUserData() );

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	//// Lots of redundant code here.
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	////
	////
	// Get the out_position.
	b2Vec2 position( b2Vec2_zero );

	// It's possible for manifold->pointCount to be 0 (in the case of sensors).
	b2Manifold *manifold = contact->GetManifold();
	if( manifold->pointCount )
	{
		Real scale = physics.GetPixelsPerMeter();

		// "1": If we don't average all positions, then we only return the first one.
		int32 point_count = ( physics.GetAverageCollisionPositions() ? manifold->pointCount : 1 );

		if( physics.GetReportCollisionsInContentCoordinates() )
		{
			// Get the contact points in content-space.
			b2WorldManifold worldManifold;
			contact->GetWorldManifold( &worldManifold );

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += worldManifold.points[ i ];
			}
		}
		else
		{
			// Get the contact points in local-space.

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += manifold->points[ i ].localPoint;
			}
		}

		// Average.
		position *= ( 1.0f / (Real)point_count );

		// Scale.
		position *= scale;
	}
	////
	////
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	if ( object1 && ! object1->IsOrphan()
		 && object2 && ! object2->IsOrphan() )
	{
		UserdataWrapper *contactWrapper = PhysicsContact::CreateWrapper( fRuntime.VMContext().LuaState(), contact );
		{
			CollisionEvent e( * object1, * object2, position.x, position.y, (int) fixtureIndex1, (int) fixtureIndex2, phase );
			e.SetContact( contactWrapper );

			fRuntime.DispatchEvent( e );
		}
		contactWrapper->Invalidate();
	}
}

void
PhysicsContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	const PhysicsWorld& physics = fRuntime.GetPhysicsWorld();

	if ( ! physics.IsProperty( PhysicsWorld::kPreCollisionListenerExists ) )
	{
		// Nothing to do.
		return;
	}

	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	
	size_t fixtureIndex1 = (size_t)fixtureA->GetUserData();
	size_t fixtureIndex2 = (size_t)fixtureB->GetUserData();
	
	b2Body *bodyA = fixtureA->GetBody();
	b2Body *bodyB = fixtureB->GetBody();
	
	DisplayObject *object1 = static_cast< DisplayObject* >( bodyA->GetUserData() );
	DisplayObject *object2 = static_cast< DisplayObject* >( bodyB->GetUserData() );

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	//// Lots of redundant code here.
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	////
	////
	// Get the out_position.
	b2Vec2 position( b2Vec2_zero );

	// It's possible for manifold->pointCount to be 0 (in the case of sensors).
	b2Manifold *manifold = contact->GetManifold();
	if( manifold->pointCount )
	{
		Real scale = physics.GetPixelsPerMeter();

		// "1": If we don't average all positions, then we only return the first one.
		int32 point_count = ( physics.GetAverageCollisionPositions() ? manifold->pointCount : 1 );

		if( physics.GetReportCollisionsInContentCoordinates() )
		{
			// Get the contact points in content-space.
			b2WorldManifold worldManifold;
			contact->GetWorldManifold( &worldManifold );

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += worldManifold.points[ i ];
			}
		}
		else
		{
			// Get the contact points in local-space.

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += manifold->points[ i ].localPoint;
			}
		}

		// Average.
		position *= ( 1.0f / (Real)point_count );

		// Scale.
		position *= scale;
	}
	////
	////
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	if ( object1 && ! object1->IsOrphan()
		 && object2 && ! object2->IsOrphan() )
	{
		UserdataWrapper *contactWrapper = PhysicsContact::CreateWrapper( fRuntime.VMContext().LuaState(), contact );
		{
			PreCollisionEvent e( * object1, * object2, position.x, position.y, (int) fixtureIndex1, (int) fixtureIndex2);
			e.SetContact( contactWrapper );

			fRuntime.DispatchEvent( e );
		}
		contactWrapper->Invalidate();
	}
}

void
PhysicsContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
	const PhysicsWorld& physics = fRuntime.GetPhysicsWorld();

	if ( ! physics.IsProperty( PhysicsWorld::kPostCollisionListenerExists ) )
	{
		// Nothing to do.
		return;
	}

	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	
	size_t fixtureIndex1 = (size_t)fixtureA->GetUserData();
	size_t fixtureIndex2 = (size_t)fixtureB->GetUserData();
	
	b2Body *bodyA = fixtureA->GetBody();
	b2Body *bodyB = fixtureB->GetBody();

	DisplayObject *object1 = static_cast< DisplayObject* >( bodyA->GetUserData() );
	DisplayObject *object2 = static_cast< DisplayObject* >( bodyB->GetUserData() );

	float32 maxNormalImpulse = 0.0f;
	float32 maxTangentImpulse = 0.0f;

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	//// Lots of redundant code here.
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	////
	////
	// Get the out_position.
	b2Vec2 position( b2Vec2_zero );

	// It's possible for manifold->pointCount to be 0 (in the case of sensors).
	b2Manifold *manifold = contact->GetManifold();
	if( manifold->pointCount )
	{
		Real scale = physics.GetPixelsPerMeter();

		// "1": If we don't average all positions, then we only return the first one.
		int32 point_count = ( physics.GetAverageCollisionPositions() ? manifold->pointCount : 1 );

		if( physics.GetReportCollisionsInContentCoordinates() )
		{
			// Get the contact points in content-space.
			b2WorldManifold worldManifold;
			contact->GetWorldManifold( &worldManifold );

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += worldManifold.points[ i ];
			}
		}
		else
		{
			// Get the contact points in local-space.

			// Sum.
			for ( int32 i = 0;
					i < point_count;
					++i )
			{
				position += manifold->points[ i ].localPoint;
			}
		}

		// Average.
		position *= ( 1.0f / (Real)point_count );

		// Scale.
		position *= scale;
	////
	////
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

		// For the contact forces, we take the maximum within each set
		int32 count = contact->GetManifold()->pointCount;	
		for (int32 i = 0; i < count; ++i) {
			maxNormalImpulse = b2Max( maxNormalImpulse, impulse->normalImpulses[i] );
			maxTangentImpulse = b2Max( maxTangentImpulse, impulse->tangentImpulses[i] );
		}
	}

	if ( object1 && ! object1->IsOrphan()
		 && object2 && ! object2->IsOrphan() )
	{
		UserdataWrapper *contactWrapper = PhysicsContact::CreateWrapper( fRuntime.VMContext().LuaState(), contact );
		{
			PostCollisionEvent e( * object1, * object2, position.x, position.y, (int) fixtureIndex1, (int) fixtureIndex2, Rtt_FloatToReal( maxNormalImpulse ), Rtt_FloatToReal( maxTangentImpulse ) );
			e.SetContact( contactWrapper );

			fRuntime.DispatchEvent( e );
		}
		contactWrapper->Invalidate();
	}
}

void PhysicsContactListener::BeginContact( b2ParticleSystem *particleSystem,
											b2ParticleBodyContact *particleBodyContact )
{
	const PhysicsWorld& physics = fRuntime.GetPhysicsWorld();

	if( ! physics.IsProperty( PhysicsWorld::kParticleCollisionListenerExists ) )
	{
		// Nothing to do.
		return;
	}

	fRuntime.DispatchEvent( BeginParticleCollisionEvent( fRuntime,
															particleSystem,
															particleBodyContact ) );
}

void PhysicsContactListener::EndContact( b2Fixture *fixture,
											b2ParticleSystem *particleSystem,
											int32 particleIndex )
{
	const PhysicsWorld& physics = fRuntime.GetPhysicsWorld();

	if( ! physics.IsProperty( PhysicsWorld::kParticleCollisionListenerExists ) )
	{
		// Nothing to do.
		return;
	}
	
	fRuntime.DispatchEvent( EndParticleCollisionEvent( fRuntime,
														fixture,
														particleSystem,
														particleIndex ) );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS	

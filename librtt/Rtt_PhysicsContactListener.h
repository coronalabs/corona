//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_PhysicsContactListener__
#define __Rtt_PhysicsContactListener__

#ifdef Rtt_PHYSICS	

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Runtime;
class DisplayObject;

// ----------------------------------------------------------------------------

class PhysicsContactListener : public b2ContactListener
{
	public:
		PhysicsContactListener( Runtime& runtime );

	public:
		// b2ContactListener
		// Fixture <-> Fixture contact.
		virtual void BeginContact(b2Contact* contact);
		virtual void EndContact(b2Contact* contact);
		virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
		virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);

		// b2ContactListener
		// Fixture <-> Particle contact.
		virtual void BeginContact( b2ParticleSystem *particleSystem,
									b2ParticleBodyContact *particleBodyContact );
		virtual void EndContact( b2Fixture *fixture,
									b2ParticleSystem *particleSystem,
									int32 particleIndex );

	private:

		bool GetCollisionParams( b2Contact* contact,
									DisplayObject *&out_object1,
									DisplayObject *&out_object2,
									b2Vec2 &out_position,
									size_t &out_fixtureIndex1,
									size_t &out_fixtureIndex2 );

		Runtime& fRuntime;
};


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS	

#endif // __Rtt_PhysicsContactListener__

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

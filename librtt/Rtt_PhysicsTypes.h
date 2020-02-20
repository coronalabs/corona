//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_PhysicsTypes_H__
#define _Rtt_PhysicsTypes_H__

// ----------------------------------------------------------------------------

#ifdef __cplusplus

// ----------------------------------------------------------------------------

#include <queue>
#include <vector>

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

typedef std::vector<b2Vec2> b2Vec2Vector;
typedef std::vector<b2Vec2Vector> b2Vec2VectorVector;
typedef std::queue<b2Vec2Vector> b2Vec2VectorQueue;
typedef std::vector< b2PolygonShape * > b2PolygonShapePtrVector;
typedef std::vector< b2PolygonShape * >::iterator b2PolygonShapePtrVectorIter;

// ----------------------------------------------------------------------------

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // _Rtt_PhysicsTypes_H__

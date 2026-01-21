#ifndef _b2Seperator_H__
#define _b2Seperator_H__

// ----------------------------------------------------------------------------

#ifdef __cplusplus

// ----------------------------------------------------------------------------

#include "Rtt_PhysicsTypes.h"

// ----------------------------------------------------------------------------

class b2Separator {
    
public:
    
    b2Separator() {}
        
		/**
		 * Separates a non-convex polygon into convex polygons and adds them as fixtures to the <code>body</code> parameter.<br/>
		 * There are some rules you should follow (otherwise you might get unexpected results) :
		 * <ul>
		 * <li>This class is specifically for non-convex polygons. If you want to create a convex polygon, you don't need to use this class - Box2D's <code>b2PolygonShape</code> class allows you to create convex shapes with the <code>setAsArray()</code>/<code>setAsVector()</code> method.</li>
		 * <li>The vertices must be in clockwise order.</li>
		 * <li>No three neighbouring points should lie on the same line segment.</li>
		 * <li>There must be no overlapping segments and no "holes".</li>
		 * </ul> <p/>
		 * @param body The b2Body, in which the new fixtures will be stored.
		 * @param fixtureDef A b2FixtureDef, containing all the properties (friction, density, etc.) which the new fixtures will inherit.
		 * @param verticesVec The vertices of the non-convex polygon, in clockwise order.
		 * @param scale <code>[optional]</code> The scale which you use to draw shapes in Box2D. The bigger the scale, the better the precision. The default value is 30.
		 * @see b2PolygonShape
		 * @see b2PolygonShape.SetAsArray()
		 * @see b2PolygonShape.SetAsVector()
		 * @see b2Fixture
		 **/
	typedef void (*FixtureCreator_t)( b2Body *body,
										b2FixtureDef *fixtureDef,
										int &fixtureIndex );

	// "translate" is applied first, then "scale", to all vertices in vertices_vec.
	bool SeparateAndCreateFixtures( b2Body *body,
									b2FixtureDef *fixtureDef,
									int &fixtureIndex,
									FixtureCreator_t fixture_creator,
									b2Vec2Vector &vertices_vec,
									b2Vec2 &translate,
									b2Vec2 &scale, bool needHull);

		/**
		 * Checks whether the vertices in <code>verticesVec</code> can be properly distributed into the new fixtures (more specifically, it makes sure there are no overlapping segments and the vertices are in clockwise order).
		 * It is recommended that you use this method for debugging only, because it may cost more CPU usage.
		 * <p/>
		 * @param verticesVec The vertices to be validated.
		 * @return A boolean which can have the following values:
		 * <ul>
		 * <li>true if ok.</li>
		 * <li>false if there was an error.</li>
		 * </ul>
		 * */
        
    int Validate(const b2Vec2Vector& verticesVec);

    void calcShapes(b2Vec2Vector &pVerticesVec, b2Vec2VectorVector &result);

private:

	bool hitRay( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, b2Vec2 *optionalOutputVec );
	bool hitSegment( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, b2Vec2 *optionalOutputVec );
	bool isOnSegment(float px, float py, float x1, float y1, float x2, float y2);
    bool pointsMatch(float x1, float y1, float x2,float y2);
    bool isOnLine(float px, float py, float x1, float y1, float x2, float y2);
    float det( float x1, float y1, float x2, float y2, float x3, float y3);
};

// ----------------------------------------------------------------------------

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // _b2Seperator_H__

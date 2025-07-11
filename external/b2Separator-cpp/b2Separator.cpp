#include "b2Separator.h"

// ----------------------------------------------------------------------------

#include <algorithm> // std::reverse().

// ----------------------------------------------------------------------------

# define ENABLE_DEBUG_PRINT	( 0 )
#
# if ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )	printf( __VA_ARGS__ )
#
# else // Not ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )
#
# endif // ENABLE_DEBUG_PRINT

#define MAX_VALUE 2147483647

// ----------------------------------------------------------------------------

//! \TODO CoronaSDK: Rtt_LogException() is from "Rtt_Assert.h".
//! If the original function changes, we need to keep the
//! prototype here in sync. A better solution is to have a small
//! common header we can include here.
extern "C" int Rtt_LogException( const char *format, ... );

// ----------------------------------------------------------------------------

bool b2Separator::SeparateAndCreateFixtures( b2Body *body,
												b2FixtureDef *fixtureDef,
												int &fixtureIndex,
												FixtureCreator_t fixture_creator,
												b2Vec2Vector &vertices_vec,
												b2Vec2 &translate,
												b2Vec2 &scale,
                                                bool needHull
                                            )
{
	bool vertices_added = false;

	DEBUG_PRINT( "*** %s() : Scale: %f, %f Translate: %f, %f\n",
					__FUNCTION__,
					translate.x,
					translate.y,
					scale.x,
					scale.y );

	b2PolygonShape polyShape;
	fixtureDef->shape = &polyShape;

	// Tesselate the outline.
	{
		b2Vec2VectorVector figsVec;
		calcShapes( vertices_vec, figsVec );

		DEBUG_PRINT( "*** %s()\nnumber of bodies tesselated: %d\n",
						__FUNCTION__,
						figsVec.size() );

	    for( size_t i = 0;
				i < figsVec.size();
				++i )
		{
		    b2Vec2Vector &vertices = figsVec[i];

	        size_t m = vertices.size();

			// Verify the vertex count is valid.
			if( m < 3 )
			{
				// Discard.
				DEBUG_PRINT( "skipped shape index: %d\n", i );
				continue;
			}

			if( m > b2_maxPolygonVertices )
			{
				// Clamp the maximum number of vertices to b2_maxPolygonVertices.
				// THIS SHOULD NEVER HAPPEN, BUT IT DOES!!!
				DEBUG_PRINT( "clamp the number of vertices in shape index: %d\n", i );
				m = b2_maxPolygonVertices;
			}

			// Scale and translate all the vertices in figsVec.
	        for( size_t j = 0;
					j < m;
					++j )
			{
				b2Vec2 &v = vertices[j];

				v += translate;
				v.x *= scale.x;
				v.y *= scale.y;

				DEBUG_PRINT( "*** Fixture %03d : Vertex %03d : x, y: %f, %f\n", i, j, v.x, v.y );
	        }

	        bool ok = polyShape.Set( &( vertices[ 0 ] ), (int)m, needHull);
			if( ! ok )
			{
				// Set() failed. Skip this polygon.
				DEBUG_PRINT( "skipped shape index: %d\n", i );
				continue;
			}

			// Create the fixtures.
			fixture_creator( body,
								fixtureDef,
								fixtureIndex );

			vertices_added = true;
	    }
	}

	fixtureDef->shape = NULL;

	return vertices_added;
}
		/**
		 * Checks whether the vertices in can be properly distributed into the new fixtures (more specifically, it makes sure there are no overlapping segments and the vertices are in clockwise order). 
		 * It is recommended that you use this method for debugging only, because it may cost more CPU usage.
		 * @param verticesVec The vertices to be validated.
		 * @return An integer which can have the following values:
		 * 0 if the vertices can be properly processed.
		 * 1 If there are overlapping lines.
		 * 2 if the points are <b>not</b> in clockwise order.
		 * 3 if there are overlapping lines and the points are not in clockwise order.
		 * */        
int b2Separator::Validate(const b2Vec2Vector &verticesVec) {
    int i, n = (int) verticesVec.size(), ret = 0;
	float j, j2, i2, i3, d;
    bool fl, fl2=false;
    
    for (i=0; i<n; i++) {
        i2=(i<n-1)?i+1:0;
        i3=(i>0)?i-1:n-1;
        
        fl=false;
        for (j=0; j<n; j++) {
            if ((j!=i)&&(j!=i2)) {
                if (! fl) {
                    d = det(verticesVec[i].x,verticesVec[i].y,verticesVec[i2].x,verticesVec[i2].y,verticesVec[j].x,verticesVec[j].y);
                    if ((d>0)) {
                        fl=true;
                    }
                }
                
                if ((j!=i3)) {
                    j2=(j<n-1)?j+1:0;
                    if (hitSegment(verticesVec[i].x,verticesVec[i].y,verticesVec[i2].x,verticesVec[i2].y,verticesVec[j].x,verticesVec[j].y,verticesVec[j2].x,verticesVec[j2].y, NULL)) {
                        ret=1; // TODO: This may be wrong!!!
                    }
                }
            }
        }
        
        if (! fl) {
            fl2=true;
        }
    }
    
    if (fl2) {
        if (ret==1) {
            ret=3;
        }
        else {
            ret=2;
        }
        
    }
    return ret;
}
        
void b2Separator::calcShapes(b2Vec2Vector &pVerticesVec, b2Vec2VectorVector &result) {
    b2Vec2Vector vec;
    int i, n, j,minLen;
    float d, t, dx, dy;
    int i1, i2, i3;
    b2Vec2 p1, p2, p3;
    int j1, j2;
    b2Vec2 v1, v2;
    int k=0, h=0;
    b2Vec2 hitV(0,0);
    bool isConvex;
    b2Vec2VectorVector figsVec;
    b2Vec2VectorQueue queue;
    
    queue.push(pVerticesVec);

	size_t limit_iteration_count = 0;

    while (!queue.empty()) {

		if( queue.size() > 64 )
		{
			// 64: This is an arbitrary threshold.
			// Early-out to avoid problems.
			Rtt_LogException( "WARNING: The generation of the physics body for this outline had to be cut short. Please provide a simpler input.\n" );

			break;
		}

        vec = queue.front();
        n = (int) vec.size();
        isConvex=true;
        
        for (i=0; i<n; i++) {
            i1=i;
            i2=(i<n-1)?i+1:i+1-n;
            i3=(i<n-2)?i+2:i+2-n;
            
            p1 = vec[i1];
            p2 = vec[i2];
            p3 = vec[i3];
            
            d = det(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
            if ((d<0)) {
                isConvex=false;
                minLen = MAX_VALUE;
                
                for (j=0; j<n; j++) {
                    if ((j!=i1)&&(j!=i2)) {
                        j1=j;
                        j2=(j<n-1)?j+1:0;
                        
                        v1=vec[j1];
                        v2=vec[j2];

						b2Vec2 v;
                        if (hitRay(p1.x,p1.y,p2.x,p2.y,v1.x,v1.y,v2.x,v2.y, &v)) {
                            dx=p2.x-v.x;
                            dy=p2.y-v.y;
                            t=dx*dx+dy*dy;

                            if ((t<minLen)) {
                                h=j1;
                                k=j2;
                                hitV=v;
                                minLen=t;
                            }
                        }
                    }
                }
                
                if (minLen==MAX_VALUE) {
                    //TODO: Throw Error !!!
                }

                b2Vec2Vector *vec1 = new b2Vec2Vector();
                b2Vec2Vector *vec2 = new b2Vec2Vector();
                
                j1=h;
                j2=k;
                v1=vec[j1];
                v2=vec[j2];
                
                if (! pointsMatch(hitV.x,hitV.y,v2.x,v2.y)) {
                    vec1->push_back(hitV);
                }
                if (! pointsMatch(hitV.x,hitV.y,v1.x,v1.y)) {
                    vec2->push_back(hitV);
                }
                
                h=-1;
                k=i1;

				// This is a HACK to prevent the code from hanging.
				limit_iteration_count = 0;

                while (true) {

					if( ++limit_iteration_count > 128 )
					{
						// 128: This is an arbitrary threshold.
						// Early-out to avoid problems.
						break;
					}

                    if ((k!=j2)) {
                        vec1->push_back(vec[k]);
                    }
                    else {
                        if (((h<0)||h>=n)) {
                            //TODO: Throw Error !!!
							//assert( ! "This happens with large objects and small \"coarsness\" values." );
							break;
                        }
                        if (! isOnSegment(v2.x,v2.y,vec[h].x,vec[h].y,p1.x,p1.y)) {
                            vec1->push_back(vec[k]);
                        }
                        break;
                    }
                    
                    h=k;
                    if (((k-1)<0)) {
                        k=n-1;
                    }
                    else {
                        k--;
                    }
                }
                
                std::reverse(vec1->begin(), vec1->end());
                
                h=-1;
                k=i2;

				// This is a HACK to prevent the code from hanging.
				limit_iteration_count = 0;

                while (true) {

					if( ++limit_iteration_count > 128 )
					{
						// 128: This is an arbitrary threshold.
						// Early-out to avoid problems.
						break;
					}

                    if ((k!=j1)) {
                        vec2->push_back(vec[k]);
                    }
                    else {
                        if (((h<0)||h>=n)) {
                            //TODO: Throw Error !!!
							//assert( ! "This happens with large objects and small \"coarsness\" values." );
							break;
                        }
                        if (((k==j1)&&! isOnSegment(v1.x,v1.y,vec[h].x,vec[h].y,p2.x,p2.y))) {
                            vec2->push_back(vec[k]);
                        }
                        break;
                    }
                    
                    h=k;
                    if (((k+1)>n-1)) {
                        k=0;
                    }
                    else {
                        k++;
                    }
                }
                
                queue.push(*vec1);
                queue.push(*vec2);
                queue.pop();

                delete vec1;
                delete vec2;

                break;
            }
        }
        
        if (isConvex) {
            figsVec.push_back(queue.front());
            queue.pop();
        }
    }
    result = figsVec;
}
        
bool b2Separator::hitRay( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, b2Vec2 *optionalOutputVec )
{
    float t1 = x3-x1;
    float t2 = y3-y1;
    float t3 = x2-x1;
    float t4 = y2-y1;
    float t5 = x4-x3;
    float t6 = y4-y3;
    float t7 = t4*t5-t3*t6;

    //DBZ Error. Undefined hit segment.
    if(t7 == 0) return NULL;
    
    float  a = (((t5*t2) - t6*t1) / t7);
    float px = x1+a*t3;
    float py = y1+a*t4;
    bool b1 = isOnSegment(x2,y2,x1,y1,px,py);
    bool b2 = isOnSegment(px,py,x3,y3,x4,y4);
    
	bool hit = ( b1 && b2 );

	if( hit && optionalOutputVec )
	{
		optionalOutputVec->Set( px, py );
	}

	return hit;
}

bool b2Separator::hitSegment( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, b2Vec2 *optionalOutputVec )
{
    float t1 = x3-x1;
    float t2 = y3-y1;
    float t3 = x2-x1;
    float t4 = y2-y1;
    float t5 = x4-x3;
    float t6 = y4-y3;
    float t7 = t4*t5 - t3*t6;
    
    //DBZ Error. Undefined hit segment.
    if(t7 == 0) return NULL;
    
    float  a = (((t5*t2) - t6*t1) / t7);
    float px = x1+a*t3;
    float py = y1+a*t4;
    bool b1 = isOnSegment(px,py,x1,y1,x2,y2);
    bool b2 = isOnSegment(px,py,x3,y3,x4,y4);

	bool hit = ( b1 && b2 );

	if( hit && optionalOutputVec )
	{
		optionalOutputVec->Set( px, py );
	}

	return hit;
}

bool b2Separator::isOnSegment(float px, float py, float x1, float y1, float x2, float y2) {
    bool b1 = ((x1+0.1>=px&&px>=x2-0.1)||(x1-0.1<=px&&px<=x2+0.1));
    bool b2 = ((y1+0.1>=py&&py>=y2-0.1)||(y1-0.1<=py&&py<=y2+0.1));
    return (b1&&b2&&isOnLine(px, py, x1, y1, x2, y2));
}
        
bool b2Separator::pointsMatch(float x1 ,float y1 ,float x2 ,float y2) {
    float dx = (x2>=x1) ? x2-x1 : x1-x2;
    float dy = (y2>=y1) ? y2-y1 : y1-y2;
    return ((dx<0.1f) && dy<0.1f);
}
        
bool b2Separator::isOnLine(float px ,float py ,float x1 ,float y1 ,float x2 ,float y2) {
    if(x2-x1>0.1f||x1-x2>0.1f) {
        float a=(y2-y1)/(x2-x1);
		float possibleY=a*(px-x1)+y1;
		float diff=(possibleY>py?possibleY-py:py-possibleY);
        return (diff<0.1f);
    }
    return (px-x1<0.1f||x1-px<0.1f);
}
        
float b2Separator::det(float x1, float y1, float x2, float y2, float x3, float y3) {
			return x1*y2+x2*y3+x3*y1-y1*x2-y2*x3-y3*x1;
}
        
//		private function err():void {
//			throw new Error("A problem has occurred. Use the Validate() method to see where the problem is.");
//		}

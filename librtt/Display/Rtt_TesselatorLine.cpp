//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TesselatorLine.h"

#include "Rtt_Matrix.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TesselatorLine::TesselatorLine( const ArrayVertex2& controlPoints, Mode mode )
:	Super(),
	fControlPoints( controlPoints ),
	fMode( mode )
{
}

U32
TesselatorLine::StrokeVertexCount() const
{
	return VertexCountFromPoints( fControlPoints, false );
}

U32
TesselatorLine::VertexCountFromPoints( const ArrayVertex2& vertices, bool isClosed )
{
	return VertexCountFromPoints( vertices.Length(), isClosed );
}

U32
TesselatorLine::VertexCountFromPoints( U32 count, bool isClosed )
{
	/*
	if (isClosed)
	{
	}
	*/

	return (count + 1) * 2U;
}


static bool
Normalize( Real& x, Real& y )
{
// TODO: Find a non-float alternative
#if defined( Rtt_REAL_FIXED )
	float xFloat = (float)x;
	float yFloat = (float)y;
	float rFloat = Rtt_FloatSqrt( xFloat*xFloat + yFloat*yFloat ) / 65536.0;
	Real length = Rtt_FloatToReal( rFloat );

	#ifdef Rtt_DEBUG
	{
		float xx = Rtt_RealToFloat( x );
		float yy = Rtt_RealToFloat( y );
		float ll = Rtt_FloatSqrt( xx*xx + yy*yy );
		Real lll = Rtt_FloatToReal( ll );
		Real delta = ( lll > length ? lll - length : length - lll );
		Rtt_ASSERT( delta <= Rtt_FIXED_EPS );
	}
	#endif

#else
	Real length = Rtt_RealSqrt( Rtt_RealMul( x, x ) + Rtt_RealMul( y, y ) );
#endif

	bool result = ( length >= Rtt_REAL_0 );

	if ( result )
	{
		Real invLength = Rtt_RealDivNonZeroAB( Rtt_REAL_1, length );
		x = Rtt_RealMul( x, invLength );
		y = Rtt_RealMul( y, invLength );
	}

	return result;
}

static Vertex2
ProjectVertex( const Vertex2& v, Real dx, Real dy, Real w, bool isInner )
{
	Real x = v.x;
	Real y = v.y;

	if ( ! isInner )
	{
		w = -w;
	}

	Vertex2 result = { x + Rtt_RealMul( dx, w ), y + Rtt_RealMul( dy, w ) };
	return result;
}

static S32
AddBoundaryPoints( ArrayVertex2& dst, const Vertex2& v, Real dx, Real dy, Real innerWidth, Real outerWidth )
{
	Real x = v.x;
	Real y = v.y;

	Vertex2 endPtInner = { x + Rtt_RealMul( dx, innerWidth ), y + Rtt_RealMul( dy, innerWidth ) };
	Vertex2 endPtOuter = { x - Rtt_RealMul( dx, outerWidth ), y - Rtt_RealMul( dy, outerWidth ) };
	dst.Append( endPtInner );
	dst.Append( endPtOuter );

	return 2;
}

static S32
StrokeSegment(
	ArrayVertex2& dstVertices,
	const Vertex2 *v1,
	const Vertex2 *v2,
	Real& dx,
	Real& dy,
	Real& x1_n,
	Real& y1_n, 
	Real innerWidth,
	Real outerWidth )
{
	S32 numAdded = -1;

	Real dxOrig = dx;
	Real dyOrig = dy;

	// Vector p2(x2,y2)
	Real x2 = v2->x - v1->x;
	Real y2 = v2->y - v1->y;

	Real r2Squared = Rtt_RealMul( x2, x2 ) + Rtt_RealMul( y2, y2 );

	// TODO: Need to exit both loops
	if ( Rtt_VERIFY( Normalize( x2, y2 ) ) )
	{
		// Rotations axis (+/- z-axis) is determined by cross product.
		// Positive z points into screen (b/c positive y is down)
		// If x2*y1 - x1*y2 <= 0, it's positive-z; otherwise negative-z
		// Here, we do x2*(-y1) - (-x1)*(y2) <= 0
		bool isPositiveZ = ( Rtt_RealMul( x1_n, y2 ) - Rtt_RealMul( x2, y1_n ) ) <= 0;

		// DotProduct( p1, p2 ) = - DotProduct( -p1, p2 )
		Real p1p2 = -(Rtt_RealMul( x1_n, x2 ) + Rtt_RealMul( y1_n, y2 ));
		Real cosTheta = Rtt_RealDiv2( Rtt_REAL_1 + p1p2 );
		Real innerR = innerWidth;
		Real outerR = outerWidth;
		if ( Rtt_RealIsZero( cosTheta ) )
		{
			// v0, v1, v2 are collinear
			dx = y2;
			dy = -x2;
		}
		else
		{
			Real sinTheta = Rtt_RealDiv2( Rtt_REAL_1 - p1p2 );

			numAdded = 0;
	
			cosTheta = Rtt_RealSqrt( cosTheta );
			sinTheta = Rtt_RealSqrt( sinTheta );

			// When angle between 2 vectors is less than 30 degrees, we should avoid
			// miter joints.  The angle "theta" is half the angle between the two vectors,
			// so only when cosTheta < cos(15) should we do miter joints.
			const float kMiterJointThreshold = 0.965925826289068f;
			bool shouldAddMiterJoint = ( cosTheta < kMiterJointThreshold );

			// Additional threshold when the angle between the 2 vectors < 6 degrees.
			// which corresponds to cosTheta > cos( 3 )
			const float kDegenerateThreshold = 0.998629534754574f;
			bool isDegenerate = ( cosTheta > kDegenerateThreshold );

			if ( ! isDegenerate )
			{
				if ( ! isPositiveZ )
				{
					cosTheta = -cosTheta;
				}

				// n is 1 for vector(x1,y1) and -1 for vector(x2,y2)
				dx = -Rtt_RealMul( cosTheta, x1_n ) + Rtt_RealMul( sinTheta, y1_n ); // dx = (cosTheta, -n*sinTheta).(vector)
				dy = -Rtt_RealMul( sinTheta, x1_n ) - Rtt_RealMul( cosTheta, y1_n ); // dy = (n*sinTheta,  cosTheta).(vector)

				innerR = Rtt_RealDiv( innerR, sinTheta );
				outerR = Rtt_RealDiv( outerR, sinTheta );

				// Final threshold: projected length used to find intersection point cannot be too large
				isDegenerate =
					( Rtt_RealMul( innerR, innerR ) > r2Squared )
					|| ( Rtt_RealMul( outerR, outerR ) > r2Squared );
			}

			if ( ! isDegenerate )
			{
				if ( shouldAddMiterJoint )
				{
					numAdded += AddBoundaryPoints( dstVertices, * v1, dx, dy, innerR, outerR );
				}
				else
				{
					if ( isPositiveZ )
					{
						Vertex2 a = ProjectVertex( * v1, dx, dy, innerR, true );
						Vertex2 b = ProjectVertex( * v1, y1_n, -x1_n, outerWidth, false );

						dstVertices.Append( a );
						dstVertices.Append( b );
						numAdded += 2;
						
						b = ProjectVertex( * v1, y2, -x2, outerWidth, false );
						dstVertices.Append( a );
						dstVertices.Append( b );
						numAdded += 2;
					}
					else
					{
						Vertex2 a = ProjectVertex( * v1, y1_n, -x1_n, innerWidth, true );
						Vertex2 b = ProjectVertex( * v1, dx, dy, outerR, false );

						dstVertices.Append( a );
						dstVertices.Append( b );
						numAdded += 2;
						
						a = ProjectVertex( * v1, y2, -x2, outerWidth, true );
						dstVertices.Append( a );
						dstVertices.Append( b );
						numAdded += 2;
					}
				}
			}
			else
			{
				numAdded += AddBoundaryPoints( dstVertices, * v1, dxOrig, dyOrig, innerWidth, outerWidth );
				numAdded += AddBoundaryPoints( dstVertices, * v1, y2, -x2, innerWidth, outerWidth );
			}
		}

		dx = y2;
		dy = -x2;
	//				numAdded += AddBoundaryPoints( dstVertices, * v1, dx, dy, innerWidth, outerWidth );

		v1 = v2;
		x1_n = x2;
		y1_n = y2;
	}

	return numAdded;
}

void
TesselatorLine::GenerateStrokeLine( ArrayVertex2& vertices )
{
	const Vertex2 *controlVertices = fControlPoints.ReadAccess();

	ArrayVertex2& dstVertices = vertices;

	const Real innerWidth = fInnerWidth;
	const Real outerWidth = fOuterWidth;

	const S32 numControlPoints = fControlPoints.Length();

	if ( numControlPoints > 1 )
	{
		S32 numAdded = 0;

		const Vertex2* v0 = & controlVertices[0];
		const Vertex2* v1 = & controlVertices[1];

		// Vector p1(x1,y1). Here, we actually are calculating -p1(x1_n,y1_n)
		Real x1_n = v1->x - v0->x;
		Real y1_n = v1->y - v0->y;
		if ( ! Rtt_VERIFY( Normalize( x1_n, y1_n ) ) )
		{
			// Give up!
			goto exit_gracefully;
		}

		Real dx = y1_n; // The "left" normal to vector p1
		Real dy = -x1_n;

		// First endpoint
		numAdded += AddBoundaryPoints( dstVertices, * v0, dx, dy, innerWidth, outerWidth );

		const Vertex2* v2 = NULL;
		if ( numControlPoints < 3 )
		{
			// Only the endpoints v0 and v2 exist. There is no v1
			v2 = v1;
		}
		else
		{
			// Handle all intermediate points
			for ( S32 j = 2; j < numControlPoints; j++ )
			{
//				numAdded += AddBoundaryPoints( dstVertices, * v1, dx, dy, innerWidth, outerWidth );
				v2 = & controlVertices[j];
				S32 numPoints = StrokeSegment( dstVertices, v1, v2, dx, dy, x1_n, y1_n, innerWidth, outerWidth );

				if ( numPoints > 0 )
				{
					numAdded += numPoints;
				}
				v1 = v2;
			}
		}

		// Last endpoint
		numAdded += AddBoundaryPoints( dstVertices, * v2, dx, dy, innerWidth, outerWidth );

		controlVertices += numControlPoints;
	}

exit_gracefully:
	return;
}

void
TesselatorLine::GenerateStrokeLoop( ArrayVertex2& vertices )
{
	const Vertex2 *controlVertices = fControlPoints.ReadAccess();

	ArrayVertex2& dstVertices = vertices;

	const Real innerWidth = fInnerWidth;
	const Real outerWidth = fOuterWidth;

	const S32 numControlPoints = fControlPoints.Length();

	// Loop requires at least 3 pts
	if ( numControlPoints > 2 )
	{
		S32 numAdded = 0;

		const Vertex2* v0 = & controlVertices[numControlPoints - 2];
		const Vertex2* v1 = & controlVertices[numControlPoints - 1];
		const Vertex2* v2;

		// Base case: p1's normal is based on the previous ("last") line of loop
		// Vector p1(x1,y1). Here, we actually are calculating -p1(x1_n,y1_n)
		Real x1_n = v1->x - v0->x;
		Real y1_n = v1->y - v0->y;
		if ( ! Rtt_VERIFY( Normalize( x1_n, y1_n ) ) )
		{
			// Give up!
			goto exit_gracefully;
		}

		Real dx = y1_n; // The "left" normal to vector p1
		Real dy = -x1_n;

		// Stroke control points starting from "N-1",
		// i.e. controlVertices[numControlPoints-1]
		for ( S32 j = 0; j < numControlPoints; j++ )
		{
			v2 = & controlVertices[j];
			const S32 numPoints = StrokeSegment( dstVertices, v1, v2, dx, dy, x1_n, y1_n, innerWidth, outerWidth );

			if ( numPoints > 0 )
			{
				numAdded += numPoints;
			}
			v1 = v2;
		}

		// Last point
		// We need to handle the "wrap-around" condition, i.e. N+1 is the same as 0
		// StrokeSegment always operates on v1, and since v1 is now set to
		// controlVertices[numControlPoints-1], we need to set v2 to be the first point
		// to complete the loop:
		v2 = & controlVertices[0];
		const S32 numPoints = StrokeSegment( dstVertices, v1, v2, dx, dy, x1_n, y1_n, innerWidth, outerWidth );
		if (numPoints > 0 )
		{
			numAdded += numPoints;
		}
		else if (vertices.Length() >= 2)
		{
			//case is clearly degenerate. Lets just manually add some points to make it a loop
			vertices.Append( Vertex2(vertices[vertices.Length()-numAdded]) ); // copy, because references mess up when moving memory around
			vertices.Append( Vertex2(vertices[vertices.Length()-numAdded]) ); // ditto
			numAdded += 2;
		}

		controlVertices += numControlPoints;
	}

exit_gracefully:
	return;
}

void
TesselatorLine::GenerateStroke( ArrayVertex2& vertices )
{
	switch ( fMode )
	{
		case kLoopMode:
			GenerateStrokeLoop( vertices );
			break;
		case kLineMode:
		default:
			GenerateStrokeLine( vertices );
			break;
	}
}

void
TesselatorLine::GenerateStrokeTexture( ArrayVertex2& outTexCoords, int numVertices )
{
	ArrayVertex2& vertices = outTexCoords;

	Rtt_ASSERT( vertices.Length() == 0 );

	AppendStrokeTextureClosed( vertices, numVertices );
}

void
TesselatorLine::GetSelfBounds( Rect& rect )
{
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


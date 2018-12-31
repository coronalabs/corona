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

#include "Core/Rtt_Build.h"

#include "Rtt_StrokeTesselatorStream.h"

#include "Display/Rtt_ClosedPath.h"
#include "Display/Rtt_OpenPath.h"
#include "Rtt_Matrix.h"
#include "Display/Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

StrokeTesselatorStream::StrokeTesselatorStream(
	const Matrix* srcToDstSpace, VertexCache& cache, const ClosedPath& shape )
:	Super( srcToDstSpace, cache ),
	fInnerWidth( Rtt_IntToReal( shape.GetInnerStrokeWidth() ) ),
	fOuterWidth( Rtt_IntToReal( shape.GetOuterStrokeWidth() ) )
{
}

StrokeTesselatorStream::StrokeTesselatorStream(
	const Matrix* srcToDstSpace, VertexCache& cache, const OpenPath& shape )
:	Super( srcToDstSpace, cache ),
	fInnerWidth( Rtt_RealDiv2( shape.GetWidth() ) ),
	fOuterWidth( fInnerWidth )
{
}

void
StrokeTesselatorStream::Submit( const VertexArray& vertices )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

/*
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
*/

/*
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
*/

/*
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
*/

/*
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
			const Real kMiterJointThreshold = Rtt_FloatToReal( 0.965925826289068 );
			bool shouldAddMiterJoint = ( cosTheta < kMiterJointThreshold );

			// Additional threshold when the angle between the 2 vectors < 6 degrees.
			// which corresponds to cosTheta > cos( 3 )
			const Real kDegenerateThreshold = Rtt_FloatToReal( 0.998629534754574 );
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
*/

/*
void
StrokeTesselatorStream::Submit( RenderTypes::Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts )
{
	ArrayVertex2& dstVertices = fCache.Vertices();

	const Real innerWidth = fInnerWidth;
	const Real outerWidth = fOuterWidth;

	for ( S32 i = 0; i < numCounts; i++ )
	{
		S32 numAdded = 0;
		const S32 numVertices = counts[i];

		if ( ! Rtt_VERIFY( numVertices >= 2 ) ) { continue; }

		const Vertex2* v0 = & vertices[0];
		const Vertex2* v1 = & vertices[1];

		// Vector p1(x1,y1). Here, we actually are calculating -p1(x1_n,y1_n)
		Real x1_n = v1->x - v0->x;
		Real y1_n = v1->y - v0->y;
		if ( ! Rtt_VERIFY( Normalize( x1_n, y1_n ) ) ) { continue; }

		Real dx = y1_n; // The "left" normal to vector p1
		Real dy = -x1_n;

		// First endpoint
		numAdded += AddBoundaryPoints( dstVertices, * v0, dx, dy, innerWidth, outerWidth );

		const Vertex2* v2;
		if ( numVertices < 3 )
		{
			// Only the endpoints v0 and v2 exist. There is no v1
			v2 = v1;
		}
		else
		{
			// Handle all intermediate points
			for ( S32 j = 2; j < numVertices; j++ )
			{
//				numAdded += AddBoundaryPoints( dstVertices, * v1, dx, dy, innerWidth, outerWidth );
				v2 = & vertices[j];
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

		vertices += numVertices;
		fCache.Counts().Append( numAdded );
	}

	Vertex2 origin = { Rtt_REAL_0, Rtt_REAL_0 };
	ApplyTransform( origin );
}
*/
static const Vertex2&
Vertex2_Offset( const Vertex2& src, Vertex2& dst, Real delta )
{
	if ( delta == Rtt_REAL_0 )
	{
		return src;
	}

	// grow/shrink the point relative to origin
	// delta < 0 means to shrink
	// delta > 0 means to grow
	dst.x = src.x + ( src.x < Rtt_REAL_0 ? -delta : delta );
	dst.y = src.y + ( src.y < Rtt_REAL_0 ? -delta : delta );
	return dst;
}

void
StrokeTesselatorStream::Submit( const Quad quad )
{
	// Rtt_STATIC_ASSERT( 4 == sizeof( Quad ) / sizeof( quad[0] ) );

	const Real innerWidth = fInnerWidth;
	const Real outerWidth = fOuterWidth;

	Vertex2 inner, outer;
	ArrayVertex2& vertices = fCache.Vertices();

	// Map triangle strip ordering to fan ordering
	const int indices[] = { 0, 1, 3, 2 };
	Rtt_STATIC_ASSERT(
		( sizeof(indices)/sizeof(indices[0]) ) == ( sizeof(Quad)/sizeof(quad[0]) ) );

	for ( int i = 0, iMax = sizeof( Quad ) / sizeof( quad[0] ); i < iMax; i++ )
	{
		int index = indices[i];
		const Vertex2& v = quad[index];
		const Vertex2& vInner = Vertex2_Offset( v, inner, -innerWidth );
		const Vertex2& vOuter = Vertex2_Offset( v, outer, outerWidth );
		vertices.Append( vInner );
		vertices.Append( vOuter );
	}

	vertices.Append( Vertex2_Offset( quad[0], inner, -innerWidth ) );
	vertices.Append( Vertex2_Offset( quad[0], outer, outerWidth ) );
	fCache.Counts().Append( 10 );

	Vertex2 origin = { Rtt_REAL_0, Rtt_REAL_0 };
	ApplyTransform( origin );
}

void
StrokeTesselatorStream::Submit( const Vertex2& origin, Real rectHalfW, Real rectHalfH, Real radius )
{
	AppendCircleStroke( radius, true );

	Real halfW = rectHalfW - radius;
	Real halfH = rectHalfH - radius;

	ArrayVertex2& vertices = fCache.Vertices();
	const S32 numVertices = vertices.Length();

	// An add'l 2 vertices were added at the end to close the loop
	S32 length = numVertices - 2; Rtt_ASSERT( ( length & 0x3 ) == 0 );
	length = length >> 2;
	Vertex2* pVertices = vertices.WriteAccess();

	// Position origin of each circle segment appropriately
	Vertex2_Translate( pVertices, length, halfW, halfH ); pVertices += length;
	Vertex2_Translate( pVertices, length, -halfW, halfH ); pVertices += length;
	Vertex2_Translate( pVertices, length, -halfW, -halfH ); pVertices += length;
	Vertex2_Translate( pVertices, length, halfW, -halfH ); pVertices += length;

	// Last 2 vertices close the loop
	Vertex2_Translate( pVertices, 2, halfW, halfH );

	ApplyTransform( origin );
}

void
StrokeTesselatorStream::Submit( const Vertex2& origin, Real radius )
{
	AppendCircleStroke( radius, false );
	ApplyTransform( origin );
}

void
StrokeTesselatorStream::Submit( const Vertex2& origin, Real a, Real b )
{
}

void
StrokeTesselatorStream::AppendCircleStroke( Real radius, bool appendEndPoints )
{
	ArrayVertex2& vertices = fCache.Vertices();
	const S32 oldLen = vertices.Length();

	// Generate vertices along perimeter of circle. Each vertex is added twice,
	// one for the inner and one for the outer radius.
	U32 options = kNoScale | kAppendDuplicate;
	if ( appendEndPoints )
	{
		options |= kAppendArcEndPoints;
	}

	const Real rInner = Max( radius - fInnerWidth, Rtt_REAL_0 ); // Ensure > 0
	const Real rOuter = radius + fOuterWidth;
	AppendCircleArc( rOuter, options );
	const S32 newLen = vertices.Length() - oldLen;

	// newLen should be multiple of 2 b/c each vertex was added twice
	Rtt_ASSERT( (newLen %2) == 0 );

	// Vertices are from a unit circle. Scale them out to the correct radius
	Vertex2* iVertices = vertices.WriteAccess() + oldLen;
	for ( S32 i = 0; i < newLen; i++ )
	{
		Rtt_ASSERT( (i+1) < newLen );

		Vertex2& inner = iVertices[i];
		Vertex2& outer = iVertices[++i];

		inner.x = Rtt_RealMul( inner.x, rInner );
		inner.y = Rtt_RealMul( inner.y, rInner );

		outer.x = Rtt_RealMul( outer.x, rOuter );
		outer.y = Rtt_RealMul( outer.y, rOuter );
	}

	fCache.Counts().Append( newLen );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


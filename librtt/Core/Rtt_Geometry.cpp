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

#include "Rtt_Geometry.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const Vertex2 kZeroQuad[] =
{
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_0 }
};

const Vertex2*
Quad_Zero()
{
	return kZeroQuad;
}

void
Quad_Invalidate( Quad vertices )
{
	vertices[0].x = kCoordinateNegInfinity;
}

bool
Quad_IsValid( const Quad vertices )
{
	return ( kCoordinateNegInfinity != vertices[0].x );
}

bool
Quad_IsAligned( const Quad vertices )
{
	const Vertex2& v0 = vertices[0];
	const Vertex2& v1 = vertices[1];

	bool result = ( v0.x == v1.x ) || ( v0.y == v1.y );

#ifdef Rtt_DEBUG
	if ( result )
	{
		// Quad is a strip not a fan but v1-v4 assumes fan
		const Vertex2& v3 = vertices[2];
		const Vertex2& v2 = vertices[3];

		#if 1
			#ifdef Rtt_REAL_FIXED
			const Real kEps = 0;
			#else
			const Real kEps = 0.00005f; // round off error means we don't use FLT_EPS
			#endif

			if ( Rtt_RealEqual( v0.x, v1.x ) )
			{
				Rtt_ASSERT(
					Rtt_RealEqualEps( v1.y, v2.y, kEps )
					&& Rtt_RealEqualEps( v2.x, v3.x, kEps )
					&& Rtt_RealEqualEps( v3.y, v0.y, kEps ) );
			}
			else if ( Rtt_RealEqual( v0.y, v1.y ) )
			{
				Rtt_ASSERT(
					Rtt_RealEqualEps( v1.x, v2.x, kEps )
					&& Rtt_RealEqualEps( v2.y, v3.y, kEps )
					&& Rtt_RealEqualEps( v3.x, v0.x, kEps ) );
			}
		#else
			if ( v0.x == v1.x )
			{
				Rtt_ASSERT( v1.y == v2.y && v2.x == v3.x && v3.y == v0.y );
			}
			else if ( v0.y == v1.y )
			{
				Rtt_ASSERT( v1.x == v2.x && v2.y == v3.y && v3.x == v0.x );
			}
		#endif
	}
#endif

	return result;
}

/*
void
Quad_GetBoundsAxisAligned( Quad dst, const Quad src )
{
	const Vertex2 v0 = src[0];
	const Vertex2 v1 = src[1];
	const Vertex2 v2 = src[2];
	const Vertex2 v3 = src[3];

	// TODO: Optimize. There must be a better way
	Real xMin = Min( v0.x, Min( v1.x, Min( v2.x, v3.x ) ) );
	Real xMax = Max( v0.x, Max( v1.x, Max( v2.x, v3.x ) ) );

	Real yMin = Min( v0.y, Min( v1.y, Min( v2.y, v3.y ) ) );
	Real yMax = Max( v0.y, Max( v1.y, Max( v2.y, v3.y ) ) );

	dst[0].x = xMin;
	dst[0].y = yMin;
	dst[1].x = xMin;
	dst[1].y = yMax;
	dst[2].x = xMax;
	dst[2].y = yMax;
	dst[3].x = xMax;
	dst[3].y = yMin;
}
*/

void
Quad_Enlarge( Quad dst, const Quad src )
{
	const Vertex2 center = { Rtt_RealDiv2( src[0].x + src[2].x ), Rtt_RealDiv2( src[0].y + src[2].y ) };
//	if ( ! Rtt_RealIsZero( center.x ) && ! Rtt_RealIsZero( center.y ) )
	{
		for ( int i = 0, iMax = sizeof( Quad ) / sizeof( src[0] ); i < iMax; i++ )
		{
			const Vertex2& v = src[i];
			Real x = v.x;
			x += ( x - center.x > Rtt_REAL_0 ? 1 : -1 );
			Real y = v.y;
			y += ( y - center.y > Rtt_REAL_0 ? 1 : -1 );
			dst[i].x = x;
			dst[i].y = y;
		}
	}
//	else
//	{
//		memcpy( dst, src, sizeof( Quad ) );
//	}
}

void
Vertex2_Translate( Vertex2* pVertices, S32 numVertices, Real dx, Real dy )
{
	Rtt_ASSERT( pVertices );

	for ( int i = 0; i < numVertices; i++ )
	{
		pVertices[i].x += dx;
		pVertices[i].y += dy;
	}
}

void
Vertex2_Scale( Vertex2* pVertices, S32 numVertices, Real sx, Real sy )
{
	Rtt_ASSERT( pVertices );

	const bool isSxNotOne = ( sx != Rtt_REAL_1 );
	const bool isSyNotOne = ( sy != Rtt_REAL_1 );

	if ( isSxNotOne || isSyNotOne )
	{
		for ( int i = 0; i < numVertices; i++ )
		{
			pVertices[i].x = Rtt_RealMul( pVertices[i].x, sx );
			pVertices[i].y = Rtt_RealMul( pVertices[i].y, sy );
		}
	}
}

void
Vertex2_ScaleAndTranslate( Vertex2* pVertices, S32 numVertices, Real sx, Real sy, Real dx, Real dy )
{
	Rtt_ASSERT( pVertices );

	const bool isSxNotOne = ( sx != Rtt_REAL_1 );
	const bool isSyNotOne = ( sy != Rtt_REAL_1 );

	if ( isSxNotOne || isSyNotOne )
	{
		for ( int i = 0; i < numVertices; i++ )
		{
			pVertices[i].x = Rtt_RealMul( pVertices[i].x, sx ) + dx;
			pVertices[i].y = Rtt_RealMul( pVertices[i].y, sy ) + dy;
		}
	}
	else
	{
		Vertex2_Translate( pVertices, numVertices, dx, dy );
	}
}

/*
Vertex2
RectCenter( const Rect& r )
{
	Rtt_ASSERT( RectIsValid( r ) );

	Vertex2 result;
	result.x = Rtt_IntToReal( (r.xMin + r.xMax) >> 1 );
	result.y = Rtt_IntToReal( (r.yMin + r.yMax) >> 1 );
	return result;
}
*/

Rect::Rect( const Rect& rhs, Real borderWidth )
{
	if ( rhs.IsEmpty() )
	{
		xMin = -borderWidth;
		yMin = -borderWidth;
		xMax = borderWidth;
		yMax = borderWidth;
	}
	else
	{
		xMin = rhs.xMin - borderWidth;
		yMin = rhs.yMin - borderWidth;
		xMax = rhs.xMax + borderWidth;
		yMax = rhs.yMax + borderWidth;
	}
}

void
Rect::Initialize( Real halfW, Real halfH )
{
	xMin = -halfW;
	yMin = -halfH;
	xMax = halfW;
	yMax = halfH;
}

void
Rect::Initialize( Real xCenter, Real yCenter, Real w, Real h )
{
	Real halfW = Rtt_RealDiv2( w );
	Real halfH = Rtt_RealDiv2( h );

	xMin = xCenter - halfW;
	yMin = yCenter - halfH;
	xMax = xCenter + halfW;
	yMax = yCenter + halfH;
}

void
Rect::Translate( Real dx, Real dy )
{
	if ( NotEmpty() )
	{
		xMin += dx;
		yMin += dy;
		xMax += dx;
		yMax += dy;
	}
}

void
Rect::Scale( Real sx, Real sy )
{
	if ( NotEmpty() )
	{
		xMin *= sx;
		yMin *= sy;
		xMax *= sx;
		yMax *= sy;
	}
}

void
Rect::Union( const Vertex2& p )
{
	Real x = p.x;
	Real y = p.y;

	if ( NotEmpty() )
	{
		xMin = Min( xMin, x );
		yMin = Min( yMin, y );
		xMax = Max( xMax, x );
		yMax = Max( yMax, y );
	}
	else
	{
		xMin = xMax = x;
		yMin = yMax = y;
	}
}

void
Rect::Union( const Rect& rhs )
{
	if ( rhs.NotEmpty() )
	{
		if ( NotEmpty() )
		{
			xMin = Min( xMin, rhs.xMin );
			yMin = Min( yMin, rhs.yMin );
			xMax = Max( xMax, rhs.xMax );
			yMax = Max( yMax, rhs.yMax );
		}
		else
		{
			* this = rhs;
		}
	}
}

void
Rect::Intersect( const Rect& rhs )
{
	if ( Intersects( rhs ) )
	{
		xMin = Max( xMin, rhs.xMin );
		yMin = Max( yMin, rhs.yMin );
		xMax = Min( xMax, rhs.xMax );
		yMax = Min( yMax, rhs.yMax );
	}
	else
	{
		SetEmpty();
	}
}

bool
Rect::Intersects( const Rect& rhs ) const
{
	if ( NotEmpty() && rhs.NotEmpty() )
	{
		// http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
		// A (rhs), B (this)
		// 
		// ( A's Left Edge to left of B's right edge )
		// AND ( A's right edge to right of B's left edge )
		// AND ( A's top above B's bottom )
		// AND ( A's bottom below B's Top )
		return ( rhs.xMin < xMax && rhs.xMax > xMin && rhs.yMin < yMax && rhs.yMax > yMin );
	}
	else
	{
		return false;
	}
}

bool
Rect::CompletelyEncloses( const Vertex2& p ) const
{
	bool result = NotEmpty();

	if ( result )
	{
		Real x = p.x;
		Real y = p.y;

		result = ( xMin < x && xMax > x && yMin < y && yMax > y );
	}

	return result;
}

bool
Rect::Encloses( const Rect& rhs ) const
{
	bool result =  NotEmpty() && rhs.NotEmpty();

	if ( result )
	{
		return xMin <= rhs.xMin && xMax >= rhs.xMax && yMin <= rhs.yMin && yMax >= rhs.yMax;
	}

	return result;
}

bool
Rect::HitTest( Real x, Real y ) const
{
	return NotEmpty() && ( xMin <= x && xMax >= x && yMin <= y && yMax >= y );
}

void
Rect::GetCenter( Vertex2& pt ) const
{
	pt.x = Rtt_RealAvg( xMin, xMax );
	pt.y = Rtt_RealAvg( yMin, yMax );
}

void
Rect::GetQuad( Vertex2 *quad ) const
{
	quad[0].x = xMin;
	quad[0].y = yMin;
	quad[1].x = xMin;
	quad[1].y = yMax;
	quad[3].x = xMax;
	quad[3].y = yMax;
	quad[2].x = xMax;
	quad[2].y = yMin;	
}

void
Rect::MoveCenterToOrigin()
{
	Real halfW = Rtt_RealDiv2( xMax - xMin ); Rtt_ASSERT( xMax > xMin );
	Real halfH = Rtt_RealDiv2( yMax - yMin ); Rtt_ASSERT( yMax > yMin );

	xMin = -halfW;
	xMax = halfW;
	yMin = -halfH;
	yMax = halfH;
}

#ifdef Rtt_DEBUG
void
Rect::Trace() const
{
	// TODO: Assumes Real is float; may not work for Fixed!
	Rtt_TRACE( ( "Rect(xMin,yMin:xMax,yMax) = (%g,%g:%g,%g)\n", xMin,yMin,xMax,yMax ) );
}
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


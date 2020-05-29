//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_Geometry_H__
#define __Rtt_Geometry_H__

#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Real.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

#ifdef __cplusplus

namespace Rtt
{

// ----------------------------------------------------------------------------

typedef enum GeometricProperty
{
	kOriginX = 0,
	kOriginY,
	kScaleX,
	kScaleY,
	kRotation,

	kWidth,
	kHeight,
	kStageWidth,
	kStageHeight,
//	kStagePositionX,
//	kStagePositionY,

	kNumGeometricProperties
}
GeometricProperty;

typedef S32 RefCount;

typedef Rtt_Real Coordinate;

struct Vertex2
{
	Rtt_Real x;
	Rtt_Real y;
};

// This is based on Apple/GLKit's GLKVector4.
// Use Box2D's b2Vec2 for 2D vectors.
union Vector4
{
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    struct { float s, t, p, q; };
    float v[4];
};

const Vertex2 kVertexOrigin = { Rtt_REAL_0, Rtt_REAL_0 };

const Vector4 kVector4Zero = { Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0 };

typedef Vertex2 Quad[4];

const Coordinate kCoordinateNegInfinity = 0x80000000;

const Vertex2* Quad_Zero();
void Quad_Invalidate( Quad vertices );
bool Quad_IsValid( const Quad vertices );
bool Quad_IsAligned( const Quad vertices );
// void Quad_GetBoundsAxisAligned( Quad dst, const Quad src );
void Quad_Enlarge( Quad dst, const Quad src );
void Quad_Intersect( const Quad q1, const Quad q2 );

struct Rect
{
	Rect() : xMin( kCoordinateNegInfinity ) {}
	Rect( const Vertex2& minCorner, const Vertex2& maxCorner )
	: xMin( minCorner.x ), yMin( minCorner.y ), xMax( maxCorner.x ), yMax( maxCorner.y )
	{
	}

	Rect( const Rect& rhs, Real borderWidth );

	void Initialize( Real halfW, Real halfH );
	void Initialize( Real xCenter, Real yCenter, Real w, Real h );

	Rtt_FORCE_INLINE bool IsEmpty() const { return kCoordinateNegInfinity == xMin; }
	Rtt_FORCE_INLINE bool NotEmpty() const { return kCoordinateNegInfinity != xMin; }
	Rtt_FORCE_INLINE void SetEmpty() 
	{ 
		xMin = kCoordinateNegInfinity; 
	}

	Rtt_INLINE Coordinate Width() const { return ( NotEmpty() ? xMax - xMin : Rtt_REAL_0 ); }
	Rtt_INLINE Coordinate Height() const { return ( NotEmpty() ? yMax - yMin : Rtt_REAL_0 ); }

	void Translate( Real dx, Real dy );
	void Scale( Real sx, Real sy );
	void Union( const Vertex2& p );
	void Union( const Rect& rhs );
	void Intersect( const Rect& rhs );

	bool Intersects( const Rect& rhs ) const;
	bool CompletelyEncloses( const Vertex2& p ) const;
	bool Encloses( const Rect& rhs ) const;
	bool HitTest( Real x, Real y ) const;

	void GetCenter( Vertex2& pt ) const;
	void GetQuad( Vertex2 *quad ) const;

	void MoveCenterToOrigin();

#ifdef Rtt_DEBUG
	void Trace() const;
#endif

	Coordinate xMin;
	Coordinate yMin;
	Coordinate xMax;
	Coordinate yMax;
};

void Vertex2_Translate( Vertex2* pVertices, S32 numVertices, Real dx, Real dy );
void Vertex2_Scale( Vertex2* pVertices, S32 numVertices, Real sx, Real sy );
void Vertex2_ScaleAndTranslate( Vertex2* pVertices, S32 numVertices, Real sx, Real sy, Real dx, Real dy );

// Vertex2 RectCenter( const Rect& r );


// ----------------------------------------------------------------------------

} // Rtt

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // __Rtt_Geometry_H__

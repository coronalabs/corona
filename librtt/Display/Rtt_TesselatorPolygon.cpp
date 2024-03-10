//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TesselatorPolygon.h"

#include "Display/Rtt_TesselatorLine.h"
#include "Rtt_Matrix.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class Triangulate
{
	public:
		// Tesselate a contour/polygon into a series of triangles.
		static bool Process(
			const ArrayVertex2 &contour, ArrayVertex2 &result, Rect& bounds, ArrayIndex * triangulationIndices );

		// Compute area of a contour/polygon
		static float Area(const ArrayVertex2 &contour);

		// Decide if point Px/Py is inside triangle defined by (Ax,Ay) (Bx,By) (Cx,Cy)
		static bool InsideTriangle(
			float Ax, float Ay,
			float Bx, float By,
			float Cx, float Cy,
			float Px, float Py);

	private:
		static bool Snip(
			const ArrayVertex2 &contour,
			int u, int v, int w, int n, int *V );
};

float
Triangulate::Area( const ArrayVertex2 &contour )
{
	int n = contour.Length();

	float A = 0.0f;

	for( int p = n-1, q = 0; q < n; p = q++ )
	{
		A += contour[p].x * contour[q].y - contour[q].x * contour[p].y;
	}
	return A * 0.5f;
}

// InsideTriangle decides if point P is Inside of the triangle defined by A, B, C.
bool
Triangulate::InsideTriangle(
	float Ax, float Ay,
	float Bx, float By,
	float Cx, float Cy,
	float Px, float Py )
{
	float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	float cCROSSap, bCROSScp, aCROSSbp;

	ax  = Cx - Bx;	ay  = Cy - By;
	bx  = Ax - Cx;	by  = Ay - Cy;
	cx  = Bx - Ax;	cy  = By - Ay;
	apx = Px - Ax;	apy = Py - Ay;
	bpx = Px - Bx;	bpy = Py - By;
	cpx = Px - Cx;	cpy = Py - Cy;

	aCROSSbp = ax*bpy - ay*bpx;
	cCROSSap = cx*apy - cy*apx;
	bCROSScp = bx*cpy - by*cpx;

	return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};

static const float EPSILON=0.0000000001f;

bool
Triangulate::Snip( const ArrayVertex2 &contour, int u, int v, int w, int n, int *V )
{
	int p;
	float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

	Ax = contour[V[u]].x;
	Ay = contour[V[u]].y;

	Bx = contour[V[v]].x;
	By = contour[V[v]].y;

	Cx = contour[V[w]].x;
	Cy = contour[V[w]].y;

	if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) { return false; }

	for ( p=0; p < n; p++ )
	{
		if ( (p == u) || (p == v) || (p == w) ) { continue; }
		Px = contour[V[p]].x;
		Py = contour[V[p]].y;
		if ( InsideTriangle( Ax, Ay, Bx, By, Cx, Cy, Px, Py ) ) { return false; }
	}

	return true;
}

bool
Triangulate::Process( const ArrayVertex2 &contour, ArrayVertex2 &result, Rect& bounds, ArrayIndex * triangulationIndices )
{
	// allocate and initialize list of Vertices in polygon
	int n = contour.Length();
	if ( n < 3 ) return false;

	int *V = new int[n];

	// we want a counter-clockwise polygon in V
	if ( 0.0f < Area(contour) )
	{
		for ( int v = 0; v < n; v++ )
		{
			V[v] = v;
		}
	}
	else
	{
		for ( int v = 0; v < n; v++ )
		{
			V[v] = (n-1)-v;
		}
	}

	int nv = n;

	// remove nv-2 Vertices, creating 1 triangle every time
	int count = 2*nv;   // error detection

	for ( int m = 0, v = nv-1; nv > 2; )
	{
		// if we loop, it is probably a non-simple polygon 
		if (0 >= (count--))
		{
			//** Triangulate: ERROR - probable bad polygon!
			return false;
		}

		// three consecutive vertices in current polygon, <u,v,w>
		int u = v;   if (nv <= u) { u = 0; }	// previous
		v = u+1;     if (nv <= v) { v = 0; }	// new v   
		int w = v+1; if (nv <= w) { w = 0; }	// next    

		if ( Snip( contour, u, v, w, nv, V ) )
		{
			int a,b,c,s,t;

			// true names of the vertices
			a = V[u]; b = V[v]; c = V[w];

			// output Triangle
			result.Append( contour[a] ); bounds.Union( contour[a] );
			result.Append( contour[b] ); bounds.Union( contour[b] );
			result.Append( contour[c] ); bounds.Union( contour[c] );

            if (triangulationIndices)
            {
                triangulationIndices->Append( a );
                triangulationIndices->Append( b );
                triangulationIndices->Append( c );
            }

			m++;

			// remove v from remaining polygon
			for ( s = v, t = v+1; t < nv; s++, t++ )
			{
				V[s] = V[t];
			}
			nv--;

			// resest error detection counter
			count = 2*nv;
		}
	}

	delete [] V;

	return true;
}

// ----------------------------------------------------------------------------

TesselatorPolygon::TesselatorPolygon( Rtt_Allocator *allocator )
:	Super(),
	fContour( allocator ),
	fFill( allocator ),
	fSelfBounds(),
	fCenter( kVertexOrigin ),
	fIsFillValid( false ),
	fIsBadPolygon( false ),
    fTriangulationIndices( NULL ),
	fFillCount( -1 )
{
}

void
TesselatorPolygon::GenerateFill( ArrayVertex2& vertices )
{
	Update();

	for (int i=0; i<fFill.Length(); i++) {
		vertices.Append(fFill[i]);
	}
}

void
TesselatorPolygon::GenerateFillTexture( ArrayVertex2& texCoords, const Transform& t )
{
	Update();

	Real w = fSelfBounds.Width();
	Real h = fSelfBounds.Height();

	Real invW = Rtt_RealDiv( Rtt_REAL_1, w );
	Real invH = Rtt_RealDiv( Rtt_REAL_1, h );

	const ArrayVertex2& src = fFill;
	ArrayVertex2& vertices = texCoords;

	// Transform
	if ( t.IsIdentity() )
	{
		for ( int i = 0, iMax = src.Length(); i < iMax; i++ )
		{
			// Normalize: assume src is centered about (0,0)
			Vertex2 v =
			{
				( Rtt_RealMul( src[i].x, invW ) + Rtt_REAL_HALF ),
				( Rtt_RealMul( src[i].y, invH ) + Rtt_REAL_HALF ),
			};
			vertices.Append( v );
		}
	}
	else
	{
		// Normalize/Transform: assume src is centered about (0,0)
		Matrix m;
		m.Scale( Rtt_RealMul( invW, t.GetSx() ), Rtt_RealMul( invH, t.GetSy() ) );
		m.Rotate( - t.GetRotation() );
		m.Translate( t.GetX() + Rtt_REAL_HALF, t.GetY() + Rtt_REAL_HALF );
		m.Apply( vertices.WriteAccess(), vertices.Length() );

		for ( int i = 0, iMax = src.Length(); i < iMax; i++ )
		{
			Vertex2 v = src[i];
			m.Apply( v );
			vertices.Append( v );
		}
	}
}

void
TesselatorPolygon::GenerateStroke( ArrayVertex2& vertices )
{
	TesselatorLine t( fContour, TesselatorLine::kLoopMode );
	t.SetInnerWidth( GetInnerWidth() );
	t.SetOuterWidth( GetOuterWidth() );

	t.GenerateStroke( vertices );
}

void
TesselatorPolygon::GetSelfBounds( Rect& rect )
{
	Update();
	rect = fSelfBounds;
}

Geometry::PrimitiveType
TesselatorPolygon::GetFillPrimitive() const
{
	return Geometry::kTriangles;
}

U32
TesselatorPolygon::FillVertexCount() const
{
	if ( -1 == fFillCount )
	{
		TesselatorPolygon dummy( fContour.Allocator() );

		dummy.fContour.Reserve( fContour.Length() );

		for (int i = 0, iMax = fContour.Length(); i < iMax; ++i)
		{
			dummy.fContour.Append( fContour[i] );
		}

		dummy.Update();

		fFillCount = S32( dummy.fFill.Length() );
	}

	return U32( fFillCount );
}

U32
TesselatorPolygon::StrokeVertexCount() const
{
	return TesselatorLine::VertexCountFromPoints( fContour, true );
}

void
TesselatorPolygon::Invalidate()
{
	fIsFillValid = false;
	fIsBadPolygon = false;
}

void
TesselatorPolygon::Update()
{
	// NOTE: fIsBadPolygon can only be true if there was already an
	// attempt to Update() in which Process() failed.
	if ( ! fIsFillValid
		 && ! fIsBadPolygon )
	{
		fSelfBounds.SetEmpty();

        if (fTriangulationIndices)
        {
            fTriangulationIndices->Clear();
        }

		fIsFillValid = Triangulate::Process( fContour, fFill, fSelfBounds, fTriangulationIndices );
		fIsBadPolygon = ! fIsFillValid;

		if ( fIsFillValid )
		{
			// Center vertices about the origin
			Vertex2 center;
			fSelfBounds.GetCenter( center );

			fCenter = center;
		}
		else
		{
			// Failure case
			Rtt_TRACE_SIM( ( "WARNING: Polygon could not be generated. The polygon outline is invalid, possibly due to holes or self-intersection.\n" ) );
			fFill.Empty();
			fSelfBounds.SetEmpty();
		}
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


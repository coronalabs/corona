//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_RectPath.h"

//#include "Display/Rtt_BitmapPaint.h"
//#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_ShapeAdapterRect.h"
#include "Display/Rtt_TesselatorRect.h"
#include "Renderer/Rtt_Geometry_Renderer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

RectPath *
RectPath::NewRect( Rtt_Allocator *pAllocator, Real width, Real height )
{
	TesselatorRect *tesselator = Rtt_NEW( pAllocator, TesselatorRect( width, height ) );
	RectPath *result = Rtt_NEW( pAllocator, RectPath( pAllocator, tesselator ) );
	result->SetAdapter( & ShapeAdapterRect::Constant() );
	result->SetDelegate( result );
	return result;
}

// ----------------------------------------------------------------------------

// NOT USED: static const S32 kRectPathNumVertices = sizeof(Quad) / sizeof(Vertex2);

RectPath::RectPath( Rtt_Allocator* pAllocator, TesselatorShape *tesselator )
:	Super( pAllocator, tesselator )
{
	fFillGeometry->Resize( 4, false );
	fStrokeGeometry->Resize( 10, false );

	SetProperty( kIsRectPath, true );
}

void
RectPath::Update( RenderData& data, const Matrix& srcToDstSpace )
{
	TesselatorRect *tesselator = (TesselatorRect *)GetTesselator();
	if ( ! tesselator->HasOffset() )
	{
		Super::Update( data, srcToDstSpace );
	}
	else
	{
		UpdateFill( data, srcToDstSpace );
		UpdateStroke( srcToDstSpace );
	}
}

#if 0

RectPath::~RectPath()
{
	Rtt_DELETE( fStrokeGeometry );
	Rtt_DELETE( fFillGeometry );
}

// Assumes points are passed in triangle fan order, not tri strip order
// Results also returned in fan order.
static void
getDgnlIntrsctP12andP34(
	float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	float *outDiagonals )
{
	float xp2 = x2 - x1; // P2.x - P1.x ;
	float xp3 = x3 - x1; // P3.x - P1.x ;
	float xp4 = x4 - x1; // P4.x - P1.x ;
	float yp2 = y2 - y1; // P2.y - P1.y ;
	float yp3 = y3 - y1; // P3.y - P1.y ;
	float yp4 = y4 - y1; // P4.y - P1.y ;
	float x, y;

	// http://mathworld.wolfram.com/Line-LineIntersection.html
	// x = x1 + xp2*(xp3*yp4-xp4*yp3) / ((x3-x4)*yp2-xp2*(y3-y4))
	// y = y1 + (xp3*yp4-xp4*yp3)*yp2 / ((x3-x4)*yp2-xp2*(y3-y4))
	float x3y4mx4y3 = xp3 * yp4 - xp4 * yp3 ;
	float denominator = ( x3 - x4 ) * yp2 - xp2 * ( y3 - y4 ) ;
	// float denominator = ( P3.x - P4.x ) * yp2 - xp2 * ( P3.y - P4.y ) ;

	x = xp2 * x3y4mx4y3 / denominator ;
	y = yp2 * x3y4mx4y3 / denominator ;

	xp2 -= x ;
	xp3 -= x ;
	xp4 -= x ;
	yp2 -= y ;
	yp3 -= y ;
	yp4 -= y ;

	outDiagonals[ 0 ] = sqrt( x * x + y * y ) ;
	outDiagonals[ 1 ] = sqrt( xp2 * xp2 + yp2 * yp2 ) ;
	outDiagonals[ 2 ] = sqrt( xp3 * xp3 + yp3 * yp3 ) ;
	outDiagonals[ 3 ] = sqrt( xp4 * xp4 + yp4 * yp4 ) ;
// printf( "\t{%4.3g %4.3g %4.3g %4.3g}\n" , outDiagonals[ 0 ] , outDiagonals[ 1 ] , outDiagonals[ 2 ] , outDiagonals[ 3 ] ) ;
}

// Standard texture coordinates
static const Vertex2 kUV[] =
{
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_1 },
	{ Rtt_REAL_1, Rtt_REAL_0 },
	{ Rtt_REAL_1, Rtt_REAL_1 },
};

namespace /*anonymous*/ 
{
	inline void AssignVertex(
		Rtt::Geometry::Vertex& vertex,
		Rtt::Real x, Rtt::Real y,
		const Vertex2& t, Real tCoeff )
	{
		vertex.x = x;
		vertex.y = y;
		vertex.z = 0.f;
		vertex.u = t.x * tCoeff;
		vertex.v = t.y * tCoeff;
		vertex.q = 1.f * tCoeff;
	}
}

void
RectPath::UpdateFill( RenderData& data, const Matrix& srcToDstSpace )
{
	// Init fFillGeometry at origin
	Real halfW = fHalfW;
	Real halfH = fHalfH;

	// Get correct permutation of tex coords based on potetial JPEG rotation
//	int i = 0, j = 1, k = 2, l = 3; // Assign defaults
//	GetTextureCoordinates( i, j, k, l );

	Real x0, y0, x1, y1, x2, y2, x3, y3;
	Real c0, c1, c2, c3; // coefficients for tex coords

	if ( ! HasOffset() )
	{
		x0 = -halfW;
		y0 = -halfH;
		x1 = -halfW;
		y1 =  halfH;
		x2 =  halfW;
		y2 = -halfH;
		x3 =  halfW;
		y3 =  halfH;

		c0 = Rtt_REAL_1;
		c1 = Rtt_REAL_1;
		c2 = Rtt_REAL_1;
		c3 = Rtt_REAL_1;
	}
	else
	{
		// NOTE: Enum offsets are in counterclockwise fan order
		// while the x,y variables are in strip order
		x0 = -halfW + GetOffset( kX0 );
		y0 = -halfH + GetOffset( kY0 );
		x1 = -halfW + GetOffset( kX1 );
		y1 =  halfH + GetOffset( kY1 );
		x2 =  halfW + GetOffset( kX3 );
		y2 = -halfH + GetOffset( kY3 );
		x3 =  halfW + GetOffset( kX2 );
		y3 =  halfH + GetOffset( kY2 );

		float diagonals[4];
		// Note: This function expects P1 and P2 to be a diagonal, likewise for P3 and P4.
		// Therefore, we need to permute our points (tri strip order):
		//    P1 <= (x0,y0), P2 <= (x3,y3), P3 <= (x1,y1), P4 <= (x2,y2)
		getDgnlIntrsctP12andP34( x0, y0, x3, y3, x1, y1, x2, y2, diagonals );

		// The resulting diagonals come out in a corresponding order, so we need to
		// permute them back so the coefficients are in tri-strip order:
		//    c0 <= P1, c1 <= P3, c2 <= P4, c3 <= P2
		//    c0(d0,d1), c1(d2,d3), c2(d3,d2), c3(d1,d0)
		c0 = 1 + diagonals[0] / diagonals[1];
		c1 = 1 + diagonals[2] / diagonals[3];
		c2 = 1 + diagonals[3] / diagonals[2];
		c3 = 1 + diagonals[1] / diagonals[0];
	}

	const Paint *p = GetFill(); Rtt_ASSERT( p );
	const Vertex2 *texCoords = p->GetTextureQuad();
	if ( ! texCoords )
	{
		texCoords = kUV;
	}

	Rtt::Geometry::Vertex *vertices = fFillGeometry->GetVertexData();
	AssignVertex( vertices[0], x0, y0, texCoords[0], c0 );
	AssignVertex( vertices[1], x1, y1, texCoords[1], c1 );
	AssignVertex( vertices[2], x2, y2, texCoords[2], c2 );
	AssignVertex( vertices[3], x3, y3, texCoords[3], c3 );
	fFillGeometry->SetVerticesUsed( kRectPathNumVertices );

	// Transform fill
	srcToDstSpace.Apply( * fFillGeometry );

	data.fGeometry = fFillGeometry;
}

static const Geometry::Vertex&
Vertex_Offset( const Geometry::Vertex& src, Geometry::Vertex& dst, Real delta )
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
RectPath::UpdateStroke( const Matrix& srcToDstSpace )
{
	if ( HasStroke() )
	{
		RenderData *data = GetStrokeData();

		Real halfW = fHalfW;
		Real halfH = fHalfH;
		const Real innerWidth = GetInnerStrokeWidth();
		const Real outerWidth = GetOuterStrokeWidth();

		// Src vertices in strip order
		Vertex2 src[] = {
			{ -halfW + innerWidth, -halfH + innerWidth },
			{ -halfW - outerWidth, -halfH - outerWidth },

			{ -halfW + innerWidth,  halfH - innerWidth },
			{ -halfW - outerWidth,  halfH + outerWidth },

			{  halfW - innerWidth,  halfH - innerWidth },
			{  halfW + outerWidth,  halfH + outerWidth },

			{  halfW - innerWidth, -halfH + innerWidth },
			{  halfW + outerWidth, -halfH - outerWidth },

			// Repeat first point to close the loop
			{ -halfW + innerWidth, -halfH + innerWidth },
			{ -halfW - outerWidth, -halfH - outerWidth },
		};
		const S32 kNumSrc = sizeof( src ) / sizeof( src[0] );

		if ( fStrokeGeometry->GetVerticesAllocated() <= kNumSrc )
		{
			fStrokeGeometry->Resize( kNumSrc, false );
		}

		Geometry::Vertex *dst = fStrokeGeometry->GetVertexData();
		for ( int i = 0; i < kNumSrc; i++ )
		{
			Vertex2& vSrc = src[i];
			srcToDstSpace.Apply( vSrc );
			Geometry::Vertex& v = dst[i];
			v.x = vSrc.x;
			v.y = vSrc.y;
		}

		fStrokeGeometry->SetVerticesUsed( kNumSrc );

		data->fGeometry = fStrokeGeometry;
	}
}

void
RectPath::Translate( Real dx, Real dy )
{
	// TODO: Translate/Scale/etc to return a bool so that callers can detect
	// whether they need a Build() needs to happen or can be skipped. Currently,
	// even if the path is valid, the vectorobject will still call the path's
	// Build() which ends up doing nothing.  We should be able to eliminate 
	// this overhead.
	Super::Translate( dx, dy );

	for ( int i = 0, iMax = fFillGeometry->GetVerticesUsed(); i < iMax; i++ )
	{
		Geometry::Vertex& v = fFillGeometry->GetVertexData()[i];
		v.x += dx;
		v.y += dy;
	}
}

void
RectPath::GetSelfBounds( Rect& rect ) const
{
	// Pad border with one extra pixel (+1)
	int outerWidth = GetOuterStrokeWidth();
	//if ( outerWidth ) { ++outerWidth; }
	Real borderWidth = Rtt_IntToReal( outerWidth );
	rect.Initialize( fHalfW + borderWidth, fHalfH + borderWidth );
/*
	Real halfW = fHalfW;
	Real halfH = fHalfH;

	rect.xMin = -halfW;
	rect.yMin = -halfH;
	rect.xMax = halfW;
	rect.yMax = halfH;
*/
}
#endif

void
RectPath::UpdateGeometry(
	Geometry& dst, const VertexCache& src, const Matrix& srcToDstSpace, U32 flags ) const
{
	if ( 0 == flags ) { return; }

	const ArrayVertex2& vertices = src.Vertices();
	const ArrayVertex2& texVertices = src.TexVertices();
	U32 numVertices = vertices.Length();

	if ( dst.GetVerticesAllocated() < numVertices )
	{
		dst.Resize( numVertices, false );
	}
	Geometry::Vertex *dstVertices = dst.GetVertexData();

	bool updateVertices = ( flags & kVerticesMask );
	bool updateTexture = ( flags & kTexVerticesMask );

	Rtt_ASSERT( ! updateTexture || ( vertices.Length() == texVertices.Length() ) );

	const TesselatorRect *tesselator = NULL;
	bool hasOffset = false;
	if ( updateTexture )
	{
		tesselator = (const TesselatorRect *)GetTesselator();
		hasOffset = tesselator->HasOffset();
	}

    const ArrayFloat * floatArray = src.ExtraFloatArray( ZKey() );
    Rtt_ASSERT( ! floatArray || ( floatArray->Length() == vertices.Length() ) );
    const float zero = 0.f, * zsource = floatArray ? floatArray->ReadAccess() : &zero;
    size_t step = floatArray ? 1U : 0U;

    for ( U32 i = 0, iMax = vertices.Length(); i < iMax; i++, zsource += step )
	{
		Rtt_ASSERT( i < dst.GetVerticesAllocated() );

		Geometry::Vertex& dst = dstVertices[i];

		if ( updateVertices )
		{
			Vertex2 v = vertices[i];
			srcToDstSpace.Apply( v );

			dst.x = v.x;
			dst.y = v.y;
            dst.z = *zsource;
		}

		if ( updateTexture )
		{
			if ( ! hasOffset )
			{
				dst.u = texVertices[i].x;
				dst.v = texVertices[i].y;
				dst.q = 1.f;
			}
			else
			{
				Real c = tesselator->GetCoefficient( i );
				dst.u = c * texVertices[i].x;
				dst.v = c * texVertices[i].y;
				dst.q = c;
			}
		}
	}

	dst.SetVerticesUsed( numVertices );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


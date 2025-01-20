//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TesselatorCircle.h"

#include "Rtt_Matrix.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TesselatorCircle::TesselatorCircle( Real radius )
:	Super(),
	fRadius( radius )
{
}

void
TesselatorCircle::GenerateFill( ArrayVertex2& vertices )
{
	// TODO: Everytime this is called, we subdivide the same exact unit circle
	// Precalculate this and put it into a constant table for lookup.
	// This would completely eliminate the need to call Subdivide
	// Also can be used for the rounded rects

	AppendCircle( vertices, fRadius, 0 );
}

void
TesselatorCircle::GenerateFillTexture( ArrayVertex2& texCoords, const Transform& t )
{
	ArrayVertex2& vertices = texCoords;

	AppendCircle( vertices, fRadius, kNoScale ); // Unit circle
	Normalize( vertices );

	if ( t.IsIdentity() )
	{
		for ( int i = 0, iMax = vertices.Length(); i < iMax; i++ )
		{
			Vertex2& v = vertices[i];
			v.x = Rtt_RealMul( v.x + Rtt_REAL_1, Rtt_REAL_HALF );
			v.y = Rtt_RealMul( v.y + Rtt_REAL_1, Rtt_REAL_HALF );
		}
	}
	else
	{
		Matrix m;
		m.Scale( Rtt_REAL_HALF * t.GetSx(), Rtt_REAL_HALF * t.GetSy() );
		m.Rotate( - t.GetRotation() );
		m.Translate( t.GetX() + Rtt_REAL_HALF, t.GetY() + Rtt_REAL_HALF );
		m.Apply( vertices.WriteAccess(), vertices.Length() );
	}
}

void
TesselatorCircle::GenerateStroke( ArrayVertex2& vertices )
{
	AppendCircleStroke( vertices, fRadius, fInnerWidth, fOuterWidth, false );
}

void
TesselatorCircle::GetSelfBounds( Rect& rect )
{
	rect.Initialize( fRadius, fRadius );
}

U32
TesselatorCircle::FillVertexCount() const
{
	return AppendCircleCount( fRadius, 0 );
}

U32
TesselatorCircle::StrokeVertexCount() const
{
	return AppendCircleStrokeCount( fRadius, false );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


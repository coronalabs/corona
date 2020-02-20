//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TesselatorShape.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TesselatorShape::TesselatorShape()
{
	ResetNormalization();
}

void
TesselatorShape::GenerateStrokeTexture( ArrayVertex2& outTexCoords, int numVertices )
{
	ArrayVertex2& vertices = outTexCoords;

	Rtt_ASSERT( vertices.Length() == 0 );

	AppendStrokeTextureClosed( vertices, numVertices );
}

Geometry::PrimitiveType
TesselatorShape::GetFillPrimitive() const
{
	return Geometry::kTriangleStrip;
}

bool
TesselatorShape::SetSelfBounds( Real width, Real height )
{
	return false;
}

void
TesselatorShape::Normalize( ArrayVertex2& vertices )
{
	if ( ShouldNormalize() )
	{
		Vertex2_Scale( vertices.WriteAccess(), vertices.Length(),
					  Rtt_RealIsZero(fScaleX)?Rtt_REAL_1:fScaleX,
					  Rtt_RealIsZero(fScaleY)?Rtt_REAL_1:fScaleY );
	}
}

bool
TesselatorShape::ShouldNormalize() const
{
	return ! Rtt_RealIsZero( fScaleX ) || ! Rtt_RealIsZero( fScaleY );
}

void
TesselatorShape::ResetNormalization()
{
	fScaleX = Rtt_REAL_0;
	fScaleY = Rtt_REAL_0;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


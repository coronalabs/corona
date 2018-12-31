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


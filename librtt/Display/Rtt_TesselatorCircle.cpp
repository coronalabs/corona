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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


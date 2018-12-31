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

#include "Display/Rtt_CompositePaint.h"

#include "Renderer/Rtt_RenderData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CompositePaint::CompositePaint( Paint *paint0, Paint *paint1 )
:	Super(),
	fPaint0( paint0 ),
	fPaint1( paint1 )
{
	Initialize( kMultitexture );
}

CompositePaint::~CompositePaint()
{
	Rtt_DELETE( fPaint1 );
	Rtt_DELETE( fPaint0 );
}

void
CompositePaint::UpdatePaint( RenderData& data )
{
	Super::UpdatePaint( data );

	data.fFillTexture0 = fPaint0->GetTexture();
	data.fFillTexture1 = fPaint1->GetTexture();
}

Texture *
CompositePaint::GetTexture() const
{
	// Rtt_ASSERT_NOT_REACHED();

	// Just in case...
	return fPaint0->GetTexture();
}

const Paint*
CompositePaint::AsPaint( Type type ) const
{
	const Paint *result = Super::AsPaint( type );

	if ( ! result )
	{
		result = fPaint0->AsPaint( type );
	}

	if ( ! result )
	{
		result = fPaint1->AsPaint( type );
	}

	return result;
}
	
	
void
CompositePaint::ApplyPaintUVTransformations( ArrayVertex2& vertices ) const
{
	fPaint0->ApplyPaintUVTransformations( vertices );
}

/*
const MLuaUserdataAdapter&
CompositePaint::GetAdapter() const
{
}
*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


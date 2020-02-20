//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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


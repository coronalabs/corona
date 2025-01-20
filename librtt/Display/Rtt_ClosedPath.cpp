//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ClosedPath.h"

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_Shader.h"
#include "Renderer/Rtt_Program.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ClosedPath::ClosedPath( Rtt_Allocator* pAllocator )
:	fFill( NULL ),
	fStroke( NULL ),
	fStrokeData( NULL ),
	fProperties( 0 ),
	fDirtyFlags( kDefault ),
	fInnerStrokeWidth( 0 ),
	fOuterStrokeWidth( 0 )
{
}

ClosedPath::~ClosedPath()
{
	Rtt_DELETE( fStroke );
	if ( ! IsProperty( kIsFillWeakReference ) )
	{
		Rtt_DELETE( fFill );
	}
}

void
ClosedPath::Update( RenderData& data, const Matrix& srcToDstSpace )
{

	if ( HasFill() && ! fFill->IsValid(Paint::kTextureTransformFlag) )
	{
		Invalidate( kFillSourceTexture );
	}
	
	if ( HasStroke() && ! fStroke->IsValid(Paint::kTextureTransformFlag) )
	{
		Invalidate( kStrokeSourceTexture );
	}

}

void
ClosedPath::Translate( Real dx, Real dy )
{
	if ( HasFill() )
	{
		fFill->Translate( dx, dy );
	}

	if ( HasStroke() )
	{
		fStroke->Translate( dx, dy );
	}
}

bool
ClosedPath::SetSelfBounds( Real width, Real height )
{
	return false;
}

void
ClosedPath::UpdatePaint( RenderData& data )
{
	if ( HasFill() )
	{
		fFill->UpdatePaint( data );
	}

	if ( HasStroke() && fStrokeData )
	{
		fStroke->UpdatePaint( * fStrokeData );
	}
}

void
ClosedPath::UpdateColor( RenderData& data, U8 objectAlpha )
{
	if ( HasFill() )
	{
		fFill->UpdateColor( data, objectAlpha );
	}

	if ( HasStroke() && fStrokeData )
	{
		fStroke->UpdateColor( * fStrokeData, objectAlpha );
	}
}

void
ClosedPath::SetFill( Paint* newValue )
{
	if ( IsProperty( kIsFillLocked ) )
	{
		// Caller expects receiver to own this, so we delete it
		// b/c the fill is locked. Otherwise it will leak.
		Rtt_DELETE( newValue );
		return;
	}

	if ( fFill != newValue )
	{
		if ( ! fFill )
		{
			// If fill was NULL, then we need to ensure
			// source vertices are generated
			Invalidate( kFillSource | kFillSourceTexture );
		}

		if ( ! IsProperty( kIsFillWeakReference ) )
		{
			Rtt_DELETE( fFill );
		}
		fFill = newValue;

		if ( newValue )
		{
			newValue->SetObserver( GetObserver() );
		}
	}
}

void
ClosedPath::SwapFill( ClosedPath& rhs )
{
	Paint* paint = rhs.fFill;
	rhs.fFill = fFill;
	fFill = paint;

	if ( fFill )
	{
		fFill->SetObserver( GetObserver() );
	}

	if ( rhs.fFill )
	{
		rhs.fFill->SetObserver( rhs.GetObserver() );
	}

	Invalidate( kFillSource );
}

void
ClosedPath::SetStroke( Paint* newValue )
{
	if ( fStroke != newValue )
	{
		if ( ! fStroke )
		{
			// If stroke was NULL, then we need to ensure
			// source vertices are generated
			Invalidate( kStrokeSource | kStrokeSourceTexture );
		}

		Rtt_DELETE( fStroke );
		fStroke = newValue;

		if ( newValue )
		{
			newValue->SetObserver( GetObserver() );
		}
	}
}

void
ClosedPath::SetInnerStrokeWidth( U8 newValue )
{
	fInnerStrokeWidth = newValue;
	Invalidate( kStrokeSource );
}

void
ClosedPath::SetOuterStrokeWidth( U8 newValue )
{
	fOuterStrokeWidth = newValue;
	Invalidate( kStrokeSource );
}

bool
ClosedPath::IsFillVisible() const
{
	bool result = false;

	if ( HasFill() )
	{
		result = ( fFill->GetRGBA().a > Rtt_REAL_0 );
	}

	return result;
}

bool
ClosedPath::IsStrokeVisible() const
{
	bool result = false;

	if ( HasStroke() )
	{
		result = ( fStroke->GetRGBA().a > Rtt_REAL_0 ) && GetStrokeWidth() > Rtt_REAL_0;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


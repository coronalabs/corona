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


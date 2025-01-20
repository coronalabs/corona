//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LineObject.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_OpenPath.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Renderer/Rtt_Renderer.h"
#include "Rtt_LuaProxyVTable.h"

#include "Rtt_Profiling.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

LineObject*
LineObject::NewLine( Rtt_Allocator* pAllocator, Real xStart, Real yStart, Real xEnd, Real yEnd )
{
	OpenPath* path = Rtt_NEW( pAllocator, OpenPath( pAllocator ) );
	Vertex2 vStart = { xStart, yStart };
	path->Append( vStart );

	Vertex2 vEnd = { xEnd, yEnd };
	path->Append( vEnd );

	return Rtt_NEW( pAllocator, LineObject( path ) );
}

// ----------------------------------------------------------------------------

LineObject::LineObject( OpenPath* path )
:	Super(),
	fStrokeData(),
	fShaderColor( ColorZero() ),
	fPath( path ),
	fStrokeShader( NULL ),
	fAnchorSegments( false )
{
	Rtt_ASSERT( fPath );
	fPath->SetObserver( this );

    SetObjectDesc("LineObject"); // for introspection
}

LineObject::~LineObject()
{
	Rtt_DELETE( fPath );
}

bool
LineObject::UpdateTransform( const Matrix& parentToDstSpace )
{
	bool shouldUpdate = Super::UpdateTransform( parentToDstSpace );

	if ( shouldUpdate )
	{
		fPath->Invalidate( OpenPath::kStroke );
	}

	return shouldUpdate;
}

void
LineObject::Prepare( const Display& display )
{
	Rtt_ASSERT( fPath );

	Super::Prepare( display );

	SUMMED_TIMING( lp, "Line: post-Super::Prepare" );

	if ( ShouldPrepare() )
	{
		// NOTE: We need to update paint *prior* to geometry
		// b/c in the case of image sheets, the paint needs to be updated
		// in order for the texture coordinates to be updated.
		if ( ! IsValid( kPaintFlag ) )
		{
			fPath->GetStroke()->UpdatePaint( fStrokeData );
			SetValid( kPaintFlag );
		}

		if ( ! IsValid( kGeometryFlag ) )
		{
			const Matrix& xform = GetSrcToDstMatrix();
			fPath->Update( fStrokeData, xform );
			SetValid( kGeometryFlag );
		}

		if ( ! IsValid( kColorFlag ) )
		{
			fPath->GetStroke()->UpdateColor( fStrokeData, AlphaCumulative() );
			SetValid( kColorFlag );
		}

		if ( ! IsValid( kProgramDataFlag ) )
		{
			SetValid( kProgramDataFlag );
		}

		// Program
		if ( ! IsValid( kProgramFlag ) )
		{
			Rect bounds;
			fPath->GetSelfBounds( bounds );
			int w = Rtt_RealToInt( bounds.Width() );
			int h = Rtt_RealToInt( bounds.Height() );

			ShaderFactory& factory = display.GetShaderFactory();

			Paint *stroke = fPath->GetStroke();
			if ( stroke )
			{
				Shader *shader = stroke->GetShader(factory);
				shader->Prepare( fStrokeData, w, h, ShaderResource::kDefault );
				fStrokeShader = shader;
			}

			SetValid( kProgramFlag );
		}
	}
}

/*
void
LineObject::Translate( Real dx, Real dy )
{
	Rtt_ASSERT( fPath );

	Super::Translate( dx, dy );

	if ( IsValid() && IsNotHidden() )
	{
//		if ( ! IsProperty( kIsTransformLocked ) )
		{
			fPath->Translate( dx, dy );
		}
	}
}
*/

void
LineObject::Draw( Renderer& renderer ) const
{
	if ( ShouldDraw() )
	{
		SUMMED_TIMING( ld, "Line: Draw" );

		Rtt_ASSERT( fPath );

		fPath->UpdateResources( renderer );

		if ( fPath->HasStroke() && fPath->IsStrokeVisible() )
		{
			fStrokeShader->Draw( renderer, fStrokeData );
		}
	}
}

void
LineObject::GetSelfBounds( Rect& rect ) const
{
	fPath->GetSelfBounds( rect );
}

const LuaProxyVTable&
LineObject::ProxyVTable() const
{
	return LuaLineObjectProxyVTable::Constant();
}

void
LineObject::SetStroke( Paint* newValue )
{
	DirtyFlags flags = ( kPaintFlag | kProgramFlag );
	if ( Paint::ShouldInvalidateColor( fPath->GetStroke(), newValue ) )
	{
		flags |= kColorFlag;
	}
	if ( newValue && NULL == fPath->GetStroke() )
	{
		// When paint goes from NULL to non-NULL,
		// ensure geometry is prepared
		flags |= kGeometryFlag;
	}
	Invalidate( flags );

	fPath->SetStroke( newValue );

	DidChangePaint( fStrokeData );
}

void
LineObject::SetStrokeColor( Color newValue )
{
	Paint *paint = GetPath().GetStroke();
	if ( paint )
	{
		paint->SetColor( newValue );
		Invalidate( kGeometryFlag | kColorFlag );
	}	
}

void
LineObject::SetStrokeWidth( Real newValue )
{
	if ( GetStrokeWidth() != newValue )
	{
		fPath->SetWidth( newValue );
		Invalidate( kGeometryFlag | kStageBoundsFlag );
	}
}

void
LineObject::Append( const Vertex2& p )
{
	const Matrix& m = GetMatrix();
	Vertex2 v = { p.x - m.Tx(), p.y - m.Ty() };
	fPath->Append( v );
	Invalidate( kGeometryFlag | kStageBoundsFlag );
}
    
void
LineObject::SetBlend( RenderTypes::BlendType newValue )
{
    Paint *paint = fPath->GetStroke();
    paint->SetBlend( newValue );
}

RenderTypes::BlendType
LineObject::GetBlend() const
{
	const Paint *paint = fPath->GetStroke();
	return paint->GetBlend();
}

void
LineObject::SetAnchorSegments( bool should_anchor )
{
	if( fAnchorSegments != should_anchor )
	{
		Invalidate( kGeometryFlag | kTransformFlag | kMaskFlag | kStageBoundsFlag );
	}

	fAnchorSegments = should_anchor;
}

bool
LineObject::ShouldOffsetWithAnchor() const
{
	if( IsV1Compatibility() )
	{
		return Super::ShouldOffsetWithAnchor();
	}
	else
	{
		return fAnchorSegments;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_CompositeObject.h"

#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_StageObject.h"
#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Renderer.h"
#include "Rtt_LuaProxy.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CompositeObject::CompositeObject( Rtt_Allocator* pAllocator )
:	Super(),
	fData()
{
	for ( int i = 0; i < kNumInputs; i++ )
	{
		fInputs[i] = NULL;
		fFBOs[i] = NULL;
	}

	const U32 kVerticesPerQuad = 4;
	Geometry *geometry = Rtt_NEW( pAllocator,
		Geometry( pAllocator, Geometry::kTriangleStrip, kVerticesPerQuad, 0, false ) );
	fData.fGeometry = geometry;

    SetObjectDesc( "CompositeObject" );

//	Rtt::Geometry::Vertex* data = geometry->GetVertexData();
}

CompositeObject::~CompositeObject()
{
	for ( int i = kNumInputs; --i >= 0; )
	{
		Rtt_DELETE( fFBOs[i] );
		Rtt_DELETE( fInputs[i] );
	}

	Rtt_DELETE( fData.fGeometry );
}

bool
CompositeObject::UpdateTransform( const Matrix& parentToDstSpace )
{
	bool shouldUpdate = Super::UpdateTransform( parentToDstSpace );

	if ( shouldUpdate )
	{
		for ( int i = 0; i < kNumInputs; i++ )
		{
			DisplayObject *input = fInputs[i];
			if ( input )
			{
				input->UpdateTransform( parentToDstSpace );
			}
		}
	}
	
	return shouldUpdate;
}

void
CompositeObject::Prepare( const Display& display )
{
	Super::Prepare( display );

	if ( ShouldPrepare() )
	{
		for ( int i = 0; i < kNumInputs; i++ )
		{
			DisplayObject *input = fInputs[i];
			if ( input )
			{
				input->Prepare( display );
			}
		}
	}
}

void
CompositeObject::Draw( Renderer& renderer ) const
{
	if ( ShouldDraw() )
	{
//		fData.fProgram
	}
}

bool
CompositeObject::CanCull() const
{
	return false;
}

void
CompositeObject::DidInsert( bool childParentChanged )
{
	if ( childParentChanged )
	{
		Invalidate( kStageBoundsFlag );
	}
	else
	{
		InvalidateDisplay();
	}
}

void
CompositeObject::DidRemove()
{
	Invalidate( kStageBoundsFlag );
}

void
CompositeObject::SetInput( Input input, DisplayObject *child )
{
	Rtt_ASSERT( input < kNumInputs );

	DisplayObject *oldChild = fInputs[input];
	if ( oldChild != child )
	{
		fInputs[input] = child;
	}
}

DisplayObject *
CompositeObject::ReleaseInput( Input input )
{
	Rtt_ASSERT( input < kNumInputs );

	DisplayObject *oldChild = fInputs[input];

	fInputs[input] = NULL;
	
	return oldChild;
}

void
CompositeObject::SetBlend( RenderTypes::BlendType newValue )
{
/*
	Paint *paint = fPath->GetFill();
	paint->SetBlend( newValue );

	paint = fPath->GetStroke();
	if ( paint )
	{
		paint->SetBlend( newValue );
	}
*/
	Invalidate( kPaintFlag );
}

RenderTypes::BlendType
CompositeObject::GetBlend() const
{
/*
	const Paint *paint = fPath->GetFill();

	// Either there's no stroke or (if there is one), it's blend type should match the fill's
	Rtt_ASSERT( ! fPath->GetStroke() || paint->GetBlend() == fPath->GetStroke()->GetBlend() );
	return paint->GetBlend();
*/
	return RenderTypes::kNormal;
}

void
CompositeObject::SetShader( const Shader *newValue )
{
	fShader = newValue;
	Invalidate( kProgramFlag );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ContainerObject.h"

#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_TextureFactory.h"
#include "Renderer/Rtt_Renderer.h"
#include "Renderer/Rtt_Uniform.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Resource.h"
#include "Rtt_Runtime.h"

#include "Rtt_Profiling.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ContainerObject::ContainerObject(
	Rtt_Allocator* pAllocator, StageObject *canvas, Real width, Real height )
:	Super( pAllocator, canvas ),
	fContainerMask( NULL ),
	fContainerMaskUniform( NULL ),
	fWidth( width ),
	fHeight( height )
{
	Invalidate( kContainerFlag );

	SetProperty( kIsAnchorChildren, true );

    SetObjectDesc( "ContainerObject" );     // for introspection
}

ContainerObject::~ContainerObject()
{
	QueueRelease( fContainerMaskUniform );
	Rtt_DELETE( fContainerMask );
}

void
ContainerObject::Initialize( Display& display )
{
	if ( Rtt_VERIFY( NULL == fContainerMask ) )
	{
//		PlatformBitmap *maskBitmap = display.GetTextureFactory().GetContainerMaskBitmap();
//		BitmapPaint *paint =
//			BitmapPaint::NewBitmap( display.GetTextureFactory(), maskBitmap, true );

		SharedPtr< TextureResource > resource = display.GetTextureFactory().GetContainerMask();
		BitmapPaint *paint = Rtt_NEW( display.GetAllocator(), BitmapPaint( resource ) );

		fContainerMask = Rtt_NEW( library.GetAllocator(), BitmapMask( paint, fWidth, fHeight ) );
		fContainerMaskUniform = Rtt_NEW( Allocator(), Uniform( Allocator(), Uniform::kMat3 ) );
	}
}

bool
ContainerObject::UpdateTransform( const Matrix& parentToDstSpace )
{
	SUMMED_TIMING( cut, "ContainerObject: UpdateTransform" );

	bool shouldUpdate = Super::UpdateTransform( parentToDstSpace );

	if ( shouldUpdate || ! IsValid( kContainerFlag ) )
	{
		Rtt_ASSERT( fContainerMaskUniform );

		SUMMED_TIMING( cutps, "ContainerObject: post-Super::UpdateTransform" );

		Matrix dstToMask;

		// Initialize container mask transform, so that the resulting
		// container bounds are rendered with the correct anchoring/registration.
		Transform& maskTransform = fContainerMask->GetTransform();
		maskTransform.SetIdentity();
		maskTransform.Invalidate();

		// If the transform does not include the offset, we need to include it here
		if ( ShouldOffsetClip() )
		{
			Vertex2 offset = GetAnchorOffset();
			maskTransform.Translate( offset.x, offset.y );
		}

		// First, initialize using standard mask that maps UV to (0,0) (1,1)
		CalculateMaskMatrix( dstToMask, GetSrcToDstMatrix(), * fContainerMask );
		
		// Then, append transforms which map UV to (0.25,0.25) (0.75,0.75)
		// This subregion is the portion of the mask that's white.
		// Outside of that region, mask is black (border).
		// NOTE: Recall that mask bits are from NewBufferBitmap()
		dstToMask.Scale( Rtt_REAL_HALF, Rtt_REAL_HALF );
		dstToMask.Translate( Rtt_REAL_FOURTH, Rtt_REAL_FOURTH );

		dstToMask.ToGLMatrix3x3( reinterpret_cast< Real * >( fContainerMaskUniform->GetData() ) );
		fContainerMaskUniform->Invalidate();
	}

	return shouldUpdate;
}

void
ContainerObject::Draw( Renderer& renderer ) const
{
	if ( ShouldDraw()
		 && ( fWidth > Rtt_REAL_0 && fHeight > Rtt_REAL_0 ) )
	{
		SUMMED_TIMING( cd, "Container: Draw" );

		Rtt_ASSERT( ! IsDirty() );
		Rtt_ASSERT( ! IsOffScreen() );

		const BitmapMask *mask = fContainerMask;

		if ( mask )
		{
			Texture *texture = const_cast< BitmapPaint * >( mask->GetPaint() )->GetTexture();
			Uniform *uniform = const_cast< Uniform * >( fContainerMaskUniform );
			renderer.PushMask( texture, uniform );
		}

		Super::Draw( renderer );

		if ( mask )
		{
			renderer.PopMask();
		}
	}
}

void
ContainerObject::GetSelfBounds( Rect& rect ) const
{
	GetSelfBoundsForAnchor( rect );
	
	// The self bounds is based on the clipping bounds,
	// so only offset here if the transform does not include the offset
	// since we need to include the offset somewhere.
	if ( ShouldOffsetClip() )
	{
		Real dx = -( fWidth * GetInternalAnchorX() );
		Real dy = -( fHeight * GetInternalAnchorY() );
		rect.Translate( dx, dy );
	}
}

// NOTE: This is what is used to calculate the anchor offset lengths
void
ContainerObject::GetSelfBoundsForAnchor( Rect& rect ) const
{
	rect.Initialize( Rtt_RealDiv2( fWidth ), Rtt_RealDiv2( fHeight ) );
}

bool
ContainerObject::HitTest( Real contentX, Real contentY )
{
	BuildStageBounds();
	return StageBounds().HitTest( contentX, contentY );
}

bool
ContainerObject::CanCull() const
{
	return true;
}

void
ContainerObject::SetSelfBounds( Real width, Real height )
{
	if ( ! Rtt_RealEqual( width, fWidth ) || ! Rtt_RealEqual( height, fHeight ) )
	{
		if ( !( width < Rtt_REAL_0 ) ) // (width >= 0)
		{
			fWidth = width;
		}
		if ( !( height < Rtt_REAL_0 ) ) // (height >= 0)
		{
			fHeight = height;
		}

		fContainerMask->SetSelfBounds( width, height );

		// TODO: Also kGeometryFlag?
		Invalidate( kContainerFlag | kTransformFlag );
		
		// TODO:
		// * For pure groups: If children invalidate stagebounds and anchorChildren is true, then invalidate transform
		const_cast< Transform& >( GetTransform() ).Invalidate();
	}
}

bool
ContainerObject::ShouldOffsetClip() const
{
	// Only offset here if we do not offset the transform
	return ( ! ShouldOffsetWithAnchor() && ! IsAnchorOffsetZero() );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


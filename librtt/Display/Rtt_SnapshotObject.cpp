//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_SnapshotObject.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_BufferBitmap.h"
#include "Display/Rtt_ClosedPath.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_GroupObject.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_StageObject.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Matrix_Renderer.h"
#include "Renderer/Rtt_Renderer.h"
#include "Renderer/Rtt_Texture.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"

#include "Rtt_Profiling.h"

#include "Display/Rtt_ClosedPath.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// static const char kGroupString[] = "group";
static const char kCanvasString[] = "canvas";

SnapshotObject::RenderFlag
SnapshotObject::RenderFlagForString( const char *str )
{
	RenderFlag result = kGroupFlag;
	
	if ( str )
	{
		if ( 0 == strcmp( str, kCanvasString ) )
		{
			result = kCanvasFlag;
		}
	}

	return result;
}

/*
const char *
SnapshotObject::StringForRenderFlag( RenderFlag flag )
{
	const char *result = kGroupString;
	
	if ( flag & kCanvasFlag )
	{
		result = kCanvasString;
	}

	return result;
}
*/

// ----------------------------------------------------------------------------

static const char kAppendString[] = "append";
static const char kDiscardString[] = "discard";

SnapshotObject::CanvasMode
SnapshotObject::CanvasModeForString( const char *str )
{
	CanvasMode result = kDefaultMode;

	if ( str )
	{
		if ( 0 == strcmp( str, kDiscardString ) )
		{
			result = kDiscardMode;
		}
	}

	return result;
}

const char *
SnapshotObject::StringForCanvasMode( CanvasMode mode )
{
	const char *result = kAppendString;
	
	switch ( mode )
	{
		case kDiscardMode:
			result = kDiscardString;
			break;
		default:
			break;
		
	}

	return result;
}

// ----------------------------------------------------------------------------

SnapshotObject::SnapshotObject(
	Rtt_Allocator *pAllocator,
	Display& display,
	Real contentW, 
	Real contentH )
:	Super( RectPath::NewRect( pAllocator, contentW, contentH ) ),
	fGroup( Rtt_NEW( pAllocator, GroupObject( pAllocator, display.GetStageOffscreen() ) ) ),
	fCanvas( Rtt_NEW( pAllocator, GroupObject( pAllocator, display.GetStageOffscreen() ) ) ),
	fFrameBufferObject( NULL ),
	fClearColor( ColorZero() ),
	fOrphanage( * display.Orphanage() ),
	fDirtyFlags( kDefaultFlag ),
	fCanvasMode( kDefaultMode )
{
	// At the Lua level, prevent certain operations on fGroup and fCanvas. In particular:
	// * they cannot be an argument to newParent:insert()
	// * they cannot have removeSelf() called on them
	// We want to prevent transfer of ownership. The snapshot is the sole owner of these groups.
	fGroup->SetRenderedOffScreen( true );
	fCanvas->SetRenderedOffScreen( true );

    SetObjectDesc( "SnapshotObject" );     // for introspection
}

SnapshotObject::~SnapshotObject()
{
	QueueRelease( fFrameBufferObject );

	Rtt_DELETE( fCanvas );
	Rtt_DELETE( fGroup );
}

void
SnapshotObject::FinalizeSelf( lua_State *L )
{
	Super::FinalizeSelf( L );

	// Members have Lua resources that need to be released
	fGroup->FinalizeSelf( L );
	fCanvas->FinalizeSelf( L );

	// Since this 'fake' groups are never moved to Orphanage, we should release their children proxys
	fGroup->ReleaseChildrenLuaReferences( L );
	fCanvas->ReleaseChildrenLuaReferences( L );
}

void
SnapshotObject::Initialize( lua_State *L, Display& display, Real contentW, Real contentH )
{
	// Group proxy init
	fGroup->InitProxy( L );
	fCanvas->InitProxy( L );

	Rtt_ASSERT( GetStage() );
	
	// Calculate pixel dimensions for texture
	S32 unused = 0;
	S32 pixelW = Rtt_RealToInt( contentW );
	S32 pixelH = Rtt_RealToInt( contentH );
	display.ContentToScreen( unused, unused, pixelW, pixelH );

	TextureFactory& factory = display.GetTextureFactory();

	RenderTypes::TextureFilter magFilter = display.GetDefaults().GetMagTextureFilter();
	RenderTypes::TextureWrap wrap = display.GetDefaults().GetTextureWrapX();

	// TODO: Support minFilter and both wrapX/Y.
	// We're cheating right now.
	SharedPtr< TextureResource > resource = factory.Create(
		pixelW,
		pixelH,
		Texture::kRGBA,
		RenderTypes::Convert( magFilter ),
		RenderTypes::Convert( wrap ),
		false );

	// Set texture to be "retina" which ensures that the Renderer passes
	// correct scale factors to the shader
	resource->GetTexture().SetRetina( true );

	BitmapPaint *paint = Rtt_NEW( display.GetAllocator(), BitmapPaint( resource ) );
	SetFill( paint );

	// Prevent 'paint' from being replaced by a different paint
	// Also, the FBO instance holds a reference to the Texture instance
	// that's inside 'paint', so both FBO and paint should be destroyed
	// at the same time.
	GetPath().SetFillLocked( true );


	if (!fFrameBufferObject)
	{
		FrameBufferObject::ExtraOptions opts = {};
		/* TODO
			fHasDepth = display.GetDefaults().GetAddDepthToResource();
			fHasStencil = display.GetDefaults().GetAddStencilToResource();
			fDepthClearValue = display.GetDefaults().GetAddedDepthClearValue();
			fStencilClearValue = display.GetDefaults().GetAddedStencilClearValue();
		 
			TODO:
				if (opts.depthBits) add appropriate frame buffer resources...
				if (opts.stencilBits) ditto
		 */

		fFrameBufferObject = Rtt_NEW( display.GetAllocator(), FrameBufferObject( display.GetAllocator(), & resource->GetTexture(), &opts ) ); // n.b. mustClear = false
	}
}

bool
SnapshotObject::UpdateTransform( const Matrix& parentToDstSpace )
{
	if ( IsDirty() )
	{
		GroupObject& group = GetGroup();

		// "Push" the bounds to the stage so offscreen culling is done
		// via the snapshot's boundary instead of the screen's
		Rect bounds;
		GetSelfBounds( bounds );
		group.GetStage()->SetSnapshotBounds( & bounds );
		{
			// During the offscreen render, group is relative to (0,0)
			Matrix identity;

			if ( ShouldRenderCanvas() )
			{
				// fCanvas cannot have a mask set on it. Otherwise there's
				// a possibility that it will be out of sync with fGroup
				fCanvas->SetMask( NULL, NULL );
				
				// Ensure alpha is the same
				fCanvas->SetAlpha( group.Alpha() );

				if ( ShouldRenderGroup()
					 && kAppendMode == GetCanvasMode() )
				{
					// Special case:
					// If fGroup will be re-rendered anyway,
					// then append fCanvas onto fGroup, so we render to FBO only once.

					// Append/release children of fCanvas into fGroup
					MoveChildren( * fCanvas );

					// Clear canvas flag, since fCanvas will be empty
					ClearFlag( kCanvasFlag );
				}
				else
				{
					// Ensure fCanvas has same transform as fGroup
					const Transform& t = static_cast< const GroupObject & >(group).GetTransform();
					fCanvas->SetTransform( t );

					fCanvas->UpdateTransform( identity );
				}
			}

			if ( ShouldRenderGroup() )
			{
				group.UpdateTransform( identity );
			}
		}
		group.GetStage()->SetSnapshotBounds( NULL );
	}

	return Super::UpdateTransform( parentToDstSpace );
}

void
SnapshotObject::Prepare( const Display& display )
{
	SUMMED_TIMING( sp, "Snapshot: Prepare" );

	if ( ShouldRenderGroup() )
	{
		GroupObject& group = GetGroup();

		if ( group.NumChildren() > 0 )
		{
			group.Prepare( display );
		}
	}

	if ( ShouldRenderCanvas() )
	{
		GroupObject& group = GetCanvas();

		if ( group.NumChildren() > 0 )
		{
			group.Prepare( display );
		}
	}

	Super::Prepare( display );
}

void
SnapshotObject::RenderToFBO(
	Renderer& renderer,
	FrameBufferObject *dstFBO,
	const Rect& bounds,
	const DisplayObject& object,
	const Color *clearColor )
{
	// Save current state so we can restore it later
	FrameBufferObject *fbo = renderer.GetFrameBufferObject();

	Rtt::Real viewMatrix[16];
	Rtt::Real projMatrix[16];
	renderer.GetFrustum( viewMatrix, projMatrix );

	S32 x, y, width, height;
	renderer.GetViewport( x, y, width, height );

	// Render all children to the offscreen texture
	Rtt::Real offscreenViewMatrix[16];
	Rtt::CreateViewMatrix( 0.0f, 0.0f, 0.5f,
							0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f,
							offscreenViewMatrix );

	Rtt::Real offscreenProjMatrix[16];
	Real texW = Rtt_IntToReal( dstFBO->GetTexture()->GetWidth() );
	Real texH = Rtt_IntToReal( dstFBO->GetTexture()->GetHeight() );

	// The orthographic projection is set up so that the snapshot bounds
	// are centered about the group's origin (0,0)
	// TODO: Should we remove fContentBounds???

	Rtt::CreateOrthoMatrix(
		bounds.xMin, bounds.xMax,
		bounds.yMin, bounds.yMax,
		0.0f, 1.0f, offscreenProjMatrix );

	renderer.SetFrameBufferObject( dstFBO );
	renderer.PushMaskCount();
	{
		renderer.SetFrustum( offscreenViewMatrix, offscreenProjMatrix );
		renderer.SetViewport( 0, 0, texW, texH );
		if ( clearColor )
		{
			/* TODO
				Renderer::ExtraClearOptions extra;
				
				extra.clearDepth = fHasDepth;
				extra.clearStencil = fHasStencil;
				extra.depthClearValue = fDepthClearValue;
				extra.stencilClearValue = fStencilClearValue;
			  */
			ColorUnion color;
			color.pixel = * clearColor;
			const Real inv255 = 1.f / 255.f;
			renderer.Clear( color.rgba.r * inv255, color.rgba.g * inv255, color.rgba.b * inv255, color.rgba.a * inv255/*, &extra */ );
		}
		
		renderer.BeginDrawing();
		object.Draw( renderer );
	}
	renderer.PopMaskCount();

	// Restore state so further rendering is unaffected
	renderer.SetViewport( x, y, width, height );
	renderer.SetFrustum( viewMatrix, projMatrix );
	renderer.SetFrameBufferObject( fbo );
}

void
SnapshotObject::DrawGroup( Renderer& renderer, const GroupObject& group, const Color *clearColor ) const
{
	if ( group.NumChildren() > 0 || clearColor )
	{
		// NOTE: We want undistorted bounds (ignoring 2.5D corner offsets)
		Rect bounds;
		GetSelfBoundsForAnchor( bounds );

		RenderToFBO( renderer, fFrameBufferObject, bounds, group, clearColor );
	}
}

void
SnapshotObject::Draw( Renderer& renderer ) const
{
	SUMMED_TIMING( sd, "Snapshot: Draw" );

	if ( ShouldRenderGroup() )
	{
		SUMMED_TIMING( srg, "Snapshot: Render Group" );

		DrawGroup( renderer, GetGroup(), & fClearColor );
	}

	if ( ShouldRenderCanvas() )
	{
		SUMMED_TIMING( srg, "Snapshot: Render Canvas" );

		DrawGroup( renderer, GetCanvas(), NULL );
		
		// Append/release children of other fCanvas into fGroup
		GroupObject& src = const_cast< GroupObject& >( GetCanvas() );
		if ( kAppendMode == GetCanvasMode() )
		{
			SnapshotObject *dst = const_cast< SnapshotObject * >( this );
			dst->MoveChildren( src );
		}
		else
		{
			// Remove starting from end of src
			for ( int i = src.NumChildren(); i-- > 0; )
			{
				DisplayObject* child = src.Release( i );
				fOrphanage.Insert( -1, child, false );
			}
		}
	}
	
	fDirtyFlags = 0; // Clear all flags

	Super::Draw( renderer );
}

const LuaProxyVTable&
SnapshotObject::ProxyVTable() const
{
	return LuaSnapshotObjectProxyVTable::Constant();
}

void
SnapshotObject::MoveChildren( GroupObject& src )
{
	// Move children of src into fGroup, preserving ordering
	for ( int i = 0, iMax = src.NumChildren(); i < iMax; i++ )
	{
		// Always take first object
		DisplayObject *object = src.Release( 0 );

		// Append to group
		fGroup->Insert( -1, object, false );
	}
}

void
SnapshotObject::ClearFlag( DirtyFlags flags )
{
	fDirtyFlags &= (U8)(~flags);
}

void
SnapshotObject::SetDirty( DirtyFlags flags )
{
	Invalidate( kPaintFlag );
	fDirtyFlags |= flags;
}

RenderTypes::TextureFilter
SnapshotObject::GetTextureFilter() const
{
	Rtt_ASSERT( GetPath().GetFill() );

	const Texture *texture = GetPath().GetFill()->GetTexture();
	return RenderTypes::Convert( texture->GetFilter() );
}

RenderTypes::TextureWrap
SnapshotObject::GetTextureWrapX() const
{
	Rtt_ASSERT( GetPath().GetFill() );

	const Texture *texture = GetPath().GetFill()->GetTexture();
	return RenderTypes::Convert( texture->GetWrapX() );
}

RenderTypes::TextureWrap
SnapshotObject::GetTextureWrapY() const
{
	Rtt_ASSERT( GetPath().GetFill() );

	const Texture *texture = GetPath().GetFill()->GetTexture();
	return RenderTypes::Convert( texture->GetWrapY() );
}

void
SnapshotObject::SetTextureFilter( RenderTypes::TextureFilter newValue )
{
	Rtt_ASSERT( GetPath().GetFill() );

	Texture *texture = GetPath().GetFill()->GetTexture();
	texture->SetFilter( RenderTypes::Convert( newValue ) );
	
	InvalidateDisplay();
}

void
SnapshotObject::SetTextureWrapX( RenderTypes::TextureWrap newValue )
{
	Rtt_ASSERT( GetPath().GetFill() );

	Texture *texture = GetPath().GetFill()->GetTexture();
	texture->SetWrapX( RenderTypes::Convert( newValue ) );
	
	InvalidateDisplay();
}

void
SnapshotObject::SetTextureWrapY( RenderTypes::TextureWrap newValue )
{
	Rtt_ASSERT( GetPath().GetFill() );

	Texture *texture = GetPath().GetFill()->GetTexture();
	texture->SetWrapY( RenderTypes::Convert( newValue ) );
	
	InvalidateDisplay();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


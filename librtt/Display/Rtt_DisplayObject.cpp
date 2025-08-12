//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"

#include "Rtt_DisplayObjectExtensions.h"

#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Scene.h"
#include "Display/Rtt_StageObject.h"
#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_RenderingStream.h"
#include "Core/Rtt_StringHash.h"
#include "Renderer/Rtt_Uniform.h"

#ifdef Rtt_PHYSICS
    #include "Rtt_Runtime.h"
    #include "Box2D/Box2D.h"
#endif


#include "Display/Rtt_ShaderFactory.h"

#include "Rtt_Profiling.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

DisplayObject::ListenerMask
DisplayObject::MaskForString( const char *name )
{
    ListenerMask result = kUnknownListener;

    if ( name )
    {
        if ( 0 == strcmp( name, TouchEvent::kName ) )
        {
            result = kTouchListener;
        }
        else if ( 0 == strcmp( name, TapEvent::kName ) )
        {
            result = kTapListener;
        }
        else if ( 0 == strcmp( name, SpriteEvent::kName ) )
        {
            result = kSpriteListener;
        }
        else if ( 0 == strcmp( name, MouseEvent::kName ) )
        {
            result = kMouseListener;
        }
        else if ( 0 == strcmp( name, FinalizeEvent::kName ) )
        {
            result = kFinalizeListener;
        }
    }

    return result;
}

static StringHash * GeometricPropertiesHash = NULL;

int
DisplayObject::KeysForProperties( const char **&keys )
{
    Rtt_ASSERT( GeometricPropertiesHash != NULL );
    
    return GeometricPropertiesHash->GetKeys( keys );
}

GeometricProperty
DisplayObject::PropertyForKey( Rtt_Allocator *allocator, const char key[] )
{
    GeometricProperty result = kNumGeometricProperties;

#if 1
    // Note: Needs to match "typedef enum GeometricProperty" in librtt/Core/Rtt_Geometry.h
    static const char * keys[] =
    {
        "xOrigin#",      // 0 - DEPRECATED
        "yOrigin#",      // 1 - DEPRECATED
        "xScale",        // 2
        "yScale",        // 3
        "rotation",      // 4
        "width",         // 5
        "height",        // 6
        "stageWidth#",   // 7 - DEPRECATED
        "stageHeight#"   // 8 - DEPRECATED
    };

    if ( !GeometricPropertiesHash )
    {
        static StringHash sHash( *allocator, keys, sizeof( keys ) / sizeof( const char * ), 9, 2, 3, __FILE__, __LINE__ );
        GeometricPropertiesHash = &sHash;
    }

    int index = GeometricPropertiesHash->Lookup( key );

    switch ( index )
    {
    case 0:
        {
            result = kOriginX;
        }
        break;
    case 1:
        {
            result = kOriginY;
        }
        break;
    case 2:
        {
            result = kScaleX;
        }
        break;
    case 3:
        {
            result = kScaleY;
        }
        break;
    case 4:
        {
            result = kRotation;
        }
        break;
    case 5:
        {
            result = kWidth;
        }
        break;
    case 6:
        {
            result = kHeight;
        }
        break;
    case 7:
        {
            result = kStageWidth;
        }
        break;
    case 8:
        {
            result = kStageHeight;
        }
        break;
    default:
        {
        }
        break;
    }

#else
    // TODO: perfect hash
    if ( strcmp( "xOrigin", key ) == 0 )
    {
        result = kOriginX;
    }
    else if ( strcmp( "yOrigin", key ) == 0 )
    {
        result = kOriginY;
    }
    else if ( strcmp( "xScale", key ) == 0 )
    {
        result = kScaleX;
    }
    else if ( strcmp( "yScale", key ) == 0 )
    {
        result = kScaleY;
    }
    else if ( strcmp( "rotation", key ) == 0 )
    {
        result = kRotation;
    }
    else if ( strcmp( "width", key ) == 0 )
    {
        result = kWidth;
    }
    else if ( strcmp( "height", key ) == 0 )
    {
        result = kHeight;
    }
    else if ( strcmp( "stageWidth", key ) == 0 )
    {
        result = kStageWidth;
    }
    else if ( strcmp( "stageHeight", key ) == 0 )
    {
        result = kStageHeight;
    }


#endif

    return result;
}

// This is *not* a full clear. This is only to clear parts of the RenderData
// when we change the object's paint (fill or stroke)
void
DisplayObject::DidChangePaint( RenderData& data )
{
    data.fFillTexture0 = NULL;
    data.fFillTexture1 = NULL;
    data.fProgram = NULL;
    data.fUserUniform0 = NULL;
    data.fUserUniform1 = NULL;
    data.fUserUniform2 = NULL;
    data.fUserUniform3 = NULL;
}

// ----------------------------------------------------------------------------

DisplayObject::DisplayObject()
:    fParent( NULL ),
    fSrcToDst(),
    fTransform(),
    fStageBounds(),
    fLuaProxy( NULL ),
    fExtensions( NULL ),
    fFocusId( NULL ),
    fMask( NULL ),
    fMaskUniform( NULL ),
    fAnchorX( Rtt_REAL_0 ),
    fAnchorY( Rtt_REAL_0 ),
    fDirtyFlags( kRenderDefault ),
    fProperties( kIsVisible | kIsHitTestMasked ),
    fAlpha( 0xFF ),
    fAlphaCumulative( fAlpha ),
    fObjectDesc("DisplayObject"),
    fWhereDefined(NULL),
    fWhereChanged(NULL)
{
}

DisplayObject::~DisplayObject()
{
    // TODO: Consider removing this
    //       (1) Only on CoronaView teardown is this 'stage' the current stage.
    //           Normally, it's the offscreen or orphanage stage
    //       (2) Reset focus of current stage happens in GroupObject's MakeUnreachable().
    // Remove from focus object array so it won't have stale ptr to "this"
    StageObject *stage = const_cast< StageObject *>( GetStage() );
    if ( stage )
    {
        // Nothing good can come of keeping focus on this destructed object...
        if (stage->GetFocus() == this)
        {
            stage->SetFocus(NULL);
        }
        
        stage->SetFocus( this, NULL ); // Explicit removal
    }

    QueueRelease( fMaskUniform );
    Rtt_DELETE( fMask );
    Rtt_DELETE( fExtensions );

    // The final owner of fLuaProxy should be the Lua reference system
    // So LuaProxy should be deleted when shutting down Lua triggers a GC
    if ( fLuaProxy ) //if ( ! Rtt_VERIFY( ! fLuaProxy ) )
    {
        // If for whatever reason, the Lua VM fails to GC this, force a release.
        // This disconnects DisplayObject from the LuaProxy.
        ReleaseProxy();
    }

    free((void *)fWhereDefined);
    free((void *)fWhereChanged);
}

void
DisplayObject::FinalizeSelf( lua_State *L )
{
    Rtt_ASSERT( L ); // Must have a valid L state.

    // We need a local reference since a side effect of making this object
    // unreachable from Lua is that fLuaProxy is set to NULL.
    LuaProxy *proxy = fLuaProxy;

    if ( Rtt_VERIFY( proxy ) )
    {
        // Dispatch the object's "finalize" event before releasing its Lua proxy below.
        if ( HasListener( kFinalizeListener ) )
        {
            FinalizeEvent event;
            DispatchEvent( L, event );
        }
        
        // Ensure object is no longer reachable by Lua by converting the
        // Lua-side reference to a plain table
        proxy->RestoreTable( L );
        proxy->ReleaseTableRef( L );
    }
}

void
DisplayObject::UpdateSelfBounds( Rect& rRect ) const
{
    // If restricting to masked region, then intersect object's self bounds
    // with mask's self bounds before applying transform
    if ( IsHitTestMasked() )
    {
        Rect maskBounds;
        fMask->GetSelfBounds( maskBounds );
        rRect.Intersect( maskBounds );
    }
}

void
DisplayObject::BuildStageBounds()
{
    if ( ! IsValid( kStageBoundsFlag ) )
    {
		{
			SUMMED_TIMING( gsb, "DisplayObject: GetSelfBounds" );
        GetSelfBounds( fStageBounds );
		}
		{
			SUMMED_TIMING( usb, "DisplayObject: UpdateSelfBounds" );
        UpdateSelfBounds( fStageBounds );
		}
		{
			SUMMED_TIMING( as2db, "DisplayObject: Apply source-to-dest matrix to bounds" );
        GetSrcToDstMatrix().Apply( fStageBounds );
		}
        SetValid( kStageBoundsFlag );
    }
}

void
DisplayObject::CullOffscreen( const Rect& screenBounds )
{
    // (1) Only bother to cull objects that will actually get drawn
    // (2) Only do test on non-empty screenBounds
    if ( IsNotHidden() )
    {
        // Only cull 'leaf' nodes (or groups that have finite bounds)
        Rtt_ASSERT( NULL == AsGroupObject() || (!SkipsCull() && CanCull()) );
        Rtt_ASSERT( IsStageBoundsValid() );

        // The PREVIOUS isOffScreen state.
        bool wasOffscreen = IsOffScreen();

        // The CURRENT isOffScreen state.
        bool isOffScreen = false;
        if(screenBounds.NotEmpty())
        {
            const Rect& childBounds = StageBounds();
            isOffScreen = !screenBounds.Intersects(childBounds);
        }

        if( wasOffscreen ^ isOffScreen )
        {
            // wasOffscreen and isOffScreen are different.
            // This DisplayObject has transitioned in/out of screen bounds.
            if( isOffScreen )
            {
                // Has become invisible.
                SetOffScreen( true );
                DidMoveOffscreen();
            }
            else
            {
                // Has become visible.
                WillMoveOnscreen();
                SetOffScreen( false );
            }
        }
        else
        {
            // Nothing to do because the on/off screen state hasn't changed.
        }
    }
}

bool
DisplayObject::UpdateTransform( const Matrix& parentToDstSpace )
{
    SUMMED_TIMING( dut, "DisplayObject: UpdateTransform" );

    // By default, assume transform was not updated
    bool result = false;

    // Only update if the object is hit-testable
    if ( ShouldHitTest() )
    {
        // (1) If the build is valid, then IsValid( kTransformFlag ) == true
        // (2) If the build is not valid, then IsValid( kTransformFlag ) can be either true or false
        Rtt_ASSERT(
            ( ! IsDirty() ) == IsValid( kTransformFlag )
            || IsDirty() );

/*
        if ( ! IsValid( kAnchorFlag ) )
        {
            // NOTE: Only adjust for objects the have an anchor offset
            // This implies objects with finite extent

            // Update position so position is adjusted to preserve
            // the screen position of the object
            if ( ! IsAnchorAligned() && ShouldOffsetWithAnchor() )
            {
                Rect bounds;
                GetSelfBounds( bounds );

                // Calculate negative width,height
                Real negativeW = bounds.xMin - bounds.xMax;
                Real negativeH = bounds.yMin - bounds.yMax;

                Real anchorDx = fAnchorPrevious.x - fAnchorX;
                Real anchorDy = fAnchorPrevious.y - fAnchorY;

                Real dx = Rtt_RealMul( negativeW, anchorDx );
                Real dy = Rtt_RealMul( negativeH, anchorDy );

                Translate( dx, dy );
            }

            // Update previous anchor
            fAnchorPrevious.x = fAnchorX;
            fAnchorPrevious.y = fAnchorY;

            SetValid( kAnchorFlag );
        }
*/
        if ( ! IsValid( kTransformFlag ) )
        {
            result = true;

            // Object's transform gets applied first, then parentToDstSpace
            fSrcToDst = parentToDstSpace;
            fSrcToDst.Concat( GetMatrix() );

            // After applying transform, allow subclasses to further customize,
            // e.g. trimmed frames from imagesheets
            DidUpdateTransform( fSrcToDst );

            SetValid( kTransformFlag );

            // If the child's matrix is changed,
            // then retransform vertices, recalculate bounding rect, and retransform mask
            Invalidate( kGeometryFlag | kMaskFlag );
            SetDirty( kStageBoundsFlag );
        }
    }

    return result;
}

void
DisplayObject::Prepare( const Display& display )
{
	SUMMED_TIMING( dp, "Display Object: Prepare" );

    // If UpdateTransform() was called, then either:
    // (1) certain flags should be valid
    // (2) it was a no-op b/c ShouldHitTest() was false
    Rtt_ASSERT( IsValid( kTransformFlag ) || ! ShouldHitTest() );

    if ( ! IsValid( kMaskFlag ) )
    {
        UpdateMask();

        SetValid( kMaskFlag );
    }
}

void
DisplayObject::Translate( Real dx, Real dy )
{
    if( Rtt_RealIsZero( dx ) && Rtt_RealIsZero( dy ) )
    {
        // Nothing to do.
        return;
    }

//    if ( ! IsProperty( kIsTransformLocked ) )
    {
        fTransform.Translate( dx, dy );
        
#ifdef Rtt_PHYSICS
        if ( fExtensions && ! IsProperty( kIsExtensionsLocked ) )
        {
            b2Body *body = fExtensions->GetBody();
            if ( body )
            {
                StageObject *stage = GetStage();
                if ( Rtt_VERIFY( stage ) )
                {
                    const PhysicsWorld& physics = stage->GetDisplay().GetRuntime().GetPhysicsWorld();

                    if ( physics.GetWorld()->IsLocked() )
                    {
                        Rtt_TRACE_SIM( ( "ERROR: Cannot translate an object before collision is resolved.\n" ) );
                    }
                    else
                    {
                        Real scale = physics.GetPixelsPerMeter();
                        
                        float angle = body->GetAngle();
                        
                        Real x = fTransform.GetProperty( kOriginX );
                        Real y = fTransform.GetProperty( kOriginY );
                        
                        x = Rtt_RealDiv( x, scale );
                        y = Rtt_RealDiv( y, scale );
                        
                        b2Vec2 position( Rtt_RealToFloat( x ), Rtt_RealToFloat( y ) );
                        body->SetAwake( true );
                        body->SetTransform( position, angle );
                    }
                }
            }
        }
#endif
    }

    Invalidate( kGeometryFlag | kTransformFlag | kMaskFlag );
}

bool
DisplayObject::HitTest( Real contentX, Real contentY )
{
    //Rtt_ASSERT_NOT_IMPLEMENTED();
    return false;
}

void
DisplayObject::DidUpdateTransform( Matrix& srcToDst )
{
    // No-op
}

void
DisplayObject::Scale( Real sx, Real sy, bool isNewValue )
{

    // No-op unless one of them is not 1
    if ( isNewValue
         || ( ! Rtt_RealIsOne( sx ) || ! Rtt_RealIsOne( sy ) ) )
    {
        fTransform.Scale( sx, sy, isNewValue );

#ifdef Rtt_PHYSICS
        // Box2D doesn't support scaling rigid bodies, so we can't do that here.
#endif

        Invalidate( kGeometryFlag | kTransformFlag | kStageBoundsFlag );
    }
}

void
DisplayObject::Rotate( Real deltaTheta )
{
    // No-op unless deltaTheta is non-zero
    if ( ! Rtt_RealIsZero( deltaTheta ) )
    {
        fTransform.Rotate( deltaTheta );
        
#ifdef Rtt_PHYSICS
        if ( fExtensions && ! IsProperty( kIsExtensionsLocked ) )
        {
            b2Body *body = fExtensions->GetBody();
            if ( body )
            {
                StageObject *stage = GetStage();
                if ( Rtt_VERIFY( stage ) )
                {
                    const PhysicsWorld& physics = stage->GetDisplay().GetRuntime().GetPhysicsWorld();

                    if ( physics.GetWorld()->IsLocked() )
                    {
                        Rtt_TRACE_SIM( ( "ERROR: Cannot rotate an object before collision is resolved.\n" ) );
                    }
                    else
                    {
                        Real scale = physics.GetPixelsPerMeter();
                        
                        Real x = fTransform.GetProperty( kOriginX );
                        Real y = fTransform.GetProperty( kOriginY );
                        Real angle = fTransform.GetProperty( kRotation );
                        
                        x = Rtt_RealDiv( x, scale );
                        y = Rtt_RealDiv( y, scale );
                        angle = Rtt_RealDegreesToRadians( angle );
                        
                        b2Vec2 position( Rtt_RealToFloat( x ), Rtt_RealToFloat( y ) );
                        body->SetAwake( true );
                        body->SetTransform( position, Rtt_RealToFloat( angle ) );
                    }
                }
            }
        }
#endif
        
        Invalidate( kGeometryFlag | kTransformFlag | kStageBoundsFlag );
    }
}

void
DisplayObject::DidMoveOffscreen()
{
}

void
DisplayObject::WillMoveOnscreen()
{
}

bool
DisplayObject::CanCull() const
{
    return true;
}

bool
DisplayObject::CanHitTest() const
{
    return true;
}

void
DisplayObject::InitProxy( lua_State *L )
{
    if ( Rtt_VERIFY( ! fLuaProxy ) )
    {
        const char kProxyClassName[] = "DisplayObject";

        fLuaProxy = Rtt_NEW(
            LuaContext::GetAllocator( L ),
            LuaProxy( L, * const_cast< DisplayObject* >( this ), ProxyVTable(), kProxyClassName ) );
    }

    // This is called for all display objects so take the opportunity to record where definition occurred
    // (this is a noop on non-debug builds because lua_where returns an empty string)
    luaL_where(L, 1);
    const char *where = lua_tostring( L, -1 );

    if (where[0] != 0)
    {
        if (fWhereDefined != NULL)
        {
            free((void *) fWhereDefined);
        }

        // If this fails, the pointer will be NULL and that's handled gracefully
        fWhereDefined = strdup(where);
    }

    lua_pop(L, 1);
}

LuaProxy*
DisplayObject::GetProxy() const
{
    Rtt_ASSERT( fLuaProxy );

    return fLuaProxy;
}

void
DisplayObject::ReleaseProxy()
{
    if ( fLuaProxy )
    {
        fLuaProxy->Invalidate();
        fLuaProxy = NULL;
    }
}

const LuaProxyVTable&
DisplayObject::ProxyVTable() const
{
    return LuaDisplayObjectProxyVTable::Constant();
}

void
DisplayObject::QueueRelease( CPUResource *resource )
{
    if ( resource && Rtt_VERIFY( GetStage() ) )
    {
        GetStage()->GetScene().QueueRelease( resource );
    }
}

void
DisplayObject::QueueRelease( LuaUserdataProxy *proxy )
{
    if ( proxy && Rtt_VERIFY( GetStage() ) )
    {
        GetStage()->GetScene().QueueRelease( proxy );
    }
}

void
DisplayObject::AddEventListener( lua_State *L, int index, const char *eventName ) const
{
    Rtt_ASSERT( eventName );
    Rtt_ASSERT( index > 0 );

#ifdef Rtt_DEBUG
    int top = lua_gettop( L );
#endif

    if ( Lua::IsListener( L, index, eventName ) )
    {
        // Lua call to self.addEventListener( self, eventName, listener )
        // where listener is at 'index' on the stack
        GetProxy()->PushTable( L );
        lua_getfield( L, -1, "addEventListener" );
        lua_insert( L, -2 ); // swap table and function

        lua_pushstring( L, eventName );
        lua_pushvalue( L, index );

        int status = LuaContext::DoCall( L, 3, 0 ); Rtt_UNUSED( status );
        Rtt_ASSERT( 0 == status );
    }

    Rtt_ASSERT( lua_gettop( L ) == top );
}

int
DisplayObject::DispatchEventWithTarget( lua_State *L, const MEvent& e, int nresults ) const
{
    // Prepare Lua call to self:dispatchEvent()
    // where 'self' is the Lua proxy for 'this'
    LuaProxy *luaProxy = GetProxy();
    
    // "luaProxy" can be null if the underlying object has been GCed
    if (luaProxy != NULL)
    {
        luaProxy->PushTable( L );
        lua_getfield( L, -1, "dispatchEvent" );
        lua_insert( L, -2 ); // swap table and function

        // Push event table as first arg
        e.Push( L );

        // Add 'self' to the event table
        luaProxy->PushTable( L );
        lua_setfield( L, -2, "target" );

        return LuaContext::DoCall( L, 2, nresults );
    }
    else
    {
        return 0;
    }
}

bool
DisplayObject::DispatchEvent( lua_State *L, const MEvent& e ) const
{
#ifdef Rtt_DEBUG
    int previousTop = lua_gettop( L );
#endif

    // In case of error, default to true to terminate event propagation.
    bool result = true;

    if ( 0 == DispatchEventWithTarget( L, e, 1 ) )
    {
        Rtt_ASSERT( lua_isboolean( L, -1 ) );
        result = ( !! lua_toboolean( L, -1 ) );
    }

    lua_pop( L, 1 );

    Rtt_ASSERT( previousTop == lua_gettop( L ) );

    return result;
}

GroupObject*
DisplayObject::AsGroupObject()
{
    return NULL;
}

const GroupObject*
DisplayObject::AsGroupObject() const
{
    return NULL;
}

void
DisplayObject::LocalToContent( Vertex2& v ) const
{
	// TODO: Use GetSrcToDstMatrix()
	const DisplayObject* object = this;
	Real dx, dy;
	if (GetTrimmedFrameOffset( dx, dy ))
	{
		v.x += dx;
		v.y += dy;
	}
	object->GetMatrix().Apply( v );

    while ( ( object = object->GetParent() ) )
    {
        object->GetMatrix().Apply( v );
    }
}

void
DisplayObject::ContentToLocal( Vertex2& v ) const
{
	// TODO: Use GetSrcToDstMatrix()
	Matrix srcToDstSpace;
	ApplyParent( srcToDstSpace );
	Matrix inverse;
	Matrix::Invert( srcToDstSpace, inverse );
	inverse.Apply( v );

	Real dx, dy;
	if (GetTrimmedFrameOffset( dx, dy ))
	{
		v.x -= dx;
		v.y -= dy;
	}
}

// IsSrcToDstValid() only tells you if the fSrcToDst matrix was explicitly
// invalidated. However, if the parent (or ancestor) matrix is invalidated,
// that implicitly implies that this instance's matrix is invalid as well.
//
// This method does a recursive walk up the ancestor tree and invalidates
// the instance during a post-traversal operation. In other words, we recurse
// all the way up to the root, and then we do the operation. In the base case,
// i.e. the root, the function is a no-op. As we unwind (post-traversal
// operation), we mark the instance based on the parent, thus propagating
// any invalidation down to the original calling instance.
void
DisplayObject::PropagateImplicitSrcToDstInvalidation()
{
    bool isParentValid = true;

    DisplayObject* parent = GetParent();
    if ( parent )
    {
        parent->PropagateImplicitSrcToDstInvalidation();
        isParentValid = parent->IsValid( kTransformFlag );
    }

    if ( ! isParentValid )
    {
        Invalidate( kTransformFlag );
    }
}

void
DisplayObject::ApplyParent( Matrix& xform ) const
{
    const DisplayObject* parent = GetParent();
    if ( parent )
    {
        parent->ApplyParent( xform );
    }

    xform.Concat( GetMatrix() );
}

void
DisplayObject::ApplyParentTransform( const DisplayObject& object, Rect& rect )
{
    const DisplayObject* parent = object.GetParent();
    if ( parent )
    {
        if ( parent->IsValid( kTransformFlag ) )
        {
            parent->GetSrcToDstMatrix().Apply( rect );
        }
        else
        {
            parent->GetMatrix().Apply( rect );
            Self::ApplyParentTransform( *parent, rect );
        }
    }
}

void
DisplayObject::SetReferencePoint( Rtt_Allocator* pAllocator, ReferencePoint location )
{
    Rtt_ASSERT( IsV1Compatibility() );

    Rtt_ASSERT( location >= kReferenceCenter && location < kNumReferencePoints );

    // In graphics 1.0, reference points were NULL until they were set.
    // This property allows callers to mimic that behavior. This is important for
    // groups that had reference points since a NULL reference point differed from
    // a center ref point.
    SetProperty( kIsV1ReferencePointUsed, true );

    Rtt_Real anchorX = Rtt_REAL_HALF;
    Rtt_Real anchorY = Rtt_REAL_HALF;

    switch( location )
    {
        case kReferenceTopLeft:
            anchorX = Rtt_REAL_0;
            anchorY = Rtt_REAL_0;
            break;
        case kReferenceCenterLeft:
            anchorX = Rtt_REAL_0;
            break;
        case kReferenceBottomLeft:
            anchorX = Rtt_REAL_0;
            anchorY = Rtt_REAL_1;
            break;
        case kReferenceTopCenter:
            anchorY = Rtt_REAL_0;
            break;
        case kReferenceCenter:
            break;
        case kReferenceBottomCenter:
            anchorY = Rtt_REAL_1;
            break;
        case kReferenceTopRight:
            anchorX = Rtt_REAL_1;
            anchorY = Rtt_REAL_0;
            break;
        case kReferenceCenterRight:
            anchorX = Rtt_REAL_1;
            break;
        case kReferenceBottomRight:
            anchorX = Rtt_REAL_1;
            anchorY = Rtt_REAL_1;
            break;
        default:
            Rtt_ASSERT_NOT_REACHED();
            break;
    }

    SetAnchorX( anchorX );
    SetAnchorY( anchorY );
}

void
DisplayObject::ResetReferencePoint()
{
    Rtt_ASSERT( IsV1Compatibility() );

    SetAnchorX( Rtt_REAL_HALF );
    SetAnchorY( Rtt_REAL_HALF );

    SetProperty( kIsV1ReferencePointUsed, false );
}

const Rect&
DisplayObject::StageBounds() const
{
    // If the object is not in a canvas, it has no stage bounds
    Rtt_ASSERT( GetStage() );

    if ( ! IsStageBoundsValid() )
    {
        // NOTE: StageBounds() is not dependent on the object being built,
        // because each parent transform is applied.
        // The implicit assumption is GetSelfBounds() does not depend on the
        // object being built via Build(), i.e. self bounds are defined in terms
        // of self-coordinates, not the parent/ancestor's
        Rect& rRect = fStageBounds;
        GetSelfBounds( rRect );

        // Ensure valid rect
        // Exclude case of childless GroupObjects
        Rtt_ASSERT(
            rRect.NotEmpty()
            || ( const_cast< DisplayObject * >( this )->AsGroupObject()
                 && 0 == const_cast< DisplayObject * >( this )->AsGroupObject()->NumChildren() ) );

        UpdateSelfBounds( rRect );

        const_cast< Self * >( this )->PropagateImplicitSrcToDstInvalidation();

		if ( IsValid( kTransformFlag ) )
		{
			GetSrcToDstMatrix().Apply( rRect );
		}
		else
		{
			Real dx, dy;
			if (GetTrimmedFrameOffset( dx, dy ))
			{
				rRect.Translate( dx, dy );
			}
			// TODO: Should we update all the parent stage bounds?
			GetMatrix().Apply( rRect );
			ApplyParentTransform( *this, rRect );
		}
		const_cast< Self * >( this )->SetValid( kStageBoundsFlag );

#ifdef Rtt_DEBUG
        // Exclude case of childless GroupObjects
        if ( NULL == const_cast< DisplayObject * >( this )->AsGroupObject()
             || const_cast< DisplayObject * >( this )->AsGroupObject()->NumChildren() > 0 )
        {
            Rtt_ASSERT( rRect.xMin <= rRect.xMax );
            Rtt_ASSERT( rRect.yMin <= rRect.yMax );
        }
#endif

//        fStageBounds = rect;
    }

    // TODO: We'd like to use this assertion, since it's more general. However, we
    // cannot use IsStageBoundsValid() as the check b/c it can still fail --- we'd
    // first have to ensure that the parent/ancestor stage bounds are up to date.
    // Therefore, we can only test fStageBounds of the receiver directly.
    // Rtt_ASSERT( IsStageBoundsValid() );
#ifdef Rtt_DEBUG
    // Exclude case of childless GroupObjects
    if ( NULL == const_cast< DisplayObject * >( this )->AsGroupObject()
         || const_cast< DisplayObject * >( this )->AsGroupObject()->NumChildren() > 0 )
    {
        Rtt_ASSERT( fStageBounds.NotEmpty() );
    }
#endif

    return fStageBounds;
}

bool
DisplayObject::Intersects( const DisplayObject& rhs ) const
{
    return StageBounds().Intersects( rhs.StageBounds() );
}

static bool
IsFirstEncounteredBeforeSecond( GroupObject& root, const DisplayObject& first, const DisplayObject& second )
{
    S8 result = -1;

    for ( S32 i = 0, iMax = root.NumChildren(); i < iMax && result < 0; i++ )
    {
        DisplayObject& child = root.ChildAt( i );
        if ( & first == & child )
        {
            result = true;
        }
        else if ( & second == & child )
        {
            result = false;
        }
        else
        {
            GroupObject* group = child.AsGroupObject();
            if ( group )
            {
                result = IsFirstEncounteredBeforeSecond( * group, first, second );
            }
        }
    }

    return ( Rtt_VERIFY( result >= 0 ) ? ( !! result ) : false );
}

bool
DisplayObject::IsAbove( const DisplayObject& object ) const
{
    bool result = false;

    if ( this == & object )
    {
        result = true;
    }
    else if ( GetStage() == object.GetStage() )
    {
        if ( fParent == object.GetParent() )
        {
            // Higher indices are rendered above lower indices
            result = fParent->Find( * this ) > fParent->Find( object );
        }
        else
        {
            result = IsFirstEncounteredBeforeSecond( * const_cast< StageObject* >( GetStage() ), object, * this );
        }
    }

    return result;
}

const StageObject*
DisplayObject::GetStage() const
{
    // Check self...
    const GroupObject* group = const_cast< DisplayObject* >( this )->AsGroupObject();
    const StageObject* result = group ? group->GetStage() : NULL;

    // ...before checking parent
    if ( ! result && fParent )
    {
        result = fParent->GetStage();
    }

    return result;
}

void
DisplayObject::SetParent( GroupObject* parent )
{
    fParent = parent;

    // fStage = parent ? parent->fStage : NULL;
    if ( parent )
    {
        StageObject* canvas = parent->GetStage();

        // Always set canvas to match parent's
        GroupObject* pThis = AsGroupObject();
        if ( pThis )
        {
            pThis->SetStage( canvas );
        }
    }

    Invalidate( kGeometryFlag | kTransformFlag | kStageBoundsFlag );
}

void
DisplayObject::Invalidate( DirtyFlags flags )
{
    if ( ! fMask )
    {
        // If no mask, then do not invalidate mask
        flags &= ( ~kMaskFlag );
    }

    fDirtyFlags |= flags;

    if ( flags & ( kGeometryFlag | kTransformFlag | kStageBoundsFlag ) )
    {
        InvalidateStageBounds();
    }

    if ( flags & ( kGeometryFlag ) )
    {
        fTransform.Invalidate();
    }

    InvalidateDisplay();


#if 0
    if ( rebuild )
    {
        InvalidateBuild();
        InvalidateMask();
        Invalidate( kTransformMask );
    }

    InvalidateDisplay();
    InvalidateStageBounds();
    // FreeSubmitQuad();
#endif
}

void
DisplayObject::SetDirty( RenderFlag mask )
{
    // Can only dirty kMaskMask when there is actually a mask
    if ( kMaskFlag != mask || NULL != fMask )
    {
        fDirtyFlags |= mask;
    }
}

// Invalidates stage bounds of receiver.
//
// Invalidate ancestor's stage bounds b/c it is the intersection of the child bounds
// and the child's has been invalidated.
void
DisplayObject::InvalidateStageBounds()
{
    SetDirty( kStageBoundsFlag );

    const DisplayObject *canvas = GetStage();

    // During shell.lua, canvas can be NULL. In this case, we're creating objects
    // that don't have a stage b/c they're placed on a special overlay group.
    if ( canvas )
    {
        // Mark the stage bounds of each ancestor (except the Stage) as invalid.
        // Otherwise, hit testing on the ancestor will fail if the child moves
        // outside the ancestor's stage bounds
        //
        // Terminate if the ancestor is the stage or the ancestor's stage bounds
        // have already been invalidated
        for ( DisplayObject *ancestor = GetParent();
              ancestor
                && ancestor->IsValid( kStageBoundsFlag )
                && ancestor != canvas;
              ancestor = ancestor->GetParent() )
        {
            ancestor->SetDirty( kStageBoundsFlag );
        }
    }
}

bool
DisplayObject::IsStageBoundsValid() const
{
    // Check self
    bool result = IsValid( kStageBoundsFlag );

    // Check ancestors.
    for ( DisplayObject *ancestor = GetParent();
          result && ancestor;
          ancestor = ancestor->GetParent() )
    {
        result = ancestor->IsValid( kTransformFlag );
    }

    return result;
}

void
DisplayObject::InvalidateDisplay()
{
    StageObject* canvas = GetStage();
     if ( canvas )
    {
        canvas->InvalidateDisplay();
    }
}

void
DisplayObject::CalculateMaskMatrix( Matrix& dstToMask, const Matrix& srcToDst, const BitmapMask& mask )
{
    Matrix srcToBitmap = srcToDst;
    srcToBitmap.Concat( mask.GetTransform().GetMatrix( NULL ) ); // Mask's transform gets applied first

    Matrix dstToBitmap;
    Matrix::Invert( srcToBitmap, dstToBitmap );

    Real w, h;
    mask.GetSelfBounds( w, h );

    Real invW = Rtt_RealDiv( Rtt_REAL_1, w );
    Real invH = Rtt_RealDiv( Rtt_REAL_1, h );

    // We need to calculate the transform that takes the object's
    // dst (world) coord space to the mask's UV coord space.
    //
    // This allows us to calculate the object's vertices
    // in the mask's UV coord system.
    //
    // dstToMask = [Scale] * [Translate] * [dstToBitmap]
    dstToMask = dstToBitmap;
    dstToMask.Translate( 0.5*w, 0.5*h );
    dstToMask.Scale( invW, invH );
}

void
DisplayObject::UpdateMaskUniform(
    Uniform& maskUniform, const Matrix& srcToDst, const BitmapMask& mask )
{
    Matrix dstToMask;
    CalculateMaskMatrix( dstToMask, srcToDst, mask );
/*
    Matrix dstToBitmap;
    Matrix::Invert( srcToDst, dstToBitmap );

    Real w, h;
    mask.GetSelfBounds( w, h );

    Real invW = Rtt_RealDiv( Rtt_REAL_1, w );
    Real invH = Rtt_RealDiv( Rtt_REAL_1, h );

    // We need to calculate the transform that takes the object's
    // dst (world) coord space to the mask's UV coord space.
    //
    // This allows us to calculate the object's vertices
    // in the mask's UV coord system.
    //
    // texMatrix = [Scale] * [Translate] * [dstToBitmap]
    Matrix texMatrix = dstToBitmap;
    texMatrix.Translate( 0.5*w, 0.5*h );
    texMatrix.Scale( invW, invH );
*/

/*
    // The scale factor from dst space to texture coordinates is:
    // ( w / w2 ) * ( 1 / w ) = ( 1 / w2 )
    // The first term is the ratio of actual width to the next power of 2
    // The second term is to normalize to texture coordinates [0,1]
    // Similarly for height.
    Rtt_glScale( fW2Inv, fH2Inv, Rtt_REAL_1 );

    Rtt_glTranslate( Rtt_IntToReal( bitmap.Width() >> 1 ), Rtt_IntToReal( bitmap.Height() >> 1 ), Rtt_REAL_0 );

    S32 angle = bitmap.DegreesToUprightBits();
    if ( 0 != angle )
    {
        Rtt_glRotate( Rtt_IntToReal( -angle ), Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_1 );
    }

    Rtt_Real m[16];
    fDstToBitmap.ToGLMatrix( m );
    Rtt_glMultMatrix( m );
*/

    dstToMask.ToGLMatrix3x3( reinterpret_cast< Real * >( maskUniform.GetData() ) );
    maskUniform.Invalidate();
}

void
DisplayObject::UpdateMask()
{
    Rtt_ASSERT( ! IsValid( kMaskFlag ) );

    if ( fMask )
    {
/*
        Matrix xform = GetSrcToDstMatrix();
        xform.Concat( fMask->GetTransform().GetMatrix( NULL ) ); // Mask's transform gets applied first

        Rtt_ASSERT( fMaskUniform );

        UpdateMaskUniform( * fMaskUniform, xform, * fMask );
*/
        Rtt_ASSERT( fMaskUniform );

        UpdateMaskUniform( * fMaskUniform, GetSrcToDstMatrix(), * fMask );
    }
}

void
DisplayObject::DidSetMask( BitmapMask *mask, Uniform *newValue )
{
}

void
DisplayObject::SetMask( Rtt_Allocator *allocator, BitmapMask *mask )
{
    if ( mask != fMask )
    {
        Rtt_DELETE( fMask );
        fMask = mask;

        if ( mask )
        {
            if( ! fMaskUniform )
            {
                fMaskUniform = Rtt_NEW( allocator, Uniform( allocator, Uniform::kMat3 ) );
            }
            DidSetMask( mask, fMaskUniform );
        }
        else if( ! mask && fMaskUniform )
        {
            QueueRelease( fMaskUniform );
            fMaskUniform = NULL;
            DidSetMask( mask, fMaskUniform );
        }

        Invalidate( kStageBoundsFlag | kMaskFlag );
    }
}

void
DisplayObject::SetMaskGeometricProperty( enum GeometricProperty p, Real newValue )
{
    if ( fMask )
    {
        Transform& t = fMask->GetTransform();
        Real currentValue = t.GetProperty( p );
        if ( currentValue != newValue )
        {
            t.SetProperty( p, newValue );

            Invalidate( kStageBoundsFlag | kMaskFlag );
        }
    }
}

Real
DisplayObject::GetMaskGeometricProperty( enum GeometricProperty p ) const
{
    Real result = Rtt_REAL_0;

    if ( fMask )
    {
        result = fMask->GetTransform().GetProperty( p );
    }

    return result;
}

void
DisplayObject::WillDraw( Renderer& renderer ) const
{
#ifdef OLD_GRAPHICS
    if ( fMask )
    {
        BitmapPaint *paint = fMask->GetPaint();
        paint->Begin( stream );
        stream.PushTexture( paint );
    }
#endif
}

void
DisplayObject::DidDraw( Renderer& renderer ) const
{
#ifdef OLD_GRAPHICS
    if ( fMask )
    {
        const BitmapPaint *paint = stream.PopTexture(); Rtt_UNUSED( paint );
        Rtt_ASSERT( fMask->GetPaint() == paint );
        fMask->GetPaint()->End( stream );
    }
#endif
}

void
DisplayObject::ResetTransform()
{
    fTransform.SetIdentity();
/*
    fX = Rtt_REAL_0;
    fY = Rtt_REAL_0;
    fRotation = Rtt_REAL_0;
    fScaleX = Rtt_REAL_1;
    fScaleY = Rtt_REAL_1;
*/
}

void
DisplayObject::SetSelfBounds( Real width, Real height )
{
    if ( width > Rtt_REAL_0 )
    {
        Real currentValue = GetGeometricProperty( kWidth );
        if ( ! Rtt_RealIsZero( currentValue ) && ! Rtt_RealIsZero( width ) )
        {
            Real sx = Rtt_RealDivNonZeroAB( width, currentValue );
            Scale( sx, Rtt_REAL_1, false );
        }
    }

    if ( height > Rtt_REAL_0 )
    {
        Real currentValue = GetGeometricProperty( kHeight );
        if ( ! Rtt_RealIsZero( currentValue ) && ! Rtt_RealIsZero( height ) )
        {
            Real sy = Rtt_RealDivNonZeroAB( height, currentValue );
            Scale( Rtt_REAL_1, sy, false );
        }
    }
}

void
DisplayObject::SetGeometricProperty( enum GeometricProperty p, Real newValue )
{
    Real currentValue;

    switch( p )
    {
        case kWidth:
            SetSelfBounds( newValue, Rtt_REAL_NEG_1 );
            break;
        case kHeight:
            SetSelfBounds( Rtt_REAL_NEG_1, newValue );
            break;
        default:
            {
                currentValue = fTransform.GetProperty( p );
                if ( currentValue != newValue )
                {
                    switch( p )
                    {
                        case kOriginX:
                            Translate( newValue - currentValue, Rtt_REAL_0 );
                            break;
                        case kOriginY:
                            Translate( Rtt_REAL_0, newValue - currentValue );
                            break;
                        case kScaleX:
                        case kScaleY:
                            Rtt_WARN_SIM(
                                ! Rtt_RealIsZero( newValue ),
                                ( "WARNING: Cannot set %cScale property of display object to zero\n",
                                    ( kScaleY == p ? 'y' : 'x' ) ) );
                            fTransform.SetProperty( p, newValue );
                            Invalidate( kGeometryFlag | kTransformFlag | kMaskFlag );
                            break;
                        case kRotation:
                            Rotate( newValue - currentValue );
                            break;
                        default:
                            Rtt_ASSERT_NOT_IMPLEMENTED();
                            break;
                    }
                }
            }
            break;
    }
}

Real
DisplayObject::GetGeometricProperty( enum GeometricProperty p ) const
{
    Real result = Rtt_REAL_0;

    switch( p )
    {
        case kWidth:
            {
                Rect bounds;
                GetSelfBounds( bounds );
                if ( bounds.NotEmpty() )
                {
                    result = bounds.xMax - bounds.xMin;
                }
            }
            break;
        case kHeight:
            {
                Rect bounds;
                GetSelfBounds( bounds );
                if ( bounds.NotEmpty() )
                {
                    result = bounds.yMax - bounds.yMin;
                }
            }
            break;
        case kStageWidth:
            {
                const Rect& bounds = StageBounds();
                if ( bounds.NotEmpty() )
                {
                    result = bounds.xMax - bounds.xMin;
                }
            }
            break;
        case kStageHeight:
            {
                const Rect& bounds = StageBounds();
                if ( bounds.NotEmpty() )
                {
                    result = bounds.yMax - bounds.yMin;
                }
            }
            break;
            /*
        case kStagePositionX:
            Rtt_ASSERT_NOT_IMPLEMENTED();
            break;
        case kStagePositionY:
            Rtt_ASSERT_NOT_IMPLEMENTED();
            break;
            */
        default:
            result = fTransform.GetProperty( p );
            break;
    }

    return result;
}

void
DisplayObject::SetAnchorX( Real newValue )
{
    // NOTE: Internal anchor uses range [-0.5,0.5]
    newValue -= Rtt_REAL_HALF;

    if ( ! Rtt_RealEqual( newValue, fAnchorX ) )
    {
        fAnchorX = newValue;
        Invalidate( kTransformFlag | kStageBoundsFlag );

        fTransform.Invalidate(); // Anchor point changes matrix calculation
    }
}

void
DisplayObject::SetAnchorY( Real newValue )
{
    // NOTE: Internal anchor uses range [-0.5,0.5]
    newValue -= Rtt_REAL_HALF;

    if ( ! Rtt_RealEqual( newValue, fAnchorY ) )
    {
        fAnchorY = newValue;
        Invalidate( kTransformFlag | kStageBoundsFlag );

        fTransform.Invalidate(); // Anchor point changes matrix calculation
    }
}
void
DisplayObject::GetSelfBoundsForAnchor( Rect& rect ) const
{
    GetSelfBounds( rect );
}

Vertex2
DisplayObject::GetAnchorOffset() const
{
    // NOTE: We want undistorted bounds (ignoring 2.5D corner offsets)
    Rect bounds;
    GetSelfBoundsForAnchor( bounds );

    if ( bounds.IsEmpty() )
    {
        Vertex2 result = { Rtt_REAL_0, Rtt_REAL_0 };
        return result;
    }

    // anchor = (0.5 + fAnchorX)
    // xMinCoeff = 1 - anchor
    // xMaxCoeff = anchor
    //
    // fAnchorX    |  anchor  ||  cx      |  (1-cx)  |  cx*xMin  + (1-cx)*xMax
    // -----------------------------------------------------------------------
    // -0.5        |  0       ||  1       |  0       |  1  *xMin + 0  *xMax
    // 0           |  0.5     ||  0.5     |  0.5     |  0.5*xMin + 0.5*xMax
    // 0.5         |  1       ||  0       |  1       |  0  *xMin + 1  *xMax

    Real cx = Rtt_REAL_HALF - fAnchorX; // xMin's coefficient
    Real cy = Rtt_REAL_HALF - fAnchorY; // yMin's coefficient

    // xMax's coeff = (1 - cx). Same for yMax.
    Real x = Rtt_RealMul( cx, bounds.xMin ) + Rtt_RealMul( (Rtt_REAL_1-cx), bounds.xMax );
    Real y = Rtt_RealMul( cy, bounds.yMin ) + Rtt_RealMul( (Rtt_REAL_1-cy), bounds.yMax );

    // Offset should be negative of calculated result
    Vertex2 result = { -x, -y };

    return result;
}

bool
DisplayObject::IsAnchorOffsetZero() const
{
    return ( Rtt_RealIsZero( fAnchorX ) && Rtt_RealIsZero( fAnchorY ) );
}

bool
DisplayObject::ShouldOffsetWithAnchor() const
{
    // NOTE: This assumed self bounds is centered in object space
    return ! IsAnchorOffsetZero();
}

void
DisplayObject::SetV1Compatibility( bool newValue )
{
    SetProperty( kIsV1Compatibility, newValue );
    fTransform.SetV1Compatibility( newValue );
}

void
DisplayObject::SetVisible( bool newValue )
{
    const bool oldValue = IsProperty( kIsVisible );
    if ( oldValue != newValue )
    {
        SetProperty( kIsVisible, newValue );
        InvalidateStageBounds();

        StageObject* canvas = GetStage();
        if ( canvas )
        {
            canvas->InvalidateDisplay();
        }
    }
}

void
DisplayObject::SetHitTestMasked( bool newValue )
{
    if ( IsProperty( kIsHitTestMasked ) != newValue )
    {
        SetProperty( kIsHitTestMasked, newValue );

        if ( fMask )
        {
            InvalidateStageBounds();
        }
    }
}

void
DisplayObject::SetExtensionsLocked( bool newValue )
{
    SetProperty( kIsExtensionsLocked, newValue );
}

void
DisplayObject::SetAnchorChildren( bool newValue )
{
	SetProperty( kIsAnchorChildren, newValue );
	
	DirtyFlags flags = kTransformFlag;
	
	// For backward compatibility purposes, these are tied to the trim correction
	// feature, since this issue was identified during its development, but some
	// kind of "invalidateAnchorChildrenImmediately" might be more appropriate.
	StageObject *canvas = GetStage();
	DisplayDefaults & defaults = canvas->GetDisplay().GetDefaults();
	
	if (defaults.IsImageSheetFrameTrimCorrected())
	{
		flags |= kStageBoundsFlag;
		
		fTransform.Invalidate();
	}
	
	Invalidate( flags );
}

void
DisplayObject::SetAlpha( U8 newValue )
{
    const U8 oldValue = fAlpha;
    if ( oldValue != newValue )
    {
        fAlpha = newValue;
        if ( ( 0 == oldValue && newValue > 0 )
             || ( 0 == newValue && oldValue > 0 ) )
        {
            InvalidateStageBounds();
        }

        Invalidate( kColorFlag );
    }
}

void
DisplayObject::UpdateAlphaCumulative( U8 alphaCumulativeFromAncestors )
{
    U8 result = fAlpha;

    if ( alphaCumulativeFromAncestors < 0xFF )
    {
        result = (((U16)result) * alphaCumulativeFromAncestors) >> 8;
    }

    if ( fAlphaCumulative != result )
    {
        Invalidate(kColorFlag);
        fAlphaCumulative = result;
    }
}

/*
Matrix&
DisplayObject::Transform()
{
    // Note on Transformations
    // -----------------------
    // There are 3 supported operations:
    // * Scale
    // * Rotation
    // * Translation
    //
    // All 3 contribute to the final transform matrix (fTransform). There is an
    // implicit order of operations: first, scale; then, transform; and finally,
    // translation.

    if ( ! IsProperty( kIsTransformValid ) )
    {
        fTransform.SetIdentity();
        fTransform.Scale( fScaleX, fScaleY );
        fTransform.Rotate( fRotation );
        fTransform.Translate( fX, fY );

        ToggleProperty( kIsTransformValid );
    }

    return fTransform;
}
*/

void
DisplayObject::SetTransform( const Transform& newValue )
{
    fTransform = newValue;
    fTransform.Invalidate();

    Invalidate( kTransformFlag );
}

const Matrix&
DisplayObject::GetSrcToDstMatrix() const
{
    //Rtt_ASSERT( IsValid( kTransformFlag ) );

    return fSrcToDst;
}

const Matrix&
DisplayObject::GetMatrix() const
{
    Vertex2 offset;
    bool shouldOffset = ShouldOffsetWithAnchor();
    if ( shouldOffset )
    {
        offset = GetAnchorOffset();
    }

	Vertex2 deltas;
	bool correct = GetTrimmedFrameOffsetForAnchor( deltas.x, deltas.y );

	return fTransform.GetMatrix( shouldOffset ? & offset : NULL, correct ? &deltas : NULL );
}

void
DisplayObject::SetProperty( U32 mask, bool value )
{
    const Properties p = fProperties;
    fProperties = ( value ? p | mask : p & ~mask );
}

#ifdef Rtt_PHYSICS
bool
DisplayObject::InitializeExtensions( Rtt_Allocator *allocator )
{
    // Should only extensions once!
    if ( Rtt_VERIFY( ! fExtensions ) )
    {
        fExtensions = Rtt_NEW( allocator, DisplayObjectExtensions( * const_cast< Self* >( this ) ) );
        GetProxy()->SetExtensionsDelegate( fExtensions );
    }

    return fExtensions;
}

void
DisplayObject::RemoveExtensions()
{
    if ( fExtensions )
    {
        Rtt_DELETE( fExtensions );
        fExtensions = NULL;

        LuaProxy *proxy = GetProxy();
        if ( proxy )
        {
            proxy->SetExtensionsDelegate( NULL );
        }
    }
}
#endif
    
void
DisplayObject::AddedToParent( lua_State * L, GroupObject * parent )
{
}

void
DisplayObject::RemovedFromParent( lua_State * L, GroupObject * parent )
{
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


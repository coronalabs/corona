//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShapePath.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_ShapeAdapterCircle.h"
#include "Display/Rtt_ShapeAdapterPolygon.h"
#include "Display/Rtt_ShapeAdapterRoundedRect.h"
#include "Display/Rtt_ShapeAdapterMesh.h"
#include "Display/Rtt_TesselatorCircle.h"
#include "Display/Rtt_TesselatorPolygon.h"
#include "Display/Rtt_TesselatorRoundedRect.h"
#include "Display/Rtt_TesselatorShape.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Renderer.h"

#include "Rtt_LuaAux.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ShapePath *
ShapePath::NewRoundedRect( Rtt_Allocator *pAllocator, Real width, Real height, Real radius )
{
    TesselatorRoundedRect *tesselator = Rtt_NEW( pAllocator,
        TesselatorRoundedRect( width, height, radius ) );
    ShapePath *result = Rtt_NEW( pAllocator, ShapePath( pAllocator, tesselator ) );
    result->SetAdapter( & ShapeAdapterRoundedRect::Constant() );
    return result;
}

ShapePath *
ShapePath::NewCircle( Rtt_Allocator *pAllocator, Real radius )
{
    TesselatorCircle *tesselator = Rtt_NEW( pAllocator,
        TesselatorCircle( radius ) );
    ShapePath *result = Rtt_NEW( pAllocator, ShapePath( pAllocator, tesselator ) );
    result->SetAdapter( & ShapeAdapterCircle::Constant() );
    return result;
}

ShapePath *
ShapePath::NewPolygon( Rtt_Allocator *pAllocator )
{
    TesselatorPolygon *tesselator = Rtt_NEW( pAllocator,
        TesselatorPolygon( pAllocator ) );
    ShapePath *result = Rtt_NEW( pAllocator, ShapePath( pAllocator, tesselator ) );
    result->SetAdapter( & ShapeAdapterPolygon::Constant() );
    return result;
}
    
    
ShapePath *
ShapePath::NewMesh( Rtt_Allocator *pAllocator, Geometry::PrimitiveType meshType )
{
    TesselatorMesh *tesselator = Rtt_NEW( pAllocator,
                                            TesselatorMesh( pAllocator, meshType ) );
    ShapePath *result = Rtt_NEW( pAllocator, ShapePath( pAllocator, tesselator ) );
    result->SetAdapter( & ShapeAdapterMesh::Constant() );
    return result;
}

// ----------------------------------------------------------------------------

ShapePath::ShapePath( Rtt_Allocator *pAllocator, TesselatorShape *tesselator )
:    Super( pAllocator ),
    fFillGeometry( Rtt_NEW( pAllocator, Geometry( pAllocator, tesselator->GetFillPrimitive(), 0, 0, tesselator->GetFillPrimitive() == Geometry::kIndexedTriangles ) ) ),
    fStrokeGeometry( Rtt_NEW( pAllocator, Geometry( pAllocator, tesselator->GetStrokePrimitive(), 0, 0, false ) ) ),
    fFillSource( pAllocator ),
    fIndexSource( pAllocator ),
    fStrokeSource( pAllocator ),
    fTesselator( tesselator ),
    fDelegate( NULL )
{
    Rtt_ASSERT( fTesselator );
}

ShapePath::~ShapePath()
{
    DisplayObject *observer = GetObserver();
    if ( observer )
    {
        Geometry::ExtensionBlock* strokeBlock = fStrokeGeometry->GetExtensionBlock();
        
        if (strokeBlock && strokeBlock->fProxy)
        {
            ReleaseProxy( strokeBlock->fProxy );
        }

        Geometry::ExtensionBlock* fillBlock = fFillGeometry->GetExtensionBlock();
        
        if (fillBlock && fillBlock->fProxy)
        {
            ReleaseProxy( fillBlock->fProxy );
        }

        observer->QueueRelease( fStrokeGeometry );
        observer->QueueRelease( fFillGeometry );
    }

    Rtt_DELETE( fTesselator );
}

void
ShapePath::CalculateUV( ArrayVertex2& texVertices, Paint *paint, bool canTransformTexture )
{
    Transform t;

    if ( canTransformTexture
            || ! paint->IsValid( Paint::kTextureTransformFlag ) )
    {
        paint->SetValid( Paint::kTextureTransformFlag );

        paint->UpdateTransform( t );
//            BitmapPaint *bitmapPaint = (BitmapPaint*)paint->AsPaint( Paint::kBitmap );
//            if ( bitmapPaint )
//            {
//                t = bitmapPaint->GetTransform();
//            }

        S32 angle = 0;

        const PlatformBitmap *bitmap = paint->GetBitmap();
        if ( bitmap )
        {
            angle = bitmap->DegreesToUprightBits();
            fTesselator->SetNormalizationScaleX( bitmap->GetNormalizationScaleX() );
            fTesselator->SetNormalizationScaleY( bitmap->GetNormalizationScaleY() );
        }

        if ( 0 != angle )
        {
            t.Rotate( Rtt_IntToReal( angle ) );
        }
    }

    texVertices.Clear();
    fTesselator->GenerateFillTexture( texVertices, t );
    paint->ApplyPaintUVTransformations( texVertices );
}

void
ShapePath::TesselateFill()
{
    Rtt_ASSERT( HasFill() );

    Paint *paint = GetFill();

    bool canTransformTexture = paint->CanTransform();
    
    if ( ! IsValid( kFillSource ) )
    {
        fFillSource.Vertices().Clear();
        fTesselator->GenerateFill( fFillSource.Vertices() );
        SetValid( kFillSource );

        if ( canTransformTexture )
        {
            Invalidate( kFillSourceTexture );
        }

        // Force renderdata update
        Invalidate( kFill );

        // Force per-vertex color data update
        GetObserver()->Invalidate( DisplayObject::kColorFlag );
    }
    
    if ( !IsValid(kFillSourceIndices) )
    {
        fIndexSource.Clear();
        fTesselator->GenerateFillIndices(fIndexSource);
        SetValid( kFillSourceIndices );
        
        Invalidate( kFillIndices );
    }

    if ( ! IsValid( kFillSourceTexture ) )
    {
        CalculateUV( fFillSource.TexVertices(), paint, canTransformTexture );

        SetValid( kFillSourceTexture );

        // Force renderdata update
        Invalidate( kFillTexture );

        Rtt_ASSERT( fFillSource.Vertices().Length() == fFillSource.TexVertices().Length() );
    }
}

void
ShapePath::TesselateStroke()
{
    Rtt_ASSERT( HasStroke() );

    Paint *paint = GetStroke();

    BitmapPaint *bitmapPaint = (BitmapPaint*)paint->AsPaint( Paint::kBitmap );
    bool hasTexture = ( NULL != bitmapPaint );

    fTesselator->SetInnerWidth( GetInnerStrokeWidth() );
    fTesselator->SetOuterWidth( GetOuterStrokeWidth() );

    ArrayVertex2& vertices = fStrokeSource.Vertices();
    if ( ! IsValid( kStrokeSource ) )
    {
        vertices.Clear();
        fTesselator->GenerateStroke( vertices );
        SetValid( kStrokeSource );

        if ( hasTexture )
        {
            Invalidate( kStrokeSourceTexture );
        }

        // Force renderdata update
        Invalidate( kStroke );

        // Force per-vertex color data update
        GetObserver()->Invalidate( DisplayObject::kColorFlag );
    }

    if ( hasTexture )
    {
        if ( ! IsValid( kStrokeSourceTexture )
             || ! paint->IsValid( Paint::kTextureTransformFlag ) )
        {
            paint->SetValid( Paint::kTextureTransformFlag );

            fStrokeSource.TexVertices().Clear();
            fTesselator->GenerateStrokeTexture( fStrokeSource.TexVertices(), vertices.Length() );
            SetValid( kStrokeSourceTexture );

            // Force renderdata update
            Invalidate( kStrokeTexture );

            Rtt_ASSERT( fStrokeSource.Vertices().Length() == fStrokeSource.TexVertices().Length() );
        }
    }
}

void
ShapePath::UpdateFill( RenderData& data, const Matrix& srcToDstSpace )
{
    if ( HasFill() )
    {
        TesselateFill();

        // The flags here are for a common helper (UpdateGeometry)
        // which is agnostic to stroke vs fill, so we have to map
        // the fill-specific flags to generic flags (e.g. kVerticesMask)
        U32 flags = 0;
        if ( ! IsValid( kFill ) )
        {
            flags |= kVerticesMask;
        }
        if ( ! IsValid( kFillTexture ) )
        {
            flags |= kTexVerticesMask;
        }
        if ( ! IsValid( kFillIndices ) )
        {
            flags |= kIndicesMask;
        }

        if ( ! fDelegate )
        {
            UpdateGeometry( *fFillGeometry, fFillSource, srcToDstSpace, flags, &fIndexSource );
        }
        else
        {
            fDelegate->UpdateGeometry( * fFillGeometry, fFillSource, srcToDstSpace, flags );
        }
        data.fGeometry = fFillGeometry;

        SetValid( kFill | kFillTexture | kFillIndices );
    }
}

void
ShapePath::UpdateStroke( const Matrix& srcToDstSpace )
{
    if ( HasStroke() )
    {
        TesselateStroke();

        // The flags here are for a common helper (UpdateGeometry)
        // which is agnostic to stroke vs fill, so we have to map
        // the stroke-specific flags to generic flags (e.g. kVerticesMask)
        U32 flags = 0;
        if ( ! IsValid( kStroke ) )
        {
            flags |= kVerticesMask;
        }
        if ( ! IsValid( kStrokeTexture ) )
        {
            flags |= kTexVerticesMask;
        }

        RenderData *data = GetStrokeData();

        if ( ! fDelegate )
        {
            UpdateGeometry( *fStrokeGeometry, fStrokeSource, srcToDstSpace, flags, NULL );
        }
        else
        {
            fDelegate->UpdateGeometry( * fStrokeGeometry, fStrokeSource, srcToDstSpace, flags );
        }
        data->fGeometry = fStrokeGeometry;

        SetValid( kStroke | kStrokeTexture );
    }
}

void
ShapePath::Update( RenderData& data, const Matrix& srcToDstSpace )
{
    Super::Update( data, srcToDstSpace );

    UpdateFill( data, srcToDstSpace );
    UpdateStroke( srcToDstSpace );
}

void
ShapePath::UpdateResources( Renderer& renderer ) const
{
    if ( HasFill() && IsFillVisible() && fFillGeometry->GetStoredOnGPU() )
    {
        renderer.QueueUpdate( fFillGeometry );
    }

    if ( HasStroke() && IsStrokeVisible() && fStrokeGeometry->GetStoredOnGPU() )
    {
        renderer.QueueUpdate( fStrokeGeometry );
    }
}

void
ShapePath::Translate( Real dx, Real dy )
{
    Super::Translate( dx, dy );

    Geometry::Vertex *fillVertices = fFillGeometry->GetVertexData();
    for ( int i = 0, iMax = fFillGeometry->GetVerticesUsed(); i < iMax; i++ )
    {
        Geometry::Vertex& v = fillVertices[i];
        v.x += dx;
        v.y += dy;
    }

    if ( HasStroke() )
    {
        Rtt_ASSERT_NOT_IMPLEMENTED();

        Geometry::Vertex *strokeVertices = fStrokeGeometry->GetVertexData();
        for ( int i = 0, iMax = fStrokeGeometry->GetVerticesUsed(); i < iMax; i++ )
        {
            Geometry::Vertex& v = strokeVertices[i];
            v.x += dx;
            v.y += dy;
        }
    }
}

void
ShapePath::GetSelfBounds( Rect& rect ) const
{
    fTesselator->GetSelfBounds( rect );
    if ( HasStroke() && GetOuterStrokeWidth() > 0 )
    {
        rect = Rect( rect, GetOuterStrokeWidth() );
    }
}

bool
ShapePath::SetSelfBounds( Real width, Real height )
{
    bool result = fTesselator->SetSelfBounds( width, height );

    if ( result )
    {
        Invalidate( kFillSource | kStrokeSource );
    }

    return result;
}

void
ShapePath::GetTextureVertices( ArrayVertex2& texVertices )
{
    Rtt_ASSERT( HasFill() );

    Paint *paint = GetFill();

    CalculateUV( texVertices, paint, paint->CanTransform() );
}

Rect
ShapePath::GetTextureExtents( const ArrayVertex2& texVertices ) const
{
    Rect extents;

    for (S32 i = 0, iMax = texVertices.Length(); i < iMax; ++i)
    {
        extents.Union(texVertices[i]);
    }

    return extents;
}

bool
ShapePath::SetFillVertexColor( U32 index, U32 color )
{
    fFillGeometry->AttachPerVertexColors( &fFillSource.Colors(), GetFillVertexCount() );

    return fFillGeometry->SetVertexColor( index, color );
}

bool
ShapePath::SetStrokeVertexColor( U32 index, U32 color )
{
    fStrokeGeometry->AttachPerVertexColors( &fStrokeSource.Colors(), GetStrokeVertexCount() );

    return fStrokeGeometry->SetVertexColor( index, color );
}

U32* ShapePath::GetFillVertexColors()
{
	fFillGeometry->AttachPerVertexColors( &fFillSource.Colors(), GetFillVertexCount() );

	return fFillGeometry->GetWriteablePerVertexColorData();
}

U32* ShapePath::GetStrokeVertexColors()
{
	fStrokeGeometry->AttachPerVertexColors( &fStrokeSource.Colors(), GetStrokeVertexCount() );

	return fStrokeGeometry->GetWriteablePerVertexColorData();
}

U32
ShapePath::GetFillVertexCount() const
{
    U32 count = fFillGeometry->GetVerticesAllocated();

    if (0U == count)
    {
        return fTesselator->FillVertexCount();
    }

    return count;
}

U32
ShapePath::GetStrokeVertexCount() const
{
    U32 count = fStrokeGeometry->GetVerticesAllocated();

    if (0U == count)
    {
        return fTesselator->StrokeVertexCount();
    }

    return count;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


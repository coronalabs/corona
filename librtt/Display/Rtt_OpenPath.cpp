//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_OpenPath.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_TesselatorLine.h"
#include "Display/Rtt_VertexCache.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Renderer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

OpenPath::OpenPath( Rtt_Allocator* pAllocator )
:    fStroke( NULL ),
    fStrokeGeometry( Rtt_NEW( pAllocator, Geometry( pAllocator, Geometry::kTriangleStrip, 0, 0, false ) ) ),
    fStrokeSource( pAllocator ),
    fStrokePoints( pAllocator ),
    fBounds(),
    fWidth( 0 ),
    fProperties( 0 ),
    fDirtyFlags( kDefault )
{
}

OpenPath::~OpenPath()
{
    Geometry::ExtensionBlock* strokeBlock = fStrokeGeometry->GetExtensionBlock();
    
    if (GetObserver() && strokeBlock && strokeBlock->fProxy)
    {
        ReleaseProxy( strokeBlock->fProxy );
    }

    Rtt_DELETE( fStrokeGeometry );
    Rtt_DELETE( fStroke );
}

void
OpenPath::Tesselate()
{
    Rtt_ASSERT( fStroke );

    Paint *paint = GetStroke();

    BitmapPaint *bitmapPaint = (BitmapPaint*)paint->AsPaint( Paint::kBitmap );
    bool hasTexture = ( NULL != bitmapPaint );

    TesselatorLine tesselator( fStrokePoints, TesselatorLine::kLineMode );
    tesselator.SetWidth( GetWidth() );

    ArrayVertex2& vertices = fStrokeSource.Vertices();
    if ( ! IsValid( kStrokeSource ) )
    {
        vertices.Clear();
        tesselator.GenerateStroke( vertices );
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
            tesselator.GenerateStrokeTexture( fStrokeSource.TexVertices(), vertices.Length() );
            SetValid( kStrokeSourceTexture );

            // Force renderdata update
            Invalidate( kStrokeTexture );
        }
    }
}

void
OpenPath::Update( RenderData& data, const Matrix& srcToDstSpace )
{
    if ( fStroke )
    {
        Tesselate();

        U32 flags = 0;
        if ( ! IsValid( kStroke ) )
        {
            flags |= kVerticesMask;
        }
        if ( ! IsValid( kStrokeTexture ) )
        {
            flags |= kTexVerticesMask;
        }

        UpdateGeometry( *fStrokeGeometry, fStrokeSource, srcToDstSpace, flags, NULL );
        data.fGeometry = fStrokeGeometry;
    }
}

void
OpenPath::UpdateResources( Renderer& renderer ) const
{
#if ! Rtt_OPENGL_CLIENT_SIDE_ARRAYS

    if ( HasStroke() && IsStrokeVisible() && fStrokeGeometry->GetStoredOnGPU() )
    {
        renderer.QueueUpdate( fStrokeGeometry );
    }
#endif
}

void
OpenPath::Translate( Real dx, Real dy )
{
    if ( fStroke )
    {
        fStroke->Translate( dx, dy );
    }

    Geometry::Vertex *strokeVertices = fStrokeGeometry->GetVertexData();
    for ( int i = 0, iMax = fStrokeGeometry->GetVerticesUsed(); i < iMax; i++ )
    {
        Geometry::Vertex& v = strokeVertices[i];
        v.x += dx;
        v.y += dy;
    }
}

void
OpenPath::GetSelfBounds( Rect& rect ) const
{
    Real borderWidth = fWidth + 1.0;
//    borderWidth += Rtt_RealDiv2( borderWidth );
    rect = Rect( fBounds, borderWidth );
//    rect = fBounds;
}

void
OpenPath::UpdatePaint( RenderData& data )
{
    if ( HasStroke() )
    {
        fStroke->UpdatePaint( data );
    }
}

void
OpenPath::UpdateColor( RenderData& data, U8 objectAlpha )
{
    if ( HasStroke() )
    {
        fStroke->UpdateColor( data, objectAlpha );
    }
}

void
OpenPath::SetStroke( Paint* newValue )
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
OpenPath::SetWidth( Real newValue )
{
    if ( newValue < Rtt_REAL_0 )
    {
        newValue = Rtt_REAL_0;
    }
    fWidth = newValue;
    Invalidate( kStrokeSource );
}

void
OpenPath::Insert( S32 index, const Vertex2& p )
{
    fStrokePoints.Insert( index, p );
    fBounds.Union( p );

    Invalidate( kStrokeSource );
}

void
OpenPath::Remove( S32 index )
{
    ArrayVertex2& vertices = fStrokePoints;
    const Vertex2& p = vertices[ index ];
    bool isVertexInsideBoundsRect = fBounds.CompletelyEncloses( p );
    vertices.Remove( index, 1, false );

    // Recalculate bounds if p is on the edge of fBounds
    if ( ! isVertexInsideBoundsRect )
    {
        fBounds.SetEmpty();
        for ( S32 i = 0, iMax = vertices.Length(); i < iMax; i++ )
        {
            fBounds.Union( vertices[i] );
        }
    }

    Invalidate( kStrokeSource );
}

bool
OpenPath::IsStrokeVisible() const
{
    bool result = false;

    if ( HasStroke() )
    {
        result = ( fStroke->GetRGBA().a > Rtt_REAL_0 ) && GetWidth() > Rtt_REAL_0;
    }

    return result;
}

bool
OpenPath::SetStrokeVertexColor( U32 index, U32 color )
{
    fStrokeGeometry->AttachPerVertexColors( &fStrokeSource.Colors(), GetStrokeVertexCount() );

    return fStrokeGeometry->SetVertexColor( index, color );
}

U32
OpenPath::GetStrokeVertexCount() const
{
    if (IsValid( kStrokeSource ))
    {
        return fStrokeGeometry->GetVerticesAllocated();
    }

    else
    {
        return TesselatorLine::VertexCountFromPoints( fStrokePoints, false );
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


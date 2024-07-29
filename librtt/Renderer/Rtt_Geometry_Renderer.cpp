//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_String.h"
#include "Core/Rtt_Assert.h"

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_FormatExtensionList.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Renderer/Rtt_RenderTypes.h"

#include "Corona/CoronaGraphics.h"

#include <cstring>
#include <stddef.h>
#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static void
GeometryCopier( void* dst, const void* src, const CoronaGeometryMappingLayout* layout, U32, U32 n )
{
	memcpy( dst, src, n * sizeof(Geometry::Vertex) );
}

const GeometryWriter&
GeometryWriter::CopyGeometryWriter()
{
    static GeometryWriter sCopyGeometry;

    if ( NULL == sCopyGeometry.fWriter )
    {
        sCopyGeometry.fMask = kAll;
        sCopyGeometry.fWriter = &GeometryCopier;

        // other properties unused, cf. GeometryCopier()
    }

    return sCopyGeometry;
}

void Geometry::Vertex::Zero()
{
    memset(this, 0, sizeof(*this));
}

void Geometry::Vertex::Set(Real x_,
    Real y_,
    Real u_,
    Real v_,
    U8* optionalColorScale,
    Real* optionalUserData)
{
    x = x_;
    y = y_;
    u = u_;
    v = v_;

    if (optionalColorScale)
    {
        rs = optionalColorScale[0];
        gs = optionalColorScale[1];
        bs = optionalColorScale[2];
        as = optionalColorScale[3];
    }

    if (optionalUserData)
    {
        ux = optionalUserData[0];
        uy = optionalUserData[1];
        uz = optionalUserData[2];
        uw = optionalUserData[3];
    }
}

void Geometry::Vertex::SetPos(Real x_,
    Real y_)
{
    x = x_;
    y = y_;
}

void Geometry::Vertex::SetColor4ub(U32 vertexCount, Vertex* vertices,
    U8 r, U8 g, U8 b, U8 a)
{
    for (U32 i = 0;
        i < vertexCount;
        ++i)
    {
        Vertex& vert = vertices[i];

        vert.rs = r;
        vert.gs = g;
        vert.bs = b;
        vert.as = a;
    }
}

void Geometry::Vertex::SetColor(U32 vertexCount, Vertex* vertices,
    Real red, Real green, Real blue, Real alpha)
{
    U8 r = (U8)(red * 255.0f);
    U8 g = (U8)(green * 255.0f);
    U8 b = (U8)(blue * 255.0f);
    U8 a = (U8)(alpha * 255.0f);

    SetColor4ub(vertexCount, vertices, r, g, b, a);
}

U32
Geometry::Vertex::SizeInVertices( U32 size )
{
    size += sizeof(Geometry::Vertex) - 1; // round up

    return size / sizeof(Geometry::Vertex);
}

Geometry::ExtensionBlock::ExtensionBlock( Rtt_Allocator* allocator )
:   fVertexData( allocator ),
    fInstanceData( NULL ),
    fCount( 0 ),
    fProxy( NULL )
{
}
 
Geometry::ExtensionBlock::ExtensionBlock( ExtensionBlock & block )
:   fVertexData( block.fVertexData.Allocator() ),
    fInstanceData( NULL ),
    fCount( block.fCount ),
    fProxy( NULL )
{
    fVertexData.Reserve( block.fVertexData.Length() );

    memcpy( fVertexData.WriteAccess(), block.fVertexData.ReadAccess(), block.fVertexData.Length() * sizeof(Geometry::Vertex) );
 
    if (block.fList.NotNull())
    {
        SetExtensionList( block.fList ); // will allocate any instance data too
    }

    if (block.fInstanceData)
    {
        Rtt_ASSERT( fList.NotNull() );
        Rtt_ASSERT( fInstanceData );

        for (U32 i = 0, count = fList->InstanceGroupCount(); i < count; ++i)
        {
            Rtt_ASSERT( fInstanceData[i] );
            
            fInstanceData[i]->Reserve( block.fInstanceData[i]->Length() );

            memcpy( fInstanceData[i]->WriteAccess(), block.fInstanceData[i]->ReadAccess(), block.fInstanceData[i]->Length() );
        }
    }
}

Geometry::ExtensionBlock::~ExtensionBlock()
{
    if (fInstanceData)
    {
        Rtt_ASSERT( fList.NotNull() );
        
        for (U32 i = 0, iMax = fList->InstanceGroupCount(); i < iMax; ++i)
        {
            Rtt_DELETE( fInstanceData[i] );
        }
        
        Rtt_FREE( fInstanceData );
    }
}

void
Geometry::ExtensionBlock::SetExtensionList( SharedPtr<FormatExtensionList> &list )
{
    Rtt_ASSERT( fList.IsNull() && list.NotNull() );
    
    fList = list;

    if (list->HasInstanceRateData())
    {
        Rtt_Allocator* allocator = fVertexData.Allocator();

        fInstanceData = (Array<U8>**)Rtt_MALLOC( allocator, sizeof(Array<U8>*) * fList->InstanceGroupCount() );

        for (U32 i = 0, count = fList->InstanceGroupCount(); i < count; ++i)
        {
            fInstanceData[i] = Rtt_NEW( allocator, Array<U8>( allocator ) );
        }
    }
}

void
Geometry::ExtensionBlock::UpdateData( bool storedOnGPU, U32 count )
{
    Rtt_ASSERT( fList.NotNull() );
    
    if (count > 0)
    {
        U32 extraCount = fList->ExtraVertexCount();
        
        // For (vertex-rate) GPU data, we consolidate the original + extended
        // vertex data before uploading it. To avoid too much complexity, the
        // original streams are left intact, whereas the extended GPU streams
        // are laid out thus: "stub" vertex #1, extended components #1; "stub"
        // vertex #2, extended part #2; etc. The stubs are populated from the
        // original stream just before uploading.
        // (TODO? The extended part comprises one or more Vertex values; this
        // might waste some space, but lets us ignore alignment concerns when
        // splicing. However, consider using a U8 stream instead.)
        if (storedOnGPU)
        {
            ++extraCount;
        }
        
        fVertexData.PadToSize( count * extraCount, Geometry::Vertex{} );
    }
}

Geometry::Geometry(Rtt_Allocator* allocator, PrimitiveType type, U32 vertexCount, U32 indexCount, bool storeOnGPU)
    : CPUResource(allocator),
    fPrimitiveType(type),
    fVerticesAllocated(vertexCount),
    fIndicesAllocated(indexCount),
#if Rtt_OPENGL_CLIENT_SIDE_ARRAYS
    fStoredOnGPU(storeOnGPU),
#else
    fStoredOnGPU(true), // Force this to be true if client side arrays are NOT supported
#endif
    fPerVertexColors(NULL),
    fVertexData(NULL),
    fIndexData(NULL),
    fVerticesUsed(0),
    fIndicesUsed(0),
    fExtension(NULL)
{
    // Indexed triangles are only supported for the VBO path
    Rtt_ASSERT(!(type == Geometry::kIndexedTriangles && storeOnGPU == false));

    Allocate();
}

Geometry::Geometry(const Geometry& geometry)
    : CPUResource(geometry.GetAllocator()),
    fPrimitiveType(geometry.fPrimitiveType),
    fVerticesAllocated(geometry.fVerticesAllocated),
    fIndicesAllocated(geometry.fIndicesAllocated),
    fStoredOnGPU(geometry.fStoredOnGPU),
    fVertexData(NULL),
    fIndexData(NULL),
    fVerticesUsed(geometry.fVerticesUsed),
    fIndicesUsed(geometry.fIndicesUsed),
    fExtension(NULL)
{
    // Indexed triangles are only supported for the VBO path
    Rtt_ASSERT(!(fPrimitiveType == Geometry::kIndexedTriangles && fStoredOnGPU == false));
    // Copy the given geometry's vertex and index arrays.
    Allocate();
    if (fVertexData)
    {
        memcpy(fVertexData, geometry.fVertexData, fVerticesAllocated * sizeof(Vertex));
    }
    if (fIndexData)
    {
        memcpy(fIndexData, geometry.fIndexData, fIndicesAllocated * sizeof(Index));
    }
    if (geometry.fExtension)
    {
        fExtension = Rtt_NEW( geometry.GetAllocator(), ExtensionBlock( *geometry.fExtension ) );
    }
}

Geometry::~Geometry()
{
    Deallocate();
}

CPUResource::ResourceType Geometry::GetType() const
{
    return CPUResource::kGeometry;
}

void
    Geometry::Allocate()
{
    Deallocate();

    fVertexData = fVerticesAllocated > 0 ? new Vertex[fVerticesAllocated] : NULL;
    fIndexData = fIndicesAllocated > 0 ? new Index[fIndicesAllocated] : NULL;
}

void
    Geometry::Deallocate()
{
    if (fVertexData)
    {
        delete[] fVertexData;
        fVertexData = NULL;
    }

    if (fIndexData)
    {
        delete[] fIndexData;
        fIndexData = NULL;
    }

    if (fExtension)
    {
        delete fExtension;
        fExtension = NULL;
    }
}

bool
    Geometry::RequiresCopy() const
{
    return true;
}

void
    Geometry::SetPrimitiveType(Geometry::PrimitiveType primitive_type)
{
    /**/ //THIS WILL ONLY WORK IF NOT GPU-SIDE?????????
    fPrimitiveType = primitive_type;
}

Geometry::PrimitiveType
    Geometry::GetPrimitiveType() const
{
    return fPrimitiveType;
}

U32
    Geometry::GetVerticesAllocated() const
{
    return fVerticesAllocated;
}

U32
    Geometry::GetIndicesAllocated() const
{
    return fIndicesAllocated;
}

bool
    Geometry::GetStoredOnGPU() const
{
    return fStoredOnGPU;
}

U32
    Geometry::GetVerticesUsed() const
{
    return fVerticesUsed;
}

U32
    Geometry::GetIndicesUsed() const
{
    return fIndicesUsed;
}

void
    Geometry::SetVerticesUsed(U32 count)
{
    Rtt_ASSERT(count <= fVerticesAllocated);
    fVerticesUsed = count;
}

void
    Geometry::SetIndicesUsed(U32 count)
{
    Rtt_ASSERT(count <= fIndicesAllocated);
    fIndicesUsed = count;
}

Geometry::Vertex*
    Geometry::GetVertexData()
{
    return fVertexData;
}

Geometry::Index*
    Geometry::GetIndexData()
{
    return fIndexData;
}

// TODO: Remove. Deprecated
void
    Geometry::Resize(U32 vertexCount, bool copyData)
{
    Resize(vertexCount, 0, copyData);
}

static void SyncPerVertexColors(ArrayU32* perVertexColors, U32 size)
{
    Rtt_ASSERT(perVertexColors);

    perVertexColors->PadToSize(size, ColorWhite());
}

void
    Geometry::Resize(U32 vertexCount, U32 indexCount, bool copyData)
{
    /**/
    //WHY NOT PREVENT new/delete IF WE ALREADY HAVE ENOUGH DATA ALLOCATED???????
    //IN ITS CURRENT STATE, IT'S THE CALLER OF THIS FUNCTION THAT HAS TO PREVENT
    //ITSELF FROM CALLING THIS FUNCTION WHEN IT'S UNNECESSARY!!!!!!!!

    fVerticesAllocated = vertexCount;
    fVerticesUsed = Min(fVerticesUsed, fVerticesAllocated);

    fIndicesAllocated = indexCount;
    fIndicesUsed = Min(fIndicesUsed, fIndicesAllocated);

    // Save the currently allocated data.
    Vertex* existingVertexData = fVertexData;
    Index* existingIndexData = fIndexData;

    // Allocate new data.
    fVertexData = fVerticesAllocated > 0 ? new Vertex[fVerticesAllocated] : NULL;
    fIndexData = fIndicesAllocated > 0 ? new Index[fIndicesAllocated] : NULL;

    // Copy and free the old data.
    if (existingVertexData)
    {
        if (copyData)
        {
            memcpy(fVertexData, existingVertexData, fVerticesUsed * sizeof(Vertex));
        }
        delete[] existingVertexData;
    }

    // Copy and free the old data.
    if (existingIndexData)
    {
        if (copyData)
        {
            memcpy(fIndexData, existingIndexData, fIndicesUsed * sizeof(Index));
        }
        delete[] existingIndexData;
    }

    if (fPerVertexColors && fVerticesAllocated > 0)
    {
        SyncPerVertexColors(fPerVertexColors, fVerticesAllocated);
    }
}

void
    Geometry::Append(const Vertex& vertex)
{
    if (fVerticesUsed == fVerticesAllocated)
    {
        const U32 expansionFactor = 2;
        Resize(expansionFactor * fVerticesAllocated, fIndicesAllocated, true);
    }

    fVertexData[fVerticesUsed] = vertex;
    fVerticesUsed++;
}

// ----------------------------------------------------------------------------

static inline bool
    PointInTriangle(const Real& x, Real const& y, const Geometry::Vertex& p0, const Geometry::Vertex& p1, const Geometry::Vertex& p2)
{
    Real e0x = x - p0.x;
    Real e0y = y - p0.y;
    Real e1x = p1.x - p0.x;
    Real e2x = p2.x - p0.x;
    Real e1y = p1.y - p0.y;
    Real e2y = p2.y - p0.y;

    Real u, v;
    if (Rtt_RealIsZero(e1x))
    {
        if (Rtt_RealIsZero(e2x)) { return false; }
        u = Rtt_RealDiv(e0x, e2x);
        if (u < Rtt_REAL_0 || u > Rtt_REAL_1) { return false; }
        if (Rtt_RealIsZero(e1y)) { return false; }
        v = Rtt_RealDiv((e0y - Rtt_RealMul(e2y, u)), e1y);
        if (v < Rtt_REAL_0) { return false; }
    }
    else
    {
        Real d = Rtt_RealMul(e2y, e1x) - Rtt_RealMul(e2x, e1y);
        if (Rtt_RealIsZero(d)) { return false; }
        u = Rtt_RealDiv((Rtt_RealMul(e0y, e1x) - Rtt_RealMul(e0x, e1y)), d);
        if (u < Rtt_REAL_0 || u > Rtt_REAL_1) { return false; }
        v = Rtt_RealDiv((e0x - Rtt_RealMul(e2x, u)), e1x);
        if (v < Rtt_REAL_0) { return false; }
    }

    if ((u + v) <= Rtt_REAL_1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static bool
    PointInTriangles(Real x, Real y, const Geometry::Vertex* vertices, int numVertices)
{
    for (int i = 0, iMax = (numVertices / 3) * 3; i < iMax; i += 3)
    {
        if (PointInTriangle(x, y, vertices[i], vertices[i + 1], vertices[i + 2]))
            return true;
    }
    return false;
}


static bool
    PointInIndexedTriangles(Real x, Real y, const Geometry::Vertex* vertices, int numVertices, const Geometry::Index* indices, int numIndices)
{
    if (!indices || !numIndices)
        return false;
    for (int i = 0, iMax = (numIndices / 3) * 3; i < iMax; i += 3)
    {
        const Geometry::Index i1 = indices[i];
        const Geometry::Index i2 = indices[i + 1];
        const Geometry::Index i3 = indices[i + 2];
        if ((i1 < numVertices && i2 < numVertices && i3 < numVertices) && PointInTriangle(x, y, vertices[i1], vertices[i2], vertices[i3]))
            return true;
    }
    return false;
}


static bool
    PointInTriangleStrip(Real x, Real y, const Geometry::Vertex* vertices, int numVertices)
{
    for (int i = 0, iMax = numVertices - 2; i < iMax; i++)
    {
        if (PointInTriangle(x, y, vertices[i], vertices[i + 1], vertices[i + 2]))
            return true;
    }
    return false;
}

static bool
    PointInTriangleFan(Real x, Real y, const Geometry::Vertex* vertices, int numVertices)
{
    for (int i = 0, iMax = numVertices - 1; i < iMax; i++)
    {
        if (PointInTriangle(x, y, vertices[0], vertices[i], vertices[i + 1]))
            return true;
    }
    return false;
}

// ----------------------------------------------------------------------------

bool
    Geometry::HitTest(Real x, Real y) const
{
    switch (GetPrimitiveType()) {
    case kIndexedTriangles:
        return PointInIndexedTriangles(x, y, fVertexData, fVerticesUsed, fIndexData, fIndicesUsed);
    case kTriangles:
        return PointInTriangles(x, y, fVertexData, fVerticesUsed);
    case kTriangleFan:
        return PointInTriangleFan(x, y, fVertexData, fVerticesUsed);
    case kTriangleStrip:
    default:
        return PointInTriangleStrip(x, y, fVertexData, fVerticesUsed);

    }

}

// ----------------------------------------------------------------------------

void
    Geometry::AttachPerVertexColors(ArrayU32* colors, U32 size)
{
    Rtt_ASSERT(!fPerVertexColors || fPerVertexColors == colors);

    bool justBound = !fPerVertexColors;

    fPerVertexColors = colors;

    if (justBound)
    {
        SyncPerVertexColors(fPerVertexColors, size);
    }
}

const U32*
    Geometry::GetPerVertexColorData() const
{
    return fPerVertexColors ? fPerVertexColors->ReadAccess() : NULL;
}

U32*
	Geometry::GetWriteablePerVertexColorData()
{
	return fPerVertexColors ? fPerVertexColors->WriteAccess() : NULL;
}

bool 
	Geometry::SetVertexColor(U32 index, U32 color)
{
    if (!fPerVertexColors || index >= fPerVertexColors->Length())
    {
        return false;
    }

    U32& entry = fPerVertexColors->WriteAccess()[index];
    bool changed = entry != color;
        
    entry = color;

    return changed;
}

const FormatExtensionList*
Geometry::GetExtensionList() const
{
    if (!fExtension)
    {
        return NULL;
    }

    return &*fExtension->fList;
}

const Geometry::Vertex*
    Geometry::GetExtendedVertexData() const
{
    if (!fExtension)
    {
        return NULL;
    }
    
    return fExtension->fVertexData.ReadAccess();
}

Geometry::Vertex*
Geometry::GetWritableExtendedVertexData( S32 * length )
{
    if (!fExtension)
    {
        return NULL;
    }
    
    if (NULL != length)
    {
        *length = fExtension->fVertexData.Length();
    }
    
    return fExtension->fVertexData.WriteAccess();
}

Geometry::ExtensionBlock*
Geometry::EnsureExtension()
{
    if (!fExtension)
    {
        fExtension = Rtt_NEW( GetAllocator(), ExtensionBlock( GetAllocator() ) );
    }
    
    return fExtension;
}
 
bool
Geometry::UsesInstancing( const ExtensionBlock* block, const FormatExtensionList* list )
{
    bool instancedByID = list && list->IsInstancedByID();

    return block && (block->fInstanceData || instancedByID);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

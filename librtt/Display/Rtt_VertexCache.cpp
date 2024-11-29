//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

VertexCache::VertexCache( Rtt_Allocator* pAllocator )
:	fVertices(pAllocator),
	fTexVertices(pAllocator),
	fColors(pAllocator),
	fCounts(pAllocator),
    fExtraFloatArrays( NULL ),
    fExtraIndexArrays( NULL )
{
}

VertexCache::~VertexCache()
{
    for (ArrayFloatBox * cur = fExtraFloatArrays, * next; cur; cur = next)
    {
        next = cur->fNext;

        Rtt_DELETE( cur );
    }

    for (ArrayIndexBox * cur = fExtraIndexArrays, * next; cur; cur = next)
    {
        next = cur->fNext;

        Rtt_DELETE( cur );
    }
}

void
VertexCache::Invalidate()
{
	fVertices.Empty();
	fTexVertices.Empty();
	fCounts.Empty();
}

VertexCache::ArrayFloatBox::ArrayFloatBox( Rtt_Allocator * allocator )
    :    fArray( allocator ),
        fKey( NULL ),
        fNext( NULL )
{
}

bool
VertexCache::AddExtraFloatArray( const void * key )
{
    if (!FindFloatArray( key ))
    {
        ArrayFloatBox * box = Rtt_NEW( fVertices.Allocator(), ArrayFloatBox( fVertices.Allocator() ) );

        box->fKey = key ? const_cast< void * >( key ) : box;
        box->fNext = fExtraFloatArrays;
        fExtraFloatArrays = box;

        return true;
    }

    return false;
}

ArrayFloat *
VertexCache::FindFloatArray( const void * key ) const
{
    for (ArrayFloatBox * cur = fExtraFloatArrays; cur; cur = cur->fNext)
    {
        if (key == cur->fKey)
        {
            return &cur->fArray;
        }
    }

    return NULL;
}

U32
VertexCache::ExtraFloatArrayCount() const
{
    U32 count = 0U;

    for (ArrayFloatBox * cur = fExtraFloatArrays; cur; cur = cur->fNext)
    {
        ++count;
    }

    return count;
}

VertexCache::ArrayIndexBox::ArrayIndexBox( Rtt_Allocator * allocator )
    :    fArray( allocator ),
        fKey( NULL ),
        fNext( NULL )
{
}

bool
VertexCache::AddExtraIndexArray( const void * key )
{
    if (!FindIndexArray( key ))
    {
        ArrayIndexBox * box = Rtt_NEW( fVertices.Allocator(), ArrayIndexBox( fVertices.Allocator() ) );
    
        box->fKey = key ? const_cast< void * >( key ) : box;
        box->fNext = fExtraIndexArrays;
        fExtraIndexArrays = box;

        return true;
    }

    return false;
}

ArrayIndex *
VertexCache::FindIndexArray( const void * key ) const
{
    for (ArrayIndexBox * cur = fExtraIndexArrays; cur; cur = cur->fNext)
    {
        if (key == cur->fKey)
        {
            return &cur->fArray;
        }
    }

    return NULL;
}

U32
VertexCache::ExtraIndexArrayCount() const
{
    U32 count = 0U;

    for (ArrayIndexBox * cur = fExtraIndexArrays; cur; cur = cur->fNext)
    {
        ++count;
    }

    return count;
}

ArrayFloat *
VertexCache::ExtraFloatArray( const void * key, bool addIfAbsent )
{
    if (key)
    {
        ArrayFloat * floatArray = FindFloatArray( key );

        if (!floatArray && addIfAbsent)
        {
            AddExtraFloatArray( key );

            floatArray = &fExtraFloatArrays->fArray;
        }

        return floatArray;
    }

    return NULL;
}

const ArrayFloat *
VertexCache::ExtraFloatArray( const void * key ) const
{
    return key ? FindFloatArray( key ) : NULL;
}

ArrayIndex *
VertexCache::ExtraIndexArray( const void * key, bool addIfAbsent )
{
    if (key)
    {
        ArrayIndex * indexArray = FindIndexArray( key );

        if (!indexArray && addIfAbsent)
        {
            AddExtraIndexArray( key );

            indexArray = &fExtraIndexArrays->fArray;
        }

        return indexArray;
    }

    return NULL;
}

const ArrayIndex *
VertexCache::ExtraIndexArray( const void * key ) const
{
    return key ? FindIndexArray( key ) : NULL;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


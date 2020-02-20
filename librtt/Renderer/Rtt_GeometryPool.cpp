//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_GeometryPool.h"

#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Core/Rtt_Allocator.h"

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
	U32 LogBase2( U32 value )
	{
		const U32 base = 2;
		return log( (double)value ) / log( (double)base );
	}
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GeometryPool::Bucket::Bucket( Rtt_Allocator* allocator, U32 vertexCount )
:   fAllocator( allocator ),
	fGeometry( allocator ),
	fVertexCount( vertexCount ),
	fUsedCount( 0 ) {}

Geometry*
GeometryPool::Bucket::GetOrCreate()
{
	if( fUsedCount == fGeometry.Length() )
	{
		const U32 indexCount = 0;
		const bool storeOnGPU = false;
		fGeometry.Append( Rtt_NEW( fAllocator, Geometry( fAllocator, Geometry::kTriangleStrip,
			fVertexCount, indexCount, storeOnGPU ) ) );
	}

	Geometry* result = fGeometry[fUsedCount++];
	result->SetVerticesUsed( 0 );
	return result;
}

GeometryPool::GeometryPool( Rtt_Allocator* allocator, U32 minimumVertexCount )
:	fAllocator( allocator ),
	fFrontPool( Rtt_NEW( allocator, Array<Bucket*>( allocator ) ) ),
	fBackPool( Rtt_NEW( allocator, Array<Bucket*>( allocator) ) ),
	fMinimumVertexCount( minimumVertexCount ),
	fMinimumPower( LogBase2( fMinimumVertexCount ) ) {}

GeometryPool::~GeometryPool()
{
	fBackPool->Empty();
	fFrontPool->Empty();

	Rtt_DELETE( fBackPool );
	Rtt_DELETE( fFrontPool );
}

Geometry*
GeometryPool::GetOrCreate( U32 requiredVertexCount )
{
	U32 finalCount = Max( fMinimumVertexCount, NextPowerOf2( requiredVertexCount ) );
	U32 bucketIndex = LogBase2( finalCount ) - fMinimumPower;
	
	const S32 length = fBackPool->Length();
	if( bucketIndex >= length )
	{
		U32 vertexCount = fMinimumVertexCount;
		if ( length > 0 )
		{
			// Double the count of the last bucket
			vertexCount = (*fBackPool)[length - 1]->fVertexCount * 2;
		}

		for( S32 i = length; i <= bucketIndex; ++i)
		{
			fBackPool->Append( Rtt_NEW( fAllocator, Bucket( fAllocator, vertexCount ) ) );
			vertexCount *= 2;
		}
	}
	return (*fBackPool)[bucketIndex]->GetOrCreate();
}

void
GeometryPool::Swap()
{
	Array<Bucket*>* temp = fFrontPool;
	fFrontPool = fBackPool;
	fBackPool = temp;
	
	const S32 length = fBackPool->Length();
	for( S32 i = 0; i < length; ++i )
	{
		(*fBackPool)[i]->fUsedCount = 0;
	}
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

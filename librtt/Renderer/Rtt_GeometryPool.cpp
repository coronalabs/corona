//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

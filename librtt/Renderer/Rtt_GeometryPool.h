//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GeometryPool_H__
#define _Rtt_GeometryPool_H__

#include "Core/Rtt_Config.h"
#include "Core/Rtt_Macros.h" // TODO: Needed by Rtt_Math.h
#include "Core/Rtt_Types.h"
#include "Core/Rtt_Math.h" // TODO: Needed by Rtt_Array.h
#include "Core/Rtt_Array.h"

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

class Geometry;

// ----------------------------------------------------------------------------

class GeometryPool
{
	public:
		// Geometry instances allocated through the pool are always powers of
		// two in size. The smallest size is given by the minimum vertex count.
		GeometryPool( Rtt_Allocator* allocator, U32 minimumVertexCount = 1024 );
		~GeometryPool();

		// Return an unused Geometry large enough to store the specified number
		// of vertices. If no such Geometry is available, create a new one.
		Geometry* GetOrCreate( U32 requiredVertexCount );

		// Swap the front and back pools. Draw commands pull their data from
		// Geometry in the front pool while Geometry in the back is filled.
		void Swap();

	private:
	
		struct Bucket
		{
			Bucket( Rtt_Allocator* allocator, U32 vertexCount );
			Geometry* GetOrCreate();
			
			Rtt_Allocator* fAllocator;
			Array<Geometry*> fGeometry;
			U32 fVertexCount;
			U32 fUsedCount;
		};

		Rtt_Allocator* fAllocator;
		Array<Bucket*>* fFrontPool;
		Array<Bucket*>* fBackPool;
		U32 fMinimumVertexCount;
		U32 fMinimumPower;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GeometryPool_H__

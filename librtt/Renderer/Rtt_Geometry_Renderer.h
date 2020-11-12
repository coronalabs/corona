//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Geometry_Renderer_H__
#define _Rtt_Geometry_Renderer_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Core/Rtt_Real.h" // TODO: Rtt_Real.h depends on Rtt_Types being included before it

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

// ----------------------------------------------------------------------------

class Geometry : public CPUResource
{
	public:
		typedef CPUResource Super;
		typedef CPUResource Self;

		typedef enum _Mode
		{
			kTriangleStrip,
			kTriangleFan,
			kTriangles,
			kIndexedTriangles,
			kLineLoop,
			kLines
		}
		Mode;
		typedef Mode PrimitiveType; // TODO: Rename Mode to PrimitiveType

		struct Vertex
		{
			void Zero();

			void Set( Real x_,
				Real y_,
				Real u_,
				Real v_,
				U8* optionalColorScale, /* This is an array of length 4. */
				Real* optionalUserData /* This is an array of length 4. */ );

			void SetPos( Real x_,
				Real y_ );

			// 'vertices' is an array of length "vertexCount"
			static void SetColor4ub( U32 vertexCount, Vertex* vertices,
				U8 r, U8 g, U8 b, U8 a );
			static void SetColor( U32 vertexCount, Vertex* vertices,
				Real red, Real green, Real blue, Real alpha );

			Real x, y, z;	     // 12 bytes
			Real u, v, q;	     // 12 bytes
			U8 rs, gs, bs, as;	 // 4 bytes
			Real ux, uy, uz, uw; // 16 bytes
		};

		typedef U16 Index;

		// Generic vertex attribute indices
#ifdef Rtt_WIN_PHONE_ENV
		// Note: These are the indexes that the pre-compiled shaders have assigned to these attributes on Windows Phone.
		//       This is not a good solution. These should be assigned when compiling the shaders or fetched at runtime.
		static const U32 kVertexPositionAttribute = 1;
		static const U32 kVertexTexCoordAttribute = 2;
		static const U32 kVertexColorScaleAttribute = 0;
		static const U32 kVertexUserDataAttribute = 3;
#else
		static const U32 kVertexPositionAttribute = 0;
		static const U32 kVertexTexCoordAttribute = 1;
		static const U32 kVertexColorScaleAttribute = 2;
		static const U32 kVertexUserDataAttribute = 3;
#endif

	public:
		// If storeOnGPU is true, a copy of the vertex data will be stored
		// in GPU memory. For large, infrequently changing data, this can
		// improve performance by avoiding the per-frame copy of data from
		// main memory to GPU memory. For smaller, frequently changing data
		// this can actually reduce performance.
		Geometry( Rtt_Allocator* allocator, PrimitiveType type, U32 vertexCount, U32 indexCount, bool storeOnGPU );
		Geometry( const Geometry& geometry );
		~Geometry();

		virtual ResourceType GetType() const;
		virtual void Allocate();
		virtual void Deallocate();
		virtual bool RequiresCopy() const;

		void SetPrimitiveType( PrimitiveType primitive_type );
		PrimitiveType GetPrimitiveType() const;

		U32 GetVerticesAllocated() const;
		U32 GetIndicesAllocated() const;
		bool GetStoredOnGPU() const;

		void AttachPerVertexColors( ArrayU32* colors, U32 size );

		const U32* GetPerVertexColorData() const;
		bool SetVertexColor( U32 index, U32 color );

		// More space may be allocated than is initially needed. By default,
		// the use count is zero and must be set for Geometry to be useful.
		U32 GetVerticesUsed() const;
		U32 GetIndicesUsed() const;

		void SetVerticesUsed( U32 count );
		void SetIndicesUsed( U32 count );

		// To avoid excess copying, vertex data may be manipulated directly.
		// Invalidate() will result in the data being subloaded to the GPU.
		Vertex* GetVertexData();
		Index* GetIndexData();

		void Resize( U32 vertexCount, bool copyData ); // TODO: Deprecated. Remove.

		// Resize this Geometry's data store. The original data, or as much
		// of it as possible, will be copied to the newly allocated memory.
		void Resize( U32 vertexCount, U32 indexCount, bool copyData );

		// A convenience function which will insert the given Vertex into the
		// data buffer immediately following the last "used" Vertex, resizing
		// as necessary. It is the caller's responsibility to Invalidate().
		void Append( const Vertex& vertex );

	public:
		bool HitTest( Real x, Real y ) const;

	private:
		// Assignment operator made private until we add copy support.
		void operator=( const Geometry& geometry ) { };

		PrimitiveType fPrimitiveType;
		U32 fVerticesAllocated;
		U32 fIndicesAllocated;
		bool fStoredOnGPU;
		ArrayU32* fPerVertexColors;
		Vertex* fVertexData;
		Index* fIndexData;
		U32 fVerticesUsed;
		U32 fIndicesUsed;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Geometry_Renderer_H__

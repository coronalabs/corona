//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TesselatorMesh_H__
#define _Rtt_TesselatorMesh_H__

#include "Display/Rtt_TesselatorShape.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class TesselatorMesh : public TesselatorShape
{
	public:
		typedef Array< U16 > ArrayIndex;
		typedef TesselatorShape Super;

	public:
		TesselatorMesh( Rtt_Allocator *allocator, Geometry::PrimitiveType meshType );

	public:
		virtual Tesselator::eType GetType() override { return Tesselator::kType_Mesh; }

	public:
		virtual void GenerateFillIndices( ArrayIndex& outVertices ) override;
		virtual void GenerateFill( ArrayVertex2& outVertices ) override;
		virtual void GenerateFillTexture( ArrayVertex2& outTexCoords, const Transform& t ) override;
		virtual void GenerateStroke( ArrayVertex2& outVertices ) override;
		virtual void GetSelfBounds( Rect& rect ) override;
		virtual Geometry::PrimitiveType GetFillPrimitive() const override;

		virtual U32 FillVertexCount() const override;
		virtual U32 StrokeVertexCount() const override;

	public:
		// Call when the contour has changed
		void Invalidate();

		ArrayVertex2& GetMesh() { return fMesh; }
		ArrayVertex2& GetUV() { return fUVs; }
		ArrayIndex&	  GetIndices() { return fIndices; }

		const Vertex2 & GetVertexOffset() const { return fVertexOffset; } ;
		void SetVertexOffset(const Vertex2 & offset) { fVertexOffset = offset; } ;

		const U16 GetLowestIndex() const { return fLowestIndex; }
		void SetLowestIndex(const U16 index) { fLowestIndex = index; }

	public:
		// Update the polygons
		void Update();

	private:
		const Geometry::PrimitiveType fMeshType;
		ArrayVertex2 fMesh;
		ArrayVertex2 fUVs;
		ArrayIndex   fIndices;

		Vertex2 fVertexOffset;
		Rect fSelfBounds;
		U16 fLowestIndex;
		bool fIsMeshValid;
		mutable S32 fStrokeCount;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorMesh_H__

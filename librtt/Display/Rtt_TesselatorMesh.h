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

	public:
		// Call when the contour has changed
		void Invalidate();

		ArrayVertex2& GetMesh() { return fMesh; }
		ArrayVertex2& GetUV() { return fUVs; }
		ArrayIndex&	  GetIndices() { return fIndices; }

		const Vertex2 & GetVertexOffset() const { return fVertexOffset; } ;
		void SetVertexOffset(const Vertex2 & offset) { fVertexOffset = offset; } ;

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
		bool fIsMeshValid;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorMesh_H__

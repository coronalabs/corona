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

#ifndef _Rtt_Tesselator_H__
#define _Rtt_Tesselator_H__

#include "Core/Rtt_Geometry.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Renderer/Rtt_Geometry_Renderer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Geometry;
class Matrix;

// ----------------------------------------------------------------------------

// Subclass: TesselatorFill, TesselatorStroke
class Tesselator
{
	protected:
		static const Vertex2 kUnitCircleVertices[];

	public:
		// If non-NULL, srcToDstSpace is used to transform all generated
		// vertices. It overrides the "origin" parameter for the Generate() 
		// methods (i.e. rounded rectangle, circle, ellipse). If NULL, then
		// the origin parameter is used to displace (translate) the vertices.
		Tesselator();
		virtual ~Tesselator();

	public:
		virtual void GenerateStroke( ArrayVertex2& outVertices ) = 0;
		virtual void GenerateStrokeTexture( ArrayVertex2& outTexCoords, int numVertices ) = 0;
		virtual void GetSelfBounds( Rect& rect ) = 0;
		virtual Geometry::PrimitiveType GetStrokePrimitive() const;

	public:
		enum eType
		{
			kType_None,
			kType_Line,
			kType_Circle,
			kType_Polygon,
			kType_Rect,
			kType_RoundedRect,
			kType_Mesh,

			kType_Count
		};

		virtual eType GetType(){ return kType_None; }

	public:
		Real GetInnerWidth() const { return fInnerWidth; }
		Real GetOuterWidth() const { return fOuterWidth; }
		Real GetWidth() const { return GetInnerWidth() + GetOuterWidth(); }
		void SetInnerWidth( Real newValue );
		void SetOuterWidth( Real newValue );
		void SetWidth( Real newValue );
		bool HasStroke() const;

	protected:
		// Adds degenerate triangles so two distinct tri strips can coexist in the same array
		void MoveTo( ArrayVertex2& vertices, const Vertex2& p );

	protected:
		void ApplyTransform( ArrayVertex2& vertices, const Vertex2& origin );

	private:
		void SubdivideCircleSector( ArrayVertex2& vertices, const Vertex2& p1, const Vertex2& p2, int depth );
		void SubdivideCircleArc( ArrayVertex2& vertices, const Vertex2& p1, const Vertex2& p2, int depth, bool appendDuplicate );

	protected:
		// Fill
		enum _Constants
		{
			kNoScale = 0x1,
			kAppendDuplicate = 0x2,
			kAppendArcEndPoints = 0x4
		};

		void AppendCircle( ArrayVertex2& vertices, Real radius, U32 options );
		void AppendCircleQuadrants( ArrayVertex2& vertices, Real radius, U32 options );
		void AppendCircleArc( ArrayVertex2& vertices, Real radius, U32 options );

		static void AppendRect( ArrayVertex2& vertices, Real halfW, Real halfH );

		static void MoveCenterToOrigin( ArrayVertex2& vertices, Vertex2 currentCenter );

	protected:
		// Stroke
		void AppendCircleStroke(
				ArrayVertex2& vertices,
				Real radius, Real innerWidth, Real outerWidth,
				bool appendEndPoints );
		void AppendStrokeTextureClosed( ArrayVertex2& vertices, int numVertices );
		void AppendStrokeTextureEndCap( ArrayVertex2& vertices, int numVertices );

	private:
		int fMaxSubdivideDepth;

	protected:
		Real fInnerWidth;
		Real fOuterWidth;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorStream_H__

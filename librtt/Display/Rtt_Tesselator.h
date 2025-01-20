//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

		virtual U32 StrokeVertexCount() const = 0;

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

	public:
		static U32 CountForDepth( int depth );

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

		U32 AppendCircleCount( Real radius, U32 options ) const;
		U32 AppendCircleQuadrantsCount( Real radius, U32 options ) const;
		U32 AppendCircleArcCount( Real radius, U32 options ) const;

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

		U32 AppendCircleStrokeCount( Real radius, bool appendEndPoints ) const;

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

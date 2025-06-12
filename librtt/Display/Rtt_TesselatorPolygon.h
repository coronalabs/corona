//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TesselatorPolygon_H__
#define _Rtt_TesselatorPolygon_H__

#include "Display/Rtt_TesselatorShape.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class TesselatorPolygon : public TesselatorShape
{
	public:
		typedef TesselatorShape Super;

	public:
		TesselatorPolygon( Rtt_Allocator *allocator );

	public:
		virtual Tesselator::eType GetType(){ return Tesselator::kType_Polygon; }

	public:
		virtual void GenerateFill( ArrayVertex2& outVertices );
		virtual void GenerateFillTexture( ArrayVertex2& outTexCoords, const Transform& t );
		virtual void GenerateStroke( ArrayVertex2& outVertices );
		virtual void GetSelfBounds( Rect& rect );
		virtual Geometry::PrimitiveType GetFillPrimitive() const;

		virtual U32 FillVertexCount() const override;
		virtual U32 StrokeVertexCount() const override;

	public:
		// Call when the contour has changed
		void Invalidate();

		ArrayVertex2& GetContour() { return fContour; }
    
        void SetTriangulationArray( ArrayIndex * triangulationIndices ) { fTriangulationIndices = triangulationIndices; }

	protected:
		// Update the polygons
		void Update();

	private:
		ArrayVertex2 fContour;
		ArrayVertex2 fFill; // cache the fill
		Rect fSelfBounds;
		Vertex2 fCenter;
		bool fIsFillValid;
		bool fIsBadPolygon;
        ArrayIndex * fTriangulationIndices;
		mutable S32 fFillCount;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorPolygon_H__

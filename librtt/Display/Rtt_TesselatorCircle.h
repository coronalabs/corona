//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TesselatorCircle_H__
#define _Rtt_TesselatorCircle_H__

#include "Display/Rtt_TesselatorShape.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class TesselatorCircle : public TesselatorShape
{
	public:
		typedef TesselatorShape Super;

	public:
		TesselatorCircle( Real radius );

	public:
		virtual Tesselator::eType GetType(){ return Tesselator::kType_Circle; }

	public:
		virtual void GenerateFill( ArrayVertex2& outVertices );
		virtual void GenerateFillTexture( ArrayVertex2& outTexCoords, const Transform& t );
		virtual void GenerateStroke( ArrayVertex2& outVertices );
		virtual void GetSelfBounds( Rect& rect );

		virtual U32 FillVertexCount() const override;
		virtual U32 StrokeVertexCount() const override;

	public:
		Real GetRadius() const { return fRadius; }
		void SetRadius( Real newValue ) { fRadius = newValue; }

	private:
		Real fRadius;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorCircle_H__

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TesselatorRoundedRect_H__
#define _Rtt_TesselatorRoundedRect_H__

#include "Display/Rtt_TesselatorRect.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class TesselatorRoundedRect : public TesselatorRectBase
{
	public:
		typedef TesselatorRectBase Super;

	public:
		TesselatorRoundedRect( Real w, Real h, Real radius );

	public:
		virtual Tesselator::eType GetType(){ return Tesselator::kType_RoundedRect; }

	protected:
		void AppendRoundedRect( ArrayVertex2& vertices, Real halfW, Real halfH, Real radius );

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

#endif // _Rtt_TesselatorRoundedRect_H__

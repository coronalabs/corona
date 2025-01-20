//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TesselatorRect_H__
#define _Rtt_TesselatorRect_H__

#include "Display/Rtt_TesselatorShape.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Move this to a separate file
class TesselatorRectBase : public TesselatorShape
{
	public:
		typedef TesselatorShape Super;

	public:
		TesselatorRectBase( Real w, Real h );

	public:
		virtual bool SetSelfBounds( Real width, Real height );

	public:
		Real GetWidth() const { return Rtt_RealMul2( fHalfW ); }
		void SetWidth( Real newValue ) { fHalfW = Rtt_RealDiv2( newValue ); }

		Real GetHeight() const { return Rtt_RealMul2( fHalfH ); }
		void SetHeight( Real newValue ) { fHalfH = Rtt_RealDiv2( newValue ); }

	protected:
		// Rect is centered at origin in object space
		// (fHalfW,fHalfH) is the corner corresponding to xMax,yMax
		Real fHalfW;
		Real fHalfH;
};

// ----------------------------------------------------------------------------

class TesselatorRect : public TesselatorRectBase
{
	public:
		typedef TesselatorRectBase Super;

		// NOTE: These are in counterclockwise fan order from top-left
		// But the internal geometry is in strip order.
		typedef enum _RectOffset
		{
			kX0 = 0,
			kY0,
			kX1,
			kY1,
			kX2,
			kY2,
			kX3,
			kY3,

			kNumRectOffsets
		}
		RectOffset;

	public:
		TesselatorRect( Real w, Real h );

	public:
		virtual Tesselator::eType GetType(){ return Tesselator::kType_Rect; }

	protected:

	public:
		virtual void GenerateFill( ArrayVertex2& outVertices );
		virtual void GenerateFillTexture( ArrayVertex2& outTexCoords, const Transform& t );
		virtual void GenerateStroke( ArrayVertex2& outVertices );
		virtual void GetSelfBounds( Rect& rect );
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const;

		virtual U32 FillVertexCount() const override;
		virtual U32 StrokeVertexCount() const override;

	public:
		bool HasOffset() const { return ( 0 != fOffsetExists ); }
		Real GetOffset( RectOffset offset ) const { return fOffsets[offset]; }
		void SetOffset( RectOffset offset, Real newValue );

		Real GetCoefficient( int index ) const { return fTexCoeff[index]; }

	private:
		Real fTexCoeff[4];
		Real fOffsets[4 * 2];
		U8 fOffsetExists;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorRect_H__

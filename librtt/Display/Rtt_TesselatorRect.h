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

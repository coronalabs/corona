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

#ifndef _Rtt_TesselatorShape_H__
#define _Rtt_TesselatorShape_H__

#include "Display/Rtt_Tesselator.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Transform;

// ----------------------------------------------------------------------------

class TesselatorShape : public Tesselator
{
	public:
		typedef Tesselator Super;

	public:
		TesselatorShape();

	public:
		virtual void GenerateFill( ArrayVertex2& outVertices ) = 0;
		virtual void GenerateFillTexture( ArrayVertex2& outTexCoords, const Transform& t ) = 0;
		virtual void GenerateFillIndices( ArrayIndex&  outVertices ) {};
		virtual void GenerateStrokeTexture( ArrayVertex2& outTexCoords, int numVertices );
		virtual Geometry::PrimitiveType GetFillPrimitive() const;

		virtual bool SetSelfBounds( Real width, Real height );

	public:
		// For now, assume normalization happens around the center of the texture
		void Normalize( ArrayVertex2& vertices );
		bool ShouldNormalize() const;
		void ResetNormalization();

		void SetNormalizationScaleX( Real newValue ) { fScaleX = newValue; }
		void SetNormalizationScaleY( Real newValue ) { fScaleY = newValue; }

	protected:
		Real GetScaleX() const { return fScaleX; }
		Real GetScaleY() const { return fScaleY; }

	private:
		Real fScaleX;
		Real fScaleY;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorShape_H__

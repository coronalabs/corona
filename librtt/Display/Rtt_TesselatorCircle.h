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

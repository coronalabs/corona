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

#ifndef _Rtt_RectPath_H__
#define _Rtt_RectPath_H__

#include "Display/Rtt_ShapePath.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class RectPath : public ShapePath, public MShapePathDelegate
{
	public:
		typedef ShapePath Super;

	public:
		static RectPath *NewRect( Rtt_Allocator *pAllocator, Real width, Real height );

	protected:
		RectPath( Rtt_Allocator* pAllocator, TesselatorShape *tesselator );

	public:
		// MDrawable
		virtual void Update( RenderData& data, const Matrix& srcToDstSpace );

/*
		virtual ~RectPath();

	protected:
		void Initialize();

		void UpdateFill( RenderData& data, const Matrix& srcToDstSpace );
		void UpdateStroke( const Matrix& srcToDstSpace );

	public:
		// MDrawable
		virtual void Update( RenderData& data, const Matrix& srcToDstSpace );
		virtual void Translate( Real dx, Real dy );
		virtual void GetSelfBounds( Rect& rect ) const;
*/
//	public:
//		virtual bool SetSelfBounds( Real width, Real height );

	public:
		virtual void UpdateGeometry(
			Geometry& dst,
			const VertexCache& src,
			const Matrix& srcToDstSpace,
			U32 flags ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RectPath_H__

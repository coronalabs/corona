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

#ifndef _Rtt_TesselatorStream_H__
#define _Rtt_TesselatorStream_H__

#include "Core/Rtt_Geometry.h"
#include "Rtt_RenderingStream.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Matrix;
class VertexCache;

// ----------------------------------------------------------------------------

class TesselatorStream : public RenderingStream
{
	public:
		typedef RenderingStream Super;

		// Returns number of elements added
//		typedef int (*DidSubdivideArc)( ArrayVertex2& vertices,  );

	private:
		static const Vertex2 kUnitCircleVertices[];

	public:
		// If non-NULL, srcToDstSpace is used to transform all generated
		// vertices. It overrides the "origin" parameter for the Submit() 
		// methods (i.e. rounded rectangle, circle, ellipse). If NULL, then
		// the origin parameter is used to displace (translate) the vertices.
		TesselatorStream( const Matrix* srcToDstSpace, VertexCache& cache );
//			const Matrix* srcToDstSpace,
//			ArrayVertex2& vertices,
//			ArrayS32& counts );

	protected:
		void ApplyTransform( const Vertex2& origin );

	private:
		void SubdivideCircleSegment( const Vertex2& p1, const Vertex2& p2, int depth );
		void SubdivideCircleArc( const Vertex2& p1, const Vertex2& p2, int depth, bool appendDuplicate );

	protected:
		enum _Constants
		{
			kNoScale = 0x1,
			kAppendDuplicate = 0x2,
			kAppendArcEndPoints = 0x4
		};

		void AppendCircle( Real radius, bool circularSegmentOnly );
		void AppendCircleArc( Real radius, U32 options );

		void AppendRectangle( Real halfW, Real halfH );

	protected:
		const Matrix* fSrcToDstSpace;
		VertexCache& fCache;

	private:
		int fMaxSubdivideDepth;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorStream_H__

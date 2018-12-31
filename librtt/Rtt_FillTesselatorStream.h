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

#ifndef _Rtt_FillTesselatorStream_H__
#define _Rtt_FillTesselatorStream_H__

#include "Rtt_TesselatorStream.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class VertexCache;

// ----------------------------------------------------------------------------

// TODO: Rename this to FillTesselatorStream
class FillTesselatorStream : public TesselatorStream
{
	public:
		typedef TesselatorStream Super;

	public:
		// If non-NULL, srcToDstSpace is used to transform all generated
		// vertices. It overrides the "origin" parameter for the Submit() 
		// methods (i.e. rounded rectangle, circle, ellipse). If NULL, then
		// the origin parameter is used to displace (translate) the vertices.
		FillTesselatorStream( const Matrix* srcToDstSpace, VertexCache& cache );

	public:
		virtual void Submit( const VertexArray& vertices );
///		virtual void Submit( RenderTypes::Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts );
		virtual void Submit( const Quad vertices );
		virtual void Submit( const Vertex2& origin, Real rectHalfW, Real rectHalfH, Real radius );
		virtual void Submit( const Vertex2& origin, Real radius );
		virtual void Submit( const Vertex2& origin, Real a, Real b );

	private:
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_FillTesselatorStream_H__

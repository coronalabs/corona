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

#ifndef _Rtt_VertexCache_H__
#define _Rtt_VertexCache_H__

#include "Display/Rtt_DisplayTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class VertexCache
{
	public:
		VertexCache( Rtt_Allocator* pAllocator );

	public:
		void Invalidate();
		bool IsValid() const { return fVertices.Length() > 0; }

	public:
		ArrayVertex2& Vertices() { return fVertices; }
		ArrayVertex2& TexVertices() { return fTexVertices; }
		ArrayS32& Counts() { return fCounts; }

		const ArrayVertex2& Vertices() const { return fVertices; }
		const ArrayVertex2& TexVertices() const { return fTexVertices; }
		const ArrayS32& Counts() const { return fCounts; }

	private:
		ArrayVertex2 fVertices;
		ArrayVertex2 fTexVertices;
		ArrayS32 fCounts;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VertexCache_H__

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		VertexCache(Rtt_Allocator* pAllocator);

	public:
		void Invalidate();
		bool IsValid() const { return fVertices.Length() > 0; }

	public:
		ArrayVertex2& Vertices() { return fVertices; }
		ArrayVertex2& TexVertices() { return fTexVertices; }
		Array<U32>& Colors() { return fColors; }
		ArrayS32& Counts() { return fCounts; }

		const ArrayVertex2& Vertices() const { return fVertices; }
		const ArrayVertex2& TexVertices() const { return fTexVertices; }
		const Array<U32>& Colors() const { return fColors; }
		const ArrayS32& Counts() const { return fCounts; }

	private:
		ArrayVertex2 fVertices;
		ArrayVertex2 fTexVertices;
		Array<U32> fColors;
		ArrayS32 fCounts;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VertexCache_H__

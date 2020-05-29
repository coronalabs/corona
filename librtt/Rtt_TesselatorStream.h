//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

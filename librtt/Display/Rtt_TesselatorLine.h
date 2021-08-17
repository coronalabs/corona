//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TesselatorLine_H__
#define _Rtt_TesselatorLine_H__

#include "Display/Rtt_Tesselator.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class TesselatorLine : public Tesselator
{
	public:
		typedef Tesselator Super;

	public:
		typedef enum _Mode
		{
			kLineMode, // Leaves endpoints alone
			kLoopMode, // Connects endpoints

			kNumModes,
		}
		Mode;

	public:
		TesselatorLine( const ArrayVertex2& controlPoints, Mode mode );

	public:
		virtual U32 StrokeVertexCount() const override;

	public:
		static U32 VertexCountFromPoints( const ArrayVertex2& vertices, bool isClosed );
		static U32 VertexCountFromPoints( U32 count, bool isClosed );

	public:
		virtual Tesselator::eType GetType(){ return Tesselator::kType_Line; }

	protected:
		void GenerateStrokeLine( ArrayVertex2& vertices );
		void GenerateStrokeLoop( ArrayVertex2& vertices );

	public:
		virtual void GenerateStroke( ArrayVertex2& outVertices );
		virtual void GenerateStrokeTexture( ArrayVertex2& outTexCoords, int numVertices );
		virtual void GetSelfBounds( Rect& rect );

	private:
		const ArrayVertex2& fControlPoints;
		U8 fMode;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorLine_H__

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

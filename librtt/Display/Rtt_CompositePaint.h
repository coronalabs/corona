//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CompositePaint_H__
#define _Rtt_CompositePaint_H__

#include "Display/Rtt_Paint.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class CompositePaint : public Paint
{
	public:
		typedef Paint Super;
		typedef CompositePaint Self;

	public:
		CompositePaint( Paint *paint0, Paint *paint1 );
		virtual ~CompositePaint();

	public:
		virtual void UpdatePaint( RenderData& data );
		virtual Texture *GetTexture() const;

	public:
		virtual const Paint* AsPaint( Type t ) const;
		virtual void ApplyPaintUVTransformations( ArrayVertex2& vertices ) const override;

//		virtual const MLuaUserdataAdapter& GetAdapter() const;

	private:
		Paint *fPaint0;
		Paint *fPaint1;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CompositePaint_H__

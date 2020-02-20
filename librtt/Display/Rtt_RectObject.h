//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_RectObject_H__
#define _Rtt_RectObject_H__

#include "Display/Rtt_ShapeObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class RectPath;

// ----------------------------------------------------------------------------

// The RectObject is the base representation of all closed rects
class RectObject : public ShapeObject
{
	Rtt_CLASS_NO_COPIES( RectObject )

	public:
		typedef ShapeObject Super;
		typedef RectObject Self;

	public:
		static RectObject *NewRect( Rtt_Allocator*, Real width, Real height );
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const;
		virtual ShaderResource::ProgramMod GetProgramMod() const;

	protected:
		RectObject( RectPath *path );

	protected:
		RectPath& GetRectPath()
		{
			return const_cast< RectPath& >(
				((const Self *)this)->GetRectPath() );
		}
		const RectPath& GetRectPath() const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RectObject_H__

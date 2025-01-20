//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifdef OLD_GRAPHICS

#ifndef _Rtt_EllipsePath_H__
#define _Rtt_EllipsePath_H__

#include "Display/Rtt_RectPath.h"
#include "Rtt_SimpleCachedPath.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EllipsePath : public BaseCachedRectPath
{
	public:
		typedef BaseCachedRectPath Super;

	public:
		EllipsePath( Rtt_Allocator* pAllocator, Real a, Real b );

	public:
		virtual void Draw( Renderer& renderer, const DisplayObject* object ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_EllipsePath_H__

#endif
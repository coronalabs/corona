//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MDrawable_H__
#define _Rtt_MDrawable_H__

#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;
class Renderer;
class Matrix;
struct Rect;

// ----------------------------------------------------------------------------

class MDrawable
{
	public:
		enum
		{
			kDefaultAlpha = 0xFF
		};

	public:
		// Returns true if srcToDst transform is updated
		virtual bool UpdateTransform( const Matrix& parentToDstSpace ) = 0;
		virtual void Prepare( const Display& display ) = 0;
		virtual void Translate( Real deltaX, Real deltaY ) = 0;
		virtual void Draw( Renderer& renderer ) const = 0;
		virtual void GetSelfBounds( Rect& rect ) const = 0;
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MDrawable_H__

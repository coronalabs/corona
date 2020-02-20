//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ContainerObject_H__
#define _Rtt_ContainerObject_H__

#include "Display/Rtt_GroupObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class BitmapMask;
class Display;
class MCachedResourceLibrary;
class Uniform;

// ----------------------------------------------------------------------------

class ContainerObject : public GroupObject
{
	Rtt_CLASS_NO_COPIES( ContainerObject )

	public:
		typedef GroupObject Super;
		typedef ContainerObject Self;

	public:
		ContainerObject(
			Rtt_Allocator* pAllocator, StageObject *canvas, Real width, Real height );
		virtual ~ContainerObject();

	public:
		void Initialize( Display& display );

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const;

	public:
		virtual bool HitTest( Real contentX, Real contentY );
		virtual bool CanCull() const;

	public:
		virtual void SetSelfBounds( Real width, Real height );

	protected:
		bool ShouldOffsetClip() const;

	private:
		BitmapMask *fContainerMask;
		Uniform *fContainerMaskUniform;

		Real fWidth;
		Real fHeight;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GroupObject_H__

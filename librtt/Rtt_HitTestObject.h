//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_HitTestObject_H__
#define _Rtt_HitTestObject_H__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class DisplayObject;

// ----------------------------------------------------------------------------

// HitTestObject is a wrapper for DisplayObjects. Its sole function is to allow
// the creation of a snapshot of the display hierarchy during hit testing.

class HitTestObject
{
	Rtt_CLASS_NO_COPIES( HitTestObject )

	public:
		typedef HitTestObject Self;

	public:
		HitTestObject( DisplayObject& target, Self* parent );
		virtual ~HitTestObject();

	public:
		void Prepend( HitTestObject* child );
		int NumChildren() const { return fNumChildren; }

		Rtt_INLINE DisplayObject& Target() { return fTarget; }
		Rtt_INLINE Self* Child() { return fChild; }
		Rtt_INLINE Self* Sibling() { return fSibling; }

	public:
//		Rtt_Allocator* Allocator() const { return fHitChildren.Allocator(); }

	private:
		DisplayObject& fTarget;
		Self* fParent;
		Self* fChild; // first child of 'this' in linked list; the head of the list
		Self* fSibling; // next sibling of 'this' (which is also a child of fParent)
		S32 fNumChildren;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_HitTestObject_H__

//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

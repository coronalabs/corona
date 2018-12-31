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

#ifndef _Rtt_GroupObject_H__
#define _Rtt_GroupObject_H__

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_DisplayTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Scene;

// ----------------------------------------------------------------------------

class GroupObject : public DisplayObject
{
	Rtt_CLASS_NO_COPIES( GroupObject )

	public:
		typedef DisplayObject Super;
		typedef GroupObject Self;

	public:
		static void CollectUnreachables( lua_State *L, Scene& scene, GroupObject& group );
		void ReleaseChildrenLuaReferences( lua_State *L );

	public:
		GroupObject( Rtt_Allocator* pAllocator, StageObject* canvas );

	public:
		// Super
		virtual GroupObject* AsGroupObject();

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;

	public:
		virtual bool HitTest( Real contentX, Real contentY );
		virtual bool CanCull() const;

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

		virtual bool ShouldOffsetWithAnchor() const;

	public:
		Rtt_INLINE StageObject* GetStage() { return fStage; }
		Rtt_INLINE const StageObject* GetStage() const { return fStage; }
		void SetStage( StageObject* canvas ) { fStage = canvas; }

	public:
		S32 NumChildren() const { return fChildren.Length(); }
		const DisplayObject& ChildAt( S32 index ) const { return * fChildren[index]; }
		DisplayObject& ChildAt( S32 index ) { return * fChildren[index]; }

	protected:
		bool IsStage() const { return this == (GroupObject*)fStage; }

	protected:
		virtual void DidInsert( bool childParentChanged );
		virtual void DidRemove();

	public:
		void Insert( S32 index, DisplayObject* newChild, bool resetTransform );
		void Remove( S32 index );
		DisplayObject* Release( S32 index );
		S32 Find( const DisplayObject& child ) const;

	public:
		Rtt_Allocator* Allocator() const { return fChildren.Allocator(); }

	private:
		StageObject* fStage;

	protected:
		// Children are drawn in order, i.e. first child is drawn below the second
		PtrArrayDisplayObject fChildren;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GroupObject_H__

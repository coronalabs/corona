//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		virtual const GroupObject* AsGroupObject() const;

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

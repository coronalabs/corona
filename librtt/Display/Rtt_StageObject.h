//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_StageObject_H__
#define _Rtt_StageObject_H__

#include "Core/Rtt_Types.h"
#include "Display/Rtt_GroupObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;
class Scene;
class LuaProxyVTable;

// ----------------------------------------------------------------------------

class StageObject : public GroupObject
{
	public:
		typedef GroupObject Super;
		typedef StageObject Self;

	public:
		StageObject( Rtt_Allocator* pAllocator, Scene& owner );
		virtual ~StageObject();

	public:
		// MDrawable
		virtual void GetSelfBounds( Rect& rect ) const;

	public:
		virtual bool ShouldOffsetWithAnchor() const;

	public:
		void InvalidateDisplay();	// Invalidates Scene, so screen blit occurs

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

		virtual void SetV1Compatibility( bool newValue );

	public:
		Display& GetDisplay() const { return fDisplay; }
		Scene& GetScene() const { return fOwner; }

		StageObject* GetNext() const { return fNext; }
		void SetNext( StageObject* pNext );

		bool IsOnscreen() const;

	public:
		// Global focus
		void SetFocus( DisplayObject* newValue ) { fFocus = newValue; }
		DisplayObject* GetFocus() const { return fFocus; }

		// Per-object focus
		void SetFocus( DisplayObject *newValue, const void *focusId );
		DisplayObject* GetFocus( const void *focusId );

	public:
		const Rect *GetSnapshotBounds() const { return fSnapshotBounds; }
		void SetSnapshotBounds( const Rect *r ) { fSnapshotBounds = r; }

	private:
		Scene& fOwner;
		Display& fDisplay;

		StageObject* fNext;
		DisplayObject* fFocus; // Does not own
		LightPtrArray< DisplayObject > fFocusObjects; // Use light array b/c we don't own
		const Rect *fSnapshotBounds;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_StageObject_H__

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

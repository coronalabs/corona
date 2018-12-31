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

#ifndef _Rtt_Scene_H__
#define _Rtt_Scene_H__

#include <set>

#include "Core/Rtt_Types.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_StageObject.h"
#include "Renderer/Rtt_CPUResource.h"
#include "Rtt_LuaUserdataProxy.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;
class PlatformSurface;
class Runtime;
class MUpdatable;

// ----------------------------------------------------------------------------

// The Scene contains the tree of DisplayObject nodes, rooted in the StageObject.
//
// It also is the placeholder for various temporary collections. This includes
// where objects go when they are removed from the scene, i.e. the Orphanage.
//
// This is the top-level interface for rendering the entire DisplayObject tree.
class Scene
{
	public:
		Scene( Rtt_Allocator* pAllocator, Display& owner );
		~Scene();

	public:
//		GroupObject& Root() { return fRoot; }

	public:
//		Rtt_FORCE_INLINE const StageObject* operator->() const { return & fStage; }
//		Rtt_FORCE_INLINE StageObject* operator->() { return & fStage; }

		StageObject& CurrentStage() { return *fCurrentStage; }
		StageObject& OffscreenStage() { return *fOffscreenStage; }
		const Display& GetDisplay() const { return fOwner; }
		Display& GetDisplay() { return fOwner; }

		GroupObject& Orphanage() { return *fOrphanage; }
		GroupObject& SnapshotOrphanage() { return *fSnapshotOrphanage; }

		// Collects various resources passed into QueueRelease() after every 3rd and 32nd call.
		void Collect();

		// Collects all resources passed into QueueRelease() right now.
		// Intended to be called when the app is about to lose the OpenGL context and must delete GPU resources now.
		void ForceCollect();

	private:
		void OnCollectUnreachables();

	public:
		void QueueRelease( CPUResource *resource );
		void QueueRelease( LuaUserdataProxy *proxy );

	public:
		bool IsValid() const;
		void Invalidate();
		void Clear( Renderer& renderer );
		void Render( Renderer& renderer, PlatformSurface& rTarget );
		void Render( Renderer& renderer, PlatformSurface& rTarget, DisplayObject& object );

	public:
		void RenderOverlay( Display& display, Renderer& renderer, const Matrix& srcToDstSpace );
		StageObject& Overlay();

	public:
		StageObject* PushStage();
		void PopStage();

	public:
		void RemoveActiveUpdatable( MUpdatable *e ){ fActiveUpdatable.erase( e ); }
		void AddActiveUpdatable( MUpdatable *e ){ fActiveUpdatable.insert( e ); }
		void QueueUpdateOfUpdatables();

	private:
		Display& fOwner;
		PtrArray< CPUResource > *fFrontResourceOrphanage;
		PtrArray< CPUResource > *fBackResourceOrphanage;
		StageObject *fCurrentStage; // top of stack of StageObjects
		StageObject *fOffscreenStage;
		StageObject *fOrphanage; // For Lua-created display objects removed from a group
		StageObject *fSnapshotOrphanage;
		StageObject *fOverlay;
		LightPtrArray< LuaUserdataProxy > fProxyOrphanage;
		bool fIsValid;
		U8 fCounter; // DO NOT change type --- must be U8

		// IMPORTANT: The purpose of this set is to iterate over all active
		// MUpdatable. This class DOESN'T own these MUpdatable.
		std::set< MUpdatable * > fActiveUpdatable;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Scene_H__

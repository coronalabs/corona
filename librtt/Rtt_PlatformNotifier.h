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

#ifndef _Rtt_PlatformNotifier_H__
#define _Rtt_PlatformNotifier_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Types.h"
#include "Core/Rtt_ResourceHandle.h"

#include "Rtt_Event.h"
#include "Rtt_Scheduler.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: This is crappy.  If you think you should use this, come talk to me (Walter)
// TODO: Also, rename this.  Should not use "Platform" prefix since this is not meant
// as a cross-platform interface. 
// 
/**
 * A class to encapsulate a Lua callback/listener, and an API to call the listener.
 *
 * Note this is absurdly similar to LuaResource, so they should be merged at some point.
 */
class PlatformNotifier
{
	public:
		PlatformNotifier( const ResourceHandle<lua_State>& handle, bool isListenerPersistent = false );		
		~PlatformNotifier();

	public:
		// Caller must release ownership of event (it will be disposed of by the runtime)
		void ScheduleDispatch( VirtualEvent *e );

	public:
		void SetListenerRef( int index );
		bool HasListener() const;
		void CleanupNotifier();
		// This will set the reference without doing any checking
		void RawSetListenerRef( int ref );

	public:
		lua_State* GetLuaState() const;
		int GetListenerRef() const { return fListenerRef; }

	protected:
		const ResourceHandle<lua_State>& GetLuaHandle() const { return fLuaState; }

	public:
		virtual void CallListener( const char * eventName, VirtualEvent & e );

	private:
		ResourceHandle<lua_State> fLuaState;
		int fListenerRef;
		bool fListenerPersistsAfterCall;
};

// ----------------------------------------------------------------------------

// TODO: This is crappy/bad design but I've made an attempt to clean this mess up.
// If you think you should use this, come talk to me (Walter)
// 
/**
 * A task that will call a listener later, at a safe time
 */
class PlatformNotifierTask : public Task
{
	public:
		typedef Task Super;
		typedef PlatformNotifierTask Self;

	protected:
		PlatformNotifierTask( PlatformNotifier& notifier, VirtualEvent *e );

	public:
		~PlatformNotifierTask();

	public:
		virtual void operator()( Scheduler & sender );
		
	protected:
		PlatformNotifier& fNotifier;
		VirtualEvent *fEvent;

	friend class PlatformNotifier;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_PlatformNotifier_H__



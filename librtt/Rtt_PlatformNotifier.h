//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		void ScheduleDispatch( VirtualEvent *e, int ref );

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

	protected:
		void SetReference( int ref ) { fLuaRef = ref; }

	public:
		~PlatformNotifierTask();

	public:
		virtual void operator()( Scheduler & sender );
		
	protected:
		PlatformNotifier& fNotifier;
		VirtualEvent *fEvent;
		int fLuaRef;

	friend class PlatformNotifier;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_PlatformNotifier_H__



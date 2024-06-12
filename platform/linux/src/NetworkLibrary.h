//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _NetworkLibrary_H__
#define _NetworkLibrary_H__

#include <memory>

#include "Corona/CoronaLua.h"
#include "Corona/CoronaMacros.h"
#include "Rtt_Event.h"
#include "Rtt_LinuxContainer.h"
#include "NetworkSupport.h"
#include "Rtt_Scheduler.h"

namespace Rtt
{

	// Local event
	class NetworkCompletionEvent : public VirtualEvent
	{
		public:

			NetworkCompletionEvent(NetworkRequestState* requestState) :
				fRequestState(requestState)
			{
			}

			virtual ~NetworkCompletionEvent()
			{
				delete fRequestState;
			}

			static const char kName[];
			virtual const char* Name() const { return kName; };
			virtual int Push( lua_State *L ) const;

			smart_ptr<NetworkRequestState> fRequestState;
	};

	class NetworkLibrary
	{
	public:
		typedef NetworkLibrary Self;

		static const char kName[];
		static const char kEvent[];

		int fSystemEventListener;

		NetworkLibrary();
		~NetworkLibrary();

		static int Open(lua_State *L);
		static int Finalizer(lua_State *L);
		static Self *ToLibrary(lua_State *L);
		static int ValueForKey(lua_State *L);
		static int request(lua_State *L);
		static int cancel(lua_State *L);
		static int getConnectionStatus(lua_State *L);
		static int ProcessSystemEvent(lua_State *L);
		static int AddSystemEventListener(lua_State *L, NetworkLibrary *networkLibrary);
		static int RemoveSystemEventListener(lua_State *L, int systemEventListenerRef);

		void onStarted(lua_State *L);
		void onSuspended(lua_State *L);
		void onResumed(lua_State *L);
		void onExiting(lua_State *L);

		int sendRequest(lua_State *L);

		std::map<unsigned int, smart_ptr<NetworkRequestParameters> > fRequests;
	};

	class NetworkNotifierTask : public Task
	{
	public:

		NetworkNotifierTask(NetworkRequestParameters* requestParams, NetworkRequestState* requestState, NetworkLibrary* lib)
			: Task(true)
			, fRequestParams(requestParams)
			, fRequestState(requestState)
			, fNetworkLibrary(lib)
		{
		}

		virtual void operator()( Scheduler & sender );

		smart_ptr<NetworkRequestParameters> fRequestParams;
		smart_ptr<NetworkRequestState> fRequestState;
		NetworkLibrary* fNetworkLibrary;
	};
// }
#endif // _NetworkLibrary_H__

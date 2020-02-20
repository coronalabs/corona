//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleReachability_H__
#define _Rtt_AppleReachability_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_PlatformNotifier.h"
#include "Rtt_Event.h"
#include "Rtt_PlatformReachability.h"

// ----------------------------------------------------------------------------

struct lua_State;
@class Rtt_Reachability;
@class ReachabilityCallbackDelegate;

namespace Rtt
{
	
	class Runtime;
	
	// ----------------------------------------------------------------------------
	
	
	class AppleReachability : public PlatformReachability
	{
	public:
		explicit AppleReachability( const ResourceHandle<lua_State> & handle, PlatformReachabilityType type, const char* address );
		
		virtual ~AppleReachability();

		virtual bool IsValid() const;

		virtual bool IsReachable() const;
		
		virtual bool IsConnectionRequired() const;
		
		virtual bool IsConnectionOnDemand() const;
		
		virtual bool IsInteractionRequired() const;
		
		virtual bool IsReachableViaCellular() const;
		
		virtual bool IsReachableViaWiFi() const;
		
		// Helper method, but needs to be public so internal Obj-C can invoke it.
		void InvokeCallback();

	protected:

		Rtt_Reachability* networkReachability;
		ReachabilityCallbackDelegate* reachabilityCallbackDelegate;
		Rtt_Allocator* fAllocator;
	};
	
	// ----------------------------------------------------------------------------
	
} // namespace Rtt




// ----------------------------------------------------------------------------

#endif // _Rtt_AppleReachability_H__

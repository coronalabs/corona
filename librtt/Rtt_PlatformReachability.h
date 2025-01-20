//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformReachability_H__
#define _Rtt_PlatformReachability_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_PlatformNotifier.h"
#include "Rtt_Event.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{
	
class Runtime;
	
// ----------------------------------------------------------------------------
	
class PlatformReachability
{
	public:

		enum PlatformReachabilityType
		{
			kReachabilityTypeUndefined = 0,
			kHostName,
			kAddress,
			kInternet,
			kLocalWiFi,
			kNumReachabilityTypes
		};
		static const char kReachabilityListenerEvent[];
		
		explicit PlatformReachability( const ResourceHandle<lua_State> & handle, PlatformReachabilityType type, const char* address );

		virtual ~PlatformReachability();
		
		// Intended for internal use to decide if the constructor failed since we don't do exception handling.
		virtual bool IsValid() const;

		virtual bool IsReachable() const;
		
		virtual bool IsConnectionRequired() const;
		
		virtual bool IsConnectionOnDemand() const;
		
		virtual bool IsInteractionRequired() const;

		virtual bool IsReachableViaCellular() const;

		virtual bool IsReachableViaWiFi() const;

	protected:
		ResourceHandle<lua_State> fLuaState;
};
	
// ----------------------------------------------------------------------------
	
class ReachabilityChangeEvent : public VirtualEvent
{
	public:
		typedef VirtualEvent Super;
		typedef ReachabilityChangeEvent Self;
		
	public:
		static const char kName[];
		
	public:
		ReachabilityChangeEvent( const PlatformReachability& reachability, const char *address );
//		virtual ~ReachabilityChangeEvent();
		
	public:
		virtual const char* Name() const;
		virtual int Push( lua_State *L ) const;

		virtual void Dispatch( lua_State *L, Runtime& runtime ) const;

	protected:
		const PlatformReachability& fNetworkReachability;
		const char *fAddress; // Does not own this.
};
	
// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformReachability_H__

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

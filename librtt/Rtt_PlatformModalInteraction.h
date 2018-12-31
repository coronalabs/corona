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

#ifndef _Rtt_PlatformModalInteraction_H__
#define _Rtt_PlatformModalInteraction_H__

#include "Core/Rtt_Types.h"
#include "Rtt_PlatformNotifier.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class PlatformModalInteraction : public PlatformNotifier
{
	public:
		// This callback allows you to customize the properties of the event
		// table that gets passed to the listener. Assume the table 
		// representing the event is at top of stack.
		typedef void (*AddPropertiesCallback)( lua_State * L, void * userdata );

	public:
		PlatformModalInteraction( const ResourceHandle<lua_State> & handle );
		virtual ~PlatformModalInteraction();

	public:
	
		typedef enum _PlatformPropertyMask
		{
			// Picking a high number hoping none of the subclasses are using
			kDoNotSuspend = 0x80000000 // 2^31
		} PlatformPropertyMask;
	
		bool IsProperty( U32 mask ) const { return (fProperties & mask) != 0; }
		virtual void SetProperty( U32 mask, bool newValue );

	public:
		void BeginSession( Runtime & runtime );
		Runtime* GetSessionRuntime() const { return fRuntime; }

	protected:
		void EndSession();
		void DidDismiss( AddPropertiesCallback callback, void * userdata );

	private:
		Runtime * fRuntime;
		U32 fProperties;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformModalInteraction_H__

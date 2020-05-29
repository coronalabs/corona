//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

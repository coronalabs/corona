//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformVideoPlayer_H__
#define _Rtt_PlatformVideoPlayer_H__

#include "Core/Rtt_Types.h"

#include "Rtt_PlatformModalInteraction.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class PlatformVideoPlayer : public PlatformModalInteraction
{
	public:
		typedef PlatformModalInteraction Super;

	public:
		// Use methods: IsProperty() and SetProperty()
		enum PropertyMask
		{
			kShowControls = 0x1
		};

		virtual void NotificationCallback();

		PlatformVideoPlayer( const ResourceHandle<lua_State> & handle ) : PlatformModalInteraction( handle )
		{
		}
	public:
		virtual bool Load( const char* path, bool isRemote ) = 0;

	public:
		virtual void Play() = 0;
//		virtual void Stop() = 0;
//		virtual void Pause() = 0;
//		virtual void Resume() = 0;

//	protected:
//		static void AddProperties( lua_State *L, void* userdata );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformVideoPlayer_H__

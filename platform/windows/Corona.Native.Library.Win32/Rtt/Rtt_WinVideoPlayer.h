//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinVideoPlayer_H__
#define _Rtt_WinVideoPlayer_H__

#include "Rtt_PlatformVideoPlayer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class WinVideoPlayer : public PlatformVideoPlayer
{
	public:
		WinVideoPlayer( const ResourceHandle<lua_State> & handle ) : PlatformVideoPlayer( handle )	
		{
		}

	public:
		virtual bool Load( const char* path, bool isRemote );

	public:
		virtual void Play();
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinVideoPlayer_H__

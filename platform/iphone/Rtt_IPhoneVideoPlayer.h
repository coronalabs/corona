//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneVideoPlayer_H__
#define _Rtt_IPhoneVideoPlayer_H__

#include "Rtt_PlatformVideoPlayer.h"

// ----------------------------------------------------------------------------

@class AVPlayerViewController;
@class IPhoneMovieObserver;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class IPhoneVideoPlayer : public PlatformVideoPlayer
{
	public:
		typedef IPhoneVideoPlayer Self;
		typedef PlatformVideoPlayer Super;

	public:
		IPhoneVideoPlayer( const ResourceHandle<lua_State> & handle );
		virtual ~IPhoneVideoPlayer();

	protected:
		void Cleanup();

	public:
		virtual bool Load( const char* path, bool isRemote );

	public:
		virtual void Play();
		virtual void SetProperty( U32 mask, bool newValue );

		void PlaybackDone();
	private:
		IPhoneMovieObserver *fMovieObserver;
		AVPlayerViewController* fMoviePlayerViewController;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneVideoPlayer_H__

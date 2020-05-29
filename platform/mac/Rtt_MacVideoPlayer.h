//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacVideoPlayer_H__
#define _Rtt_MacVideoPlayer_H__

#include "Rtt_PlatformVideoPlayer.h"

// ----------------------------------------------------------------------------
#import <AVKit/AVKit.h>
#import <AVFoundation/AVFoundation.h>

@class Rtt_VideoPlayerView;


namespace Rtt
{

// ----------------------------------------------------------------------------

class MacVideoPlayer : public PlatformVideoPlayer
{
	public:
		MacVideoPlayer( const ResourceHandle<lua_State> & handle );
		virtual ~MacVideoPlayer();
		virtual bool Load( const char* filePath, bool isRemote );
		virtual void Play();
		virtual void SetProperty( U32 mask, bool newValue );

	public:
		void Close();
		void HandlePeriodicTimeObserver();
		void HandleEndOfVideo();

		void SetNeedsRecomputeFromRotationOrScale();
		void RecomputeFromRotationOrScale();
	
		// Generally for the simulator to pause video playback
		void SetSuspended(bool shouldsuspend);
		bool IsSuspended() const;

		void SetPlayingFlag(bool isplaying);
		bool IsPlayingFlag() const;
	
	protected:
		Rtt_VideoPlayerView* fMovieView;
		bool fNeedsRecompute;
		bool fIsPlaying;
		bool fIsSuspended;
		bool fWasPlaying;
		id fTimeObserverToken;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

@interface Rtt_VideoPlayerView : AVPlayerView
{
	Rtt::MacVideoPlayer *owner;
}

@property(nonatomic, readonly, assign) Rtt::MacVideoPlayer *owner;

- (void)setSuspended:(BOOL)newValue;

@end

// ----------------------------------------------------------------------------

#endif // _Rtt_MacVideoPlayer_H__

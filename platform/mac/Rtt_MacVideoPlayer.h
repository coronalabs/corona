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

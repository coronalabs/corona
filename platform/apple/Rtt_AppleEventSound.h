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

#ifndef _Rtt_AppleEventSound_H__
#define _Rtt_AppleEventSound_H__

#include "Rtt_PlatformEventSound.h"

#include <AudioToolbox/AudioServices.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AppleEventSound : public PlatformEventSound
{
	public:
		typedef PlatformEventSound Super;

	public:
		AppleEventSound( const ResourceHandle<lua_State> & handle );
		virtual ~AppleEventSound();
		
	public:
		virtual bool Load( const char* filePath );
		
	public:
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume( Rtt_Real volume );
		virtual Rtt_Real GetVolume() const;
		
		virtual void ReleaseOnComplete();
		
	private:
		SystemSoundID fSoundID;
		bool fPlaying;
		
		static void OnCompleteEventSound( SystemSoundID soundID, void * userData );
};

// ----------------------------------------------------------------------------

}  // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleEventSound_H__
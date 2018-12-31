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

#ifndef _Rtt_PlatformAudioPlayer_H__
#define _Rtt_PlatformAudioPlayer_H__

#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_Scheduler.h"
#include "Rtt_PlatformNotifier.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Rename PlatformAudioPlayer

class PlatformAudioPlayer : public PlatformNotifier
{
	public:
		PlatformAudioPlayer( const ResourceHandle<lua_State> & handle );
		virtual ~PlatformAudioPlayer();

	public:
		virtual bool Load( const char* filePath ) = 0;

	public:
		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual void SetVolume( Rtt_Real volume ) = 0;
		virtual Rtt_Real GetVolume() const = 0;
	
		virtual void NotificationCallback();

	public:
		void SetLooping( bool l )
		{
			fLooping = l;
		}
	
		bool GetLooping() const
		{
			return fLooping;
		}
	
	protected:
		Rtt_Real fVolume;
	
	protected:
		friend class PlatformAudioPlayerCallListenerTask;

		bool fLooping;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformAudioPlayer_H__

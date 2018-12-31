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

#pragma once

#include "Rtt_PlatformAudioPlayer.h"
#include "Rtt_WinEventSound.h"
#include "WinSound.h"


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}
namespace Rtt
{
	class BaseCallback;
}

#pragma endregion


namespace Rtt
{

class WinAudioPlayer : public PlatformAudioPlayer
{
	public:
		WinAudioPlayer(Interop::RuntimeEnvironment& environment, const ResourceHandle<lua_State> & handle);
		virtual ~WinAudioPlayer();

		void SetCompletionHandler(Rtt::BaseCallback* callbackPointer) { fCompletionCallbackPointer = callbackPointer; }
		virtual bool Load( const char* filePath );
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume( Rtt_Real volume );
		virtual Rtt_Real GetVolume() const;
		virtual void NotificationCallback();

	private:
		WinSound fSound;
		Rtt::BaseCallback* fCompletionCallbackPointer;
};

}	// namespace Rtt

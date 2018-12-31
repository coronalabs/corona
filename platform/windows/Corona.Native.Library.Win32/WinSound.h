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

#include "Interop\UI\MessageOnlyWindow.h"
#include "Interop\ScopedComInitializer.h"
#include "WinString.h"
#include <dshow.h>


namespace Rtt
{
	class WinAudioPlayer;
};

class WinSound
{
	public:
		WinSound(Interop::UI::MessageOnlyWindow& messageOnlyWindow);
		virtual ~WinSound();

		void Load(const char * name);
		void Stop();
		bool Play();
		void Pause();
		void Resume();
		void SetVolume(double fVolume);
		double GetVolume() const;
		void SetPlayer(Rtt::WinAudioPlayer *pPlayer) { m_pPlayer = pPlayer; }

	private:
		void Cleanup();

		/// <summary>Called when a Windows message has been received by the message-only window.</summary>
		/// <param name="sender">Reference to the message-only window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments);

		/// <summary>
		///  <para>Reference to the Corona runtime's "message-only window".</para>
		///  <para>
		///   Used by the IMedia system to send private messages/notifiations to this object on the main UI thread.
		///  </para>
		/// </summary>
		Interop::UI::MessageOnlyWindow& fMessageOnlyWindow;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		Interop::UI::MessageOnlyWindow::ReceivedMessageEvent::MethodHandler<WinSound> fReceivedMessageEventHandler;

		/// <summary>
		///  Windows message ID used by the IMedia system to send this object notifications on the main UI thread.
		/// </summary>
		UINT fMediaMessageId;

		/// <summary>
		///  <para>Automatically initializes/unitializes COM via this class' constructor/destructor.</para>
		///  <para>COM must be initialized in order to create the FilterGraph COM object.</para>
		/// </summary>
		Interop::ScopedComInitializer fScopedComInitializer;

		WinString mFilename;
		Rtt::WinAudioPlayer *m_pPlayer;
		IBaseFilter *pif;
		IGraphBuilder *pigb;
		IMediaControl *pimc;
		IMediaEventEx *pimex;
		IMediaSeeking *pims;
		bool mReady;
		bool mPlaying;
};

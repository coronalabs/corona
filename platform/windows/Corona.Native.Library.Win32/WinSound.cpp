//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinSound.h"
#include "Core\Rtt_Build.h"
#include "Rtt_WinAudioPlayer.h"
#include <assert.h>
#include <math.h>
#include <windows.h>


#pragma region Constructors/Destructors
WinSound::WinSound(Interop::UI::MessageOnlyWindow& messageOnlyWindow)
:	fMessageOnlyWindow(messageOnlyWindow),
	fReceivedMessageEventHandler(this, &WinSound::OnReceivedMessage),
	pif(nullptr),
	pigb(nullptr),
	pimc(nullptr),
	pimex(nullptr),
	pims(nullptr),
	mReady(false),
	mPlaying(false),
	m_pPlayer(nullptr)
{
	fMediaMessageId = fMessageOnlyWindow.ReserveMessageId();
	fMessageOnlyWindow.GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
}

WinSound::~WinSound()
{
	Stop();
	Cleanup();
	fMessageOnlyWindow.GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);
	fMessageOnlyWindow.UnreserveMessageId(fMediaMessageId);
}

#pragma endregion


#pragma region Public Methods
void WinSound::Load(const char *name)
{
	mFilename.SetUTF8(name);

	auto result = CoCreateInstance(
			CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&this->pigb);
	if (SUCCEEDED(result))
	{
		pigb->QueryInterface(IID_IMediaControl, (void **)&pimc);
		pigb->QueryInterface(IID_IMediaEventEx, (void **)&pimex);
		pigb->QueryInterface(IID_IMediaSeeking, (void**)&pims);

		BSTR bstr = SysAllocString(mFilename.GetTCHAR());

		pimex->SetNotifyWindow((OAHWND)fMessageOnlyWindow.GetWindowHandle(), fMediaMessageId, (LPARAM)m_pPlayer);

		if (SUCCEEDED(pimc->RenderFile(bstr)))
		{
			mReady = true;
		}

		SysFreeString(bstr);
	}
}


void WinSound::Stop()
{
	if (mReady)
	{
		pimc->Stop();
	}

	mPlaying = false;

	// rewind, seek to start
	if (pims)
	{
		REFERENCE_TIME rt = 0;
		pims->SetPositions(&rt, AM_SEEKING_AbsolutePositioning, nullptr, AM_SEEKING_NoPositioning);
	}
}

bool WinSound::Play()
{
	// Do not continue if not ready.
	if (!mReady)
	{
		return false;
	}

	// Play the sound.
	if (mPlaying)
	{
		// Resume the currently paused sound track.
		pimc->Run();
	}
	else
	{
		// Play the sound file from the beginning.
		Stop();
		pimc->Run();
		mPlaying = true;
	}
	return true;
}

void WinSound::Pause()
{
	if (mReady)
	{
		pimc->Pause();
	}
}

void WinSound::Resume()
{
	Play();
}

void WinSound::SetVolume(double fVolume)
{
	DWORD dwStereoVol = 0, dwLeftVol = 0;

	// Convert to Windows volume format from [0, 1]
	dwLeftVol = (DWORD)floor(fVolume * (double)0xFFFF);

	// Left & right volume the same
	dwStereoVol = dwLeftVol | (dwLeftVol << 16);

	// set mixer to new volume
	waveOutSetVolume(0, dwStereoVol);
}

double WinSound::GetVolume() const
{
	DWORD dwStereoVol = 0, dwLeftVol = 0;

	// get current volume
	waveOutGetVolume(nullptr, &dwStereoVol);

	dwLeftVol = dwStereoVol & 0xFFFF;
	//dwRightVol = (dwStereoVol & 0xFFFF0000) >> 16;

	// Convert to percentage in range [0, 1]
	double fVolume = ((double)dwLeftVol) / ((double)0xFFFF);

	return fVolume;
}

#pragma endregion


#pragma region Private Methods
void WinSound::Cleanup()
{
	if (pimc)
	{
		pimc->Stop();
	}

	if (pif)
	{
		pif->Release();
		pif = nullptr;
	}

	if (pigb)
	{
		pigb->Release();
		pigb = nullptr;
	}

	if (pimc)
	{
		pimc->Release();
		pimc = nullptr;
	}

	if (pimex)
	{
		// unset the notify window
		pimex->SetNotifyWindow(0, 0, 0);
		pimex->Release();
		pimex = nullptr;
	}

	if (pims)
	{
		pims->Release();
		pims = nullptr;
	}
}

void WinSound::OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments)
{
	// Do not continue if the message was already handled.
	if (arguments.WasHandled())
	{
		return;
	}

	// Do not continue if the message does not belong to this media object.
	if (arguments.GetMessageId() != fMediaMessageId)
	{
		return;
	}

	// This message was directed to this media object. Handle it.
	HRESULT hr = S_OK;
	long evCode, param1, param2;
	while (pimex && SUCCEEDED(pimex->GetEvent(&evCode, &param1, &param2, 0)))
	{
		switch (evCode)
		{
			// we process only the EC_COMPLETE message which is sent when the media is finished playing
		case EC_COMPLETE:
			Stop();  // clean up, rewind

			if (m_pPlayer)
			{
				if (m_pPlayer->GetLooping())
				{
					Play();
				}
				else
				{
					m_pPlayer->NotificationCallback();
				}
			}
			break;
		}

		hr = pimex->FreeEventParams(evCode, param1, param2);
	}

	// Flag that the message was handled.
	arguments.SetReturnResult(1);
	arguments.SetHandled();
}

#pragma endregion

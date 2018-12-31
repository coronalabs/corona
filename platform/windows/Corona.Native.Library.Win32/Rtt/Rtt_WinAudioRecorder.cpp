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

#include "stdafx.h"
#include "WinString.h"
#include "Core/Rtt_Build.h"
#include "Rtt_WinAudioRecorder.h"
#include "Rtt_LuaContext.h"


namespace Rtt
{

#pragma region Constructors and Destructors
/// Creates a new audio recorder.
/// @param file The path and file name to record audio to. Set to NULL to not record to file.
WinAudioRecorder::WinAudioRecorder(const ResourceHandle<lua_State> &handle, Rtt_Allocator &allocator, const char *file)
:	PlatformAudioRecorder( handle, allocator, file ),
	fFilename(&allocator)
{
    fFilename.Set(file);
	memset(&fWaveFormatEx, 0x00, sizeof(fWaveFormatEx));
	fWaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	fWaveFormatEx.nChannels = 1;
	fWaveFormatEx.wBitsPerSample = 16; 
	fWaveFormatEx.cbSize = 0;
	fWaveFormatEx.nSamplesPerSec = 22050; // default
	fFileWriter = NULL;
	fRecordingHandle = 0;
	fBufferCount = 0;
}

/// Destructor. Stops recording and destroys this object.
WinAudioRecorder::~WinAudioRecorder()
{
	Stop();
	if (fFileWriter)
	{
		delete fFileWriter;
	}
}

#pragma endregion


#pragma region Callbacks
/// C function called on another thread by the WaveIn functions.
/// @param dwInstance Provides a pointer to the WinAudioRecord object that handles audio recording.
void CALLBACK OnCallbackBlock(HWAVEIN hwl, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	WinAudioRecorder *pCallback = (WinAudioRecorder*)dwInstance;
	if (pCallback)
	{
		pCallback->OnSoundBlock(hwl, uMsg, dwInstance, dwParam1, dwParam2);
	}
}

#pragma endregion


#pragma region Public Audio Functions
/// Starts recording audio.
void WinAudioRecorder::Start()
{
	WAVEINCAPS deviceInfo;
	MMRESULT mmReturn;
	bool canRecord;

	// Do not continue if already recording.
	if (fIsRunning)
	{
		return;
	}

	// Allocate a CWriteSoundFile object on first use.
	if ((NULL == fFileWriter) && fFilename.GetString())
	{
		fFileWriter = new CWriteSoundFile();
	}

	// Fetch the audio recording device's capabilities.
	mmReturn = ::waveInGetDevCaps(WAVE_MAPPER, &deviceInfo, sizeof(deviceInfo));
	if (mmReturn)
	{
		LogWaveInError(mmReturn);
		return;
	}

	// Update recorder configuration "fWaveFormatEx" to something the current device supports.
	// If the device does not support the current configuration, then this function will
	// downgrade the audio quality to something the device does support.
	canRecord = UpdateFormatSettingsAccordingTo(&deviceInfo);
	if (!canRecord)
	{
		Rtt_LogException( "Unable to find a recording format that the current device supports.\n" );
		return;
	}

	// Open a connection to the audio recording device.
	mmReturn = ::waveInOpen(
				&fRecordingHandle, WAVE_MAPPER, &fWaveFormatEx, (DWORD)&OnCallbackBlock,
				(DWORD)this, CALLBACK_FUNCTION);
	if (mmReturn)
	{
		LogWaveInError(mmReturn);
		return;
	}

	// Start recording.
	AllocateBuffers(MAXINPUTBUFFERS);
	mmReturn = ::waveInStart(fRecordingHandle);
	if (mmReturn)
	{
		LogWaveInError(mmReturn);
		return;
	}
	fIsRunning = true;

	// Create the audio file to record to if enabled.
	if (fFileWriter)
	{
		WinString convertedFileName;
		convertedFileName.SetUTF8(fFilename.GetString());
		fFileWriter->CreateWaveFile(convertedFileName.GetTCHAR(), &fWaveFormatEx);
	}
}

/// Stops recording audio.
void WinAudioRecorder::Stop()
{
	MMRESULT mmReturn = 0;

	// Do not continue if already stopped.
	if (!fIsRunning)
	{
		return;
	}

	// Stop recording.
	fIsRunning = false;
	mmReturn = ::waveInStop(fRecordingHandle);
	mmReturn = ::waveInReset(fRecordingHandle);

	// Wait up to 1 second for the other thread to free its allocated buffers.
	int endTime = (int)::GetTickCount() + 1000;
	while ((fBufferCount > 0) && ((endTime - (int)::GetTickCount()) > 0))
	{
		::Sleep(10);
	}

	// Close the connection to the audio recording device.
	if (!mmReturn)
	{
		mmReturn = ::waveInClose(fRecordingHandle);
	}

	// Close the file being recording to, if enabled.
	if (fFileWriter)
	{
		fFileWriter->CloseSoundFile();
	}
}

#pragma endregion


#pragma region Protected Audio Functions
/// Logs an error message for the given result object returned by a Microsoft "waveIn" function.
/// @param mmResult The result object returned by a Microsoft waveIn function.
void WinAudioRecorder::LogWaveInError(MMRESULT mmResult)
{
	WinString message;

	message.Expand(MAX_PATH);
	waveInGetErrorText(mmResult, message.GetBuffer(), MAX_PATH);
	Rtt_LogException( "Error while recording:%x:%s\n", mmResult, message.GetUTF8() );
}

/// Updates the audio recorder's configuration "fWaveFormatEx" to something that the given device supports.
/// If not supported, then this function will downgrade the configuration to the next best quality level.
/// @param deviceInfoPointer Pointer to information about an audio input device. Cannot be NULL.
///                          This information can be retrieved via the waveInGetDevCaps() function.
/// @return Returns TRUE if the recorder configuration has been updated and is ready to record.
///         Returns FALSE if unable to configure this recorder to something that the given devices
///         supports, in which case this object should not attempt to record anything.
bool WinAudioRecorder::UpdateFormatSettingsAccordingTo(WAVEINCAPS *deviceInfoPointer)
{
	// Validate.
	if (!deviceInfoPointer)
	{
		return false;
	}

	// Find the next best recording format that the given device supports. (Default to mono.)
	fWaveFormatEx.nChannels = 1;
	for (fWaveFormatEx.wBitsPerSample = 16;
	     fWaveFormatEx.wBitsPerSample >= 8;
	     fWaveFormatEx.wBitsPerSample -= 8)
	{
		fWaveFormatEx.nSamplesPerSec = GetSampleRate();
		while (fWaveFormatEx.nSamplesPerSec > 0)
		{
			// If the device supports the current recording format, then stop here.
			if (IsFormatSettingsSupportedBy(deviceInfoPointer))
			{
				fWaveFormatEx.nAvgBytesPerSec =
								fWaveFormatEx.nSamplesPerSec * (fWaveFormatEx.wBitsPerSample / 8);
				fWaveFormatEx.nBlockAlign =
								(fWaveFormatEx.wBitsPerSample / 8) * fWaveFormatEx.nChannels;
				return true;
			}

			// Downgrade to the next best frequency.
			if (fWaveFormatEx.nSamplesPerSec > 44100)
			{
				fWaveFormatEx.nSamplesPerSec = 44100;
			}
			else if (fWaveFormatEx.nSamplesPerSec > 22050)
			{
				fWaveFormatEx.nSamplesPerSec = 22050;
			}
			else if (fWaveFormatEx.nSamplesPerSec > 11025)
			{
				fWaveFormatEx.nSamplesPerSec = 11025;
			}
			else
			{
				break;
			}
		}
	}

	// Failed to find a format that the given device supports.
	// Give up and inform the caller to not record audio.
	return false;
}

/// Determines if the current audio recorder configuration "fWaveFormatEx" is supported
/// by the given audio input device.
/// @param deviceInfoPointer Information about an audio input device. Cannot be NULL.
/// @return Returns TRUE if the current configuration is supported. Returns FALSE if not.
bool WinAudioRecorder::IsFormatSettingsSupportedBy(WAVEINCAPS *deviceInfoPointer)
{
	bool isSupported = false;

	// Validate.
	if (!deviceInfoPointer)
	{
		return false;
	}

	// Determine if the given device supports the current audio recorder configuration.
	switch (fWaveFormatEx.nSamplesPerSec)
	{
		case 11025:
			if ((1 == fWaveFormatEx.nChannels) && (8 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_1M08) != 0);
			}
			else if ((1 == fWaveFormatEx.nChannels) && (16 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_1M16) != 0);
			}
			if ((2 == fWaveFormatEx.nChannels) && (8 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_1S08) != 0);
			}
			else if ((2 == fWaveFormatEx.nChannels) && (16 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_1S16) != 0);
			}
			break;

		case 22050:
			if ((1 == fWaveFormatEx.nChannels) && (8 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_2M08) != 0);
			}
			else if ((1 == fWaveFormatEx.nChannels) && (16 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_2M16) != 0);
			}
			if ((2 == fWaveFormatEx.nChannels) && (8 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_2S08) != 0);
			}
			else if ((2 == fWaveFormatEx.nChannels) && (16 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_2S16) != 0);
			}
			break;

		case 44100:
			if ((1 == fWaveFormatEx.nChannels) && (8 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_4M08) != 0);
			}
			else if ((1 == fWaveFormatEx.nChannels) && (16 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_4M16) != 0);
			}
			if ((2 == fWaveFormatEx.nChannels) && (8 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_4S08) != 0);
			}
			else if ((2 == fWaveFormatEx.nChannels) && (16 == fWaveFormatEx.wBitsPerSample))
			{
				isSupported = ((deviceInfoPointer->dwFormats & WAVE_FORMAT_4S16) != 0);
			}
			break;
	}
	return isSupported;
}

/// Allocates buffer to store audio data from another thread.
/// @param nBuffers The number of buffers to allocate.
void WinAudioRecorder::AllocateBuffers(int nBuffers)
{
    int i;
	for(i=0; i < nBuffers; i++)
	{
		LPWAVEHDR lpWaveHdr = CreateWaveHeader();
		::waveInPrepareHeader(fRecordingHandle, lpWaveHdr, sizeof(WAVEHDR));
		::waveInAddBuffer(fRecordingHandle, lpWaveHdr, sizeof(WAVEHDR));			
		fBufferCount++;
	}
}

/// Creates header data to be applied to an audio buffer. Called by the AllocateBuffers() function.
LPWAVEHDR WinAudioRecorder::CreateWaveHeader()
{
	LPWAVEHDR lpWaveHdr = new WAVEHDR;
	ZeroMemory(lpWaveHdr, sizeof(WAVEHDR));
	BYTE* lpByte = new BYTE[(fWaveFormatEx.nBlockAlign * SOUNDSAMPLES)];

	lpWaveHdr->lpData = (char*)lpByte;
	lpWaveHdr->dwBufferLength = (fWaveFormatEx.nBlockAlign * SOUNDSAMPLES);
	return lpWaveHdr;
}

/// Swaps the bytes in the given buffer.
/// @param buffer The buffer to have its bytes swapped. Cannot be NULL.
/// @param bufferLength The number of bytes in the given buffer. Must be at least 2.
void WinAudioRecorder::SwapBytes(BYTE *buffer, int bufferLength)
{
	BYTE value;
	int index;

	// Validate arguments.
	if (!buffer || (bufferLength < 2))
	{
		return;
	}

	// If the number of bytes is odd, then ignore the last byte since it can't swap with anything.
	if (bufferLength % 2)
	{
		bufferLength--;
	}

	// Swap all bytes in the given buffer.
	for (index = 0; index < bufferLength; index += 2)
	{
		value = buffer[index];
		buffer[index] = buffer[index + 1];
		buffer[index + 1] = value;
	}
}

/// To be invoked by the WaveIn function on another thread when a new sound block has been recorded.
/// NOTE: Once waveInReset() has been called, you cannot call waveInUnprepareHeader() on the sound
///       blocks returned. You must be careful what you call in the callback once waveInReset() has
///       been called in the stop function.
/// @param dwInstance Provides a pointer to the WinAudioRecorder object this session belongs to.
/// @param dwParam1 Provides a pointer to audio data that was just recorded.
void WinAudioRecorder::OnSoundBlock(
		HWAVEIN hwl,UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	LPWAVEHDR lpWaveHdr = (LPWAVEHDR)dwParam1;

	if(lpWaveHdr && uMsg == WIM_DATA)
	{
		BYTE *lpInt = (BYTE*)lpWaveHdr->lpData;
		DWORD iRecorded = lpWaveHdr->dwBytesRecorded;

		if (fIsRunning)
		{
			::waveInUnprepareHeader(fRecordingHandle, lpWaveHdr, sizeof(WAVEHDR));
		}

		if (fIsRunning && fFileWriter)
		{
			WAVEHDR* pWriteHdr = new WAVEHDR;
			memcpy(pWriteHdr, lpWaveHdr, sizeof(WAVEHDR));
			BYTE * pSound = new BYTE[lpWaveHdr->dwBufferLength];
			if (!pSound)
			{
				delete pWriteHdr;
				return;
			}
			memcpy(pSound, lpWaveHdr->lpData ,lpWaveHdr->dwBufferLength);
			pWriteHdr->lpData = (char*)pSound;
			if (fFileWriter)
			{
				fFileWriter->WriteToSoundFile(GetCurrentThreadId(), (LPARAM)pWriteHdr);
			}
		}
		
		delete lpInt;
		delete lpWaveHdr;

		fBufferCount--;
		if (fIsRunning)
		{
			AllocateBuffers(1);
		}
	}
}

#pragma endregion


#pragma region CWriteSoundFile
// Modified from http://www.codeguru.com/cpp/g-m/multimedia/audio/article.php/c8877
/////////////////////////////////////////////////////////////////////////////
// CWriteSoundFile class
CWriteSoundFile::CWriteSoundFile()
{
    m_hFile = 0;
}

CWriteSoundFile::~CWriteSoundFile()
{
	CloseSoundFile();
}


// consult Microsoft Knowledge Base Article 551005
// previously known under E10955
// http://www.microsoft.com/intlkb/SPAIN/E10/9/55.ASP
//

bool CWriteSoundFile::CreateWaveFile( const TCHAR *sFilename, WAVEFORMATEX *pWaveFormatEx)
{
	int cbWaveFormatEx = sizeof(WAVEFORMATEX) + pWaveFormatEx->cbSize;

	m_hFile = ::mmioOpen((LPWSTR)sFilename,NULL, MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE | MMIO_ALLOCBUF);
	if(!m_hFile)
		return false;

	ZeroMemory(&m_MMCKInfoParent, sizeof(MMCKINFO));
	m_MMCKInfoParent.fccType = mmioFOURCC('W','A','V','E');

	MMRESULT mmResult =  ::mmioCreateChunk( m_hFile,&m_MMCKInfoParent,
							MMIO_CREATERIFF);
	
	ZeroMemory(&m_MMCKInfoChild, sizeof(MMCKINFO));
	m_MMCKInfoChild.ckid = mmioFOURCC('f','m','t',' ');
	m_MMCKInfoChild.cksize = cbWaveFormatEx;
	mmResult = ::mmioCreateChunk(m_hFile, &m_MMCKInfoChild, 0);
	mmResult = ::mmioWrite(m_hFile, (char*)pWaveFormatEx, cbWaveFormatEx); 
	mmResult = ::mmioAscend(m_hFile, &m_MMCKInfoChild, 0);
	m_MMCKInfoChild.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mmResult = ::mmioCreateChunk(m_hFile, &m_MMCKInfoChild, 0);

    return true;
}

void CWriteSoundFile::WriteToSoundFile(WPARAM wParam, LPARAM lParam)
{
	LPWAVEHDR lpHdr = (LPWAVEHDR) lParam;
	int cbLength = lpHdr->dwBufferLength;
	if(lpHdr)
	{
		char *soundbuffer = (char*) lpHdr->lpData;
		if(m_hFile && soundbuffer)
			::mmioWrite(m_hFile, soundbuffer, cbLength);
		if(soundbuffer)
			delete (BYTE*) soundbuffer;
		if(lpHdr)
			delete lpHdr;
	}
}

void CWriteSoundFile::CloseSoundFile()
{
	if(m_hFile)
	{
		::mmioAscend(m_hFile, &m_MMCKInfoChild, 0);
		::mmioAscend(m_hFile, &m_MMCKInfoParent, 0);
		::mmioClose(m_hFile, 0);
		m_hFile = NULL;
	}
}

#pragma endregion

} // namespace Rtt

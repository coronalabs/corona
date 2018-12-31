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

#ifndef _Rtt_WinAudioRecorder_H__
#define _Rtt_WinAudioRecorder_H__

#include <windows.h>
#include <mmsystem.h>
#include "Rtt_PlatformAudioRecorder.h"
#include "Rtt_String.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Modified from http://www.codeguru.com/cpp/g-m/multimedia/audio/article.php/c8877
/////////////////////////////////////////////////////////////////////////////
// CWriteSoundFile class

class CWriteSoundFile
{
	public:
		CWriteSoundFile();
		virtual ~CWriteSoundFile();

	public:
		bool CreateWaveFile( const TCHAR *sFilename, WAVEFORMATEX *pWaveFormatEx);
		void WriteToSoundFile(WPARAM wParam, LPARAM lParam);
		void CloseSoundFile();

	protected:
		BOOL IsSpeech();

		HMMIO m_hFile;
		MMCKINFO m_MMCKInfoData;
		MMCKINFO m_MMCKInfoParent;
		MMCKINFO m_MMCKInfoChild;
};


#define SOUNDSAMPLES 1000
#define MAXINPUTBUFFERS 5


/////////////////////////////////////////////////////////////////////////////
class WinAudioRecorder : public PlatformAudioRecorder
{
	public:
		WinAudioRecorder( const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file );
		virtual ~WinAudioRecorder();

		virtual void Start();
		virtual void Stop();
		void SwapBytes(BYTE *buffer, int bufferLength);
		void OnSoundBlock(HWAVEIN hwl,UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

	protected:
		void LogWaveInError(MMRESULT mmResult);
		bool UpdateFormatSettingsAccordingTo(WAVEINCAPS *deviceInfoPointer);
		bool IsFormatSettingsSupportedBy(WAVEINCAPS *deviceInfoPointer);
		LPWAVEHDR CreateWaveHeader();
		void AllocateBuffers(int nBuffers);

		String fFilename;
		HWAVEIN fRecordingHandle;
		WAVEFORMATEX fWaveFormatEx; 
		CWriteSoundFile* fFileWriter;
		int fBufferCount;
};
	

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_WinAudioRecorder_H__

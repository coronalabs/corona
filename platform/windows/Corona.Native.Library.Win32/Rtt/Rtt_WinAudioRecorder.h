//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

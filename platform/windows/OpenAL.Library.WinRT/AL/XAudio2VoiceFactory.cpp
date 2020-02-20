//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "XAudio2VoiceFactory.h"


namespace AL {

#pragma region Constructors/Destructors
XAudio2VoiceFactory::XAudio2VoiceFactory()
:	fAudioEnginePointer(nullptr)
{
}

XAudio2VoiceFactory::XAudio2VoiceFactory(IXAudio2 *audioEnginePointer)
:	fAudioEnginePointer(audioEnginePointer)
{
}

XAudio2VoiceFactory::~XAudio2VoiceFactory()
{
}

#pragma endregion


#pragma region Public Methods
HRESULT XAudio2VoiceFactory::CreateSourceVoice(
	IXAudio2SourceVoice **sourceVoicePointer,
	const WAVEFORMATEX *sourceFormatPointer,
	UINT32 flags /* = 0 */,
	float maxFrequencyRatio /* = XAUDIO2_DEFAULT_FREQ_RATIO */,
	IXAudio2VoiceCallback *callbackPointer /* = nullptr */,
	const XAUDIO2_VOICE_SENDS *sendListPointer /* = nullptr */,
	const XAUDIO2_EFFECT_CHAIN *effectChainPointer /* = nullptr */)
{
	if (nullptr == fAudioEnginePointer)
	{
		return E_POINTER;
	}

	return fAudioEnginePointer->CreateSourceVoice(
					sourceVoicePointer, sourceFormatPointer, flags, maxFrequencyRatio,
					callbackPointer, sendListPointer, effectChainPointer);
}

#pragma endregion

}	// namespace AL

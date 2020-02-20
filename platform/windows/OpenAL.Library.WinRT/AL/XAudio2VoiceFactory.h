//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <xaudio2.h>


namespace AL {

/// <summary>Creates IXAudio2SourceVoice objects without exposing the IXAudio object to the rest of the system.</summary>
class XAudio2VoiceFactory
{
	public:
		/// <summary>Creates an invalid factory that is unable to create voice objects.</summary>
		XAudio2VoiceFactory();

		/// <summary>Creates a new factory using the given XAudio2 engine to create voices.</summary>
		XAudio2VoiceFactory(IXAudio2 *audioEnginePointer);

		/// <summary>Destroys this object.</summary>
		virtual ~XAudio2VoiceFactory();

		/// <summary>Creates and configures a new source voice.</summary>
		/// <param name="sourceVoicePointer">If successful, returns a pointer to the new IXAudio2SourceVoice object.</param>
		/// <param name="sourceFormatPointer">Indicates the PCM format of the audio data this source will be processing.</param>
		/// <param name="flags">
		///  <para>Optional parameter that can have the following flags applied to it:</para>
		///  <para>XAUDIO2_VOICE_NOPITCH, XAUDIO2_VOICE_NOSRC, and XAUDIO2_VOICE_USEFILTER</para>
		/// </param>
		/// <param name="maxFrequencyRatio">
		///  <para>
		///   Optional parameter that can apply a maximum frequency to the source's audio processing to reduce memory usage.
		///  </para>
		///  <para>Must be set between XAUDIO2_MIN_FREQ_RATIO and XAUDIO2_MAX_FREQ_RATIO.</para>
		/// </param>
		/// <param name="callbackPointer">Optional parameter for adding audio playback callbacks.</param>
		/// <param name="sendListPointer">
		///  <para>
		///   Optional parameter to an array of XAUDIO2_VOICE_SENDS objects for applying a set of destination voices
		///   for the source's audio to be sent to.
		///  </para>
		///  <para>Set to null to send the source's audio to the master voice (ie: listener).</para>
		/// </param>
		/// <param name="effectChainPointer">
		///  Optional paramter to an array of XAUDIO2_EFFECT_CHAIN objects to be applied to the source's audio.
		/// </param>
		/// <returns>
		///  <para>
		///   Returns S_OK if the source voice was successfully created. In this case, parameter "sourceVoicePointer"
		///   will be set to the memory address of the newly created source voice object.
		///  </para>
		///  <para>Returns E_POINTER if this factory was not given a valid IXAudio2 object upon construction.</para>
		///  <para>
		///   Returns XAUDIO2_E_INVALID_CALL if given invalid parameters or if creating a source within another
		///   source's callback.
		///  </para>
		/// </returns>
		HRESULT CreateSourceVoice(
					IXAudio2SourceVoice **sourceVoicePointer,
					const WAVEFORMATEX *sourceFormatPointer,
					UINT32 flags = 0,
					float maxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO,
					IXAudio2VoiceCallback *callbackPointer = nullptr,
					const XAUDIO2_VOICE_SENDS *sendListPointer = nullptr,
					const XAUDIO2_EFFECT_CHAIN *effectChainPointer = nullptr);

	private:
		IXAudio2 *fAudioEnginePointer;
};

}	// namespace AL

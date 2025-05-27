//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

'use strict';

var audioLibrary =
{
	$audioCtx: null,
	$audioReservedChannels: 0,
	$audioSounds: [],		// loaded sounds
	$audioChannels: [],

	$clamp: function(a,b,c)
	{
		return Math.max(b,Math.min(c,a));
	},

	// channel
	$audioChannel: function (channel) {
		this.fChannel = channel;
		this.fVolume = 1;
		this.fFade1 = 0;
		this.fFade2 = 0;
		this.fFadeDuration = 0;
		this.fSource = null;
		this.fStartedAt = 0;
		this.fSoundID = -1;
		this.fLoops = 0;
		this.fTicks = 0;
		this.fStartPosition = 0;
		this.fLuaCallback = null;
		this.fMinVolume = 0;
		this.fMaxVolume = 1;

		this.getSource = function () { return this.fSource; }

		this.startSound = function () {
			if (this.fSoundID < 0 || this.fSoundID >= audioSounds.length || audioSounds[this.fSoundID].buffer == null) {
				return;
			}

			// delete old sound
			this.stopSound();

			this.fSource = audioCtx.createBufferSource();
			this.fSource.buffer = audioSounds[this.fSoundID].buffer;	
			this.fSource.parent = this;

			if (this.fPitch) {
				this.fSource.playbackRate.value = this.fPitch;
			}

			var gain = audioCtx.createGain();	// Create a gain node to control channel volume
			this.fSource.connect(gain);

			gain.connect(audioCtx.gainNode);
			this.fSource.gainNode = gain;
			this.fSource.loop = this.fLoops == -1;

			//console.log('sound started: ', this);

			// fade in
			if (this.fFade1 > 0 || this.Fade2 > 0) {
				gain.gain.setValueAtTime(clamp(this.fFade1, 0.01, 1), audioCtx.currentTime);		// init volume must start from 0.01
				gain.gain.linearRampToValueAtTime(clamp(this.fFade2, 0.01, 1), audioCtx.currentTime + this.fFadeDuration);
			}
			else
			{
				this.setVolume(this.fVolume);
			}
			this.fFade1 = 0;
			this.fFade2 = 0;
			this.fFadeDuration = 0;

			this.fSource.onended = function () {
				var ch = this.parent;
				console.log('channel finished', ch);
				console.log(ch.fLuaCallback);

				ch.fStartedAt = 0;
				if (ch.fLoops == 0) {
					_jsOnSoundEnded(ch.fChannel, ch.fLuaCallback, true);

					// free channel
					ch.clear();
				}
				else {
//					this.currentTime = 0;
//					this.play();
				}
			}

			var source = this.getSource();
			if (audioCtx.state == 'running' && (source.playbackState == undefined || source.playbackState === 0)) {
				var pos = Math.min(this.fStartPosition, source.buffer.duration);
				var duration = this.fTicks > 0 ? Math.min(this.fTicks, source.buffer.duration) : 0;

				if (duration > 0 && pos > 0) {
					source.start(0, pos, duration);
				}
				else
					if (pos > 0) {
					source.start(0, pos);
				}
				else {
					source.start(0);
				}

				this.fStartedAt = audioCtx.currentTime;
			}
			else {
				//console.log('failed to start sound, auduo context suspended', this);
			}
		}

		this.clear = function () {
			this.fSource = null;
			this.fStartedAt = 0;
			this.fSoundID = -1;
			this.fLoops = 0;
			this.fTicks = 0;
			this.fStartPosition = 0;
			this.fLuaCallback = null;
		}
		 
		this.stopSound = function () {
			var source = this.getSource();
			if (source) {
				//console.log('stopSound: ', this);

				// stop if it was started
				// fix for safari bug and old FF
				if (this.fStartedAt > 0 && (source.playbackState == undefined || source.playbackState === 2)) {
					this.fStartedAt = 0;
					source.stop();
				}
				source.gainNode.disconnect();
				source.disconnect();
			}
			this.fSource = null;
		};

		this.resumeSound = function () {
			this.startSound();
		}

		this.seekSound = function (position) {
			//console.log('seek sound: ', this);
			var source = this.getSource();
			if (source && source.buffer) {
				this.fStartPosition = position;
				this.stopSound();
				this.startSound();
			}
		};

		this.pauseSound = function () {
			var source = this.getSource();
			this.fStartPosition = 0;
			if (source && source.buffer)
			{
				var playtime = audioCtx.currentTime - this.fStartedAt;
				var playloops = Math.floor(playtime / source.buffer.duration);
				this.fStartPosition = playtime - playloops * source.buffer.duration;
				//console.log('pauseSound: ', playtime, source.buffer.duration, playloops, this.fStartPosition, this);
			}
			this.stopSound();
		}

		this.setVolume = function (vol) {
			this.fVolume = vol;
			var source = this.getSource();
			if (source) {
				source.gainNode.gain.value = this.fVolume;
			}
		}

		this.getVolume = function () {
			var source = this.getSource();
			if (source) {
				this.fVolume = source.gainNode.gain.value;
			}
			return this.fVolume;
		};

		this.attachSound = function (soundID, loops, ticks, callback, fade_duration) {

			// sanity check
			if (soundID < 0 || soundID >= audioSounds.length) {
				//console.log('invalid soundID');
				return;
			}

			// remove and stop old sound
			this.stopSound();

			this.fSoundID = soundID;
			this.fLoops = loops;
			this.fTicks = ticks;
			this.fLuaCallback = callback;
			this.fFade1 = 0;
			this.fFade2 = 1;
			this.fFadeDuration = fade_duration;
			//console.log('attachSound: ', this, soundID, loops, ticks, callback, ',fade_duration=', fade_duration);
		};

		this.isUsed = function () {
			return this.fSoundID != -1;
		}

		this.isFree = function () {
			return this.fSoundID == -1;
		}

		this.startFading = function (fade_ticks, from_volume, to_volume) {
			this.fFade1 = clamp(from_volume, 0, 1);
			this.fFade2 = clamp(to_volume, 0, 1);
			this.fFadeDuration = fade_ticks;
			//console.log('startFading: ch=', this.fChannel, from_volume, to_volume, fade_ticks);
			if (this.fSource && this.fSource.gainNode) {
				var gain = this.fSource.gainNode;
				gain.gain.setValueAtTime(clamp(this.fFade1, 0.01, 1), audioCtx.currentTime);
				gain.gain.linearRampToValueAtTime(clamp(this.fFade2, 0.01, 1), audioCtx.currentTime + fade_ticks);
				return 1;
			}
			return 0;
		}

		this.getMinVolume = function () {	return this.fMinVolume;	}
		this.getMaxVolume = function () {	return this.fMaxVolume;	}
		this.setMinVolume = function (val) { this.fMinVolume = val; }
		this.setMaxVolume = function (val) { this.fMaxVolume = val; }
		this.setPitch = function (val) {
			this.fPitch = val;
			var source = this.getSource();
			if (source) {
				source.playbackRate.value = val;
			}
		}
	},

	//
	//  audio c++ interface
	//

	jsAudioInit: function () {

		// create channels
		for (var i = 0; i < 32; i++) {
			audioChannels.push(new audioChannel(i));
		}

		// Fix up for prefixing
		window.AudioContext = window.AudioContext || window.webkitAudioContext;
		if (!window.AudioContext)
		{
			console.log('Failed to init sound subsystem, browser does not support AudioContext');
			return;
		}

		audioCtx = new AudioContext();
		if (audioCtx.state == undefined) {
			// old Android has no 'state' item
			audioCtx.state = 'running';
		}

		// Create a gain node to control master volume
		audioCtx.gainNode = audioCtx.createGain();
		audioCtx.gainNode.connect(audioCtx.destination);

		// suspend/resume player
		audioCtx.onstatechange = function (e) {
			//console.log('audioCtx.onstatechange: ', this.state, this);
			if (this.state == 'running')
				for (var i = 0; i < audioChannels.length; i++) {
					var source = audioChannels[i].getSource();
					if (source) {
					//	audioChannels[i].startSound(source.startPosition > 0 ? source.startPosition : 0);
					}
				}
		};

	},

	jsAudioGet: function (file) {
		var file_path = UTF8ToString(file);
		for (var i = 0; i < audioSounds.length; i++)
		{
			if (audioSounds[i] && audioSounds[i].file_path == file_path)
			{
				return i;
			}
		}
		return -1;
	},

	jsAudioDecode: function (file, data, size) {
		var file_path = UTF8ToString(file);

		// get ID
		var soundID = -1;
		for (var i = 0; i < audioSounds.length; i++)
		{
			if (audioSounds[i] == null)
			{
				soundID = i;
				break;
			}
		}
		if (soundID < 0) {
			audioSounds.push(null);
			soundID = audioSounds.length;
		}
		audioSounds[soundID] = { buffer: null, file_path: file_path, duration: 0 };

		//console.log('start decoder: id='+ soundID + ', file='+ file_path);

		// fixme.. optimize
		var buf = new Uint8Array(size);
		for (var i = 0; i < size; i++) {
			buf[i] = getValue(data + i, 'i8');
		}
		var audio = buf.buffer;		// convert to ArrayBuffer

		audioCtx.decodeAudioData(audio, function (audioBuffer) {
			//console.log('sound decoded: id= ',  soundID + ', duration=' + audioBuffer.duration + ', file='+ file_path);

			// ensure sound not deleted
			var snd = audioSounds[soundID];
			if (snd && snd.buffer == null && snd.file_path == file_path && snd.duration == 0) {
				snd.buffer = audioBuffer;
				snd.duration = audioBuffer.duration;

				// reset audio data in all channels assigned to soundID 
				for (var i = 0; i < audioChannels.length; i++) {
					if (audioChannels[i].fSoundID == soundID) {
						//console.log('decoded sound assigned to channel: ', i);
						audioChannels[i].startSound();
					}
				}
			}
		}, function (e) {
			console.log('Failed to decode audio ', e);
		});

		return soundID;
	}, // end of decodeAudio

	jsAudioPlay: function (channel, soundID, loops, ticks, callback, fade_ticks) {
		// seek unused item
		var ch = channel;
		if (ch < 0)
		{
			for (var i = audioReservedChannels; i < audioChannels.length; i++) {
				if (audioChannels[i].fSoundID == -1)
				{
					ch = i;
					break;
				}
			}
		}

		if (ch < 0 || ch >= audioChannels.length)
		{
			// no available channels
			return -1;
		}

		audioChannels[ch].attachSound(soundID, loops, ticks / 1000, callback, fade_ticks / 1000);
		audioChannels[ch].startSound(); 

		//console.log('jsAudioPlay: id=', soundID, ', ch=', ch, ', loops=', loops, ', duration=', ticks, ', callback=', callback);
		return ch;
	},

	jsAudioPause: function (channel) {
		//console.log('jsAudioPause', channel);
		if (channel == -1) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].pauseSound();
			}
		}
		else {
			if (channel >= 0 && channel < audioChannels.length) {
				audioChannels[channel].pauseSound();
			}
		}
		return true;
	},

	jsAudioResume: function (channel) {
		//console.log('jsAudioResume', channel);
		if (channel == -1) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].resumeSound();
			}
		}
		else {
			if (channel >= 0 && channel < audioChannels.length) {
				audioChannels[channel].resumeSound();
			}
		}
		return true;
	},

	jsAudioStop: function (channel) {
		//console.log('jsAudioStop', channel);
		if (channel == -1) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].stopSound();
			}
		}
		else {
			if (channel >= 0 && channel < audioChannels.length) {
				audioChannels[channel].stopSound();
			}
		}
		return true;
	},

	jsAudioSeek: function (channel, number_of_milliseconds) {
		if (channel >= 0 && channel < audioChannels.length) {
			audioChannels[channel].seekSound(number_of_milliseconds / 1000);
		}
		return true;
	},

	jsAudioRewind: function (channel) {
		if (channel >= 0 && channel < audioChannels.length) {
			audioChannels[channel].seekSound(0);
		}
		return true;
	},

	jsAudioIsPlaying: function (channel) {
		var source = audioChannels[channel].getSource();
		return source != null;
	},

	jsAudioCountAllFreeChannels: function () {
		var k = 0;
		for (var i = 0; i < audioChannels.length; i++) {
			if (audioChannels[i].fSoundID == -1) {
				k++;
			}
		}
		return k;
	},

	jsAudioGetVolume: function (channel) {
		if (channel == -1) {
			var sum = 0;
			for (var i = 0; i < audioChannels.length; i++) {
				sum += audioChannels[i].getVolume();
			}
			return sum / audioChannels.length;
		}
		else {
			if (channel >= 0 && channel < audioChannels.length) {
				return audioChannels[channel].getVolume();
			}
		}
		return 0;
	},

	jsAudioSetVolume: function (channel, vol) {
		if (channel == -1)
		{
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].setVolume(vol);
			}
			return true;
		}
		else
		{
			if (channel >= 0 && channel < audioChannels.length) {
				audioChannels[channel].setVolume(vol);
				return true;
			}
		}
		return false;
	},

	jsAudioGetMasterVolume: function () {
		return audioCtx.gainNode.gain.value;
	},

	jsAudioSetMasterVolume: function (vol) {
		audioCtx.gainNode.gain.value = vol;
		return true;
	},

	jsAudioSuspendPlayer: function () {
		if (audioCtx && audioCtx.suspend) {
			audioCtx.suspend();
		}
	},

	jsAudioResumePlayer: function () {
		if (audioCtx && audioCtx.resume) {
			audioCtx.resume();
		}
	},

	jsAudioGetTotalTime: function(soundID)
	{
		if (soundID >= 0 && soundID < audioSounds.length && audioSounds[soundID].buffer != null) {
			return Math.floor(audioSounds[soundID].buffer.duration * 1000);
		}
		return 0;
	},

	jsAudioIsActiveChannel: function(ch)
	{
		// Returns true if the specified channel is currently playing or paused, 
		// or if - 1 is passed the number of channels that are currently playing or paused.
		if (ch >= 0 && ch < audioChannels.length)
		{
			return audioChannels[ch].isUsed();
		}

		if (ch < 0)
		{
			var n = 0;
			for (var i = 0; i < audioChannels.length; i++) if (audioChannels[i].isUsed())
			{
				n++;
			}
			return n;
		}
		return 0;
	},

	jsAudioCountUnreservedFreeChannels: function()
	{
		// Returns the number of channels that are currently available for playback (not playing, not paused),
		// excluding the channels that have been reserved.
		// ALmixer_CountUnreservedFreeChannels();
		var ret = jsAudioCountAllFreeChannels() - audioReservedChannels;
		return ret < 0 ? 0 : ret; 
	},

	jsAudioCountTotalChannels: function()
	{
		return audioChannels.length;
	},

	jsAudioCountReservedChannels: function()
	{
		return audioReservedChannels;
	},

	jsAudioReserveChannels: function (number_of_reserve_channels)
	{
		// Allows you to reserve a certain number of channels so they won't be automatically allocated to play on.
		// number_of_reserve_channels =	The number of channels / sources to reserve.
		// Or pass - 1 to find out how many channels are currently reserved.
		// ALmixer_ReserveChannels(number_of_reserve_channels);
		if (number_of_reserve_channels >= 0) {
			audioReservedChannels = clamp(number_of_reserve_channels, 0, audioChannels.length);
		}
		return audioReservedChannels;
	},

	jsFreeData: function(soundID)
	{
		if (soundID >= 0 && soundID < audioSounds.length) {

			// stop all related playing threads
			for (var i = 0; i < audioChannels.length; i++)
			{
				if (audioChannels[i].fSoundID == soundID)
				{
					audioChannels[i].stopSound();
					audioChannels[i].clear();
				}
			}
			audioSounds[soundID] = null;
		}
	},

	jsAudioFadeChannel: function (ch, fade_ticks, to_volume) {
		if (ch >= 0 && ch < audioChannels.length)
		{
			return audioChannels[ch].startFading(fade_ticks / 1000, audioChannels[ch].getVolume(), to_volume);
		}

		var countFadedChannels = 0;
		if (ch < 0)
		{
			for (var i = 0; i < audioChannels.length; i++) if (audioChannels[i].isUsed())
			{
				countFadedChannels += audioChannels[i].startFading(fade_ticks / 1000, audioChannels[i].getVolume(), to_volume);
			}
		}
		return countFadedChannels;
	},

	jsAudioFindFreeChannel: function(start_channel)
	{
		// start_channel	The channel number you want to start looking at.
		// Returns A channel available or -1 if none could be found.
		// ALmixer_FindFreeChannel(start_channel);
		for (var i = start_channel; i >= 0 && i < audioChannels.length; i++)
		{
			if (audioChannels[i].isFree())
			{
				return i;
			}
		}
		return -1;
	},

	jsAudioSetMaxVolumeChannel: function (ch, val) {
		// which_channel =	The channel to set the volume to or -1 to set on all channels.
		//	volume	The new volume to use.Valid values are 0.0 to 1.0.
		//	Returns	AL_TRUE on success or AL_FALSE on error.
		if (ch >= 0 && ch < audioChannels.length)
		{
			return (audioChannels[ch] && audioChannels[ch].setMaxVolume(val)) ? true : false;
		}

		if (ch < 0)
		{
			for (var i = 0; i < audioChannels.length; i++)
			{
				audioChannels[i].setMaxVolume(val);
			}
			return true;
		}
		return false;
	},

	jsAudioGetMaxVolumeChannel: function (ch) {
		// which_channel	= The channel to get the volume from. - 1 will return the average volume set across all channels.
		// Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
		// ALmixer_GetMaxVolumeChannel(which_channel);
		if (ch >= 0 && ch < audioChannels.length)
		{
			return audioChannels[ch].getMaxVolume();
		}

		if (ch < 0)
		{
			var sum = 0;
			for (var i = 0; i < audioChannels.length; i++)
			{
				sum += audioChannels[i].getMaxVolume();
			}
			return sum / audioChannels.length;
		}
		return 0;
	},

	jsAudioSetMinVolumeChannel: function (which_channel, new_volume) {
		// which_channel =	The channel to set the volume to or -1 to set on all channels.
		//	volume	The new volume to use.Valid values are 0.0 to 1.0.
		//	Returns	AL_TRUE on success or AL_FALSE on error.
		if (ch >= 0 && ch < audioChannels.length) {
			return (audioChannels[ch] && audioChannels[ch].setMinVolume(val)) ? true : false;
		}

		if (ch < 0) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].setMinVolume(val);
			}
			return true;
		}
		return false;
	},

	jsAudioGetMinVolumeChannel: function (which_channel) {
		// which_channel	= The channel to get the volume from. - 1 will return the average volume set across all channels.
		// Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
		// ALmixer_GetMaxVolumeChannel(which_channel);
		if (ch >= 0 && ch < audioChannels.length) {
			return audioChannels[ch].getMinVolume();
		}

		if (ch < 0) {
			var sum = 0;
			for (var i = 0; i < audioChannels.length; i++) {
				sum += audioChannels[i].getMinVolume();
			}
			return sum / audioChannels.length;
		}
		return 0;
	},

	jsAudioStopWithDelay: function()
	{
		// todo
		return 1;
	},

	jsAudioSetPitch: function(ch, val) {
		if (ch >= 0 && ch < audioChannels.length) {
			audioChannels[ch].setPitch(val);
			return true;
		}
		return false;
	},

}

autoAddDeps(audioLibrary, '$clamp');
autoAddDeps(audioLibrary, '$audioReservedChannels');
autoAddDeps(audioLibrary, '$audioCtx');
autoAddDeps(audioLibrary, '$audioSounds');
autoAddDeps(audioLibrary, '$audioChannel');
autoAddDeps(audioLibrary, '$audioChannels');
mergeInto(LibraryManager.library, audioLibrary);

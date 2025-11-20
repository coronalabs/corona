var audioLibrary =
{
	$audioCtx: null,
	$audioReservedChannels: 0,
	$audioSounds: [],		// loaded sounds
	$audioChannels: [],
	$audioContextResumed: false,

	$clamp: function(a,b,c)
	{
		return Math.max(b,Math.min(c,a));
	},

	// Ensure audio context is running (fixes Firefox/Chrome/Safari autoplay issues)
	$ensureAudioContext: function() {
		if (!audioCtx) return false;
		
		if (audioCtx.state === 'suspended') {
			audioCtx.resume().then(function() {
				audioContextResumed = true;
			}).catch(function(err) {
				console.log('Failed to resume audio context:', err);
			});
			return false;
		}
		return audioCtx.state === 'running';
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
		this.fPitch = 1;
		this.fPaused = false;
		this.fStopRequested = false;  // NEW: Track if stop was requested

		this.getSource = function () { return this.fSource; }

		this.startSound = function () {
			if (this.fSoundID < 0 || this.fSoundID >= audioSounds.length || !audioSounds[this.fSoundID] || audioSounds[this.fSoundID].buffer == null) {
				return;
			}

			// Ensure audio context is running
			if (!ensureAudioContext()) {
				return;
			}

			// Stop any existing sound on this channel
			this.stopSound();
			
			// Reset stop flag
			this.fStopRequested = false;

			try {
				this.fSource = audioCtx.createBufferSource();
				this.fSource.buffer = audioSounds[this.fSoundID].buffer;	
				this.fSource.parent = this;
				this.fSource.channelIndex = this.fChannel;  // Store for debugging

				if (this.fPitch) {
					this.fSource.playbackRate.value = this.fPitch;
				}

				var gain = audioCtx.createGain();
				// Initialize gain: if fading, start at 0; otherwise use channel volume
				if (this.fFadeDuration > 0) {
					gain.gain.value = 0;  // Will fade from 0
				} else {
					gain.gain.value = this.fVolume;  // Use channel volume immediately
				}
				
				this.fSource.connect(gain);
				gain.connect(audioCtx.gainNode);
				this.fSource.gainNode = gain;
				this.fSource.loop = this.fLoops == -1;

				// Set volume BEFORE starting playback
				var scheduleTime = audioCtx.currentTime + 0.08;  // Tiny delay
				
				if (this.fFadeDuration > 0) {
					// Fade enabled - use fade values (allow 0 volume)
					var startVol = Math.max(0, Math.min(1, this.fFade1));
					var endVol = Math.max(0, Math.min(1, this.fFade2));
					// Schedule the fade
					gain.gain.setValueAtTime(startVol, scheduleTime);
					gain.gain.linearRampToValueAtTime(endVol, scheduleTime + this.fFadeDuration);
				} else {
					// No fade - use current channel volume
					gain.gain.setValueAtTime(this.fVolume, scheduleTime);
				}
				
				this.fFade1 = 0;
				this.fFade2 = 0;
				this.fFadeDuration = 0;

				// Handle sound completion
				this.fSource.onended = function () {
					var ch = this.parent;
					if (!ch) return;
					
					// If this source is not the current source, ignore (already replaced)
					if (ch.fSource !== this) {
						return;
					}
					
					// If stop was requested, don't treat as natural completion
					if (ch.fStopRequested) {
						ch.fStopRequested = false;
						return;
					}

					ch.fStartedAt = 0;
					ch.fPaused = false;
					
					if (ch.fLoops == 0) {
						_jsOnSoundEnded(ch.fChannel, ch.fLuaCallback, true);
						ch.clear();
					}
				}

				// Start playback with the same schedule delay
				var pos = Math.min(this.fStartPosition, this.fSource.buffer.duration);
				var duration = this.fTicks > 0 ? Math.min(this.fTicks, this.fSource.buffer.duration) : 0;

				if (audioCtx.state === 'running') {
					if (duration > 0 && pos > 0) {
						this.fSource.start(scheduleTime, pos, duration);
					} else if (pos > 0) {
						this.fSource.start(scheduleTime, pos);
					} else {
						this.fSource.start(scheduleTime);
					}
					this.fStartedAt = scheduleTime;
				}
			} catch(e) {
				console.log('Error starting sound on channel', this.fChannel, ':', e);
				this.fSource = null;
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
			this.fPaused = false;
			this.fStopRequested = false;
		}
		 
		this.stopSound = function () {
			this.fStopRequested = true;
			var source = this.getSource();
			
			if (source) {
				try {
					// Mark as stopped immediately
					this.fStartedAt = 0;
					
					// Try to stop the source
					if (source.stop) {
						try {
							source.stop();
						} catch(e) {
							// Already stopped or never started - this is OK
						}
					}
					
					// Disconnect nodes
					if (source.gainNode) {
						try {
							source.gainNode.disconnect();
						} catch(e) {
							// Already disconnected
						}
					}
					
					try {
						source.disconnect();
					} catch(e) {
						// Already disconnected
					}
				} catch(e) {
					// Ignore errors - channel will be cleared anyway
				}
			}
			
			this.fSource = null;
			this.fPaused = false;
			this.fStartPosition = 0;
		};

		this.resumeSound = function () {
			if (this.fPaused && this.fSoundID >= 0) {
				this.startSound();
				this.fPaused = false;
			}
		}

		this.seekSound = function (position) {
			var source = this.getSource();
			if (source && source.buffer) {
				this.fStartPosition = position;
				this.stopSound();
				this.startSound();
			}
		};

		this.pauseSound = function () {
			var source = this.getSource();
			if (!source || !source.buffer) {
				return;
			}
			
			if (this.fStartedAt > 0) {
				var playtime = audioCtx.currentTime - this.fStartedAt;
				var playloops = Math.floor(playtime / source.buffer.duration);
				this.fStartPosition = playtime - playloops * source.buffer.duration;
				this.fPaused = true;
			}
			this.stopSound();
		}

		this.setVolume = function (vol) {
			this.fVolume = vol;
			var source = this.getSource();
			if (source && source.gainNode) {
				try {
					// Cancel any active ramps/fades
					source.gainNode.gain.cancelScheduledValues(audioCtx.currentTime);
					// Set volume immediately
					source.gainNode.gain.setValueAtTime(vol, audioCtx.currentTime);
				} catch(e) {
					console.log('Error setting volume:', e);
				}
			}
		}

		this.getVolume = function () {
			var source = this.getSource();
			if (source && source.gainNode) {
				try {
					this.fVolume = source.gainNode.gain.value;
				} catch(e) {
					// Ignore
				}
			}
			return this.fVolume;
		};

		this.attachSound = function (soundID, loops, ticks, callback, fade_duration) {
			if (soundID < 0 || soundID >= audioSounds.length) {
				return;
			}

			this.stopSound();

			this.fSoundID = soundID;
			this.fLoops = loops;
			this.fTicks = ticks;
			this.fLuaCallback = callback;
			
			// Store the CURRENT volume before any potential Lua setVolume calls
			var targetVolume = this.fVolume;
			
			// If fade_duration provided, fade from 0 to the volume that was set BEFORE play
			if (fade_duration > 0) {
				this.fFade1 = 0;
				this.fFade2 = targetVolume;
				this.fFadeDuration = fade_duration;
			} else {
				this.fFade1 = 0;
				this.fFade2 = 0;
				this.fFadeDuration = 0;
			}
			
			this.fStartPosition = 0;
			this.fPaused = false;
		};

		this.isUsed = function () {
			return this.fSoundID != -1;
		}

		this.isFree = function () {
			return this.fSoundID == -1 && this.fSource == null;
		}

		this.startFading = function (fade_ticks, from_volume, to_volume) {
			this.fFade1 = clamp(from_volume, 0, 1);
			this.fFade2 = clamp(to_volume, 0, 1);
			this.fFadeDuration = fade_ticks;
			
			if (this.fSource && this.fSource.gainNode) {
				try {
					var gain = this.fSource.gainNode;
					gain.gain.setValueAtTime(clamp(this.fFade1, 0.01, 1), audioCtx.currentTime);
					gain.gain.linearRampToValueAtTime(clamp(this.fFade2, 0.01, 1), audioCtx.currentTime + fade_ticks);
					return 1;
				} catch(e) {
					console.log('Error fading:', e);
				}
			}
			return 0;
		}

		this.getMinVolume = function () { return this.fMinVolume; }
		this.getMaxVolume = function () { return this.fMaxVolume; }
		this.setMinVolume = function (val) { this.fMinVolume = val; }
		this.setMaxVolume = function (val) { this.fMaxVolume = val; }
		this.setPitch = function (val) {
			this.fPitch = val;
			var source = this.getSource();
			if (source) {
				try {
					source.playbackRate.value = val;
				} catch(e) {
					// Ignore
				}
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
		if (!window.AudioContext) {
			console.log('Failed to init sound subsystem, browser does not support AudioContext');
			return;
		}

		audioCtx = new AudioContext();
		if (audioCtx.state == undefined) {
			audioCtx.state = 'running';
		}

		// Create a gain node to control master volume
		audioCtx.gainNode = audioCtx.createGain();
		audioCtx.gainNode.connect(audioCtx.destination);

		// Try to resume audio context immediately
		if (audioCtx.state === 'suspended') {
			ensureAudioContext();
		}

		// Add user interaction listeners to resume audio context
		var resumeAudio = function() {
			if (audioCtx && audioCtx.state === 'suspended') {
				ensureAudioContext();
			}
		};
		
		document.addEventListener('click', resumeAudio, { once: false });
		document.addEventListener('touchstart', resumeAudio, { once: false });
		document.addEventListener('keydown', resumeAudio, { once: false });

		// Handle state changes
		audioCtx.onstatechange = function (e) {
			if (this.state == 'running') {
				audioContextResumed = true;
				// Restart any channels that were waiting
				for (var i = 0; i < audioChannels.length; i++) {
					if (audioChannels[i].fSoundID >= 0 && !audioChannels[i].getSource()) {
						audioChannels[i].startSound();
					}
				}
			}
		};
	},

	jsAudioGet: function (file) {
		var file_path = UTF8ToString(file);
		for (var i = 0; i < audioSounds.length; i++) {
			if (audioSounds[i] && audioSounds[i].file_path == file_path) {
				return i;
			}
		}
		return -1;
	},

	jsAudioDecode: function (file, data, size) {
		var file_path = UTF8ToString(file);

		// get ID
		var soundID = -1;
		for (var i = 0; i < audioSounds.length; i++) {
			if (audioSounds[i] == null) {
				soundID = i;
				break;
			}
		}
		if (soundID < 0) {
			audioSounds.push(null);
			soundID = audioSounds.length - 1;
		}
		audioSounds[soundID] = { buffer: null, file_path: file_path, duration: 0 };

		// Convert to ArrayBuffer
		var buf = new Uint8Array(size);
		for (var i = 0; i < size; i++) {
			buf[i] = getValue(data + i, 'i8');
		}
		var audio = buf.buffer;

		audioCtx.decodeAudioData(audio, function (audioBuffer) {
			var snd = audioSounds[soundID];
			if (snd && snd.buffer == null && snd.file_path == file_path && snd.duration == 0) {
				snd.buffer = audioBuffer;
				snd.duration = audioBuffer.duration;

				// Start any channels waiting for this sound
				for (var i = 0; i < audioChannels.length; i++) {
					if (audioChannels[i].fSoundID == soundID) {
						audioChannels[i].startSound();
					}
				}
			}
		}, function (e) {
			console.log('Failed to decode audio', e);
		});

		return soundID;
	},

	jsAudioPlay: function (channel, soundID, loops, ticks, callback, fade_ticks) {
		ensureAudioContext();

		var ch = channel;
		if (ch < 0) {
			// Auto-select from unreserved channels
			for (var i = audioReservedChannels; i < audioChannels.length; i++) {
				if (audioChannels[i].isFree()) {
					ch = i;
					break;
				}
			}
			
			if (ch < 0) {
				return -1;
			}
		} else {
			// Explicit channel - stop whatever is there
			if (ch >= 0 && ch < audioChannels.length && audioChannels[ch].isUsed()) {
				audioChannels[ch].stopSound();
				audioChannels[ch].clear();
			}
		}

		if (ch < 0 || ch >= audioChannels.length) {
			return -1;
		}

		// IMPORTANT: Attach sound FIRST, but don't start yet
		audioChannels[ch].attachSound(soundID, loops, ticks / 1000, callback, fade_ticks / 1000);
		
		// NOW start the sound (after attachSound has set up fade parameters)
		audioChannels[ch].startSound();

		return ch;
	},

	jsAudioPause: function (channel) {
		if (channel == -1) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].pauseSound();
			}
		} else {
			if (channel >= 0 && channel < audioChannels.length) {
				audioChannels[channel].pauseSound();
			}
		}
		return true;
	},

	jsAudioResume: function (channel) {
		ensureAudioContext();
		
		if (channel == -1) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].resumeSound();
			}
		} else {
			if (channel >= 0 && channel < audioChannels.length) {
				audioChannels[channel].resumeSound();
			}
		}
		return true;
	},

	jsAudioStop: function (channel) {
		if (channel == -1) {
			// Stop all unreserved channels only
			for (var i = audioReservedChannels; i < audioChannels.length; i++) {
				audioChannels[i].stopSound();
				audioChannels[i].clear();
			}
		} else {
			if (channel >= 0 && channel < audioChannels.length) {
				audioChannels[channel].stopSound();
				audioChannels[channel].clear();
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
		if (channel >= 0 && channel < audioChannels.length) {
			var source = audioChannels[channel].getSource();
			return source != null && !audioChannels[channel].fPaused;
		}
		return false;
	},

	jsAudioCountAllFreeChannels: function () {
		var k = 0;
		for (var i = 0; i < audioChannels.length; i++) {
			if (audioChannels[i].isFree()) {
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
		} else {
			if (channel >= 0 && channel < audioChannels.length) {
				return audioChannels[channel].getVolume();
			}
		}
		return 0;
	},

	jsAudioSetVolume: function (channel, vol) {
		if (channel == -1) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].setVolume(vol);
			}
			return true;
		} else {
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

	jsAudioGetTotalTime: function(soundID) {
		if (soundID >= 0 && soundID < audioSounds.length && audioSounds[soundID] && audioSounds[soundID].buffer != null) {
			return Math.floor(audioSounds[soundID].buffer.duration * 1000);
		}
		return 0;
	},

	jsAudioIsActiveChannel: function(ch) {
		if (ch >= 0 && ch < audioChannels.length) {
			return audioChannels[ch].isUsed();
		}

		if (ch < 0) {
			var n = 0;
			for (var i = 0; i < audioChannels.length; i++) {
				if (audioChannels[i].isUsed()) {
					n++;
				}
			}
			return n;
		}
		return 0;
	},

	jsAudioCountUnreservedFreeChannels: function() {
		var allFree = 0;
		for (var i = 0; i < audioChannels.length; i++) {
			if (audioChannels[i].isFree()) {
				allFree++;
			}
		}
		var ret = allFree - audioReservedChannels;
		return ret < 0 ? 0 : ret; 
	},

	jsAudioCountTotalChannels: function() {
		return audioChannels.length;
	},

	jsAudioCountReservedChannels: function() {
		return audioReservedChannels;
	},

	jsAudioReserveChannels: function (number_of_reserve_channels) {
		if (number_of_reserve_channels >= 0) {
			audioReservedChannels = clamp(number_of_reserve_channels, 0, audioChannels.length);
		}
		return audioReservedChannels;
	},

	jsFreeData: function(soundID) {
		if (soundID >= 0 && soundID < audioSounds.length) {
			// Check if any channel is actively playing this sound
			var activeChannels = [];
			for (var i = 0; i < audioChannels.length; i++) {
				if (audioChannels[i].fSoundID == soundID && audioChannels[i].getSource() != null) {
					activeChannels.push(i);
				}
			}
			
			// If sound is playing, refuse to dispose
			if (activeChannels.length > 0) {
				console.log('WARNING: Cannot dispose soundID', soundID, '- still playing on channels:', activeChannels);
				return;
			}

			// Safe to dispose - stop and clear all channels using this sound
			for (var i = 0; i < audioChannels.length; i++) {
				if (audioChannels[i].fSoundID == soundID) {
					audioChannels[i].stopSound();
					audioChannels[i].clear();
				}
			}
			
			audioSounds[soundID] = null;
		}
	},

	jsAudioFadeChannel: function (ch, fade_ticks, to_volume) {
		if (ch >= 0 && ch < audioChannels.length) {
			return audioChannels[ch].startFading(fade_ticks / 1000, audioChannels[ch].getVolume(), to_volume);
		}

		var countFadedChannels = 0;
		if (ch < 0) {
			for (var i = 0; i < audioChannels.length; i++) {
				if (audioChannels[i].isUsed()) {
					countFadedChannels += audioChannels[i].startFading(fade_ticks / 1000, audioChannels[i].getVolume(), to_volume);
				}
			}
		}
		return countFadedChannels;
	},

	jsAudioFindFreeChannel: function(start_channel) {
		for (var i = start_channel; i >= 0 && i < audioChannels.length; i++) {
			if (audioChannels[i].isFree()) {
				return i;
			}
		}
		return -1;
	},

	jsAudioSetMaxVolumeChannel: function (ch, val) {
		if (ch >= 0 && ch < audioChannels.length) {
			audioChannels[ch].setMaxVolume(val);
			return true;
		}

		if (ch < 0) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].setMaxVolume(val);
			}
			return true;
		}
		return false;
	},

	jsAudioGetMaxVolumeChannel: function (ch) {
		if (ch >= 0 && ch < audioChannels.length) {
			return audioChannels[ch].getMaxVolume();
		}

		if (ch < 0) {
			var sum = 0;
			for (var i = 0; i < audioChannels.length; i++) {
				sum += audioChannels[i].getMaxVolume();
			}
			return sum / audioChannels.length;
		}
		return 0;
	},

	jsAudioSetMinVolumeChannel: function (ch, val) {
		if (ch >= 0 && ch < audioChannels.length) {
			audioChannels[ch].setMinVolume(val);
			return true;
		}

		if (ch < 0) {
			for (var i = 0; i < audioChannels.length; i++) {
				audioChannels[i].setMinVolume(val);
			}
			return true;
		}
		return false;
	},

	jsAudioGetMinVolumeChannel: function (ch) {
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

	jsAudioStopWithDelay: function() {
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
autoAddDeps(audioLibrary, '$audioContextResumed');
autoAddDeps(audioLibrary, '$ensureAudioContext');
mergeInto(LibraryManager.library, audioLibrary);
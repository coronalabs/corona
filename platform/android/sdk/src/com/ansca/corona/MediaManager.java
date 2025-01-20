//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.util.HashMap;

import android.app.Activity;
import android.content.Context;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.view.SurfaceHolder;
import android.webkit.URLUtil;
import android.widget.MediaController;
import android.content.res.AssetManager;
import android.content.res.AssetFileDescriptor;


public class MediaManager {
	
	private static final int			SOUNDPOOL_STREAMS = 4;
	private Context 					myContext;
	private SoundPool					mySoundPool;
	private HashMap<Long,Integer>		myIdToSoundPoolIdMap;
	private HashMap<Long,MediaPlayer>   myMediaPlayers;
	private float						myVolume;
	private AudioRecorder				myAudioRecorder;
	private Handler 					myHandler;
	private CoronaRuntime 				myCoronaRuntime;
	
	public MediaManager( CoronaRuntime runtime, Context context )
	{
		myContext = context;
		myCoronaRuntime = runtime;
	}

	public void init()
	{
		myIdToSoundPoolIdMap = new HashMap<Long,Integer>();
		mySoundPool = new SoundPool( SOUNDPOOL_STREAMS, AudioManager.STREAM_MUSIC, 0);
		
		myMediaPlayers = new HashMap<Long,MediaPlayer>();

		// The main looper lives on the main thread of the application.  
		// This will bind the handler to post all runnables to the main ui thread.
		myHandler = new Handler(Looper.getMainLooper());
	}
	
	public void release()
	{
		if ( mySoundPool != null ) {
			mySoundPool.release();
			mySoundPool = null;
			myMediaPlayers = null;
			myIdToSoundPoolIdMap = null;
		}
	}
	
	public void loadSound( final long id, String soundName )
	{
		try {
			MediaPlayer mp = null;
			
			// Notify the application that audio is being used.
			onUsingAudio();

			if ( soundName.startsWith("/") || soundName.startsWith("http:") ) {
				
		        if ( !URLUtil.isNetworkUrl(soundName) ) {
		        	File file = new File(soundName);
		        	if ( !file.exists() ) {
						System.err.println("Could not load sound " + soundName);
						return;
		        	}
		        
		            FileInputStream fis = new FileInputStream(file);
		        	mp = new MediaPlayer();
		            mp.setDataSource(fis.getFD());
		            mp.setAudioStreamType(AudioManager.STREAM_MUSIC);
		            mp.prepare();
		        } else {
					Uri uri = Uri.parse(soundName);
			        mp = MediaPlayer.create(myCoronaRuntime.getController().getContext(), uri );
				}
			}

			if ( mp == null ) {
				System.err.println("Could not load sound " + soundName);
				return;
			}
				
			mp.setOnErrorListener(new MediaPlayer.OnErrorListener() {

				public boolean onError(MediaPlayer mp, int what, int extra) {
					mp.release();
					if ( myMediaPlayers != null ) {
						myMediaPlayers.remove( id );
					}

					if (myCoronaRuntime != null) {
						myCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.SoundEndedTask(id));
					}
					return true;
				}
			});

			mp.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
				public void onCompletion(MediaPlayer mp) {
					mp.release();
					if ( myMediaPlayers != null ) {
						myMediaPlayers.remove( id );
					}
					
					if (myCoronaRuntime != null) {
						myCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.SoundEndedTask(id));
					}
				}
			});
			
			myMediaPlayers.put( id, mp );
		} catch ( Exception e ) {
				
		}
	}

	public void loadEventSound( long id, String inSoundName )
	{
		// Validate.
		if ((inSoundName == null) || (inSoundName.length() <= 0)) {
			return;
		}

		// Fetch the activity context.
		android.content.Context context = myContext;
		if (context == null) {
			return;
		}

		// Notify the application that audio is being used.
		onUsingAudio();

		// Load the sound file.
		com.ansca.corona.storage.FileServices fileServices;
		fileServices = new com.ansca.corona.storage.FileServices(context);
		if (fileServices.isAssetFile(inSoundName)) {
			final AssetFileDescriptor fd = fileServices.openAssetFileDescriptorFor(inSoundName);
			if (fd != null) {
				int soundId = mySoundPool.load( fd, 1 );
				myIdToSoundPoolIdMap.put(id, soundId);
			}
		}
		else {
			int soundId = mySoundPool.load( inSoundName, 1 );
			myIdToSoundPoolIdMap.put(id, soundId);
		}
	}
	
	public void playMedia( final long id, boolean loop )
	{
		// Notify the application that audio is being used.
		onUsingAudio();

		// Play the given audio.
		MediaPlayer mp = null;
		if ( myMediaPlayers != null ) {
			mp = myMediaPlayers.get( id );
		}

		if ( mp != null ) {
			try {
				mp.setLooping(loop);
				mp.start();
			} catch (Exception e) {
				android.util.Log.e("Corona", "Error playing file", e);
			}
		} else {
			Integer soundId = null;
			if ( myIdToSoundPoolIdMap != null) {
				soundId = myIdToSoundPoolIdMap.get( id );
			}

			if ( soundId != null ) {
	            AudioManager mgr = (AudioManager) myContext.getSystemService(Context.AUDIO_SERVICE); 
	            float streamVolume = mgr.getStreamVolume(AudioManager.STREAM_MUSIC); 
	            float streamVolumeMax = mgr.getStreamMaxVolume(AudioManager.STREAM_MUSIC);      
	            float volume = streamVolume / streamVolumeMax;  
	            
	            int result = mySoundPool.play(soundId, volume, volume, 1, 0, 1f);
	            if ( result == 0 )
					System.out.println( "playSound failed " + soundId );
			}
		}
	}
	
    public void stopMedia( final long id )
	{
		MediaPlayer mp = null;
		if ( myMediaPlayers != null ) {
			mp = myMediaPlayers.get( id );
		}

		if ( mp != null ) {
			try {
				mp.stop();
			} catch (Exception e) {
				android.util.Log.e("Corona", "Error while stopping player", e);
			}
		} else {
			Integer soundId = null;
			if ( myIdToSoundPoolIdMap != null) {
				soundId = myIdToSoundPoolIdMap.get( id );
			}

			if ( soundId != null ) {
				mySoundPool.stop( soundId );
			}
		}
	}

	public void pauseMedia( final long id )
	{
		MediaPlayer mp = null;
		if ( myMediaPlayers != null ) {
			mp = myMediaPlayers.get( id );
		}

		if ( mp != null ) {
			try {
				mp.pause();
			} catch (Exception e) {
				android.util.Log.e("Corona", "Error while stopping player", e);
			}
		} else {
			Integer soundId = null;
			if ( myIdToSoundPoolIdMap != null) {
				soundId = myIdToSoundPoolIdMap.get( id );
			}

			if ( soundId != null ) {
				mySoundPool.pause( soundId );
			}
		}
	}

	public void resumeMedia( final long id  )
	{
		MediaPlayer mp = null;
		if ( myMediaPlayers != null ) {
			mp = myMediaPlayers.get( id );
		}

		if ( mp != null ) {
			try {
				mp.start();
			} catch (Exception e) {
				android.util.Log.e("Corona", "Error while stopping player", e);
			}
		} else {
			Integer soundId = null;
			if ( myIdToSoundPoolIdMap != null) {
				soundId = myIdToSoundPoolIdMap.get( id );
			}

			if ( soundId != null ) {
				mySoundPool.resume( soundId );
			}
		}
	}

	public void pauseAll()
	{
		if ( myMediaPlayers != null ) {
			for ( Long key : myMediaPlayers.keySet() ) {
				pauseMedia( key );
			}
		}

		if ( myIdToSoundPoolIdMap != null ) {
			for ( Long key : myIdToSoundPoolIdMap.keySet() ) {
				pauseMedia( key );
			}
		}
		// mySoundPool.autoPause(); // API level 8
	}
	
	public void resumeAll()
	{
		if ( myMediaPlayers != null ) {
			for ( Long key : myMediaPlayers.keySet() ) {
				resumeMedia( key );
			}
		}

		if ( myIdToSoundPoolIdMap != null ) {
			for ( Long key : myIdToSoundPoolIdMap.keySet() ) {
				resumeMedia( key );
			}
		}
		// mySoundPool.autoResume(); // API level 8
	}
	
	public void playVideo( final long id, final String path, final boolean mediaControlsEnabled )
	{
		final Context context = myContext;
		if (context == null) {
			return;
		}

		if (path == null || path.length() < 1) {
			return;
		}

		// Pause all playback.
		pauseAll();
		
		// Play the specified video on the main UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				Uri videoUri = createVideoURLFromString(path, context);

				// Display the video.
				if (null != videoUri) {
					android.content.Intent intent = new android.content.Intent(context, VideoActivity.class);
					intent.putExtra("video_uri", videoUri.toString());
					intent.putExtra("video_id", id);
					intent.putExtra("media_controls_enabled", mediaControlsEnabled);
					intent.setFlags(android.content.Intent.FLAG_ACTIVITY_NO_ANIMATION);
					context.startActivity(intent);
				}
			}
		});
	}
	
	public void setVolume( long id, float volume )
	{
		if ( volume < 0.0f )
			volume = 0.0f;
		if ( volume > 1.0f )
			volume = 1.0f;
		
		MediaPlayer mp = null;
		if ( myMediaPlayers != null ) {
			mp = myMediaPlayers.get(id);
		}

		if ( mp != null )
			mp.setVolume(volume, volume);

		myVolume = volume;
	}
	
	public float getVolume(long id)
	{
//		MediaPlayer mp = myMediaPlayers.get(id);

		// TODO: this is wrong
		return myVolume;
	}
	
	public AudioRecorder getAudioRecorder( long id )
	{
		if ( myAudioRecorder == null && myHandler != null) {
			myAudioRecorder = new AudioRecorder( myCoronaRuntime, myHandler );
		}
		myAudioRecorder.setId( id );
		return myAudioRecorder;
	}

	/**
	 * To be called when audio has been loaded/played.
	 * <p>
	 * Changes the Corona activity's volume controls to adjust the "media" stream instead of the
	 * default "ringer" stream when audio has been played at least once in the application.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 */
	static void onUsingAudio() {
		// Hold on to a local reference in case it is set null on the main UI thread.
		final CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			return;
		}

		// Do not continue if the activity's volume control stream has already been set.
		if (activity.getVolumeControlStream() != android.media.AudioManager.USE_DEFAULT_STREAM_TYPE) {
			return;
		}

		// Set up the volume control to use the "media" stream instead of the default "ringer" stream.
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (activity != null) {
					activity.setVolumeControlStream(android.media.AudioManager.STREAM_MUSIC);
				}
			}
		});
	}

	static public Uri createVideoURLFromString(String path, Context context) {
		Uri videoUri = null;
		String lowerCasePath = path.toLowerCase();

		if (URLUtil.isNetworkUrl(path) || 
			lowerCasePath.startsWith("rtsp:") ||
			lowerCasePath.startsWith("file:") || // Creating a video can create a file uri
			lowerCasePath.startsWith("content:") ) {
			
			// The URL is a web address or file. Take it as is.
			videoUri = Uri.parse(path);
		}
		else {
			if (context == null) {
				return null;
			}
			// The URL is likely a path to a local file.
			// Creates a URI with a "content://" scheme so that both asset files and external files are easily accessible.
			videoUri = com.ansca.corona.storage.FileContentProvider.createContentUriForFile(context, path);
		}
		return videoUri;
	}
}

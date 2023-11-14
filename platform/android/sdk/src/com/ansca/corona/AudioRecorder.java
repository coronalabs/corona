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
import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.Iterator;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.MediaRecorder.OnErrorListener;
import android.media.MediaRecorder.OnInfoListener;
import android.os.Handler;
import android.util.Log;

import com.ansca.corona.permissions.PermissionsServices;
import com.ansca.corona.permissions.PermissionsSettings;
import com.ansca.corona.permissions.PermissionState;

public class AudioRecorder {
	private AudioRecorderAbstract fRecorderImplementation;

	// Used to post objects to the ui thread
	private Handler fHandler;

	// If the instance is recording right now
	private boolean fIsRunning;

	// The id of the current instance
	private long fId;

	// If the current thread is notifying the Runtime
	private boolean fIsNotifying = false;

	// The path to save the current recording to
	private String fPath;

	private CoronaRuntime fCoronaRuntime;

	private AudioRecorder fInstance;

	private static final int STATUS_IS_RUNNING = -3;
	private static final int STATUS_NEED_PERMISSION = -2;
	private static final int STATUS_ERROR = -1;
	private static final int STATUS_OK = 0;

	/** Default handling of the microphone permission on Android 6+. */
	private class MicrophoneRequestPermissionsResultHandler 
		implements CoronaActivity.OnRequestPermissionsResultHandler {

		@Override
		public void onHandleRequestPermissionsResult(
				CoronaActivity activity, int requestCode, String[] permissions, int[] grantResults) {

			PermissionsSettings permissionsSettings = activity.unregisterRequestPermissionsResultHandler(this);

			// Advance the PermissionsSettings queue if possible!
			if (permissionsSettings != null) {
				permissionsSettings.markAsServiced();
			}

			if (hasPermission()) {
				fInstance.startRecording();
			}
			// Otherwise, we have nothing to do.
		}
	}

	AudioRecorder(CoronaRuntime runtime, Handler handler) {
		fHandler = handler;
		fCoronaRuntime = runtime;
		fInstance = this;
	}
	
	/**
	 * Checks if we can start recording
	 * @return Returns a status indicating whether we can start recording, or need to prepare the environment more.
	 */
	private int canStartRecording() {
		int recordingStatus = STATUS_OK;
		if (!hasMicrophone()) {
			recordingStatus = STATUS_ERROR;
		} else if (fIsRunning) {
			recordingStatus = STATUS_IS_RUNNING;
		} else {
			// Check on the status of the Microphone permission.
			PermissionsServices permissionsServices = new PermissionsServices(CoronaEnvironment.getApplicationContext());
			PermissionState recordAudioPermissionState = 
				permissionsServices.getPermissionStateFor(PermissionsServices.Permission.RECORD_AUDIO);
			switch(recordAudioPermissionState) {
				case MISSING:
					// The Corona developer forgot to add the permission to the AndroidManifest.xml.
					fCoronaRuntime.getController().showPermissionMissingFromManifestAlert(
						PermissionsServices.Permission.RECORD_AUDIO, "object:startRecording() needs Microphone access!");
					recordingStatus = STATUS_ERROR;
					break;
				case DENIED:
					if (!permissionsServices.shouldNeverAskAgain(PermissionsServices.Permission.RECORD_AUDIO)) {
						// Only possible on Android 6.
						recordingStatus = STATUS_NEED_PERMISSION;
					} else {
						recordingStatus = STATUS_ERROR;
					}
				default:
					// Permission is granted!
					break;
			}
		}

		// Let the C-side know that we aren't recording if an error occurs.
		if (recordingStatus == STATUS_ERROR) {
			fIsRunning = false;
			JavaToNativeShim.setAudioRecorderState(fCoronaRuntime, fId, fIsRunning);
		}

		return recordingStatus;
	}

	/**
	 * Starts the recorder
	 * @param path The location to save the file to when the recording stops.
	 * @return Returns whether recording has started or not.
	 */
	public boolean startRecording( String path ) {
		fPath = path;
		switch(canStartRecording()) {
			case STATUS_ERROR:
				postStatus(STATUS_ERROR);
				return false;
			case STATUS_NEED_PERMISSION:
				requestMicrophonePermission();
				return false;
			case STATUS_IS_RUNNING:
				return false;
			default:
				break;
		}

		startRecording();
		return true;
	}

	/**
	 * Starts the recorder for the last path passed to startRecording(path).
	 */
	private void startRecording() {
		fIsRunning = true;

		// If there is not save path then the file is kept in memory
		if (fPath == null || fPath.length() < 1) {
			fRecorderImplementation = new MemoryRecorder();
		}

		File outFile = new File(fPath);
		if (outFile.exists()) {
			outFile.delete();
		}

		// Record with 3gp only if they specify that ending.  This is for backwards compatibility and because the media.* api doesn't play wav files
		if (fPath.endsWith(".3gp")) {
			fRecorderImplementation = new ThreeGPRecorder(fPath);
		} else {
			fRecorderImplementation = new WavRecorder(fPath);
		}

		fRecorderImplementation.startRecording();

		// Let the C-side know that we're recording.
		JavaToNativeShim.setAudioRecorderState(fCoronaRuntime, fId, fIsRunning);
	}
	
	/**
	 * Stops recording and writes the data to a file.
	 */
	public void stopRecording() {
		fIsRunning = false;
		if (fRecorderImplementation != null) {
			fRecorderImplementation.stopRecording();
		}
	}

	/**
	 * Sets the id for the instance of the object.
	 * @param id The id to be set to
	 */
	public void setId(long id) {
		fId = id;
	}

	/**
	 * Checks if this application has permission to use the microphone.
	 * @return Returns true if this application has permission. Returns false if not.
	 */
	private static boolean hasPermission() {
		PermissionsServices permissionsServices = new PermissionsServices(CoronaEnvironment.getApplicationContext());
		return permissionsServices.getPermissionStateFor(PermissionsServices.Permission.RECORD_AUDIO) == PermissionState.GRANTED;
	}

	/**
	 * Brings up a dialog, asking the user for permission to access the microphone.
	 */
	private void requestMicrophonePermission() {

		// Validate.
		if (!hasMicrophone() || android.os.Build.VERSION.SDK_INT < 23) {
			return;
		}
		
		PermissionsServices permissionsServices = new PermissionsServices(CoronaEnvironment.getApplicationContext());
		
		// Create our Permissions Settings to compare against in the handler.
		PermissionsSettings settings = new PermissionsSettings(PermissionsServices.Permission.RECORD_AUDIO);

		// Request the RECORD_AUDIO permission.
		permissionsServices.requestPermissions(settings, new AudioRecorder.MicrophoneRequestPermissionsResultHandler());
	}

	/**
	 * Determines if this device has a microphone.
	 * @return Returns true if this device has a microphone. Returns false if not.
	 */
	private boolean hasMicrophone()
	{
		boolean hasSensor = true;
		android.content.pm.PackageManager packageManager = CoronaEnvironment.getApplicationContext().getPackageManager();
		if (!packageManager.hasSystemFeature("android.hardware.microphone"))
		{
			hasSensor = false;
			Log.i("Corona", "Device does not have a microphone");
		}
		return hasSensor;
	}

	AudioByteBufferHolder getNextBuffer() {
		if (fRecorderImplementation != null) {
			return fRecorderImplementation.getNextBuffer();
		}
		return null;
	}

	AudioByteBufferHolder getCurrentBuffer() {
		if (fRecorderImplementation != null) {
			return fRecorderImplementation.getCurrentBuffer();
		}
		return null;
	}

	void releaseCurrentBuffer() {
		if (fRecorderImplementation != null) {
			fRecorderImplementation.releaseCurrentBuffer();
		}
	}

	synchronized private void postStatus( final int status ) {
		if (fIsNotifying) {
			return;
		}
		fIsNotifying = true;
		fHandler.post(new Runnable() {
			public void run() {
				JavaToNativeShim.recordCallback(fCoronaRuntime, fId, status);
			}
		});
	}

	private abstract class AudioRecorderAbstract {
		abstract public void startRecording();
		abstract public void stopRecording();

		public AudioByteBufferHolder getNextBuffer() {
			return null;
		}

		public AudioByteBufferHolder getCurrentBuffer() {
			return null;
		}

		public void releaseCurrentBuffer() {

		}
	}

	private class WavRecorder extends AudioRecorderAbstract {
		// Settings for the AudioRecord
		private static final int RECORDER_CHANNELS = AudioFormat.CHANNEL_IN_MONO;
		private static final int RECORDER_AUDIO_ENCODING = AudioFormat.ENCODING_PCM_16BIT;
		private static final int RECORDER_BPP = 16;
		private static final int RECORDER_SAMPLERATE = 44100;

		// The thread that will read the data from fRecorder and write it to a buffer
		private Thread fRecordingThread = null;

		// Path of where the file should be saved
		String fPath;

		// Recorder object that will actually listen in to the microphone and then supply us with the data
		AudioRecord fRecorder;

		// Stream to write to file
		FileOutputStream fOutputStream;

		// Used to know how many bytes of audio data there are
		volatile long fTotalRead;
		
		volatile boolean fIsRecording;

		// This holds the minimum size of the buffer thats needed by AudioRecord
		int fBufferSize = 0;

		public WavRecorder(String path) {
			fPath = path;
			fBufferSize = AudioRecord.getMinBufferSize(RECORDER_SAMPLERATE, RECORDER_CHANNELS, RECORDER_AUDIO_ENCODING);
			fTotalRead = 0;
		}

		@Override
		public void startRecording() {
			fIsRecording = true;

			fRecorder = new AudioRecord(MediaRecorder.AudioSource.MIC, RECORDER_SAMPLERATE, RECORDER_CHANNELS,RECORDER_AUDIO_ENCODING, fBufferSize);
			fRecorder.startRecording();

			fRecordingThread = new Thread(new Runnable() {
				@Override
				public void run() {
					writeAudioDataToFile();
				}
			});
			fRecordingThread.start();
		}

		@Override
		public void stopRecording() {
			if (fRecorder != null) {
				fRecorder.stop();
				fRecorder.release();
			}

			if (fOutputStream != null) {
				try{
					// Make sure that the stream is closed and the new data is written with the information we have.
					synchronized(fOutputStream) {
						fOutputStream.close();
						overwriteData(fPath, fTotalRead);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}

			fRecorder = null;
			fRecordingThread = null;
			fOutputStream = null;
		}

		/**
		 * Writes the header for a wav file and then puts the data in after.  When all the data is writen it goes to change the header
		 * so that it contains the audio length and data length information.
		 */
		private void writeAudioDataToFile() {
			byte data[] = new byte[fBufferSize];
			try {
				fOutputStream = new FileOutputStream(fPath);

				int channels = 1;
				long byteRate = RECORDER_BPP * RECORDER_SAMPLERATE * channels/8;

				// Writes the header information for a wav file with fake data length and audio length.
				// Those values will be over written once we know the real values.
				writeWaveFileHeader(fOutputStream, 1, 2, RECORDER_SAMPLERATE, channels, byteRate);

				long read = 0;
				fTotalRead = 0;
				while(fIsRunning) {
					synchronized(fRecorder) {
						if (fRecorder.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING) {
							read = fRecorder.read(data, 0, fBufferSize);
						} else {
							read = AudioRecord.ERROR_INVALID_OPERATION;
						}
					}

					if (AudioRecord.ERROR_INVALID_OPERATION != read) {
						synchronized(fOutputStream) {
							fOutputStream.write(data);
							fTotalRead = fTotalRead + read;
						}
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				if (fOutputStream != null) {
					try {
						fOutputStream.close();
						fOutputStream = null;
						File halfDoneFile = new File(fPath);
						if (halfDoneFile.exists()) {
							halfDoneFile.delete();
						}
					} catch (Exception ex) {}
				}

			} finally {
				if (fOutputStream != null) {
					try {
						fOutputStream.close();
					} catch (Exception e) {}
				}
			}
			fIsRunning = false;
		}

		/**
		 * Overwrites the totalDataLen and totalAudioLen fields.  The totalDataLen is assumed to be totalAudioLen + 36
		 * @param totalAudioLen The number of bytes of the audio data.
		 */
		private void overwriteData(String path, long totalAudioLen) {
			RandomAccessFile randomAccessFile = null;
			try {
				randomAccessFile = new RandomAccessFile(path, "rws");
				// The 36 is the length of the header before data
				long totalDataLen = totalAudioLen + 36;
				randomAccessFile.seek(4);
				byte[] data = new byte[4];
				data[0] = (byte) (totalDataLen & 0xff);
				data[1] = (byte) ((totalDataLen >> 8) & 0xff);
				data[2] = (byte) ((totalDataLen >> 16) & 0xff);
				data[3] = (byte) ((totalDataLen >> 24) & 0xff);
				randomAccessFile.write(data);

				
				randomAccessFile.seek(40);
				data[0] = (byte) (totalAudioLen & 0xff);
				data[1] = (byte) ((totalAudioLen >> 8) & 0xff);
				data[2] = (byte) ((totalAudioLen >> 16) & 0xff);
				data[3] = (byte) ((totalAudioLen >> 24) & 0xff);
				randomAccessFile.write(data);
			} catch (Exception e) {
				e.printStackTrace();
				try {
					if (randomAccessFile != null) {
						File halfDoneFile = new File(path);
						if (halfDoneFile.exists()) {
							halfDoneFile.delete();
						}
					}
				} catch (Exception ex) {}
			} finally {
				if (randomAccessFile != null) {
					try {
						randomAccessFile.close();
					} catch (Exception e) {}
				}
			}
		}

		/**
		 * Writes the header file for a wav file.
		 * @param out The stream to write the header to
		 * @param totalAudioLen The number of bytes of the audio clip
		 * @param totalDataLen 
		 * @param longSampleRate The sample rate of the clip
		 * @param channels The number of channels of the clip
		 * @param byteRate The byte rate
		 */
		private void writeWaveFileHeader(
			FileOutputStream out, long totalAudioLen,
			long totalDataLen, long longSampleRate, int channels,
			long byteRate) throws IOException {

			byte[] header = new byte[44];

			header[0] = 'R'; // RIFF/WAVE header
			header[1] = 'I';
			header[2] = 'F';
			header[3] = 'F';
			header[4] = (byte) (totalDataLen & 0xff);
			header[5] = (byte) ((totalDataLen >> 8) & 0xff);
			header[6] = (byte) ((totalDataLen >> 16) & 0xff);
			header[7] = (byte) ((totalDataLen >> 24) & 0xff);
			header[8] = 'W';
			header[9] = 'A';
			header[10] = 'V';
			header[11] = 'E';
			header[12] = 'f'; // 'fmt ' chunk
			header[13] = 'm';
			header[14] = 't';
			header[15] = ' ';
			header[16] = 16; // 4 bytes: size of 'fmt ' chunk
			header[17] = 0;
			header[18] = 0;
			header[19] = 0;
			header[20] = 1; // format = 1
			header[21] = 0;
			header[22] = (byte) channels;
			header[23] = 0;
			header[24] = (byte) (longSampleRate & 0xff);
			header[25] = (byte) ((longSampleRate >> 8) & 0xff);
			header[26] = (byte) ((longSampleRate >> 16) & 0xff);
			header[27] = (byte) ((longSampleRate >> 24) & 0xff);
			header[28] = (byte) (byteRate & 0xff);
			header[29] = (byte) ((byteRate >> 8) & 0xff);
			header[30] = (byte) ((byteRate >> 16) & 0xff);
			header[31] = (byte) ((byteRate >> 24) & 0xff);
			header[32] = (byte) (1 * RECORDER_BPP / 8); // block align
			header[33] = 0;
			header[34] = RECORDER_BPP; // bits per sample
			header[35] = 0;
			header[36] = 'd';
			header[37] = 'a';
			header[38] = 't';
			header[39] = 'a';
			header[40] = (byte) (totalAudioLen & 0xff);
			header[41] = (byte) ((totalAudioLen >> 8) & 0xff);
			header[42] = (byte) ((totalAudioLen >> 16) & 0xff);
			header[43] = (byte) ((totalAudioLen >> 24) & 0xff);
			
			out.write(header, 0, 44);
		}

	}

	private class ThreeGPRecorder extends AudioRecorderAbstract {
		String fPath;
		MediaRecorder fMediaRecorder;

		public ThreeGPRecorder(String path) {
			fPath = path;
		}

		@Override
		public void startRecording() {
			File file = new File(fPath);
			File parentDir = file.getParentFile();
			if (!parentDir.exists())
			{
				parentDir.mkdirs();
			}

			fMediaRecorder = new MediaRecorder();
			fMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
			fMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
			fMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
			fMediaRecorder.setOutputFile(fPath);

			fMediaRecorder.setOnErrorListener(new OnErrorListener() {
					
				public void onError(MediaRecorder arg0, int arg1, int arg2) {
					System.out.println("MediaRecorder error " + arg1 + " " + arg2);
					stopRecording();
				}
			});
			
			fMediaRecorder.setOnInfoListener(new OnInfoListener() {
				
				public void onInfo(MediaRecorder arg0, int arg1, int arg2) {
					System.out.println("MediaRecorder info " + arg1 + " " + arg2);
				}
			});

			try {
				fMediaRecorder.prepare();
				fMediaRecorder.start();
			}
			catch (Exception e)
			{
				postStatus( STATUS_ERROR );
			}
		}

		@Override
		public void stopRecording() {
			fMediaRecorder.stop();
			fMediaRecorder.release();
			fMediaRecorder = null;
		}
	}

	private class MemoryRecorder extends AudioRecorderAbstract {
		AudioRecorderThread fRecorderThread;

		@Override
		public void startRecording() {
			fRecorderThread = new AudioRecorderThread();
			fRecorderThread.startRecording();
		}

		@Override
		public void stopRecording() {
			if (fRecorderThread != null) {
				fRecorderThread.stopRecording();
				fRecorderThread = null;
			}
		}

		AudioByteBufferHolder fCurrentBuffer;
	
		public AudioByteBufferHolder getNextBuffer()
		{
			if ( fRecorderThread != null )
				fCurrentBuffer = fRecorderThread.getNextBuffer();
			else
				fCurrentBuffer = null;
			return fCurrentBuffer;
		}

		public AudioByteBufferHolder getCurrentBuffer()
		{
			return fCurrentBuffer;
		}
		
		public synchronized void releaseCurrentBuffer()
		{
			if ( fCurrentBuffer != null ) {
				fRecorderThread.releaseBuffer( fCurrentBuffer );
				fCurrentBuffer = null;
			}
			fIsNotifying = false;
		}
	}

	public static class AudioByteBufferHolder 
	{
		ByteBuffer myBuffer;
		int myValidBytes = 0;
		
		AudioByteBufferHolder( int capacity )
		{
			myBuffer = ByteBuffer.allocateDirect(capacity);
		}
	}

	private class AudioRecorderThread extends Thread {
		private int myBufferSize;
		private float myFrameSeconds = 0.1f; // Refresh time in seconds

		private AudioRecord myAudioRecordInstance;
		private LinkedList<AudioByteBufferHolder> myBuffers = new LinkedList<AudioByteBufferHolder>();
		private LinkedList<AudioByteBufferHolder> myFreeBuffers = new LinkedList<AudioByteBufferHolder>();

		private static final int SAMPLE_RATE = 8000;
		private static final int ENCODING = AudioFormat.ENCODING_PCM_16BIT;

		AudioRecorderThread() {
		}
	
		void startRecording() {
			start();
		}
	
		void stopRecording() {
		}
	
		static final int MAX_BUFFERS = 5;
		
		public void run() {
			try {
				android.os.Process
						.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
	
				int minBufferSize = AudioRecord.getMinBufferSize( SAMPLE_RATE,
						fCoronaRuntime.getController().getAndroidVersionSpecific().audioChannelMono(),
						ENCODING);
				int bufferSize = 
					(int) (SAMPLE_RATE * 2 /* bytes per sample */ * myFrameSeconds);
				if ( bufferSize < minBufferSize )
					bufferSize = minBufferSize;
				myBufferSize = bufferSize;

				myAudioRecordInstance = new AudioRecord(MediaRecorder.AudioSource.MIC,
						SAMPLE_RATE, 
						fCoronaRuntime.getController().getAndroidVersionSpecific().audioChannelMono(),
						ENCODING, myBufferSize);
	
				myFreeBuffers.add( new AudioByteBufferHolder( myBufferSize ) );
				
				myAudioRecordInstance.startRecording();
	
				while (fIsRunning)
				{
					AudioByteBufferHolder readBuffer = null;
					
					synchronized( this ) {
						if ( !myFreeBuffers.isEmpty() )
							readBuffer = myFreeBuffers.remove();
						else if ( myBuffers.size() < MAX_BUFFERS )
							readBuffer = new AudioByteBufferHolder( myBufferSize );
					}

					if ( readBuffer != null ) {
						int bytesRead = myAudioRecordInstance.read(readBuffer.myBuffer, myBufferSize);
						
						synchronized( this ) {
							if ( bytesRead > 0 ) {
								readBuffer.myValidBytes = bytesRead;
							
								myBuffers.add( readBuffer );
								postStatus( bytesRead );
							}
							else
							{
								myFreeBuffers.add( readBuffer );
							}
						}
					}
					sleep( 15 );
				}
	
				myAudioRecordInstance.stop();

				postStatus( STATUS_OK );
			}
			catch (Exception e)
			{
				postStatus( STATUS_ERROR );
			}
		}

		AudioByteBufferHolder getNextBuffer()
		{
			AudioByteBufferHolder result = null;
			synchronized ( myBuffers ) {
				if ( !myBuffers.isEmpty() )
				{
					result = myBuffers.remove();
					return result;
				}
			}
			return null;
		}
		
		void releaseBuffer( AudioByteBufferHolder buffer ) {
			synchronized( myFreeBuffers ) {
				myFreeBuffers.add( buffer );
				
				if ( !fIsRunning )
					myFreeBuffers.clear();
			}
		}
	}
}

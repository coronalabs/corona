//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.Signature;
import java.security.spec.X509EncodedKeySpec;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.net.URL;
import java.net.MalformedURLException;

import android.app.Activity;
import android.content.ContentValues;
import android.content.ContentResolver;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.media.MediaScannerConnection;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.location.Location;
import android.provider.MediaStore;
import android.util.Base64;
import android.util.Log;
import android.view.DisplayCutout;

import dalvik.system.DexClassLoader;

import com.ansca.corona.AudioRecorder.AudioByteBufferHolder;
import com.ansca.corona.listeners.CoronaSplashScreenApiListener;
import com.ansca.corona.listeners.CoronaStatusBarApiListener;
import com.ansca.corona.listeners.CoronaStoreApiListener;
import com.ansca.corona.listeners.CoronaSystemApiListener;
import com.ansca.corona.maps.MapType;
import com.ansca.corona.permissions.PermissionsSettings;
import com.ansca.corona.permissions.PermissionsServices;
import com.ansca.corona.permissions.PermissionState;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.StatusLine;
import org.apache.http.client.HttpResponseException;
import org.apache.http.entity.BufferedHttpEntity;
import org.json.JSONArray;
import org.json.JSONObject;

import com.naef.jnlua.JavaFunction;
import com.naef.jnlua.LuaState;


public class NativeToJavaBridge {
	
	private android.content.Context myContext;

	NativeToJavaBridge( android.content.Context context )
	{
		myContext = context;
	}

	protected static boolean callRequestSystem(
		CoronaRuntime runtime, long luaStateMemoryAddress, String actionName, int luaStackIndex)
	{
		CoronaSystemApiListener listener = runtime.getController().getCoronaSystemApiListener();
		if (listener != null) {
			return listener.requestSystem(runtime, actionName, luaStateMemoryAddress, luaStackIndex);
		}
		return false;
	}
	
	/**
	 * A test routine, to see if the bridge can be crossed.
	 */
	protected static void ping()
	{
		System.out.println( "NativeToJavaBridge.ping()" );
	}
	
	private static HashMap< String, JavaFunction > sPluginCache = new HashMap< String, JavaFunction >();

	protected static int instantiateClass( LuaState L, CoronaRuntime runtime, Class<?> c ) {
		int result = 0;
		try {
			if ( JavaFunction.class.isAssignableFrom( c ) ) {
				JavaFunction f = sPluginCache.get( c.getName() );
				// Cache miss
				if ( null == f ) {
					Object o = c.newInstance();

					if ( CoronaRuntimeListener.class.isAssignableFrom( c ) ) {

						CoronaRuntimeListener listener = (CoronaRuntimeListener)o;

						CoronaEnvironment.addRuntimeListener( listener );

						listener.onLoaded( runtime );
					}

					f = (JavaFunction)o;

					// Cache the plugin
					sPluginCache.put( c.getName(), f );
				}

				L.pushJavaFunction( f );
				result = 1;
			}
		} catch ( Exception ex ) {
			Log.i( "Corona", "ERROR: Could not instantiate class (" + c.getName() + "): " + ex.getLocalizedMessage() );
			ex.printStackTrace();
		}

		return result;
	}

	private static DexClassLoader sClassLoader = null;

	protected static int callLoadClass(
		CoronaRuntime runtime, long luaStateMemoryAddress, String libName, String className )
	{
		int result = 0;
		StringBuilder err = new StringBuilder();
		if (runtime != null) {
			// Fetch the runtime's Lua state.
			// TODO: We need to account for coroutines.
			LuaState L = runtime.getLuaState();
			if ( null == L ) {
				L = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
			}

			// DEBUG: Temporarily set to 'true' for helpful logging messages
			final boolean verbose = true;

			// Look for the class via reflection.
			final String classPath = libName + "." + className;
			try {
				if ( verbose ) { Log.v( "Corona", "> Class.forName: " + classPath ); }

				Class<?> c = Class.forName( classPath );
				if ( verbose ) { Log.v( "Corona", "< Class.forName: " + classPath ); }
				result = instantiateClass( L, runtime, c );

				if ( verbose ) { Log.v( "Corona", "Loading via reflection: " + classPath ); }
			}
			catch ( Exception ex ) {
				err.append("\n\tno Java class '").append(classPath).append("'");
			}
			catch ( Throwable ex ) {
				ex.printStackTrace();
				err.append("\n\terror loading class '").append(classPath).append("': ").append(ex);
			}
			if(result == 0) {
				L.pushString(err.toString());
				result = 1;
			}
		}
		return result;
	}

	/**
	 * This function is used by Corona Kit to load a file when you require("something").  In Corona the file is loaded
	 * automatically from the archive.  There is no archive file in Corona Kit.
	 */
	protected static int callLoadFile(CoronaRuntime runtime, long luaStateMemoryAddress, String fileName)
	{
		int result = 0;
		android.content.Context context = CoronaEnvironment.getApplicationContext();

		if (context == null) {
			return result;
		}

		if (runtime != null) {
			com.ansca.corona.storage.FileServices fileServices;
			fileServices = new com.ansca.corona.storage.FileServices(context);

			fileName = fileName.replace('.', '/');

			// Needs to append the file extension because you call require("something") with out the extension
			// but to load the file you need the full name of the file
			String filePath = runtime.getPath() + fileName + ".lua";

			java.io.File pathToOpen = new File(filePath);
			if(!fileServices.doesAssetFileExist(filePath)) {
				return result;
			}

			LuaState L = runtime.getLuaState();
			if ( null == L ) {
				L = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
			}

			java.io.InputStream inputStream = null;

			try {
				inputStream = fileServices.openFile(pathToOpen);
				L.load(inputStream, fileName);
				result = 1;
			} catch (Exception e) {
				Log.i( "Corona", "WARNING: Could not load '" + fileName + "'" );
			} finally {
				if (inputStream != null) {
					try {
						inputStream.close();
					} catch (Exception e) {}
					
				}
			}
		}

		return result;
	}

	protected static String callGetExceptionStackTraceFrom(Throwable ex)
	{
		// Validate.
		if (ex == null) {
			return null;
		}

		// Return the given exception's message and stack trace as a single string.
		com.naef.jnlua.LuaError luaError = new com.naef.jnlua.LuaError(null, ex);
		return luaError.toString();
	}
	
	protected static String getAudioOutputSettings()
	{
		if (android.os.Build.VERSION.SDK_INT >= 17) // getProperty
		{
			android.content.Context context = CoronaEnvironment.getApplicationContext();
			if (context != null)
			{
				AudioManager am = (AudioManager) context.getSystemService(android.content.Context.AUDIO_SERVICE);
				if (am != null)
				{
					String sampleRate = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
					String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
					return sampleRate + "," + framesPerBuffer;
				}
			}
		}
		return "";
	}

	protected static void callOnRuntimeLoaded(long luaStateMemoryAddress, CoronaRuntime runtime)
	{
		if (runtime != null) {
			runtime.onLoaded(luaStateMemoryAddress);
		}
	}
	
	protected static void callOnRuntimeWillLoadMain(CoronaRuntime runtime)
	{
		if (runtime != null) {
			runtime.onWillLoadMain();
		}
	}

	protected static void callOnRuntimeStarted(CoronaRuntime runtime)
	{
		if (runtime != null) {
			runtime.onStarted();
		}
	}
	
	protected static void callOnRuntimeSuspended(CoronaRuntime runtime)
	{
		if (runtime != null) {
			runtime.onSuspended();
		}
	}
	
	protected static void callOnRuntimeResumed(CoronaRuntime runtime)
	{
		if (runtime != null) {
			runtime.onResumed();
		}
	}
	
	protected static void callOnAudioEnabled() {
		MediaManager.onUsingAudio();
	}
	
	protected static int callInvokeLuaErrorHandler(long luaStateMemoryAddress)
	{
		return CoronaEnvironment.invokeLuaErrorHandler(luaStateMemoryAddress);
	}

	protected static void callPushLaunchArgumentsToLuaTable(CoronaRuntime runtime, long luaStateMemoryAddress)
	{
		CoronaSystemApiListener listener = runtime.getController().getCoronaSystemApiListener();
		if (listener != null) {
			pushArgumentsToLuaTable(runtime, luaStateMemoryAddress, listener.getInitialIntent());
		}
	}

	protected static void callPushApplicationOpenArgumentsToLuaTable(CoronaRuntime runtime, long luaStateMemoryAddress)
	{
		CoronaSystemApiListener listener = runtime.getController().getCoronaSystemApiListener();
		if (listener != null) {
			pushArgumentsToLuaTable(runtime, luaStateMemoryAddress, listener.getIntent());
		}
		
	}

	private static void pushArgumentsToLuaTable(CoronaRuntime runtime, long luaStateMemoryAddress, android.content.Intent intent)
	{
		// Validate arguments.
		if ((luaStateMemoryAddress == 0) || (intent == null)) {
			return;
		}

		// Fetch a Lua state object for the given memory address.
		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}
		// Using coroutines will give a different lua state than what the runtime has so this is to verify its the same one
		if (luaState == null || CoronaRuntimeProvider.getLuaStateMemoryAddress(luaState) != luaStateMemoryAddress) {
			luaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
		}

		// Do not continue if there is no Lua table at the top of the Lua stack.
		int rootLuaTableStackIndex = luaState.getTop();
		if (luaState.isTable(rootLuaTableStackIndex) == false) {
			return;
		}

		// Push the intent's URL to the Lua table.
		android.net.Uri uri = intent.getData();
		luaState.pushString(((uri != null) ? uri.toString() : ""));
		luaState.setField(rootLuaTableStackIndex, "url");

		// Push an "androidIntent" Lua table to the given Lua table.
		luaState.newTable();
		{
			// Get the index to the intent Lua table.
			int intentLuaTableStackIndex = luaState.getTop();

			// Push the intent's URL to the Lua table.
			luaState.pushString(((uri != null) ? uri.toString() : ""));
			luaState.setField(intentLuaTableStackIndex, "url");

			// Push the intent's action string to the Lua table.
			String action = intent.getAction();
			luaState.pushString(((action != null) ? action : ""));
			luaState.setField(intentLuaTableStackIndex, "action");

			// Push the intent's category strings to the Lua table.
			boolean wasPushed = pushToLua(luaState, intent.getCategories());
			if (wasPushed == false) {
				luaState.newTable();
			}
			luaState.setField(intentLuaTableStackIndex, "categories");

			// Push the intent's extras to the Lua table.
			luaState.newTable();
			{
				int extrasLuaTableStackIndex = luaState.getTop();
				android.os.Bundle bundle = intent.getExtras();
				if ((bundle != null) && (bundle.size() > 0)) {
					for (String key : bundle.keySet()) {
						wasPushed = pushToLua(luaState, bundle.get(key));
						if (wasPushed) {
							luaState.setField(extrasLuaTableStackIndex, key);
						}
					}
				}
			}
			luaState.setField(intentLuaTableStackIndex, "extras");
		}
		luaState.setField(rootLuaTableStackIndex, "androidIntent");

		// If there is a notification bundle in the intent, then push it to the root Lua table.
		// This is to be consistent with how Corona for iOS provides notification data.
		String notificationEventName = com.ansca.corona.events.NotificationReceivedTask.NAME;
		boolean wasPushed = pushToLua(luaState, intent.getBundleExtra(notificationEventName));
		if (wasPushed) {
			luaState.setField(rootLuaTableStackIndex, notificationEventName);
		}
	}

	private static boolean pushToLua(com.naef.jnlua.LuaState luaState, Object value)
	{
		// Validate arguments.
		if ((luaState == null) || (value == null)) {
			return false;
		}

		// Convert the given Java object to its equivalent Lua type and push it to the Lua stack.
		if (value instanceof Boolean) {
			luaState.pushBoolean(((Boolean)value).booleanValue());
		}
		else if ((value instanceof Float) || (value instanceof Double)) {
			luaState.pushNumber(((Number)value).doubleValue());
		}
		else if (value instanceof Number) {
			luaState.pushInteger(((Number)value).intValue());
		}
		else if (value instanceof Character) {
			luaState.pushString(value.toString());
		}
		else if (value instanceof String) {
			luaState.pushString((String)value);
		}
		else if (value instanceof java.io.File) {
			luaState.pushString(((java.io.File)value).getAbsolutePath());
		}
		else if (value instanceof android.net.Uri) {
			luaState.pushString(value.toString());
		}
		else if (value instanceof CoronaData) {
			((CoronaData)value).pushTo(luaState);
		}
		else if (value instanceof android.os.Bundle) {
			android.os.Bundle bundle = (android.os.Bundle)value;
			if (bundle.size() > 0) {
				luaState.newTable(0, bundle.size());
				int luaTableStackIndex = luaState.getTop();
				for (String key : bundle.keySet()) {
					boolean wasPushed = pushToLua(luaState, bundle.get(key));
					if (wasPushed) {
						luaState.setField(luaTableStackIndex, key);
					}
				}
			}
			else {
				luaState.newTable();
			}
		}
		else if (value.getClass().isArray()) {
			int arrayLength = java.lang.reflect.Array.getLength(value);
			if (arrayLength > 0) {
				luaState.newTable(arrayLength, 0);
				int luaTableStackIndex = luaState.getTop();
				for (int arrayIndex = 0; arrayIndex < arrayLength; arrayIndex++) {
					boolean wasPushed = pushToLua(luaState, java.lang.reflect.Array.get(value, arrayIndex));
					if (wasPushed) {
						luaState.rawSet(luaTableStackIndex, arrayIndex + 1);
					}
				}
			}
			else {
				luaState.newTable();
			}
		}
		else if (value instanceof java.util.Map) {
			java.util.Map map = (java.util.Map)value;
			if (map.size() > 0) {
				luaState.newTable(0, map.size());
				int luaTableStackIndex = luaState.getTop();
				for (java.util.Map.Entry entry : (java.util.Set<java.util.Map.Entry>)map.entrySet()) {
					if ((entry.getKey() instanceof String) || (entry.getKey() instanceof Number)) {
						boolean wasPushed = pushToLua(luaState, entry.getValue());
						if (wasPushed) {
							luaState.setField(luaTableStackIndex, entry.getKey().toString());
						}
					}
				}
			}
			else {
				luaState.newTable();
			}
		}
		else if (value instanceof Iterable) {
			luaState.newTable();
			int luaTableStackIndex = luaState.getTop();
			int arrayIndex = 0;
			for (Object collectionValue : (Iterable)value) {
				boolean wasPushed = pushToLua(luaState, collectionValue);
				if (wasPushed) {
					luaState.rawSet(luaTableStackIndex, arrayIndex + 1);
				}
				arrayIndex++;
			}
		}
		else {
			return false;
		}

		// Return true to indicate that we successfully pushed the value into Lua.
		return true;
	}
	
	/**
	 * Test to see if the named asset exists
	 * 
	 * @param assetName		Asset to find
	 * @return				True if it exists, false otherwise
	 */
	private boolean getRawAssetExists( String assetName )
	{
		// Validate.
		if ((assetName == null) || (assetName.length() <= 0)) {
			return false;
		}

		// Attempt to fetch the given asset name's URL scheme, if it has one.
		android.net.Uri uri = android.net.Uri.parse(android.net.Uri.encode(assetName, ":/\\."));
		String scheme = uri.getScheme();
		if (scheme == null) {
			scheme = "";
		}
		scheme = scheme.toLowerCase();

		// Determine if the given asset name exists.
		boolean wasAssetFound = false;
		if (scheme.equals("android.app.icon")) {
			// The asset is an application icon.
			try {
				String packageName = uri.getHost();
				if ((packageName == null) || (packageName.length() <= 0)) {
					wasAssetFound = true;
				}
				else {
					android.content.pm.PackageInfo packageInfo =
							CoronaEnvironment.getApplicationContext().getPackageManager().getPackageInfo(packageName, 0);
					wasAssetFound = (packageInfo != null);
				}
			}
			catch (Exception ex) { }
		}
		else {
			// The asset is likely a file within the APK or Google Play expansion file.
			android.content.Context context = CoronaEnvironment.getApplicationContext();
			if (context != null) {
				com.ansca.corona.storage.FileServices fileServices;
				fileServices = new com.ansca.corona.storage.FileServices(context);
				wasAssetFound =  fileServices.doesAssetFileExist(assetName);
			}

			// It doesn't make sense to issue a log message when testing for existence (not existing
			// may be ok in the context of the caller)
			// Log a warning if the asset was not found.
			// if (wasAssetFound == false) {
			// 	Log.i("Corona", "WARNING: Asset file \"" + assetName + "\" does not exist.");
			// }
		}

		// Return the result.
		return wasAssetFound;
	}

	protected static boolean callGetRawAssetExists( CoronaRuntime runtime, String assetName )
	{
		return runtime.getController().getBridge().getRawAssetExists(runtime.getPath() + assetName);
	}

	protected static boolean callGetCoronaResourceFileExists( CoronaRuntime runtime, String assetName )
	{
		boolean exists = false;
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) {
			java.io.File destinationFile = 
				new java.io.File(context.getFileStreamPath("coronaResources"), runtime.getPath() + assetName);
			exists = destinationFile.exists();
		}
		
		return exists;
	}

	protected static boolean callGetAssetFileLocation(String filePath, long zipFileEntryMemoryAddress) {
		// Fetch the application context.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return false;
		}

		// Fetch the asset's location.
		com.ansca.corona.storage.FileServices fileServices;
		com.ansca.corona.storage.AssetFileLocationInfo assetFileLocationInfo;
		fileServices = new com.ansca.corona.storage.FileServices(context);
		assetFileLocationInfo = fileServices.getAssetFileLocation(filePath);
		if (assetFileLocationInfo == null) {
			return false;
		}

		// Write the asset's location info to the struct referenced by the given memory address.
		JavaToNativeShim.setZipFileEntryInfo(zipFileEntryMemoryAddress, assetFileLocationInfo);

		// Return true to indicate that the asset was found.
		return true;
	}
	
	protected static byte[] callGetBytesFromFile( String filePathName ) {
		// Fetch the application context.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return null;
		}

		// Extract all bytes from the given file and return them.
		com.ansca.corona.storage.FileServices fileServices;
		fileServices = new com.ansca.corona.storage.FileServices(context);
		return fileServices.getBytesFromFile(filePathName);
	}

	protected static String callExternalizeResource( String assetName, CoronaRuntime runtime ) {
		// Fetch the application context.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return null;
		}
		// Extract the given asset file.
		com.ansca.corona.storage.FileServices fileServices = new com.ansca.corona.storage.FileServices(context);
		java.io.File destinationFile = fileServices.extractAssetFile(runtime.getPath() + assetName);
		if (destinationFile == null) {
			return null;
		}
		// Return the path to where the extracted file was written to.
		return destinationFile.getAbsolutePath();
	}

	/**
	 * Fetches the given image file's dimensions and mime type without loading the file.
	 * @param imageFileName The path\file name of the image to fetch information from.
	 * @return Returns an object providing image information via parameters: outWidth, outHeight, outMimeType
	 *         Returns null if failed to load the given image file.
	 */
	private android.graphics.BitmapFactory.Options getBitmapFileInfo(String imageFileName) {
		BitmapFactory.Options options = null;
		
		// Validate.
		if ((imageFileName == null) || (imageFileName.length() <= 0)) {
			return null;
		}

		// Attempt to fetch the given image file's information.
		InputStream stream = null;
		try {
			// Get a stream to the given file. A path that does not start with a '/' is assumed to be an asset file.
			com.ansca.corona.storage.FileServices fileServices;
			fileServices = new com.ansca.corona.storage.FileServices(myContext);
			stream = fileServices.openFile(imageFileName);
			if (stream != null) {
				// Fetch the image file's information.
				options = new BitmapFactory.Options();
				options.inJustDecodeBounds = true;
				BitmapFactory.decodeStream(stream, null, options);
				
				// The option object's dimensions will be -1 if we have failed to load the file.
				// In this case, we want to return null from this function.
				if (options.outWidth < 0) {
					options = null;
				}
			}
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		finally {
			if (stream != null) {
				try { stream.close(); }
				catch (Exception ex) { }
			}
		}
		
		// Return the result. Will be null if failed to load the image.
		return options;
	}

	private int getImageExifRotationInDegreesFrom(String imageFileName) {
		// Validate argument.
		if ((imageFileName == null) || (imageFileName.length() <= 0)) {
			return 0;
		}

		// Do not continue if the given file is an asset.
		// The ExifReader class cannot access an asset within a file.
		com.ansca.corona.storage.FileServices fileServices;
		fileServices = new com.ansca.corona.storage.FileServices(CoronaEnvironment.getApplicationContext());
		if (fileServices.isAssetFile(imageFileName)) {
			return 0;
		}

		// Attempt to fetch the EXIF orientation from the given image file.
		int rotationInDegrees = 0;
		try {
			android.media.ExifInterface exifReader = new android.media.ExifInterface(imageFileName);
			int exifOrientation = exifReader.getAttributeInt(
							android.media.ExifInterface.TAG_ORIENTATION,
							android.media.ExifInterface.ORIENTATION_NORMAL);
			switch (exifOrientation) {
				case android.media.ExifInterface.ORIENTATION_ROTATE_90:
					rotationInDegrees = 90;
					break;
				case android.media.ExifInterface.ORIENTATION_ROTATE_180:
					rotationInDegrees = 180;
					break;
				case android.media.ExifInterface.ORIENTATION_ROTATE_270:
					rotationInDegrees = 270;
					break;
			}
		}
		catch (Exception ex) { }
		return rotationInDegrees;
	}
	
	/**
	 * Instances of this class are returned from the loadBitmap() method to indicate if it successfully
	 * loaded a bitmap, and if so, provide information about that loaded bitmap.
	 */
	private static class LoadBitmapResult {
		private Bitmap fBitmap;
		private int fWidth;
		private int fHeight;
		private float fScaleFactor;
		private int fRotationInDegrees;
		
		public LoadBitmapResult(Bitmap bitmap, float scaleFactor, int rotationInDegrees) {
			fBitmap = bitmap;
			fWidth = 0;
			fHeight = 0;
			fScaleFactor = scaleFactor;
			fRotationInDegrees = rotationInDegrees;
		}
		
		public LoadBitmapResult(int width, int height, float scaleFactor, int rotationInDegrees) {
			fBitmap = null;
			fWidth = width;
			fHeight = height;
			fScaleFactor = scaleFactor;
			fRotationInDegrees = rotationInDegrees;
		}
		
		public boolean wasSuccessful() {
			return (fBitmap != null);
		}
		
		public Bitmap getBitmap() {
			return fBitmap;
		}

		public int getWidth() {
			if (fBitmap != null) {
				return fBitmap.getWidth();
			}
			return fWidth;
		}

		public int getHeight() {
			if (fBitmap != null) {
				return fBitmap.getHeight();
			}
			return fHeight;
		}
		
		public float getScaleFactor() {
			return fScaleFactor;
		}

		public int getRotationInDegrees() {
			return fRotationInDegrees;
		}
	}
	
	private LoadBitmapResult loadBitmap(String filePath, int maxWidth, int maxHeight, boolean loadImageInfoOnly) {
		Bitmap result = null;
		
		// Attempt to fetch the image file's dimensions first.
		BitmapFactory.Options options = getBitmapFileInfo(filePath);
		if (options == null) {
			// File not found. Do not continue.
			return null;
		}
		int originalWidth = options.outWidth;
		int originalHeight = options.outHeight;
		
		// Get a stream to the given file.
		// A path that does not start with a '/' is assumed to be an asset file.
		com.ansca.corona.storage.FileServices fileServices;
		fileServices = new com.ansca.corona.storage.FileServices(myContext);
		boolean isAssetFile = fileServices.isAssetFile(filePath);
		InputStream stream = fileServices.openFile(filePath);
		if (stream == null) {
			return null;
		}

		// Fetch the image file's EXIF orientation, if available.
		// Skip asset files since the ExifInterface class can only open external files.
		// Note: EXIF is optional metadata assigned to image files. It is commonly set by cameras.
		int rotationInDegrees = 0;
		if (!isAssetFile) {
			rotationInDegrees = getImageExifRotationInDegreesFrom(filePath);
		}
		
		// If a maximum pixel width and height has been provided (ie: a max value greater than zero),
		// then use the smallest of the maximum lengths to downsample the image with below.
		int maxImageLength = 0;
		if ((maxWidth > 0) && (maxHeight > 0)) {
			maxImageLength = (maxWidth < maxHeight) ? maxWidth : maxHeight;
		}
		else if (maxWidth > 0) {
			maxImageLength = maxWidth;
		}
		else if (maxHeight > 0) {
			maxImageLength = maxHeight;
		}

		// Downsample the image if it is larger than the provided maximum size provided.
		// Note: Downsampling involves skipping every other decoded pixel, which downscales
		//       the image in powers of 2 in a fast and memory efficient manner.
		options = new BitmapFactory.Options();
		options.inSampleSize = 1;
		if (maxImageLength > 0) {
			float percentLength = (float)java.lang.Math.max(originalWidth, originalHeight) / (float)maxImageLength;
			while (percentLength > 1.0f) {
				options.inSampleSize *= 2;
				percentLength /= 2.0f;
			}
			if (options.inSampleSize > 1) {
				Log.v("Corona", "Downsampling image file '" + filePath +
								   "' to fit max pixel size of " + Integer.toString(maxImageLength) + ".");
			}
		}
		
		// Check if we have enough memory to load the image at full 32-bit color quality.
		// If we don't have enough memory, then load the image at 16-bit quality.
		// Note: We can't determine the memory usage of this app, so we have to guess based on the max JVM heap size.
		long imagePixelCount = (originalWidth * originalHeight) / options.inSampleSize;
		long memoryUsage = imagePixelCount * 4;		// Calculate the image memory size of at a 32-bit color quality.
		memoryUsage += 2000000;						// Add about 2 MB for other things this app may be using memory for.
													// Note: This is a guess. We don't know how much memory the app is using.
		options.inPreferredConfig = Bitmap.Config.ARGB_8888;
		if (memoryUsage > Runtime.getRuntime().maxMemory()) {
			Log.v("Corona", "Not enough memory to load file '" + filePath +
										"' as a 32-bit image. Reducing the image quality to 16-bit.");
			options.inPreferredConfig = Bitmap.Config.RGB_565;
		}

		// Do not continue if the caller only wants image information and not the decoded pixels.
		if (loadImageInfoOnly) {
			// Predict what the downsampled width, height, and scale will be if applicable.
			int expectedWidth = originalWidth;
			int expectedHeight = originalHeight;
			float expectedScale = 1.0f;
			if (options.inSampleSize > 1) {
				expectedWidth = originalWidth / options.inSampleSize;
				if ((originalWidth % options.inSampleSize) > 0) {
					expectedWidth++;
				}
				expectedHeight = originalHeight / options.inSampleSize;
				if ((originalHeight % options.inSampleSize) > 0) {
					expectedHeight++;
				}
				expectedScale = (float)expectedWidth / (float)originalWidth;
			}

			// Return the image file's information.
			return new LoadBitmapResult(expectedWidth, expectedHeight, expectedScale, rotationInDegrees);
		}
		
		// Load the image file to an uncompressed bitmap in memory.
		Bitmap bitmap = null;
		try {
			bitmap = BitmapFactory.decodeStream(stream, null, options);
		}
		catch (OutOfMemoryError memoryException) {
			try {
				// There was not enough memory to load the image file. If we were loading it as a 32-bit color bitmap,
				// then attempt to load the image again with a lower 16-bit color quality.
				if (options.inPreferredConfig == Bitmap.Config.ARGB_8888) {
					Log.v("Corona", "Failed to load file '" + filePath +
										"' as a 32-bit image. Reducing the image quality to 16-bit.");
					System.gc();
					options.inPreferredConfig = Bitmap.Config.RGB_565;
					bitmap = BitmapFactory.decodeStream(stream, null, options);
				}
				else {
					throw memoryException;
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		try { stream.close(); }
		catch (Exception ex) {
			ex.printStackTrace();
		}
		if (bitmap == null) {
			Log.v("Corona", "Unable to decode file '" + filePath + "'");
			return null;
		}
		
		// If the image was downsampled, then calculate the scaling factor.
		float downsampleScale = 1.0f;
		if (bitmap.getWidth() != originalWidth) {
			downsampleScale = (float)bitmap.getWidth() / (float)originalWidth;
		}
		
		// Image file was loaded successfully. Return the bitmap in a result object.
		return new LoadBitmapResult(bitmap, downsampleScale, rotationInDegrees);
	}

	private static boolean callLoadBitmap(
		CoronaRuntime runtime, String filePath, long nativeImageMemoryAddress, boolean convertToGrayscale,
		int maxWidth, int maxHeight, boolean loadImageInfoOnly)
	{
		// Validate.
		if ((filePath == null) || (filePath.length() <= 0) || (nativeImageMemoryAddress == 0 || runtime == null)) {
			return false;
		}

		// When display.newImage is called with an image in the temp directory then an absolute path is passed in
		// when the image is in the resource path its not an absolute path and should be relative to the asset path
		// This assumes that if the files exists then its an absolute path
		File f = new File(filePath);
		if (!f.exists()) {
			filePath = runtime.getPath() + filePath;
		}
		
		// Attempt to fetch the given path's URL scheme, if it has one.
		android.net.Uri uri = android.net.Uri.parse(android.net.Uri.encode(filePath, ":/\\."));
		String scheme = uri.getScheme();
		if (scheme == null) {
			scheme = "";
		}
		scheme = scheme.toLowerCase();

		// Load the specified image file.
		LoadBitmapResult result = null;
		// boolean canRecycleBitmap = false;
		if (scheme.equals("android.app.icon")) {
			// Fetch the application icon's bitmap.
			try {
				android.content.Context context = CoronaEnvironment.getApplicationContext();
				android.content.pm.ApplicationInfo applicationInfo = context.getApplicationInfo();
				android.content.pm.PackageManager packageManager = context.getPackageManager();
				String packageName = uri.getHost();
				if ((packageName == null) || (packageName.length() <= 0)) {
					packageName = applicationInfo.packageName;
				}
				android.graphics.drawable.Drawable drawable = packageManager.getApplicationIcon(packageName);
				if (drawable instanceof android.graphics.drawable.BitmapDrawable) {
					android.graphics.drawable.BitmapDrawable bitmapDrawable;
					bitmapDrawable = (android.graphics.drawable.BitmapDrawable)drawable;
					if (bitmapDrawable.getBitmap() != null) {
						if (loadImageInfoOnly) {
							result = new LoadBitmapResult(
											bitmapDrawable.getBitmap().getWidth(),
											bitmapDrawable.getBitmap().getHeight(),
											1.0f, 0);
						}
						else {
							result = new LoadBitmapResult(bitmapDrawable.getBitmap(), 1.0f, 0);
						}
					}
					// canRecycleBitmap = false;
				}
				if (result == null) {
					int w = Math.max(drawable.getIntrinsicWidth(), 1);
					int h = Math.max(drawable.getIntrinsicHeight(), 1);
					if (loadImageInfoOnly) {
						result = new LoadBitmapResult(w, h, 1.0f, 0);
					} else {
						final Bitmap bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
						final Canvas canvas = new Canvas(bmp);
						drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
						drawable.draw(canvas);
						result = new LoadBitmapResult(bmp, 1.0f, 0);
					}
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
		else {
			// Fetch the bitmap from file.
			result = runtime.getController().getBridge().loadBitmap(filePath, maxWidth, maxHeight, loadImageInfoOnly);
			// canRecycleBitmap = true;
		}

		if (result == null) {
			return false;
		}
		android.graphics.Bitmap bitmap = result.getBitmap();

		// Copy the image's data to the native C/C++ image object.
		boolean wasCopied = false;
		if (loadImageInfoOnly) {
			// Only copy the image's width, height, scale, and rotation.
			wasCopied = JavaToNativeShim.copyBitmapInfo( runtime,
								nativeImageMemoryAddress, result.getWidth(), result.getHeight(),
								result.getScaleFactor(), result.getRotationInDegrees());
		}
		else if (bitmap != null) {
			// Copy all of the image's data to the native side, which includes its pixels.
			if (bitmap.getConfig() == null) {
				// The bitmap is using an unknown pixel. This typically happens with GIF files.
				// Convert the bitmap to a known pixel format that the C/C++ side can copy correctly.
				android.graphics.Bitmap convertedBitmap = null;
				try {
					if (convertToGrayscale) {
						convertedBitmap = bitmap.copy(android.graphics.Bitmap.Config.ALPHA_8, false);
					}
					else {
						convertedBitmap = bitmap.copy(android.graphics.Bitmap.Config.ARGB_8888, false);
					}
				}
				catch (Exception ex) {
					ex.printStackTrace();
				}
				// if (canRecycleBitmap) {
				// 	bitmap.recycle();
				// }
				bitmap = convertedBitmap;
				// canRecycleBitmap = true;
				if (bitmap == null) {
					return false;
				}
			}
			wasCopied = JavaToNativeShim.copyBitmap( runtime,
								nativeImageMemoryAddress, bitmap, result.getScaleFactor(),
								result.getRotationInDegrees(), convertToGrayscale);
		}

		// Free the memory used by the bitmap.
		// if (canRecycleBitmap && (bitmap != null)) {
		// 	bitmap.recycle();
		// }

		// Returns true if the image's was successfully loaded and copied to the native C/C++ image object.
		return wasCopied;
	}

	/**
	 * Generates a unique file name in the shared pictures directory on external storage.
	 * <p>
	 * Note that this method does not create a file in the pictures directory. This method only generates
	 * a unique name that the caller can use to create a new file.
	 * @param fileExtensionName The extension string to be appended to the file name.
	 *                          <p>
	 *                          If given null or empty string, then this method will generate a unique
	 *                          file name without an extension.
	 * @return Returns a file object containing a path and file name that is unique in the pictures directory.
	 *         <p>
	 *         Returns null if failed to find the pictures directory or unable to generate a unique file name.
	 */
	protected static java.io.File createUniqueFileNameInPicturesDirectory(String fileExtensionName) {
		// Make sure the given extension string is valid and is prefixed with a period.
		// Note: This method allows files without extensions.
		if (fileExtensionName == null) {
			fileExtensionName = "";
		}
		else if ((fileExtensionName.length() > 0) && (fileExtensionName.startsWith(".") == false)) {
			fileExtensionName = "." + fileExtensionName;
		}

		// Fetch the default pictures directory on external storage.
		// Create the directory if it does not already exist.
		java.io.File directory = android.os.Environment.getExternalStoragePublicDirectory(
										android.os.Environment.DIRECTORY_PICTURES);
		if (directory.exists() == false) {
			boolean didCreate = directory.mkdirs();
			if (didCreate == false) {
				// The default pictures directory will be unavailable on devices that do not have external storage.
				// The "Nook Color" is one of those devices and B&N suggests that we hard code the path as follows.
				directory = new java.io.File("/mnt/media/My Files/Pictures");
				if (directory.exists() == false) {
					// Failed to find the picture directory. Give up.
					return null;
				}
			}
		}

		// Fetch this application's name.
		String applicationName = CoronaEnvironment.getApplicationName();

		// Create a unique file name in the directory.
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault());
		String currentTimeStamp = sdf.format(Calendar.getInstance().getTime());
		String fileName = applicationName + " Picture " + currentTimeStamp + fileExtensionName;
		java.io.File uniqueFile = new java.io.File(directory, fileName);
		return uniqueFile;
	}

	/**
	 * Copies the given file to the default pictures directory.
	 * Warning: User must define: android.permission.WRITE_EXTERNAL_STORAGE
	 * @param filePathName The path and name of the image file to be copied. Cannot be null or empty.
	 * @return Returns true if the given image file was successfully copied to the pictures directory.
	 *         <p>
	 *         Returns false if given an invalid argument or if failed to copy the file.
	 */
	protected static boolean callSaveImageToPhotoLibrary( CoronaRuntime runtime, String filePathName ) {
		// Validate.
		if ((filePathName == null) || (filePathName.length() <= 0)) {
			return false;
		}

		// Fetch the controller.
		Controller controller = runtime.getController();
		if (controller == null) {
			return false;
		}

		// Create a file services object used to easily copy files.
		com.ansca.corona.storage.FileServices fileServices;
		fileServices = new com.ansca.corona.storage.FileServices(CoronaEnvironment.getApplicationContext());

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
			// For Android 10+ use MediaStore
			ContentValues values = new ContentValues();

			String fileExtensionName = fileServices.getExtensionFrom(filePathName);
			String applicationName = CoronaEnvironment.getApplicationName();
			SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault());
			String currentTimeStamp = sdf.format(Calendar.getInstance().getTime());
			String fileName = applicationName + " Picture " + currentTimeStamp + "." + fileExtensionName;

			values.put(MediaStore.Images.Media.DISPLAY_NAME, fileName);
			values.put(MediaStore.Images.Media.MIME_TYPE, getMimeType(fileExtensionName));
			values.put(MediaStore.Images.Media.RELATIVE_PATH, Environment.DIRECTORY_PICTURES);
			values.put(MediaStore.Images.Media.IS_PENDING, 1);

			ContentResolver resolver = CoronaEnvironment.getApplicationContext().getContentResolver();
			Uri collection = MediaStore.Images.Media.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY);

			try {
				Uri uri = resolver.insert(collection, values);
				if (uri != null) {
					// Copy file content to uri
					try (OutputStream out = resolver.openOutputStream(uri);
						 InputStream in = fileServices.openFile(filePathName)) {

						byte[] buffer = new byte[8192];
						int bytesRead;
						while ((bytesRead = in.read(buffer)) != -1) {
							out.write(buffer, 0, bytesRead);
						}
					}

					// Mark as not pending
					values.clear();
					values.put(MediaStore.Images.Media.IS_PENDING, 0);
					resolver.update(uri, values, null, null);
					return true;
				}
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			}
			return false;
		} else {
			// For older Android versions use the original implementation
			String fileExtensionName = fileServices.getExtensionFrom(filePathName);
			java.io.File destinationFile = createUniqueFileNameInPicturesDirectory(fileExtensionName);
			if (destinationFile == null) {
				return false;
			}
			String destinationFilePathName = destinationFile.getPath();

			SaveImageToPhotoLibraryRequestPermissionsResultHandler resultHandler
				= new SaveImageToPhotoLibraryRequestPermissionsResultHandler(
					runtime, fileServices, filePathName, destinationFilePathName);

			return resultHandler.handleSaveMedia();
		}
	}

	// Helper method to determine MIME type
	private static String getMimeType(String extension) {
		switch (extension) {
			case "png":
				return "image/png";
			case "jpg":
			case "jpeg":
				return "image/jpeg";
			case "gif":
				return "image/gif";
			case "webp":
				return "image/webp";
			default:
				return "image/jpeg";
		}
	}

	/**
	 * Save a bitmap.
	 * Warning: User must define: android.permission.WRITE_EXTERNAL_STORAGE
	 * 
	 */
	protected static boolean callSaveBitmap( CoronaRuntime runtime, int[] pixels, int width, int height, int quality, String filePathName )
	{
		// Validate.
		if (runtime.getController() == null) {
			Log.v( "Corona", "callSaveBitmap has invalid controller" );
			return false;
		}

		CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			Log.v( "Corona", "callSaveBitmap has null CoronaActivity" );
			return false;
		}
		
		// If a file name was not provided, then generate a unique file name in the pictures directory.
		boolean addToPhotoLibrary = false;
		if ((filePathName == null) || (filePathName.length() <= 0)) {
			filePathName = null;
			java.io.File newFile = createUniqueFileNameInPicturesDirectory(".png");
			if (newFile == null) {
				Log.i( "Corona", "ERROR: Failed to save bitmap to the photo library." );
				return false;
			}
			filePathName = newFile.getPath();
			addToPhotoLibrary = true;

		}
		
		// Copy the pixel array into a bitmap object.
		android.graphics.Bitmap bitmap = null;
		try {
			bitmap = android.graphics.Bitmap.createBitmap(
							pixels, width, height, android.graphics.Bitmap.Config.ARGB_8888);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		if (bitmap == null) {
			return false;
		}
		
		SaveBitmapRequestPermissionsResultHandler resultHandler = new SaveBitmapRequestPermissionsResultHandler(
			runtime, bitmap, quality, filePathName, addToPhotoLibrary);
		return resultHandler.handleSaveMedia();
	}

	/** Default handling of the write external storage permission for saving media on Android 6+. */
	private abstract static class SaveMediaRequestPermissionsResultHandler 
		implements CoronaActivity.OnRequestPermissionsResultHandler {

		protected CoronaRuntime fCoronaRuntime;
		
		protected SaveMediaRequestPermissionsResultHandler(CoronaRuntime runtime) {
			fCoronaRuntime = runtime;
		}

		public boolean handleSaveMedia() {
			// Check for WRITE_EXTERNAL_STORAGE permission.
			PermissionsServices permissionsServices = new PermissionsServices(CoronaEnvironment.getApplicationContext());
			PermissionState writeExternalStoragePermissionState = 
				permissionsServices.getPermissionStateFor(PermissionsServices.Permission.WRITE_EXTERNAL_STORAGE);
			switch(writeExternalStoragePermissionState) {
				case MISSING:
					// The Corona developer forgot to add the permission to the AndroidManifest.xml.
					permissionsServices.showPermissionMissingFromManifestAlert(PermissionsServices.Permission.WRITE_EXTERNAL_STORAGE, 
						"Saving Images requires access to the device's Storage!");
					break;
				case DENIED:
					// Only possible on Android 6.
					if (!permissionsServices.shouldNeverAskAgain(PermissionsServices.Permission.WRITE_EXTERNAL_STORAGE)) {
						// Create our Permissions Settings to compare against in the handler.
						PermissionsSettings settings = new PermissionsSettings(PermissionsServices.Permission.WRITE_EXTERNAL_STORAGE);

						// Request Write External Storage permission.
						permissionsServices.requestPermissions(settings, this);
					}
					break;
				default:
					// Permission is granted!
					return executeSaveMedia();
			}

			return false;
		}

		@Override
		public void onHandleRequestPermissionsResult(
				CoronaActivity activity, int requestCode, String[] permissions, int[] grantResults) {

			PermissionsSettings permissionsSettings = activity.unregisterRequestPermissionsResultHandler(this);

			if (permissionsSettings != null) {
				permissionsSettings.markAsServiced();
			}

			// Check for WRITE_EXTERNAL_STORAGE permission.
			PermissionsServices permissionsServices = new PermissionsServices(activity);
			if (permissionsServices.getPermissionStateFor(
					PermissionsServices.Permission.WRITE_EXTERNAL_STORAGE) == PermissionState.GRANTED) {
				executeSaveMedia();
			} // Otherwise, we have nothing to do!
		}

		abstract public boolean executeSaveMedia();
	}

	/** Default handling of the write external storage permission for saveBitmap() on Android 6+. */
	private static class SaveBitmapRequestPermissionsResultHandler 
		extends NativeToJavaBridge.SaveMediaRequestPermissionsResultHandler {

		// Arguments for the saveBitmap() method that we can now call safely.
		private Bitmap fBitmap;
		private int fQuality;
		private String fFilePathName;
		private boolean fAddToPhotoLibrary;

		public SaveBitmapRequestPermissionsResultHandler(
			CoronaRuntime runtime, Bitmap bitmap, int quality, String filePathName, boolean addToPhotoLibrary) {
			super(runtime);

			fBitmap = bitmap;
			fQuality = quality;
			fFilePathName = filePathName;
			fAddToPhotoLibrary = addToPhotoLibrary;
		}

		@Override
		public boolean handleSaveMedia() {
			// We only check for External storage permission if the user wants to add to the Photo library.
			if (fAddToPhotoLibrary) {
				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {  // Requires use to Media Store to Save on Android 10+
					ContentValues values = new ContentValues();
					String fileName = fFilePathName.substring(fFilePathName.lastIndexOf('/') + 1);
					values.put(MediaStore.Images.Media.DISPLAY_NAME, fileName);
					if(fileName.contains(".png")){
						values.put(MediaStore.Images.Media.MIME_TYPE, "image/png");
					}else{
						values.put(MediaStore.Images.Media.MIME_TYPE, "image/jpeg");
					}
					values.put(MediaStore.Images.Media.RELATIVE_PATH, Environment.DIRECTORY_PICTURES);
					values.put(MediaStore.Images.Media.IS_PENDING, 1);

					try {
						Uri uri = CoronaEnvironment.getApplicationContext().getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
						if (uri != null) {
							try (OutputStream out = CoronaEnvironment.getApplicationContext().getContentResolver().openOutputStream(uri)) {
								fBitmap.compress(Bitmap.CompressFormat.JPEG, 90, out);
							}
							values.clear();
							values.put(MediaStore.Images.Media.IS_PENDING, 0);
							CoronaEnvironment.getApplicationContext().getContentResolver().update(uri, values, null, null);
							return true;
						}
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
				else{
					return super.handleSaveMedia();
				}
			}
			return executeSaveMedia();
		}

		@Override
		public boolean executeSaveMedia() {
			boolean wasSaved = fCoronaRuntime.getController().saveBitmap(fBitmap, fQuality, fFilePathName);
			if (wasSaved && fAddToPhotoLibrary) {
				fCoronaRuntime.getController().addImageFileToPhotoGallery(fFilePathName);
			}
			// fBitmap.recycle();
			return wasSaved;
		}
	}

	/** Default handling of the write external storage permission for saveImageToPhotoLibrary() on Android 6+. */
	private static class SaveImageToPhotoLibraryRequestPermissionsResultHandler 
		extends NativeToJavaBridge.SaveMediaRequestPermissionsResultHandler {

		// Arguments/dependencies for the saveImageToPhotoLibrary() method that we can now call safely.
		private com.ansca.corona.storage.FileServices fFileServices;
		private String fSourceFilePathName;
		private String fDestinationFilePathName;

		public SaveImageToPhotoLibraryRequestPermissionsResultHandler(
			CoronaRuntime runtime, com.ansca.corona.storage.FileServices fileServices,
				String sourceFilePathName, String destinationFilePathName) {
			super(runtime);

			fFileServices = fileServices;
			fSourceFilePathName = sourceFilePathName;
			fDestinationFilePathName = destinationFilePathName;
		}

		@Override
		public boolean executeSaveMedia() {
			// Validate arguments.
			if (fFileServices == null 
				|| fSourceFilePathName == null 
				|| fSourceFilePathName.length() <= 0 
				|| fCoronaRuntime == null 
				|| fCoronaRuntime.getController() == null) {
				return false;
			}

			// Copy the given file to the pictures directory.
			boolean wasCopied = fFileServices.copyFile(fSourceFilePathName, fDestinationFilePathName);
			if (wasCopied) {
				// Add the image the Android gallery via the media scanner.
				fCoronaRuntime.getController().addImageFileToPhotoGallery(fDestinationFilePathName);
			} else {
				return false;
			}

			return true;
		}
	}
	
	protected static boolean callRenderText(
		CoronaRuntime runtime, long nativeImageMemoryAddress, String text, String fontName, float fontSize,
		boolean isBold, int wrapWidth, int clipWidth, int clipHeight, String alignment)
	{
		return callRenderText(runtime, nativeImageMemoryAddress, text, fontName, fontSize, 
							  isBold, wrapWidth, clipWidth, clipHeight, alignment, null);
	}

	protected static boolean callRenderText(
		CoronaRuntime runtime, long nativeImageMemoryAddress, String text, String fontName, float fontSize,
		boolean isBold, int wrapWidth, int clipWidth, int clipHeight, String alignment, float[] outBaseOffset)
	{
		// Validate.
		if (nativeImageMemoryAddress == 0) {
			return false;
		}

		// Fetch the application context.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return false;
		}

		// Get the horizontal alignment type.
		com.ansca.corona.graphics.HorizontalAlignment horizontalAlignment;
		horizontalAlignment = com.ansca.corona.graphics.HorizontalAlignment.fromCoronaStringId(alignment);
		if (horizontalAlignment == null) {
			horizontalAlignment = com.ansca.corona.graphics.HorizontalAlignment.LEFT;
		}

		// Set up a text renderer with the given settings.
		com.ansca.corona.graphics.TextRenderer textRenderer;
		textRenderer = new com.ansca.corona.graphics.TextRenderer(context);
		textRenderer.getFontSettings().setName(fontName);
		textRenderer.getFontSettings().setPointSize(fontSize);
		textRenderer.getFontSettings().setIsBold(isBold);
		textRenderer.setText(text);
		textRenderer.setHorizontalAlignment(horizontalAlignment);
		textRenderer.setWrapWidth(wrapWidth);
		textRenderer.setClipWidth(clipWidth);
		textRenderer.setClipHeight(clipHeight);

		// Render the text to a bitmap.
		android.graphics.Bitmap bitmap = textRenderer.createBitmap();
		if (bitmap == null) {
			return false;
		}

		if ( outBaseOffset != null && outBaseOffset.length == 1 ) {
			outBaseOffset[0] = textRenderer.getBaselineOffset();
		}
		// Copy the Java bitmap's pixels to the native C/C++ bitmap object.
		boolean convertToGrayscale = true;
		boolean wasCopied = JavaToNativeShim.copyBitmap(
								runtime, nativeImageMemoryAddress, bitmap, 1.0f, 0, convertToGrayscale);

		// Free the memory used by the bitmap.
		// bitmap.recycle();

		// Returns true if the bitmap's pixels were successfully loaded to the native bitmap object.
		return wasCopied;
	}
	
	/**
	 * Return a list of available fonts
	 * 
	 * @return 			List of fonts
	 */
	protected static String[] callGetFonts()
	{
		com.ansca.corona.graphics.FontServices fontServices;
		fontServices = new com.ansca.corona.graphics.FontServices(CoronaEnvironment.getApplicationContext());
		return fontServices.fetchAllSystemFontNames();
	}

	/**
	 * Returns font metrics map
	 * @param fontName	Given font name
	 * @param fontSize	Given font size
	 * @return
	 */
	protected static float[] callGetFontMetrics( CoronaRuntime runtime, String fontName, float fontSize, boolean isBold )
	{
		com.ansca.corona.graphics.FontServices fontServices = new com.ansca.corona.graphics.FontServices(CoronaEnvironment.getApplicationContext());
		Map<String, Float> result = new HashMap<String, Float>();
		java.util.ArrayList<Float> metricsArray = new java.util.ArrayList<Float>();
		float[] array = new float[ 4 ];
		if ( fontServices != null ) {
			result = fontServices.getFontMetrics( fontName, fontSize, isBold );
			if ( result != null && result.size() == 4 ) {
				array [ 0 ] = result.get( "ascent" ) ;
				array [ 1 ] = result.get( "descent" ) ;
				array [ 2 ] = result.get( "leading" ) ;
				array [ 3 ] = result.get( "height" ) ;
			}
		}
		return array;
	}
	
	/**
	 * Set the Corona callback timer.
	 * 
	 * @param milliseconds
	 */
	protected static void callSetTimer( int milliseconds, CoronaRuntime runtime)
	{
		runtime.getController().setTimer( milliseconds );
	}

	/**
	 * Cancel the Corona callback timer.
	 */
	protected static void callCancelTimer(CoronaRuntime runtime)
	{
		runtime.getController().cancelTimer();
	}

	protected static void callLoadSound( CoronaRuntime runtime, long id, String soundName )
	{
		runtime.getController().getEventManager().loadSound( id, soundName );
	}

	protected static void callLoadEventSound( CoronaRuntime runtime, long id, String soundName )
	{
		runtime.getController().getEventManager().loadEventSound(id, soundName);
	}

	protected static void callPlaySound( CoronaRuntime runtime, long id, String soundName, boolean loop )
	{
		runtime.getController().getEventManager().playSound(id, soundName, loop);
	}

	protected static void callStopSound( long id, CoronaRuntime runtime )
	{
		runtime.getController().getEventManager().stopSound( id);
	}

	protected static void callPauseSound( long id, CoronaRuntime runtime )
	{
		runtime.getController().getEventManager().pauseSound( id);
	}

	protected static void callResumeSound( long id, CoronaRuntime runtime )
	{
		runtime.getController().getEventManager().resumeSound(id);
	}

	protected static void callPlayVideo( CoronaRuntime runtime, long id, String url, boolean mediaControlsEnabled )
	{
		runtime.getController().getMediaManager().playVideo( id, url, mediaControlsEnabled );
	}

	protected static boolean callCanOpenUrl( CoronaRuntime runtime, String url )
	{
		return runtime.getController().canOpenUrl( url );
	}

	protected static boolean callOpenUrl( CoronaRuntime runtime, String url )
	{
		return runtime.getController().openUrl( url );
	}

	protected static void callSetIdleTimer( CoronaRuntime runtime, boolean enabled )
	{
		runtime.getController().setIdleTimer( enabled );
	}

	protected static boolean callGetIdleTimer(CoronaRuntime runtime)
	{
		return runtime.getController().getIdleTimer();
	}

	protected static void callSetStatusBarMode(final int mode, CoronaRuntime runtime)
	{
		CoronaStatusBarApiListener listener = runtime.getController().getCoronaStatusBarApiListener();
		if (listener != null) {
			listener.setStatusBarMode(CoronaStatusBarSettings.fromCoronaIntId(mode));
		}
	}
	
	protected static int callGetStatusBarMode(CoronaRuntime runtime)
	{
		CoronaStatusBarSettings mode = CoronaStatusBarSettings.HIDDEN;
		
		synchronized (runtime.getController()) {
			CoronaStatusBarApiListener listener = runtime.getController().getCoronaStatusBarApiListener();
			if (listener != null) {
				mode = listener.getStatusBarMode();
			}
		}
		return mode.toCoronaIntId();
	}
	
	protected static int callGetStatusBarHeight(CoronaRuntime runtime) {
		int height = 0;
		
		synchronized (runtime.getController()) {
			CoronaStatusBarApiListener listener = runtime.getController().getCoronaStatusBarApiListener();
			if (listener != null) {
				return listener.getStatusBarHeight();
			}
		}
		return height;
	}

	protected static float[] callGetSafeAreaInsetPixels(CoronaRuntime runtime)
	{
		float[] result = new float[4];
		synchronized (runtime.getController()) {
			CoronaStatusBarApiListener listener = runtime.getController().getCoronaStatusBarApiListener();
			CoronaStatusBarSettings statusBarMode = listener.getStatusBarMode();
			boolean hasNavigationBar = listener.HasSoftwareKeys();

			if (listener != null) {
				if (listener.IsAndroidTV()) {
					int contentHeight = JavaToNativeShim.getContentHeightInPixels(runtime);
					int contentWidth = JavaToNativeShim.getContentWidthInPixels(runtime);
					result[ 0 ] = result[ 3 ] = (float)Math.floor(contentHeight * 0.05f);
					result[ 1 ] = result[ 2 ] = (float)Math.floor(contentWidth * 0.05f);
				}
				else {
					DisplayCutout cutout = CoronaEnvironment.getCoronaActivity().getDisplayCutout();
					if ((android.os.Build.VERSION.SDK_INT >= 26) && (cutout != null)){

						result[0] = cutout.getSafeInsetTop();
						result[1] = cutout.getSafeInsetLeft();
						result[2] = cutout.getSafeInsetRight();
						//Android InsetBottom does not always return correct navbar height
						if(hasNavigationBar && cutout.getSafeInsetBottom() == 0 && !runtime.getController().getSystemUiVisibility().contains("immersive")){
							result[3] = listener.getNavigationBarHeight();
						}else{
							result[3] = cutout.getSafeInsetBottom();
						}
					}
					else {
                        result[0] = (statusBarMode != CoronaStatusBarSettings.HIDDEN) ? listener.getStatusBarHeight() : 0;
                        if (hasNavigationBar && runtime.getController().getSystemUiVisibility().contains("immersive")) {
                            result[1] = result[2] = result[3] = 0;
                        } else {
                            int navBarIndex = 4;
                            if ((statusBarMode == CoronaStatusBarSettings.LIGHT_TRANSPARENT ||
                                    statusBarMode == CoronaStatusBarSettings.DARK_TRANSPARENT) && hasNavigationBar) {
                                WindowOrientation currentOrientation = WindowOrientation.fromCurrentWindowUsing(runtime.getController().getContext());
                                navBarIndex = (currentOrientation == WindowOrientation.PORTRAIT_UPRIGHT) ? 3 : 2;
                            }
                            for (int i = 1; i < 4; i++) {
                                result[i] = (i == navBarIndex) ? listener.getNavigationBarHeight() : 0;
                            }
                        }
                    }
				}
			}
			else { 
				for (int i = 0; i < 4; i++) {
					result [ i ] = 0;
				}
			}
			
		}
		return result;
	}
	
	protected static void callShowNativeAlert( CoronaRuntime runtime, String title, String msg, String[] buttonLabels )
	{
		runtime.getController().showNativeAlert( title, msg, buttonLabels );
	}

	protected static void callCancelNativeAlert( int which, CoronaRuntime runtime )
	{
		runtime.getController().cancelNativeAlert( which );
	}

	protected static void callShowTrialAlert(CoronaRuntime runtime)
	{
		runtime.getController().showTrialAlert();
	}
	
	protected static void callShowNativeActivityIndicator(CoronaRuntime runtime)
	{
		runtime.getController().showNativeActivityIndicator();
	}
	
	protected static void callCloseNativeActivityIndicator(CoronaRuntime runtime)
	{
		runtime.getController().closeNativeActivityIndicator();
	}
	
	protected static boolean callHasMediaSource(CoronaRuntime runtime, int mediaSourceType)
	{
		return runtime.getController().hasMediaSource(mediaSourceType);
	}
	
	protected static boolean callHasAccessToMediaSource(CoronaRuntime runtime, int mediaSourceType)
	{
		return runtime.getController().hasAccessToMediaSource(mediaSourceType);
	}

	protected static void callShowImagePicker(CoronaRuntime runtime, int imageSourceType, String destinationFilePath)
	{
		runtime.getController().showImagePickerWindow(imageSourceType, destinationFilePath);
	}

	protected static void callShowVideoPicker(CoronaRuntime runtime, int videoSourceType, int maxTime, int quality)
	{
		runtime.getController().showVideoPickerWindow(videoSourceType, maxTime, quality);
	}
	
	protected static boolean callCanShowPopup(CoronaRuntime runtime, String name)
	{
		return runtime.getController().canShowPopup(name);
	}

	protected static void callShowSendMailPopup(CoronaRuntime runtime, java.util.HashMap dictionaryOfSettings)
	{
		runtime.getController().showSendMailWindow((java.util.HashMap<String, Object>)dictionaryOfSettings);
	}
	
	protected static void callShowSendSmsPopup(CoronaRuntime runtime, java.util.HashMap dictionaryOfSettings)
	{
		runtime.getController().showSendSmsWindow((java.util.HashMap<String, Object>)dictionaryOfSettings);
	}
	
	protected static boolean callShowAppStorePopup(CoronaRuntime runtime, java.util.HashMap dictionaryOfSettings)
	{
		return runtime.getController().showAppStoreWindow((java.util.HashMap<String, Object>)dictionaryOfSettings);
	}
	
	protected static void callShowRequestPermissionsPopup(CoronaRuntime runtime, java.util.HashMap dictionaryOfSettings)
	{
		runtime.getController().showRequestPermissionsWindow((java.util.HashMap<String, Object>)dictionaryOfSettings);
	}

	protected static void callDisplayUpdate(CoronaRuntime runtime)
	{
		runtime.getController().displayUpdate();
	}

	protected static void callSetAccelerometerInterval( int frequencyInHz, CoronaRuntime runtime )
	{
		runtime.getController().setAccelerometerInterval( frequencyInHz );
	}
	
	protected static void callSetGyroscopeInterval( int frequencyInHz, CoronaRuntime runtime )
	{
		runtime.getController().setGyroscopeInterval( frequencyInHz );
	}
	
	protected static boolean callHasAccelerometer(CoronaRuntime runtime)
	{
		return runtime.getController().hasAccelerometer();
	}
	
	protected static boolean callHasGyroscope(CoronaRuntime runtime)
	{
		return runtime.getController().hasGyroscope();
	}

	protected static boolean callHasHeadingHardware(CoronaRuntime runtime)
	{
		return runtime.getController().hasHeadingHardware();
	}
	
	protected static void callSetEventNotification( CoronaRuntime runtime, int eventType, boolean enable )
	{
		runtime.getController().setEventNotification( eventType, enable );
	}
	
	protected static String callGetManufacturerName(CoronaRuntime runtime)
	{
		return runtime.getController().getManufacturerName();
	}
	
	protected static String callGetModel(CoronaRuntime runtime)
	{
		return runtime.getController().getModel();
	}

	protected static String callGetName(CoronaRuntime runtime)
	{
		return runtime.getController().getName();
	}

	protected static String callGetUniqueIdentifier( int identifierType, CoronaRuntime runtime )
	{
		return runtime.getController().getUniqueIdentifier( identifierType );
	}
	
	protected static String callGetPlatformVersion(CoronaRuntime runtime)
	{
		return runtime.getController().getPlatformVersion();
	}

	protected static String callGetProductName(CoronaRuntime runtime)
	{
		return runtime.getController().getProductName();
	}
	
	protected static float callGetDefaultFontSize(CoronaRuntime runtime)
	{
		return runtime.getController().getDefaultFontSize();
	}

	protected static int callGetDefaultTextFieldPaddingInPixels(CoronaRuntime runtime)
	{
		return runtime.getController().getDefaultTextFieldPaddingInPixels();
	}

	private static android.util.DisplayMetrics getDisplayMetrics()
	{
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return null;
		}

		android.view.WindowManager windowManager;
		windowManager = (android.view.WindowManager)context.getSystemService(android.content.Context.WINDOW_SERVICE);
		if (windowManager == null) {
			return null;
		}

		android.util.DisplayMetrics metrics = new android.util.DisplayMetrics();
		windowManager.getDefaultDisplay().getMetrics(metrics);
		return metrics;
	}

	protected static int callGetApproximateScreenDpi()
	{
		int result = -1;

		android.util.DisplayMetrics metrics = getDisplayMetrics();
		if (metrics != null) {
			result = metrics.densityDpi;
		}

		return result;
	}

	protected static int callPushSystemInfoToLua(CoronaRuntime runtime, long luaStateMemoryAddress, String key)
	{
		// Validate.
		if (luaStateMemoryAddress == 0) {
			return 0;
		}

		// Fetch the LuaState object by it's memory address.
		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}

		// Using coroutines will give a different lua state than what the runtime has so this is to verify its the same one
		if (luaState == null || CoronaRuntimeProvider.getLuaStateMemoryAddress(luaState) != luaStateMemoryAddress) {
			luaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
		}

		// Fetch information about this application.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return 0;
		}
		android.content.pm.ApplicationInfo applicationInfo = context.getApplicationInfo();
		android.content.pm.PackageManager packageManager = context.getPackageManager();

		// Push the requested information to Lua.
		int valuesPushed = 0;
		if ((key == null) || (key.length() <= 0)) {
			// The given key is invalid. Ignore it.
		}
		else if (key.equals("appName")) {
			// Fetch this application's name.
			String applicationName = CoronaEnvironment.getApplicationName();
			luaState.pushString(applicationName);
			valuesPushed = 1;
		}
		else if (key.equals("appVersionString")) {
			// Fetch this application's version string.
			String versionName = null;
			try {
				android.content.pm.PackageInfo packageInfo =
						packageManager.getPackageInfo(context.getPackageName(), 0);
				versionName = packageInfo.versionName;
			}
			catch (Exception ex) { }
			if (versionName == null) {
				versionName = "";
			}
			luaState.pushString(versionName);
			valuesPushed = 1;
		}
		else if (key.equals("androidAppVersionCode")) {
			// Fetch this application's version code.
			int versionCode = 0;
			try {
				android.content.pm.PackageInfo packageInfo =
						packageManager.getPackageInfo(context.getPackageName(), 0);
				versionCode = packageInfo.versionCode;
			}
			catch (Exception ex) { }
			luaState.pushInteger(versionCode);
			valuesPushed = 1;
		}
		else if (key.equals("androidAppPackageName") || key.equals("bundleID")) {
			// Fetch this application's unique package name string.
			String packageName = applicationInfo.packageName;
			if (packageName == null) {
				packageName = "";
			}
			luaState.pushString(packageName);
			valuesPushed = 1;
		}
		else if (key.equals("androidApiLevel")) {
			luaState.pushInteger(android.os.Build.VERSION.SDK_INT);
			valuesPushed = 1;
		}
		else if (key.equals("androidDisplayDensityName")) {
			// Fetch the display's density name, such as "hdpi" or "xhdpi".
			String densityName = "unknown";
			android.util.DisplayMetrics metrics = getDisplayMetrics();
			if (metrics != null) {
				switch (metrics.densityDpi) {
					case 213: // android.util.DisplayMetrics.DENSITY_TV
						densityName = "tvdpi";
						break;
					case 480: // android.util.DisplayMetrics.DENSITY_XXHIGH
						densityName = "xxhdpi";
						break;
					case 640: // android.util.DisplayMetrics.DENSITY_XXXHIGHT
						densityName = "xxxhdpi";
						break;
					case 320: // android.util.DisplayMetrics.DENSITY_XHIGH
						densityName = "xhdpi";
						break;
					case android.util.DisplayMetrics.DENSITY_HIGH:
						densityName = "hdpi";
						break;
					case android.util.DisplayMetrics.DENSITY_MEDIUM:
						densityName = "mdpi";
						break;
					case android.util.DisplayMetrics.DENSITY_LOW:
						densityName = "ldpi";
						break;
				}
			}
			luaState.pushString(densityName);
			valuesPushed = 1;
		}
		else if (key.equals("androidDisplayApproximateDpi")) {
			// Fetches the display's approximate DPI value.
			// This is needed because some Android devices provide the wrong xdpi and ydpi values.
			android.util.DisplayMetrics metrics = getDisplayMetrics();
			if (metrics != null) {
				luaState.pushInteger(metrics.densityDpi);
				valuesPushed = 1;
			}
		}
		else if (key.equals("androidDisplayXDpi")) {
			// Fetch the display's DPI along the x-axis.
			android.util.DisplayMetrics metrics = getDisplayMetrics();
			if (metrics != null) {
				WindowOrientation orientation = WindowOrientation.fromCurrentWindowUsing(context);
				double xDpi = (double)(orientation.isPortrait() ? metrics.xdpi : metrics.ydpi);
				luaState.pushNumber(xDpi);
				valuesPushed = 1;
			}
		}
		else if (key.equals("androidDisplayYDpi")) {
			// Fetch the display's DPI along the y-axis.
			android.util.DisplayMetrics metrics = getDisplayMetrics();
			if (metrics != null) {
				WindowOrientation orientation = WindowOrientation.fromCurrentWindowUsing(context);
				double yDpi = (double)(orientation.isPortrait() ? metrics.ydpi : metrics.xdpi);
				luaState.pushNumber(yDpi);
				valuesPushed = 1;
			}
		}
		else if (key.equals("androidDisplayWidthInInches")) {
			// Fetch the width of the window in inches.
			android.util.DisplayMetrics metrics = getDisplayMetrics();
			if (metrics != null) {
				WindowOrientation orientation = WindowOrientation.fromCurrentWindowUsing(context);
				double xDpi = (double)(orientation.isPortrait() ? metrics.xdpi : metrics.ydpi);
				if (xDpi > 0) {
					luaState.pushNumber((double)metrics.widthPixels / xDpi);
					valuesPushed = 1;
				}
			}
		}
		else if (key.equals("androidDisplayHeightInInches")) {
			// Fetch the height of the window in inches.
			android.util.DisplayMetrics metrics = getDisplayMetrics();
			if (metrics != null) {
				WindowOrientation orientation = WindowOrientation.fromCurrentWindowUsing(context);
				double yDpi = (double)(orientation.isPortrait() ? metrics.ydpi : metrics.xdpi);
				if (yDpi > 0) {
					luaState.pushNumber((double)metrics.heightPixels / yDpi);
					valuesPushed = 1;
				}
			}
		}
		else if (key.equals("deniedAppPermissions") || key.equals("androidDeniedAppPermissions")) {
			// Get an array of all denied permissions
			com.ansca.corona.permissions.PermissionsServices permissionsServices = new com.ansca.corona.permissions.PermissionsServices(context);
			String[] deniedPermissions = permissionsServices.getRequestedPermissionsInState(com.ansca.corona.permissions.PermissionState.DENIED);

			if (deniedPermissions == null) {
				// Push an empty table, since we couldn't get any denied permissions.
				luaState.newTable(0, 0);
			} else {
				// Put the denied permissions in a Lua table.
				luaState.newTable(deniedPermissions.length, 0);

				// Lua arrays are 1-based so add 1 to index correctly.
				int luaTableIdx = 1;

				// Track the platform-agnostic names we've pushed to avoid duplicates.
				// This is because multiple Android permissions can be in a permission group.
				HashSet<String> usedPANames = new HashSet<String>();

		        for (int permissionIdx = 0; permissionIdx < deniedPermissions.length; permissionIdx++) {
		            if (deniedPermissions[permissionIdx] != null) {
		            	String nameToPush = deniedPermissions[permissionIdx];
		            	if (key.equals("deniedAppPermissions")) {
		            		// See if a platform-agnostic app permission name is available.
		            		if (permissionsServices.isPartOfPAAppPermission(nameToPush)) {

		            			// Only push the name if it's not already in the table.
		            			nameToPush = permissionsServices.getPAAppPermissionNameFromAndroidPermission(nameToPush);
		            			if (usedPANames.contains(nameToPush)) continue;
		            			usedPANames.add(nameToPush);
		            		}	
		            	}
			            // Push this string to the top of the stack
			            luaState.pushString(nameToPush);

			            // Assign this string to the table 2nd from the top of the stack.
			            luaState.rawSet(-2, luaTableIdx++);
			        }
		        }
		    }

			valuesPushed = 1;
		}
		else if (key.equals("grantedAppPermissions") || key.equals("androidGrantedAppPermissions")) {
			// Get an array of all granted permissions
			com.ansca.corona.permissions.PermissionsServices permissionsServices = new com.ansca.corona.permissions.PermissionsServices(context);
			String[] grantedPermissions = permissionsServices.getRequestedPermissionsInState(com.ansca.corona.permissions.PermissionState.GRANTED);

			if (grantedPermissions == null) {
				// Push an empty table, since we couldn't get any granted permissions.
				luaState.newTable(0, 0);
			} else {
				// Put the granted permissions in a Lua table.
				luaState.newTable(grantedPermissions.length, 0);

				// Lua arrays are 1-based so add 1 to index correctly.
				int luaTableIdx = 1;

				// Track the platform-agnostic names we've pushed to avoid duplicates.
				// This is because multiple Android permissions can be in a permission group.
				HashSet<String> usedPANames = new HashSet<String>();

				// Add in all granted permissions, changing the scheme for platform-agnostic names if needed.
		        for (int permissionIdx = 0; permissionIdx < grantedPermissions.length; permissionIdx++) {
		            if (grantedPermissions[permissionIdx] != null) {
		            	String nameToPush = grantedPermissions[permissionIdx];
		            	if (key.equals("grantedAppPermissions")) {
		            		// See if a platform-agnostic app permission name is available.
		            		if (permissionsServices.isPartOfPAAppPermission(nameToPush)) {

		            			// Only push the name if it's not already in the table.
		            			nameToPush = permissionsServices.getPAAppPermissionNameFromAndroidPermission(nameToPush);
		            			if (usedPANames.contains(nameToPush)) continue;
		            			usedPANames.add(nameToPush);
		            		}	
		            	}
		            	
			            // Push this string to the top of the stack
			            luaState.pushString(nameToPush);

			            // Assign this string to the table 2nd from the top of the stack.
			            luaState.rawSet(-2, luaTableIdx++);
			        }
		        }
		    }

			valuesPushed = 1;
		}
		else if (key.equals("isoCountryCode")) {
			luaState.pushString(java.util.Locale.getDefault().getCountry());
			valuesPushed = 1;
		}
		else if (key.equals("isoLanguageCode")) {
			String languageCode = java.util.Locale.getDefault().getLanguage();
			if (languageCode != null) {
				languageCode = languageCode.toLowerCase();
			}
			else {
				languageCode = "";
			}
			if ("zh".equals(languageCode) && (android.os.Build.VERSION.SDK_INT >= 21)) {
				// Special case for the Chinese language.
				// Append the ISO 15924 script to the language which identifies if it is Simplified or Traditional.
				String scriptId = NativeToJavaBridge.ApiLevel21.getScriptFrom(java.util.Locale.getDefault());
				if ((scriptId != null) && (scriptId.length() > 0)) {
					languageCode = languageCode + "-" + scriptId.toLowerCase();
				}
			}
			luaState.pushString(languageCode);
			valuesPushed = 1;
		}
		else if (key.equals("darkMode")) {
			int currentNightMode = context.getResources().getConfiguration().uiMode & android.content.res.Configuration.UI_MODE_NIGHT_MASK;
			luaState.pushBoolean(currentNightMode == android.content.res.Configuration.UI_MODE_NIGHT_YES);
			valuesPushed = 1;
		}
		else if (key.equals("hasSoftwareKeys")) {
			luaState.pushBoolean(CoronaEnvironment.getCoronaActivity().HasSoftwareKeys());
			valuesPushed = 1;
		}

		// Push nil if failed to fetch the requested value.
		if (valuesPushed <= 0) {
			luaState.pushNil();
			valuesPushed = 1;
		}

		// Return the number of values pushed to Lua.
		return valuesPushed;
	}
	
	protected static String callGetPreference( int category, CoronaRuntime runtime )
	{
		String result = "";
		switch (category) {
			case 0: // kLocaleIdentifier
				result = java.util.Locale.getDefault().toString();
				break;
			case 1: // kLocaleLanguage
				result = java.util.Locale.getDefault().getLanguage();
				break;
			case 2: // kLocaleCountry
				result = java.util.Locale.getDefault().getCountry();
				break;
			case 3: // kUILanguage
				result = java.util.Locale.getDefault().getDisplayLanguage();
				break;
			default:
				System.err.println("getPreference: Unknown category " + Integer.toString(category));
				break;
		}
		return result;
	}

	protected static Object callGetPreference(String keyName)
	{
		// Validate argument.
		if (keyName == null) {
			return new Exception("Preference key name cannot be null.");
		}

		// Fetch the application context.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return new Exception("Failed to acquire application context.");
		}

		// Attempt to read the given preference key's value.
		Object objectValue = null;
		try {
			// Fetch the app's default shared preferences object.
			android.content.SharedPreferences sharedPreferences = null;
			sharedPreferences = android.preference.PreferenceManager.getDefaultSharedPreferences(context);
			if (sharedPreferences == null) {
				return new Exception("Failed to acquire the app's default Java SharedPreferences object.");
			}

			// If the key exists, then fetch an object reference to its value.
			// Note: Calling getBoolean(), getString(), etc. will throw an exception if stored value
			//       is not of that type. Fetching a shallow copy of the SharedPreference's collection
			//       is the only nice means of fetching the value if the type is unknown.
			if (sharedPreferences.contains(keyName)) {
				java.util.Map<String, ?> collection = sharedPreferences.getAll();
				if (collection != null) {
					objectValue = collection.get(keyName);
				}
			}
		}
		catch (Exception ex) {
			return ex;
		}

		// Returns the preference's value in boxed form.
		// Will return null if the preference was not found.
		return objectValue;
	}

	protected static String callSetPreferences(java.util.HashMap collection)
	{
		// Validate argument.
		if ((collection == null) || (collection.size() <= 0)) {
			return "Given preference collection was null or empty.";
		}

		// Fetch the application context.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return "Failed to acquire application context.";
		}

		// Attempt to write the given preferences to storage.
		try {
			// Fetch the app's default shared preferences object.
			android.content.SharedPreferences sharedPreferences = null;
			sharedPreferences = android.preference.PreferenceManager.getDefaultSharedPreferences(context);
			if (sharedPreferences == null) {
				return "Failed to acquire the app's default Java SharedPreferences object.";
			}
			android.content.SharedPreferences.Editor preferencesEditor = sharedPreferences.edit();
			if (preferencesEditor == null) {
				return "Failed to acquire the Java SharedPreferences editor object.";
			}

			// Write the given key-value pairs to shared preferences.
			for (java.util.Map.Entry entry : (java.util.Set<java.util.Map.Entry>)collection.entrySet()) {
				// Fetch the next preference key name.
				if ((entry.getKey() instanceof String) == false) {
					return "Given preference collection contains a key that's not of type string.";
				}
				String keyName = (String)entry.getKey();
				if (keyName == null) {
					return "Preference key name cannot be null.";
				}

				// Attempt to write the preference value to the shared preferences editor.
				// Note: This won't write to storage until we call commit() or apply().
				Object value = entry.getValue();
				if (value instanceof Boolean) {
					preferencesEditor.putBoolean(keyName, ((Boolean)value).booleanValue());
				}
				else if (value instanceof Integer) {
					preferencesEditor.putInt(keyName, ((Integer)value).intValue());
				}
				else if (value instanceof Long) {
					preferencesEditor.putLong(keyName, ((Long)value).longValue());
				}
				else if (value instanceof Float) {
					preferencesEditor.putFloat(keyName, ((Float)value).floatValue());
				}
				else if (value instanceof String) {
					preferencesEditor.putString(keyName, (String)value);
				}
				else {
					String message =
							"Failed to write preference '" + keyName + "' because its Java value type '" +
							value.getClass().getName() + "' is not supported.";
					return message;
				}
			}

			// Commit the above chagnes to storage.
			boolean wasSuccessful = preferencesEditor.commit();
			if (wasSuccessful == false) {
				return "Failed to commit preference changes to storage.";
			}
		}
		catch (Exception ex) {
			return ex.getMessage();
		}

		// Returning null means we've succeeded.
		// Note: Strings returned by this function are assumed to be error messages.
		return null;
	}

	protected static String callDeletePreferences(String[] keyNames)
	{
		// Validate.
		if (keyNames == null) {
			return "Preference key name array cannot be null.";
		}

		// Fetch the application context.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return "Failed to acquire application context.";
		}

		// Perform the requested operation.
		try {
			// Fetch the app's default shared preferences object.
			android.content.SharedPreferences sharedPreferences = null;
			sharedPreferences = android.preference.PreferenceManager.getDefaultSharedPreferences(context);
			if (sharedPreferences == null) {
				return "Failed to acquire the app's default Java SharedPreferences object.";
			}
			android.content.SharedPreferences.Editor preferencesEditor = sharedPreferences.edit();
			if (preferencesEditor == null) {
				return "Failed to acquire the Java SharedPreferences editor object.";
			}

			// Remove the given keys from shared preferences.
			for (int index = 0; index < keyNames.length; index++) {
				String keyName = keyNames[index];
				if (keyName == null) {
					return "Preference key name cannot be null.";
				}
				preferencesEditor.remove(keyName);
			}

			// Commit the above chagnes to storage.
			boolean wasSuccessful = preferencesEditor.commit();
			if (wasSuccessful == false) {
				return "Failed to commit preference changes to storage.";
			}
		}
		catch (Exception ex) {
			return ex.getMessage();
		}

		// Returning null means we've succeeded.
		// Note: Strings returned by this function are assumed to be error messages.
		return null;
	}

	protected static void callVibrate(CoronaRuntime runtime, String hapticType, String hapticStyle)
	{
		runtime.getController().vibrate(hapticType, hapticStyle);
	}

	protected static void callSetLocationAccuracy( double meters, CoronaRuntime runtime )
	{
		// TODO: unimplemented
	}

	protected static void callSetLocationThreshold( double meters, CoronaRuntime runtime )
	{
		runtime.getController().setLocationThreshold(meters);
	}

	protected static float callGetVolume( CoronaRuntime runtime, long id )
	{
		return runtime.getController().getMediaManager().getVolume( id );
	}
	
	protected static void callSetVolume( CoronaRuntime runtime, long id, float v )
	{
		runtime.getController().getMediaManager().setVolume( id, v );
	}

	protected static int callTextFieldCreate( CoronaRuntime runtime, int id, int left, int top, int width, int height, boolean isSingleLine )
	{
		runtime.getViewManager().addTextView(id, left, top, width, height, isSingleLine);

		return 1;
	}

	protected static void callTextFieldSetSelection( CoronaRuntime runtime, int id, int startPosition, int endPosition)
	{
		runtime.getViewManager().setTextSelection( id, startPosition, endPosition );
	}

	protected static int[] callTextFieldGetSelection(CoronaRuntime runtime, int id) {
	    return runtime.getViewManager().getTextSelection(id);
	}

	protected static void callTextFieldSetReturnKey( CoronaRuntime runtime, int id, String imeType ) 
	{
		runtime.getViewManager().setTextReturnKey( id, imeType );
	}

	protected static void callTextFieldSetPlaceholder( CoronaRuntime runtime, int id, String placeholder)
	{
		runtime.getViewManager().setTextPlaceholder( id, placeholder );
	}

	protected static void callTextFieldSetColor( CoronaRuntime runtime, int id, int r, int g, int b, int a )
	{
		int color = Color.argb( a, r, g, b );
		
		runtime.getViewManager().setTextViewColor(id, color);
	}
	
	protected static void callTextFieldSetText( CoronaRuntime runtime, int id, String text )
	{
		runtime.getViewManager().setTextViewText(id, text);
	}
	
	protected static void callTextFieldSetSize( CoronaRuntime runtime, int id, float fontSize )
	{
		runtime.getViewManager().setTextViewSize(id, fontSize);
	}
	
	protected static void callTextFieldSetFont( CoronaRuntime runtime, int id, String fontName, float fontSize, boolean isBold )
	{
		runtime.getViewManager().setTextViewFont(id, fontName, fontSize, isBold);
	}
	
	protected static void callTextFieldSetAlign( CoronaRuntime runtime, int id, String align )
	{
		runtime.getViewManager().setTextViewAlign(id, align);
	}
	
	protected static void callTextFieldSetSecure( CoronaRuntime runtime, int id, boolean isSecure )
	{
		runtime.getViewManager().setTextViewPassword(id, isSecure);	
	}
	
	protected static void callTextFieldSetInputType( CoronaRuntime runtime, int id, String inputType )
	{
		runtime.getViewManager().setTextViewInputType(id, inputType);	
	}

	protected static void callTextFieldSetEditable( CoronaRuntime runtime, int id, boolean isEditable )
	{
		runtime.getViewManager().setTextViewEditable(id, isEditable);	
	}
	
	protected static int[]  callTextFieldGetColor( CoronaRuntime runtime, int id )
	{
		int argb = runtime.getViewManager().getTextViewColor(id);

		int[] result = new int[4];

		result[0] = Color.red( argb );
		result[1] = Color.green( argb );
		result[2] = Color.blue( argb );
		result[3] = Color.alpha( argb );
		    
	    return result;
	}
	
	protected static String callTextFieldGetText( int id, CoronaRuntime runtime )
	{
		return runtime.getViewManager().getTextViewText(id);
	}
	
	protected static String callTextFieldGetPlaceholder( int id, CoronaRuntime runtime )
	{
		return runtime.getViewManager().getTextViewPlaceholder(id);
	}

	protected static float callTextFieldGetSize( CoronaRuntime runtime, int id )
	{
		return runtime.getViewManager().getTextViewSize(id);
	}

	protected static String callTextFieldGetFont( int id )
	{
		return "";
	}
	
	protected static String callTextFieldGetAlign( int id, CoronaRuntime runtime )
	{
		return runtime.getViewManager().getTextViewAlign(id);
	}
	
	protected static boolean callTextFieldGetSecure( CoronaRuntime runtime, int id )
	{
		return runtime.getViewManager().getTextViewPassword(id);
	}

	protected static String callTextFieldGetInputType( int id, CoronaRuntime runtime )
	{
		return runtime.getViewManager().getTextViewInputType(id);
	}

	protected static boolean callTextFieldIsSingleLine( CoronaRuntime runtime, int id )
	{
		return runtime.getViewManager().isTextViewSingleLine(id);
	}
	
	protected static boolean callTextFieldIsEditable( CoronaRuntime runtime, int id )
	{
		return runtime.getViewManager().isTextViewEditable(id);
	}
	
	protected static void callDisplayObjectDestroy( CoronaRuntime runtime, int id )
	{
		runtime.getViewManager().destroyDisplayObject(id);
	}
	
	protected static void callDisplayObjectSetVisible( CoronaRuntime runtime, int id, boolean visible )
	{
		runtime.getViewManager().setDisplayObjectVisible(id, visible);
	}
	
	protected static void callDisplayObjectUpdateScreenBounds( CoronaRuntime runtime, int id, int x, int y, int w, int h )
	{
		runtime.getViewManager().displayObjectUpdateScreenBounds(id, x, y, w, h);
	}
	
	protected static void callDisplayObjectSetAlpha( CoronaRuntime runtime, int id, float alpha )
	{
		runtime.getViewManager().setDisplayObjectAlpha(id, alpha);
	}
	
	protected static void callDisplayObjectSetBackground( CoronaRuntime runtime, int id, boolean bg )
	{
		runtime.getViewManager().setDisplayObjectBackground(id, bg);
	}
	
	protected static boolean callDisplayObjectGetVisible( CoronaRuntime runtime, int id )
	{
		return runtime.getViewManager().getDisplayObjectVisible(id);
	}
	
	protected static float callDisplayObjectGetAlpha( CoronaRuntime runtime, int id )
	{
		return runtime.getViewManager().getDisplayObjectAlpha(id);
	}
	
	protected static boolean callDisplayObjectGetBackground( CoronaRuntime runtime, int id )
	{
		return 	runtime.getViewManager().getDisplayObjectBackground(id);
	}

	protected static void callDisplayObjectSetFocus( CoronaRuntime runtime, int id, boolean focus )
	{
		runtime.getViewManager().setTextViewFocus(id, focus);
	}

	protected static boolean callDisplayObjectSetNativeProperty( CoronaRuntime runtime, int id, String key, long luaStateMemoryAddress, int index )
	{
		return runtime.getViewManager().setNativeProperty(id, key, luaStateMemoryAddress, index);
	}

	protected static boolean callRecordStart( CoronaRuntime runtime, String file, long id )
	{
		return runtime.getController().getMediaManager().getAudioRecorder( id ).startRecording( file );
	}
	
	protected static void callRecordStop( long id, CoronaRuntime runtime )
	{
		runtime.getController().getMediaManager().getAudioRecorder( id ).stopRecording();
	}
	
	protected static ByteBuffer callRecordGetBytes( CoronaRuntime runtime, long id )
	{
		AudioByteBufferHolder buffer = runtime.getController().getMediaManager().getAudioRecorder( id ).getNextBuffer();
		if ( buffer != null ) {
			ByteBuffer directBuffer = buffer.myBuffer;
			return directBuffer;
		}
			
		return null;
	}
	
	protected static int callRecordGetCurrentByteCount( CoronaRuntime runtime, long id )
	{
		AudioByteBufferHolder buffer = runtime.getController().getMediaManager().getAudioRecorder( id ).getCurrentBuffer();
		if ( buffer != null )
			return buffer.myValidBytes;
		return 0;
	}
	
	protected static void callRecordReleaseCurrentBuffer( long id, CoronaRuntime runtime )
	{
		runtime.getController().getMediaManager().getAudioRecorder( id ).releaseCurrentBuffer();
	}

	protected static void callWebViewCreate(
		CoronaRuntime runtime, int id, int left, int top, int width, int height, boolean isPopup, boolean autoCloseEnabled)
	{
		runtime.getViewManager().addWebView(id, left, top, width, height, isPopup, autoCloseEnabled);
	}

	protected static void callWebViewRequestLoadUrl( CoronaRuntime runtime, int id, String url ) {
		runtime.getViewManager().requestWebViewLoadUrl(id, url);
	}

	protected static void callWebViewRequestReload(int id, CoronaRuntime runtime) {
		runtime.getViewManager().requestWebViewReload(id);
	}
	
	protected static void callWebViewRequestStop(int id, CoronaRuntime runtime) {
		runtime.getViewManager().requestWebViewStop(id);
	}
	
	protected static void callWebViewRequestGoBack(int id, CoronaRuntime runtime) {
		runtime.getViewManager().requestWebViewGoBack(id);
	}
	
	protected static void callWebViewRequestGoForward(int id, CoronaRuntime runtime) {
		runtime.getViewManager().requestWebViewGoForward(id);
	}

	protected static void callWebViewRequestDeleteCookies(int id, CoronaRuntime runtime) {
		runtime.getViewManager().requestWebViewDeleteCookies(id);
	}

	protected static void callVideoViewCreate(CoronaRuntime runtime, int id, int left, int top, int width, int height)
	{
		runtime.getViewManager().addVideoView(id, left, top, width, height);
	}

	protected static void callVideoViewLoad(CoronaRuntime runtime, int id, String path)
	{
		runtime.getViewManager().videoViewLoad(id, path);
	}

	protected static void callVideoViewPlay(CoronaRuntime runtime, int id)
	{
		runtime.getViewManager().videoViewPlay(id);
	}
	
	protected static void callVideoViewPause(CoronaRuntime runtime, int id)
	{
		runtime.getViewManager().videoViewPause(id);
	}

	protected static void callVideoViewSeek(CoronaRuntime runtime, int id, int seekTo)
	{
		runtime.getViewManager().videoViewSeek(id, seekTo);
	}

	protected static int callVideoViewGetCurrentTime(CoronaRuntime runtime, int id)
	{
		return runtime.getViewManager().videoViewGetCurrentTime(id);
	}

	protected static int callVideoViewGetTotalTime(CoronaRuntime runtime, int id)
	{
		return runtime.getViewManager().videoViewGetTotalTime(id);
	}

	protected static boolean callVideoViewGetIsMuted(CoronaRuntime runtime, int id)
	{
		return runtime.getViewManager().videoViewGetIsMuted(id);
	}

	protected static void callVideoViewMute(CoronaRuntime runtime, int id, boolean mute)
	{
		runtime.getViewManager().videoViewMute(id, mute);
	}

	protected static boolean callVideoViewGetIsTouchTogglesPlay(CoronaRuntime runtime, int id)
	{
		return runtime.getViewManager().videoViewGetIsTouchTogglesPlay(id);
	}

	protected static void callVideoViewTouchTogglesPlay(CoronaRuntime runtime, int id, boolean toggle)
	{
		runtime.getViewManager().videoViewTouchTogglesPlay(id, toggle);
	}

	protected static boolean callVideoViewGetIsPlaying(CoronaRuntime runtime, int id)
	{
		return runtime.getViewManager().videoViewGetIsPlaying(id);
	}

	protected static void callMapViewCreate(CoronaRuntime runtime, int id, int left, int top, int width, int height) {
		runtime.getViewManager().addMapView(id, left, top, width, height);
	}

	protected static boolean callMapViewIsCurrentLocationVisible(CoronaRuntime runtime, int id) {
		return runtime.getViewManager().isCurrentLocationVisibleInMap(id);
	}

	protected static int callMapViewPushCurrentLocationToLua(CoronaRuntime runtime, int id, long luaStateMemoryAddress) {
		return runtime.getViewManager().pushMapCurrentLocationToLua(id, luaStateMemoryAddress);
	}

	protected static boolean callMapViewIsScrollEnabled(CoronaRuntime runtime, int id) {
		return runtime.getViewManager().isMapScrollEnabled(id);
	}

	protected static void callMapViewSetScrollEnabled(CoronaRuntime runtime, int id, boolean enabled) {
		runtime.getViewManager().setMapScrollEnabled(id, enabled);
	}

	protected static boolean callMapViewIsZoomEnabled(CoronaRuntime runtime, int id) {
		return runtime.getViewManager().isMapZoomEnabled(id);
	}

	protected static void callMapViewSetZoomEnabled(CoronaRuntime runtime, int id, boolean enabled) {
		runtime.getViewManager().setMapZoomEnabled(id, enabled);
	}

	protected static String callMapViewGetType(CoronaRuntime runtime, int id) {
		MapType mapType = runtime.getViewManager().getMapType(id);
		if (mapType == null) {
			mapType = MapType.STANDARD;
		}
		return mapType.toInvariantString();
	}

	protected static void callMapViewSetType(CoronaRuntime runtime, int id, String mapTypeName) {
		MapType mapType = MapType.fromInvariantString(mapTypeName);
		if (mapType != null) {
			runtime.getViewManager().setMapType(id, mapType);
		}
	}

	protected static int callMapViewAddMarker(
		CoronaRuntime runtime, int id, double latitude, double longitude, String title, String subtitle, int listener, String imageFile)
	{
		com.ansca.corona.maps.MapMarker mapMarker = new com.ansca.corona.maps.MapMarker(latitude, longitude);
		mapMarker.setTitle(title);
		mapMarker.setSubtitle(subtitle);
		mapMarker.setListener(listener);
		mapMarker.setImageFile(imageFile);
		return runtime.getViewManager().addMapMarker(id, mapMarker);
	}

	protected static void callMapViewRemoveMarker(CoronaRuntime runtime, int id, int markerId)
	{
		runtime.getViewManager().removeMapMarker(id, markerId);
	}

	protected static void callMapViewRemoveAllMarkers(int id, CoronaRuntime runtime) {
		runtime.getViewManager().removeAllMapViewMarkers(id);
	}

	protected static void callMapViewSetCenter(CoronaRuntime runtime, int id, double latitude, double longitude, boolean isAnimated) {
		runtime.getViewManager().setMapCenter(id, latitude, longitude, isAnimated);
	}

	protected static void callMapViewSetRegion(
		CoronaRuntime runtime, int id, double latitude, double longitude, double latitudeSpan, double longitudeSpan, boolean isAnimated)
	{
		runtime.getViewManager().setMapRegion(id, latitude, longitude, latitudeSpan, longitudeSpan, isAnimated);
	}

	protected static int callCryptoGetDigestLength( String algorithm ) {
		return Crypto.GetDigestLength(algorithm);
	}

	protected static byte[] callCryptoCalculateDigest( String algorithm, byte[] data) {
		return Crypto.CalculateDigest(algorithm, data);
	}

	protected static byte[] callCryptoCalculateHMAC( String algorithm, byte[] key, byte[] data ) {
		return Crypto.CalculateHMAC(algorithm, key, data);
	}

	protected static void callRequestNearestAddressFromCoordinates(CoronaRuntime runtime, long luaStateMemoryAddress) {
		// Throw an exception if this application does not have the following permission.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) {
			context.enforceCallingOrSelfPermission(android.Manifest.permission.INTERNET, null);
		}

		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}

		// Using coroutines will give a different lua state than what the runtime has so this is to verify its the same one
		if (luaState == null || CoronaRuntimeProvider.getLuaStateMemoryAddress(luaState) != luaStateMemoryAddress) {
			luaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
		}

		luaState.checkArg(-1, CoronaLua.isListener(luaState, -1, "requestLocation"), "The third arguement of nearestAddress should be a listener.");
		int functionListener = CoronaLua.newRef( luaState, -1 );
		double longitude = luaState.checkNumber(-2);
		double latitude = luaState.checkNumber(-3);
		
		final double latitudeFinal = latitude;
		final double longitudeFinal = longitude;
		final int functionListenerFinal = functionListener;

		final CoronaRuntimeTaskDispatcher dispatcher = new CoronaRuntimeTaskDispatcher( luaState );
		//This is run on a gl thread.  We do this so that we can make non blocking calls
		Thread asyncOperation = new Thread(new Runnable() {
			@Override
			public void run() {
				android.location.Address address = null;
				String errorMessage = null;

				// Fetch the nearest address via the Geocoder.
				try {
					java.util.List<android.location.Address> addresses;
					android.location.Geocoder geocoder;
					geocoder = new android.location.Geocoder(CoronaEnvironment.getApplicationContext());
					addresses = geocoder.getFromLocation(latitudeFinal, longitudeFinal, 1);
					if ((addresses != null) && (addresses.size() > 0)) {
						address = addresses.get(0);
					}
				}
				catch (Exception ex) {
					errorMessage = ex.getMessage();
				}

				if (errorMessage == null) {
					errorMessage = "Address not found for given coordinates.";
				}
				final String errorMessageFinal = errorMessage;
				final android.location.Address addressFinal = address;

				//This is run on the gl thread
				com.ansca.corona.CoronaRuntimeTask task = new com.ansca.corona.CoronaRuntimeTask() {
					@Override
					public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {	
						LuaState L = runtime.getLuaState();
						CoronaLua.newEvent( L, "nearestAddress");
						if (addressFinal != null) {
							try {
								if ( addressFinal.getThoroughfare() != null) {
									L.pushString( addressFinal.getThoroughfare() );
									L.setField( -2, "street" );
								}
								
								if ( addressFinal.getSubThoroughfare() != null) {
									L.pushString( addressFinal.getSubThoroughfare() );
									L.setField( -2, "streetDetail" );
								}
								
								if ( addressFinal.getLocality() != null) {
									L.pushString( addressFinal.getLocality() );
									L.setField( -2, "city" );
								}
								
								if ( addressFinal.getSubLocality() != null) {
									L.pushString( addressFinal.getSubLocality() );
									L.setField( -2, "cityDetail" );
								}
								
								if ( addressFinal.getAdminArea() != null) {
									L.pushString( addressFinal.getAdminArea() );
									L.setField( -2, "region" );
								}
								
								if ( addressFinal.getSubAdminArea() != null) {
									L.pushString( addressFinal.getSubAdminArea() );
									L.setField( -2, "regionDetail" );
								}
								
								if ( addressFinal.getPostalCode() != null) {
									L.pushString( addressFinal.getPostalCode() );
									L.setField( -2, "postalCode" );
								}
								
								if ( addressFinal.getCountryName() != null) {
									L.pushString( addressFinal.getCountryName() );
									L.setField( -2, "country" );
								}
								
								if ( addressFinal.getCountryCode() != null) {
									L.pushString( addressFinal.getCountryCode() );
									L.setField( -2, "countryCode" );
								}
								
								CoronaLua.dispatchEvent( L, functionListenerFinal, 0 );
							}
							catch (Exception ex) {
								ex.printStackTrace();
							}
						} else {
							try {
								L.pushBoolean( true );
								L.setField( -2, CoronaLuaEvent.ISERROR_KEY );

								L.pushString( errorMessageFinal );
								L.setField( -2, "errorMessage" );

								CoronaLua.dispatchEvent( L, functionListenerFinal, 0 );
							} catch (Exception ex) {
								ex.printStackTrace();
							}
						}
						CoronaLua.deleteRef( L, functionListenerFinal);
					}
				};

				dispatcher.send(task);
			}
		});
		asyncOperation.start();
	}

	protected static void callRequestLocationAsync(final CoronaRuntime runtime, long luaStateMemoryAddress) {
		// Throw an exception if this application does not have the following permission.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) {
			context.enforceCallingOrSelfPermission(android.Manifest.permission.INTERNET, null);
		}

		//Get the lua state from the run time if it exists or from the memory address
		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}

		// Using coroutines will give a different lua state than what the runtime has so this is to verify its the same one
		if (luaState == null || CoronaRuntimeProvider.getLuaStateMemoryAddress(luaState) != luaStateMemoryAddress) {
			luaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
		}

		String locationString = null ;
		luaState.checkArg(-1, CoronaLua.isListener(luaState, -1, "mapLocation"), "The third arguement of requestLocation should be a listener.");
		final int functionListenerFinal = CoronaLua.newRef( luaState, -1 );
		final String locationFinal = luaState.checkString(-2);
		
		final CoronaRuntimeTaskDispatcher dispatcher = runtime.getTaskDispatcher();

		//This is run on a third thread.  We do this so that we can make none blocking calls
		Thread asyncOperation = new Thread(new Runnable() {
			@Override
			public void run() {
				Location location = NativeToJavaBridge.getLocationFromName(locationFinal);
				
				if (dispatcher != null) {
					if (location != null) {
						dispatcher.send(new com.ansca.corona.maps.MapRequestLocationTask(functionListenerFinal, location.getLatitude(), location.getLongitude(), locationFinal));
					} else {
						dispatcher.send(new com.ansca.corona.maps.MapRequestLocationFailedTask(functionListenerFinal, "", locationFinal));
					}
				}
			}
		});
		asyncOperation.start();
	}

	/**
	 * Given a location name, pushes the latitude and longitude to the lua stack.  The difference between this and callRequestLocationAsync
	 * is that this is blocking and callRequestLocationAsync isn't blocking
	 */
	protected static int callPushLocationNameCoordinatesToLua(CoronaRuntime runtime, String locationName, long luaStateMemoryAddress) {
		double latitude = 0;
		double longitude = 0;

		// Throw an exception if this application does not have the following permission.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) {
			context.enforceCallingOrSelfPermission(android.Manifest.permission.INTERNET, null);
		}

		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}
		
		// Using coroutines will give a different lua state than what the runtime has so this is to verify its the same one
		if (luaState == null || CoronaRuntimeProvider.getLuaStateMemoryAddress(luaState) != luaStateMemoryAddress) {
			luaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
		}

		Location location = getLocationFromName(locationName);
		if (location != null) {
			latitude = location.getLatitude();
			longitude = location.getLongitude();
		}

		// Push location coordinates to Lua.
		luaState.pushNumber(latitude);
		luaState.pushNumber(longitude);
		return 2;
	}


	/**
	 * Returns a location object with the latitude and longitude set from the supplied location name
	 */
	private static Location getLocationFromName(String locationName) {
		Location locationObject = null;
		if ((locationName != null) && (locationName.length() > 0)) {
			android.net.http.AndroidHttpClient httpClient = null;
			org.apache.http.HttpResponse httpResponse;
			try {
				//The sensor parameter is required but its not clear what effect it has on the request
				String url = "http://maps.googleapis.com/maps/api/geocode/json?address=" +
						java.net.URLEncoder.encode(locationName) + "&sensor=true";
				httpClient = android.net.http.AndroidHttpClient.newInstance("Android");
				httpResponse = httpClient.execute(new org.apache.http.client.methods.HttpGet(url));
				if (httpResponse.getStatusLine().getStatusCode() == org.apache.http.HttpStatus.SC_OK) {
					if (httpResponse.getEntity() != null) {
						String responseMessage = org.apache.http.util.EntityUtils.toString(httpResponse.getEntity());

						//main meat of parsing the json object returned from google
						if ((responseMessage != null) && (responseMessage.length() > 0)) {
							JSONObject response = new JSONObject(responseMessage);
							JSONArray result = response.getJSONArray("results");
							JSONObject result1 = result.getJSONObject(0);
							JSONObject geo = result1.getJSONObject("geometry");
							JSONObject location = geo.getJSONObject("location");
							locationObject = new Location("Google");
							locationObject.setLatitude(location.getDouble("lat"));
							locationObject.setLongitude(location.getDouble("lng"));
						}
					}
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
			finally {
				if (httpClient != null) {
					try { httpClient.close(); }
					catch (Exception ex) { }
				}
			}
		}
		return locationObject;
	}

	protected static void callFlurryInit( String applicationId )
	{
	}

	protected static void callFlurryEvent( String eventId )
	{
	}
	
	protected static void callStoreInit(CoronaRuntime runtime, final String storeName) {
		CoronaStoreApiListener listener = runtime.getController().getCoronaStoreApiListener();
		if (listener != null) {
			listener.storeInit(storeName);
		}
	}
	
	protected static void callStorePurchase(CoronaRuntime runtime, final String productName) {
		CoronaStoreApiListener listener = runtime.getController().getCoronaStoreApiListener();
		if (listener != null) {
			listener.storePurchase(productName);
		}
	}
	
	protected static void callStoreFinishTransaction(CoronaRuntime runtime, final String transactionStringId) {
		CoronaStoreApiListener listener = runtime.getController().getCoronaStoreApiListener();
		if (listener != null) {
			listener.storeFinishTransaction(transactionStringId);
		}
	}
	
	protected static void callStoreRestoreCompletedTransactions(CoronaRuntime runtime) {
		CoronaStoreApiListener listener = runtime.getController().getCoronaStoreApiListener();
		if (listener != null) {
			listener.storeRestore();
		}
	}
	
	protected static String[] callGetAvailableStoreNames() {
		return com.ansca.corona.purchasing.StoreServices.getAvailableInAppStoreNames();
	}

	protected static String callGetTargetedStoreName(CoronaRuntime runtime) {
		return com.ansca.corona.purchasing.StoreServices.getTargetedAppStoreName();
	}

	protected static int callNotificationSchedule(CoronaRuntime runtime, long luaStateMemoryAddress, int luaStackIndex) {
		// Validate.
		if (luaStateMemoryAddress == 0) {
			return 0;
		}

		// Fetch a Lua state object for the given memory address.
		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}

		// Using coroutines will give a different lua state than what the runtime has so this is to verify its the same one
		if (luaState == null || CoronaRuntimeProvider.getLuaStateMemoryAddress(luaState) != luaStateMemoryAddress) {
			luaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
		}

		return notificationSchedule(luaState, luaStackIndex);
	}

	// This function is used in the notifications plugins
	protected static int notificationSchedule(LuaState luaState, int luaStackIndex) {
		// Fetch the end-time argument.
		java.util.Date endTime = null;
		try {
			if (luaState.isTable(luaStackIndex)) {
				// Get the current time in GMT.
				java.util.GregorianCalendar calendarTime = new java.util.GregorianCalendar(
						java.util.TimeZone.getTimeZone("GMT"));

				// Extract date time fields from the Lua table.
				// Will use current time for fields that are missing in the Lua table.
				luaState.getField(luaStackIndex, "year");
				if (luaState.isNumber(-1)) {
					calendarTime.set(java.util.Calendar.YEAR, luaState.toInteger(-1));
				}
				luaState.pop(1);
				luaState.getField(luaStackIndex, "month");
				if (luaState.isNumber(-1)) {
					calendarTime.set(java.util.Calendar.MONTH, luaState.toInteger(-1) - 1);
				}
				luaState.pop(1);
				luaState.getField(luaStackIndex, "day");
				if (luaState.isNumber(-1)) {
					calendarTime.set(java.util.Calendar.DAY_OF_MONTH, luaState.toInteger(-1));
				}
				luaState.pop(1);
				luaState.getField(luaStackIndex, "hour");
				if (luaState.isNumber(-1)) {
					calendarTime.set(java.util.Calendar.HOUR_OF_DAY, luaState.toInteger(-1));
				}
				luaState.pop(1);
				luaState.getField(luaStackIndex, "min");
				if (luaState.isNumber(-1)) {
					calendarTime.set(java.util.Calendar.MINUTE, luaState.toInteger(-1));
				}
				luaState.pop(1);
				luaState.getField(luaStackIndex, "sec");
				if (luaState.isNumber(-1)) {
					calendarTime.set(java.util.Calendar.SECOND, luaState.toInteger(-1));
				}
				luaState.pop(1);

				// Convert the Calendar object to a Date object.
				endTime = calendarTime.getTime();
			}
			else if (luaState.type(luaStackIndex) == com.naef.jnlua.LuaType.NUMBER) {
				// Numeric argument is expected to be a time span in fractional seconds.
				java.util.Date currentTime =  new java.util.Date();
				double fractionalSeconds = luaState.toNumber(luaStackIndex);
				endTime = new java.util.Date(currentTime.getTime() + (long)(fractionalSeconds * 1000.0));
			}
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}

		// Do not continue if an invalid end-time argument was given.
		if (endTime == null) {
			return 0;
		}

		// Get access to the notification system.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		com.ansca.corona.notifications.NotificationServices notificationServices;
		notificationServices = new com.ansca.corona.notifications.NotificationServices(context);

		// Set up a new notification using the information stored in the 2nd argument.
		com.ansca.corona.notifications.ScheduledNotificationSettings settings;
		settings = com.ansca.corona.notifications.ScheduledNotificationSettings.from(
							context, luaState, luaStackIndex + 1);
		settings.setId(notificationServices.reserveId());
		settings.setEndTime(endTime);
		settings.getStatusBarSettings().setTimestamp(endTime);

		// Post the notification.
		notificationServices.post(settings);

		// Return the notification's unique integer ID.
		return settings.getId();
	}

	// This function is used in the notifications plugins
	protected static void callNotificationCancel(int id) {
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		com.ansca.corona.notifications.NotificationServices notificationServices;
		notificationServices = new com.ansca.corona.notifications.NotificationServices(context);
		notificationServices.removeById(id);
	}

	// This function is used in the notifications plugins
	protected static void callNotificationCancelAll(CoronaRuntime runtime) {
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		com.ansca.corona.notifications.NotificationServices notificationServices;
		notificationServices = new com.ansca.corona.notifications.NotificationServices(context);
		notificationServices.removeAll();
	}

	protected static void callGooglePushNotificationsRegister(final CoronaRuntime runtime, String projectNumber) {
		// Validate argument.
		if ((projectNumber == null) || (projectNumber.length() <= 0)) {
			return;
		}

		// Fetch the interface to the Google Cloud Messaging system.
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		com.ansca.corona.notifications.GoogleCloudMessagingServices gcmServices;
		gcmServices = new com.ansca.corona.notifications.GoogleCloudMessagingServices(context);

		// If this application has already been registered, then fetch its registration information.
		String registrationId = gcmServices.getRegistrationId();
		String registeredProjectNumbers = gcmServices.getCommaSeparatedRegisteredProjectNumbers();

		// Do not continue if the given project number(s) has already been registered.
		// Instead, send a "remoteRegistration" event immediately.
		if ((registrationId.length() > 0) && registeredProjectNumbers.equals(projectNumber)) {
			runtime.getTaskDispatcher().send(new com.ansca.corona.events.NotificationRegistrationTask(registrationId));
			return;
		}

		// Register for push notifications.
		// If this is a new project number, then this method will automatically unregister the last project number.
		gcmServices.register(projectNumber);
	}

	protected static void callGoogleSetMapsAPIKey(final CoronaRuntime runtime, String mapsKey) {
		runtime.getController().SetGoogleMapsAPIKey(mapsKey);
	}

	protected static void callGooglePushNotificationsUnregister(CoronaRuntime runtime) {
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		com.ansca.corona.notifications.GoogleCloudMessagingServices gcmServices;
		gcmServices = new com.ansca.corona.notifications.GoogleCloudMessagingServices(context);
		gcmServices.unregister();
	}

	protected static void callFetchInputDevice(int coronaDeviceId, CoronaRuntime runtime) {
		com.ansca.corona.input.InputDeviceServices inputDeviceServices =
					new com.ansca.corona.input.InputDeviceServices(CoronaEnvironment.getApplicationContext());
		com.ansca.corona.input.InputDeviceInterface device =
					inputDeviceServices.fetchByCoronaDeviceId(coronaDeviceId);
		if (device != null) {
			JavaToNativeShim.update(runtime, device);
		}
	}

	protected static void callFetchAllInputDevices(CoronaRuntime runtime) {
		com.ansca.corona.input.InputDeviceServices inputDeviceServices =
					new com.ansca.corona.input.InputDeviceServices(CoronaEnvironment.getApplicationContext());
		for (com.ansca.corona.input.InputDeviceInterface device : inputDeviceServices.fetchAll()) {
			JavaToNativeShim.update(runtime, device);
		}
	}

	protected static void callVibrateInputDevice(int coronaDeviceId, CoronaRuntime runtime) {
		com.ansca.corona.input.InputDeviceServices inputDeviceServices;
		com.ansca.corona.input.InputDeviceInterface inputDeviceInterface;
		android.content.Context context;

		// Fetch the application context.
		context = CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return;
		}

		// Vibrate the specified input device.
		inputDeviceServices = new com.ansca.corona.input.InputDeviceServices(context);
		inputDeviceInterface = inputDeviceServices.fetchByCoronaDeviceId(coronaDeviceId);
		if (inputDeviceInterface != null) {
			inputDeviceInterface.vibrate();
		}
	}

	protected static void callShowSplashScreen(CoronaRuntime runtime) {
		CoronaSplashScreenApiListener listener = runtime.getController().getCoronaSplashScreenApiListener();
		if (listener != null) {
			listener.showSplashScreen();
		}
	}

	protected static void callSetSystemUiVisibility(CoronaRuntime runtime, String visibility) {
		if (runtime != null) {
			runtime.getController().setSystemUiVisibility(visibility);
		}
	}

	protected static String callGetSystemUiVisibility(CoronaRuntime runtime) {
		String systemUIVisibility = null;
		if (runtime != null) {
			systemUIVisibility = runtime.getController().getSystemUiVisibility();
		}
		if (systemUIVisibility == null) {
			systemUIVisibility = "unknown";
		}
		return systemUIVisibility;
	}
	protected static void callSetNavigationBarColor(CoronaRuntime runtime, double red, double green, double blue) {
		if (runtime != null) {
			CoronaEnvironment.getCoronaActivity().setNavigationBarColor(red, green, blue);
		}
	}

	/**
	 * Provides access to API Level 21 (Android 5.0 Lollipop) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * Instead, you are expected to call its static methods instead.
	 */
	private static class ApiLevel21 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel21() {}

		/**
		 * Fetches an ISO 15924 language script string ID from the given locale.
		 * @param locale The local to fetch the script ID from.
		 * @return Returns a string matching an ISO 15924 language script ID.
		 *         <p>
		 *         Returns null if given a null argument.
		 */
		public static String getScriptFrom(java.util.Locale locale) {
			if (locale == null) {
				return null;
			}
			return locale.getScript();
		}
	}

}

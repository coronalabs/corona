//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/**
 * Wrapper for all native calls 
 * 
 * @author eherrman
 */
public class JavaToNativeShim {
	// Corresponds to MPlatformDevice::EventType
	public static final int EventTypeUnknown = -1;
	public static final int EventTypeOrientation = 0;
	public static final int EventTypeAccelerometer = 1;
	public static final int EventTypeGyroscope = 2;
	public static final int EventTypeLocation = 3;
	public static final int EventTypeHeading = 4;
	public static final int EventTypeMultitouch = 5;
	public static final int EventTypeNumTypes = 6;

	private static native String nativeGetVersion();
    private static native String nativeGetBuildId(long bridgeAddress);
    private static native void nativePause(long bridgeAddress);
    private static native void nativeResume(long bridgeAddress);
    private static native void nativeDispatchEventInLua(long bridgeAddress);
    private static native void nativeApplicationOpenEvent(long bridgeAddress);
    private static native long nativeInit(CoronaRuntime runtime);
    private static native void nativeResize(
    			long bridgeAddress, String signature, String documentsDir, String applicationSupportDir, String temporaryDir, String cachesDir,
    			String systemCachesDir, String expansionFileDir, int w, int h, int orientation, boolean isCoronaKit);
    private static native void nativeRender(long bridgeAddress);
    private static native void nativeReinitializeRenderer(long bridgeAddress);
    private static native String nativeGetKeyNameFromKeyCode(int keyCode);
    private static native int nativeGetMaxTextureSize(long bridgeAddress);
	private static native int nativeGetHorizontalMarginInPixels(long bridgeAddress);
	private static native int nativeGetVerticalMarginInPixels(long bridgeAddress);
	private static native int nativeGetContentWidthInPixels(long bridgeAddress);
	private static native int nativeGetContentHeightInPixels(long bridgeAddress);
	private static native Object nativeConvertCoronaPointToAndroidPoint(long bridgeAddress, int x, int y );
	private static native void nativeUseDefaultLuaErrorHandler();
	private static native void nativeUseJavaLuaErrorHandler();
    private static native void nativeUnloadResources(long bridgeAddress);
    private static native void nativeDone(long bridgeAddress);
    private static native boolean nativeCopyBitmapInfo(
    			long bridgeAddress, long nativeImageMemoryAddress, int width, int height,
    			float downscaleFactor, int rotationInDegrees);
    private static native boolean nativeCopyBitmap(
    			long bridgeAddress, long nativeImageMemoryAddress, android.graphics.Bitmap bitmap,
    			float downscaleFactor, int rotationInDegrees, boolean convertToGrayscale);
    private static native void nativeSetZipFileEntryInfo(
    			long zipFileEntryMemoryAddress, String packageFilePath, String entryName,
    			long byteOffsetInPackage, long byteCountInPackage, boolean isCompressed);
    private static native void nativeUpdateInputDevice(
    			long bridgeAddress, int coronaDeviceId, int androidDeviceId, int deviceType, String permanentStringId,
    			String productName, String displayName, boolean canVibrate, int playerNumber, int connectionStateId);
    private static native void nativeClearInputDeviceAxes(long bridgeAddress, int coronaDeviceId);
    private static native void nativeAddInputDeviceAxis(
    			long bridgeAddress, int coronaDeviceId, int axisTypeId, float minValue, float maxValue,
    			float accuracy, boolean isAbsolute);
    private static native void nativeInputDeviceStatusEvent(
    			long bridgeAddress, int coronaDeviceId, boolean hasConnectionStateChanged, boolean wasReconfigured);
    private static native void nativeTapEvent( long bridgeAddress, int x, int y, int count );
    private static native void nativeTouchEvent( long bridgeAddress, int x, int y, int xStart, int yStart, int type, long timestamp, int id, float pressure );
    private static native void nativeMouseEvent(
    			long bridgeAddress, int x, int y, int scrollX, int scrollY, long timestamp,
    			boolean isPrimaryButtonDown, boolean isSecondaryButtonDown, boolean isMiddleButtonDown);
    private static native boolean nativeKeyEvent(
    			long bridgeAddress, int coronaDeviceId, int phase, int keyCode,
    			boolean isShiftDown, boolean isAltDown, boolean isCtrlDown, boolean isCommandDown);
    private static native void nativeAxisEvent( long bridgeAddress, int coronaDeviceId, int axisIndex, float rawValue );
    private static native void nativeAccelerometerEvent( long bridgeAddress, double x, double y, double z, double deltaTime );
    private static native void nativeGyroscopeEvent( long bridgeAddress, double x, double y, double z, double deltaTime );
    private static native void nativeLocationEvent( long bridgeAddress, double latitude, double longitude, double altitude, double accuracy, double speed, double bearing, double time );
    private static native void nativeOrientationChanged( long bridgeAddress, int newOrientation, int oldOrientation );
    private static native void nativeResizeEvent( long bridgeAddress );
    private static native void nativeAlertCallback( long bridgeAddress, int buttonIndex, boolean cancelled );
    private static native void nativeSoundEndCallback( long bridgeAddress, long id );
    private static native void nativeVideoEndCallback( long bridgeAddress, long id );
    private static native void nativeRecordCallback( long bridgeAddress, long id, int status );
    private static native void nativeSetAudioRecorderState( long bridgeAddress, long id, boolean isRecording );
    private static native void nativeTextEvent( long bridgeAddress, int id, boolean focusLost, boolean isDone );
	private static native void nativeTextEditingEvent( long bridgeAddress, int id, int startPos, int numDeleted, String newCharacters, String oldString, String newString );
    private static native void nativeMultitouchEventBegin(long bridgeAddress);
    private static native void nativeMultitouchEventAdd( long bridgeAddress, int xLast, int yLast, int xStart, int yStart, int phaseType, long timestamp, int id, float pressure );
    private static native void nativeMultitouchEventEnd( long bridgeAddress );
    private static native void nativeWebViewShouldLoadUrl( long bridgeAddress, int id, String url, int sourceType );
    private static native void nativeWebViewFinishedLoadUrl( long bridgeAddress, int id, String url );
    private static native void nativeWebViewDidFailLoadUrl( long bridgeAddress, int id, String url, String msg, int code );
    private static native void nativeWebViewHistoryUpdated( long bridgeAddress, int id, boolean canGoBack, boolean canGoForward );
    private static native void nativeWebViewClosed( long bridgeAddress, int id );
    private static native void nativeImagePickerEvent( long bridgeAddress, String selectedImageFileName );
	private static native void nativeAbortShowingImageProvider( long bridgeAddress );
	private static native void nativeVideoPickerEvent( long bridgeAddress, String selectedVideoFileName, int duration, long size );
	private static native void nativeAbortShowingVideoProvider( long bridgeAddress );
	private static native void nativeMemoryWarningEvent( long bridgeAddress );
	private static native void nativePopupClosedEvent( long bridgeAddress, String popupName, boolean isError );
	private static native void nativeStoreTransactionEvent(
									long bridgeAddress, int state, int errorType, String errorMessage, String productId, String signature,
									String receipt, String transactionId, String transactionTime,
									String originalReceipt, String originalTransactionId, String originalTransactionTime );
	private static native void nativeMapAddressReceivedEvent(
									long bridgeAddress, String street, String streetDetails, String city, String cityDetails,
									String region, String regionDetails, String postalCode,
									String country, String countryDetails );
	private static native void nativeMapMarkerEvent( long bridgeAddress, int markerId, int listenerId, double latitude, double longitude );
	private static native void nativeMapAddressRequestFailedEvent( long bridgeAddress, String errorMessage );
	private static native void nativeMapRequestLocationEvent( long bridgeAddress, int listener, double latitude, double longitude, String originalRequest );
	private static native void nativeMapRequestLocationFailedEvent( long bridgeAddress, int listener, String errorMessage, String originalRequest );
	private static native void nativeMapTappedEvent( long bridgeAddress, int id, double latitude, double longitude );
	private static native void nativeVideoViewPrepared( long bridgeAddress, int id );
	private static native void nativeVideoViewEnded( long bridgeAddress, int id );
	private static native void nativeVideoViewFailed( long bridgeAddress, int id );
	private static native Object nativeGetCoronaRuntime( long bridgeAddress );

	// Load all C/C++ libraries and their dependencies.
	// Note: Loading a library will NOT automatically load its dependencies. We must do that explicitly here.
    static {
		System.loadLibrary("lua");
		System.loadLibrary("jnlua5.1");
		System.loadLibrary("openal");
		// Certain products include this library(Enterprise, SDK) while other products don't(Cards) so it might not exist
		try {
			System.loadLibrary("mpg123");
		} catch (Exception e) {
		} catch (UnsatisfiedLinkError e) {}

		System.loadLibrary("almixer");
		System.loadLibrary("corona");
    }
	
	/** Constructor made private to prevent instances of this class from being made. */
	private JavaToNativeShim() {}
	
    public static String getVersion()
	{
		return nativeGetVersion();
	}

	public static void pause(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativePause(runtime.getJavaToNativeBridgeAddress());
	}

	public static String getBuildId(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return "unknown";
		}
		String ret = nativeGetBuildId(runtime.getJavaToNativeBridgeAddress());
		if (ret == null) {
			ret = "unknown";
		}
		return ret;
	}

	public static void resume(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeResume(runtime.getJavaToNativeBridgeAddress());
	}

	public static void dispatchEventInLua(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeDispatchEventInLua(runtime.getJavaToNativeBridgeAddress());
	}

	public static void applicationOpenEvent(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeApplicationOpenEvent(runtime.getJavaToNativeBridgeAddress());
	}

	public static void init(CoronaRuntime runtime)
	{
		if (runtime != null) {
			long javaToNativeBridgePointer = nativeInit(runtime);
			runtime.setJavaToNativeBridgeAddress(javaToNativeBridgePointer);
		}

	}

	public static void render(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeRender(runtime.getJavaToNativeBridgeAddress());
	}

	public static void reinitializeRenderer(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeReinitializeRenderer(runtime.getJavaToNativeBridgeAddress());
	}

	public static String getKeyNameFromKeyCode(int keyCode)
	{
		return nativeGetKeyNameFromKeyCode(keyCode);
	}
	
	public static int getMaxTextureSize(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return 0;
		}
		return nativeGetMaxTextureSize(runtime.getJavaToNativeBridgeAddress());
	}
	
	public static int getHorizontalMarginInPixels(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return 0;
		}
		return nativeGetHorizontalMarginInPixels(runtime.getJavaToNativeBridgeAddress());
	}
	
	public static int getVerticalMarginInPixels(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return 0;
		}
		return nativeGetVerticalMarginInPixels(runtime.getJavaToNativeBridgeAddress());
	}
	
	public static int getContentWidthInPixels(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return 0;
		}
		return nativeGetContentWidthInPixels(runtime.getJavaToNativeBridgeAddress());
	}
	
	public static int getContentHeightInPixels(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return 0;
		}
		return nativeGetContentHeightInPixels(runtime.getJavaToNativeBridgeAddress());
	}

	public static android.graphics.Point convertCoronaPointToAndroidPoint(CoronaRuntime runtime, int x, int y )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return null;
		}
		android.graphics.Point result = null;

		Object o = nativeConvertCoronaPointToAndroidPoint( runtime.getJavaToNativeBridgeAddress(), x, y );

		if ( null != o && o instanceof android.graphics.Point ) {
			result = (android.graphics.Point)o;
		}

		return result;
	}

	public static void useDefaultLuaErrorHandler()
	{
		nativeUseDefaultLuaErrorHandler();
	}
	
	public static void useJavaLuaErrorHandler()
	{
		nativeUseJavaLuaErrorHandler();
	}
	
	public static void unloadResources(CoronaRuntime runtime)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeUnloadResources(runtime.getJavaToNativeBridgeAddress());
	}
	
	public static void destroy(CoronaRuntime runtime)
	{
		if (runtime == null) {
			return;
		}
		nativeDone(runtime.getJavaToNativeBridgeAddress());
	}

	public static boolean copyBitmapInfo(
		CoronaRuntime runtime, long nativeImageMemoryAddress, int width, int height, float downscaleFactor, int rotationInDegrees)
	{
		if (nativeImageMemoryAddress == 0) {
			return false;
		}
		if (runtime == null || runtime.wasDisposed()) {
			return false;
		}
		return nativeCopyBitmapInfo(runtime.getJavaToNativeBridgeAddress(), nativeImageMemoryAddress, width, height, downscaleFactor, rotationInDegrees);
	}

	public static boolean copyBitmap(
		CoronaRuntime runtime, long nativeImageMemoryAddress, android.graphics.Bitmap bitmap,
		float downscaleFactor, int rotationInDegrees, boolean convertToGrayscale)
	{
		// Validate.
		if ((nativeImageMemoryAddress == 0) || (bitmap == null)) {
			return false;
		}

		if (runtime == null || runtime.wasDisposed()) {
			return false;
		}

		// Have the native C/C++ side copy the Java bitmap's pixels.
		return nativeCopyBitmap( runtime.getJavaToNativeBridgeAddress(),
					nativeImageMemoryAddress, bitmap, downscaleFactor, rotationInDegrees, convertToGrayscale);
	}

	public static void setZipFileEntryInfo(
		long zipFileEntryMemoryAddress, com.ansca.corona.storage.AssetFileLocationInfo info)
	{
		// Validate.
		if ((zipFileEntryMemoryAddress == 0) || (info == null)) {
			return;
		}
		// Write the given information to the C++ ZipFileEntry object.
		String packageFilePath = null;
		if (info.getPackageFile() != null) {
			packageFilePath = info.getPackageFile().getAbsolutePath();
		}
		nativeSetZipFileEntryInfo(
					zipFileEntryMemoryAddress, packageFilePath, info.getZipEntryName(),
					info.getByteOffsetInPackage(), info.getByteCountInPackage(), info.isCompressed());
	}

	public static void update(CoronaRuntime runtime, com.ansca.corona.input.InputDeviceInterface device) {
		// Validate.
		if (device == null) {
			return;
		}
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}

		// Update the input device's information.
		nativeUpdateInputDevice(
					runtime.getJavaToNativeBridgeAddress(),
					device.getCoronaDeviceId(), device.getDeviceInfo().getAndroidDeviceId(),
					device.getDeviceInfo().getType().toCoronaIntegerId(),
					device.getDeviceInfo().getPermanentStringId(),
					device.getDeviceInfo().getProductName(), device.getDeviceInfo().getDisplayName(),
					device.getDeviceInfo().canVibrate(), device.getDeviceInfo().getPlayerNumber(),
					device.getConnectionState().toCoronaIntegerId());

		// Update the input device's axis information.
		nativeClearInputDeviceAxes(runtime.getJavaToNativeBridgeAddress(),device.getCoronaDeviceId());
		for (com.ansca.corona.input.AxisInfo axisInfo : device.getDeviceInfo().getAxes()) {
			if (axisInfo != null) {
				nativeAddInputDeviceAxis(runtime.getJavaToNativeBridgeAddress(),
						device.getCoronaDeviceId(), axisInfo.getType().toCoronaIntegerId(),
						axisInfo.getMinValue(), axisInfo.getMaxValue(), axisInfo.getAccuracy(),
						axisInfo.isProvidingAbsoluteValues());
			}
		}
	}
	
	public static void inputDeviceStatusEvent(
		CoronaRuntime runtime, 
		com.ansca.corona.input.InputDeviceInterface device,
		com.ansca.corona.input.InputDeviceStatusEventInfo eventInfo)
	{
		if ((device == null) || (eventInfo == null)) {
			return;
		}
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeInputDeviceStatusEvent(runtime.getJavaToNativeBridgeAddress(),
				device.getCoronaDeviceId(), eventInfo.hasConnectionStateChanged(), eventInfo.wasReconfigured());
	}
	
	public static void resize(
		CoronaRuntime runtime, android.content.Context context, int width, int height, WindowOrientation orientation, boolean isCoronaKit)
	{
		com.ansca.corona.storage.FileServices fileServices = new com.ansca.corona.storage.FileServices(context);

		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeResize(runtime.getJavaToNativeBridgeAddress(),
				context.getPackageName(),
				CoronaEnvironment.getDocumentsDirectory(context).getAbsolutePath(),
				CoronaEnvironment.getApplicationSupportDirectory(context).getAbsolutePath(),
				CoronaEnvironment.getTemporaryDirectory(context).getAbsolutePath(),
				CoronaEnvironment.getCachesDirectory(context).getAbsolutePath(),
				CoronaEnvironment.getInternalCachesDirectory(context).getAbsolutePath(),
				fileServices.getExpansionFileDirectory().getAbsolutePath(),
				width, height, orientation.toCoronaIntegerId(), isCoronaKit);
	}

	public static void tapEvent( CoronaRuntime runtime, int x, int y, int count )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeTapEvent( runtime.getJavaToNativeBridgeAddress(), x, y, count );
	}
	
	public static void mouseEvent(
		CoronaRuntime runtime, int x, int y, int scrollX, int scrollY, long timestamp,
		boolean isPrimaryButtonDown, boolean isSecondaryButtonDown, boolean isMiddleButtonDown)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMouseEvent(
				runtime.getJavaToNativeBridgeAddress(),
				x, y, scrollX, scrollY, timestamp,
				isPrimaryButtonDown, isSecondaryButtonDown, isMiddleButtonDown);
	}
	
	public static void touchEvent(CoronaRuntime runtime, com.ansca.corona.input.TouchTracker touchTracker) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeTouchEvent(runtime.getJavaToNativeBridgeAddress(), 
				(int)touchTracker.getLastPoint().getX(),
				(int)touchTracker.getLastPoint().getY(),
				(int)touchTracker.getStartPoint().getX(),
				(int)touchTracker.getStartPoint().getY(),
				touchTracker.getPhase().toCoronaNumericId(),
				touchTracker.getLastPoint().getTimestamp(),
				touchTracker.getTouchId(),
				touchTracker.getPressure());
	}
	
	public static void multitouchEventBegin(CoronaRuntime runtime) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMultitouchEventBegin(runtime.getJavaToNativeBridgeAddress());
	}

	public static void multitouchEventAdd(CoronaRuntime runtime, com.ansca.corona.input.TouchTracker touchTracker) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMultitouchEventAdd(runtime.getJavaToNativeBridgeAddress(), 
				(int)touchTracker.getLastPoint().getX(),
				(int)touchTracker.getLastPoint().getY(),
				(int)touchTracker.getStartPoint().getX(),
				(int)touchTracker.getStartPoint().getY(),
				touchTracker.getPhase().toCoronaNumericId(),
				touchTracker.getLastPoint().getTimestamp(),
				touchTracker.getTouchId(),
				touchTracker.getPressure());
	}

	public static void multitouchEventEnd(CoronaRuntime runtime) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMultitouchEventEnd(runtime.getJavaToNativeBridgeAddress());
	}

	public static boolean keyEvent(
		CoronaRuntime runtime, 
		com.ansca.corona.input.InputDeviceInterface device,
		com.ansca.corona.input.KeyPhase phase, int keyCode,
		boolean isShiftDown, boolean isAltDown, boolean isCtrlDown, boolean isCommandDown)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return false;
		}
		int coronaDeviceId = (device != null) ? device.getCoronaDeviceId() : 0;
		boolean wasHandled = nativeKeyEvent(
			runtime.getJavaToNativeBridgeAddress(), coronaDeviceId, phase.toCoronaNumericId(), keyCode,
			isShiftDown, isAltDown, isCtrlDown, isCommandDown);
		return wasHandled;
	}

	public static void axisEvent(
		CoronaRuntime runtime, com.ansca.corona.input.InputDeviceInterface device, com.ansca.corona.input.AxisDataEventInfo eventInfo)
	{
		if ((device == null) || (eventInfo == null)) {
			return;
		}
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		int coronaDeviceId = (device != null) ? device.getCoronaDeviceId() : 0;
		nativeAxisEvent(runtime.getJavaToNativeBridgeAddress(), coronaDeviceId, eventInfo.getAxisIndex(), eventInfo.getDataPoint().getValue());
	}
	
	public static void accelerometerEvent( CoronaRuntime runtime, double x, double y, double z, double deltaTime )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeAccelerometerEvent( runtime.getJavaToNativeBridgeAddress(), x, y, z, deltaTime );
	}
	
	public static void gyroscopeEvent( CoronaRuntime runtime, double x, double y, double z, double deltaTime )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeGyroscopeEvent( runtime.getJavaToNativeBridgeAddress(), x, y, z, deltaTime );
	}
	
	public static void locationEvent( CoronaRuntime runtime, double latitude, double longitude, double altitude, double accuracy, double speed, double bearing, double time )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeLocationEvent( runtime.getJavaToNativeBridgeAddress(), latitude, longitude, altitude, accuracy, speed, bearing, time );
	}
	
	public static void orientationChanged( CoronaRuntime runtime, int newOrientation, int oldOrientation ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeOrientationChanged( runtime.getJavaToNativeBridgeAddress(), newOrientation, oldOrientation );
	}

	public static void resizeEvent( CoronaRuntime runtime) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeResizeEvent(runtime.getJavaToNativeBridgeAddress());
	}

	public static void alertCallback( CoronaRuntime runtime, int buttonIndex, boolean cancelled ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeAlertCallback( runtime.getJavaToNativeBridgeAddress(), buttonIndex, cancelled );
	}
	
	public static void soundEndCallback( CoronaRuntime runtime, long id ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeSoundEndCallback( runtime.getJavaToNativeBridgeAddress(), id );
	}
	
	public static void videoEndCallback( CoronaRuntime runtime, long id ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeVideoEndCallback( runtime.getJavaToNativeBridgeAddress(), id );
	}
	
	public static void recordCallback( CoronaRuntime runtime, long id, int status ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeRecordCallback( runtime.getJavaToNativeBridgeAddress(), id, status );
	}

	public static void setAudioRecorderState( CoronaRuntime runtime, long id, boolean isRecording ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeSetAudioRecorderState( runtime.getJavaToNativeBridgeAddress(), id, isRecording );
	}
	
	public static void textEvent( CoronaRuntime runtime, int id, boolean hasFocus, boolean isDone ) {
		if (runtime == null || runtime.wasDisposed() || !runtime.getViewManager().hasDisplayObjectWithId(id)) {
			return;
		}
		nativeTextEvent(runtime.getJavaToNativeBridgeAddress(), id, hasFocus, isDone);
	}
	
	public static void textEditingEvent( CoronaRuntime runtime, int id, int startPos, int numDeleted, String newCharacters, String oldString, String newString ) {
		if (runtime == null || runtime.wasDisposed() || !runtime.getViewManager().hasDisplayObjectWithId(id)) {
			return;
		}
		nativeTextEditingEvent(runtime.getJavaToNativeBridgeAddress(), id, startPos, numDeleted, newCharacters, oldString, newString);
	}
	
	public static void webViewShouldLoadUrl( CoronaRuntime runtime, int id, String url, int sourceType ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeWebViewShouldLoadUrl( runtime.getJavaToNativeBridgeAddress(), id, url, sourceType );
	}
	
	public static void webViewFinishedLoadUrl( CoronaRuntime runtime, int id, String url ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeWebViewFinishedLoadUrl( runtime.getJavaToNativeBridgeAddress(), id, url );
	}
	
	public static void webViewDidFailLoadUrl( CoronaRuntime runtime, int id, String url, String msg, int code ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
    	nativeWebViewDidFailLoadUrl( runtime.getJavaToNativeBridgeAddress(), id, url, msg, code );
	}
	
	public static void webViewHistoryUpdated( CoronaRuntime runtime, int id, boolean canGoBack, boolean canGoForward ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeWebViewHistoryUpdated( runtime.getJavaToNativeBridgeAddress(), id, canGoBack, canGoForward );
	}
	
	public static void webViewClosed( CoronaRuntime runtime, int id ) {
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeWebViewClosed( runtime.getJavaToNativeBridgeAddress(), id );
	}
	
	public static void imagePickerEvent( CoronaRuntime runtime, String selectedImageFileName )
	{
		if (selectedImageFileName == null) {
			selectedImageFileName = "";
		}
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeImagePickerEvent( runtime.getJavaToNativeBridgeAddress(), selectedImageFileName );
	}

	public static void abortShowingImageProvider( CoronaRuntime runtime )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeAbortShowingImageProvider(runtime.getJavaToNativeBridgeAddress());
	}

	/**
	 * Creates and sends a video pick event on the native side.
	 * @param runtime context
	 * @param selectedVideoFileName The selected video file.  It can be a path or a uri.
	 * @param duration The number of seconds of the video in milliseconds.  The event created will be in seconds.
	 * @param size The size of the video in bytes.
	 */
	public static void videoPickerEvent( CoronaRuntime runtime, String selectedVideoFileName, int duration, long size )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		if (selectedVideoFileName == null) {
			selectedVideoFileName = "";
		}
		duration = (int)duration/1000;
		nativeVideoPickerEvent( runtime.getJavaToNativeBridgeAddress(), selectedVideoFileName, duration, size );
	}

	public static void abortShowingVideoProvider( CoronaRuntime runtime )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeAbortShowingVideoProvider(runtime.getJavaToNativeBridgeAddress());
	}
	
	public static void memoryWarningEvent( CoronaRuntime runtime )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMemoryWarningEvent(runtime.getJavaToNativeBridgeAddress());
	}
	
	public static void popupClosedEvent( CoronaRuntime runtime, String popupName, boolean wasCanceled )
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativePopupClosedEvent( runtime.getJavaToNativeBridgeAddress(), popupName, wasCanceled );
	}
	
	public static void storeTransactionEvent(
		CoronaRuntime runtime, 
		com.ansca.corona.purchasing.StoreTransactionResultSettings result)
	{
		// Validate.
		if (result == null) {
			return;
		}
		
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		// Convert transaction timestamps to localized strings.
		String transactionTimeString = "";
		String originalTransactionTimeString = "";
		if (result.hasTransactionTime()) {
			transactionTimeString = result.getTransactionTime().toString();
		}
		if (result.hasOriginalTransactionTime()) {
			originalTransactionTimeString = result.getOriginalTransactionTime().toString();
		}
		
		// Send the transaction result info to the native side of Corona.
		nativeStoreTransactionEvent(runtime.getJavaToNativeBridgeAddress(), 
				result.getState().toValue(), result.getErrorType().toValue(), result.getErrorMessage(),
				result.getProductName(), result.getSignature(), result.getReceipt(), result.getTransactionStringId(),
				transactionTimeString, result.getOriginalReceipt(), result.getOriginalTransactionStringId(),
				originalTransactionTimeString);
	}

	public static void mapAddressReceivedEvent(
		CoronaRuntime runtime, String street, String streetDetails, String city, String cityDetails, String region, String regionDetails,
		String postalCode, String country, String countryCode)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMapAddressReceivedEvent(runtime.getJavaToNativeBridgeAddress(), 
				street, streetDetails, city, cityDetails, region, regionDetails,
				postalCode, country, countryCode);
	}

	public static void mapAddressRequestFailedEvent(CoronaRuntime runtime, String errorMessage)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMapAddressRequestFailedEvent(runtime.getJavaToNativeBridgeAddress(), errorMessage);
	}

	public static void mapRequestLocationEvent(CoronaRuntime runtime, int listener, double latitude, double longitude, String originalRequest)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMapRequestLocationEvent(runtime.getJavaToNativeBridgeAddress(), listener, latitude, longitude, originalRequest);
	}

	public static void mapRequestLocationFailed(CoronaRuntime runtime, int listener, String errorMsg, String originalRequest)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMapRequestLocationFailedEvent(runtime.getJavaToNativeBridgeAddress(), listener, errorMsg, originalRequest);
	}

	public static void mapMarkerEvent(CoronaRuntime runtime, int markerId, int listenerId, double latitude, double longitude)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMapMarkerEvent(runtime.getJavaToNativeBridgeAddress(), markerId, listenerId, latitude, longitude);
	}

	public static void mapTappedEvent(CoronaRuntime runtime, int id, double latitude, double longitude)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeMapTappedEvent(runtime.getJavaToNativeBridgeAddress(), id, latitude, longitude);
	}
	

	public static void videoViewPrepared(CoronaRuntime runtime, int id)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeVideoViewPrepared(runtime.getJavaToNativeBridgeAddress(), id);
	}

	public static void videoViewEnded(CoronaRuntime runtime, int id)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeVideoViewEnded(runtime.getJavaToNativeBridgeAddress(), id);
	}

	public static void videoViewFailed(CoronaRuntime runtime, int id)
	{
		if (runtime == null || runtime.wasDisposed()) {
			return;
		}
		nativeVideoViewFailed(runtime.getJavaToNativeBridgeAddress(), id);
	}

	public static CoronaRuntime getCoronaRuntimeFromBridge(long address) {
		return (CoronaRuntime)nativeGetCoronaRuntime(address);
	}
}

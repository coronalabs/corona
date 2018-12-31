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

#ifndef _NativeToJavaBridge_H__
#define _NativeToJavaBridge_H__

#include <vector>

#include <jni.h>
#include "Core/Rtt_Data.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Array.h"
#include "Core/Rtt_OperationResult.h"
#include "Rtt_Preference.h"
#include <map>

namespace Rtt
{
	class AndroidPlatform;
	class LuaResource;
	class PlatformBitmap;
	class PreferenceCollection;
	class Runtime;
	typedef std::map<std::string, Real> FontMetricsMap;
};
struct lua_State;
class AndroidImageData;
class AndroidZipFileEntry;


class NativeToJavaBridge
{
	public:
		typedef void* DictionaryRef;
		typedef jint jpointer;

		NativeToJavaBridge(JavaVM * vm, Rtt::Runtime *runtime, jobject coronaRuntime);

	protected:
		static JNIEnv * GetJNIEnv();
		jobject GetCallbackBridge() const;
		void GetString( const char *method, Rtt::String *outValue );
		void GetStringWithInt( const char *method, int param, Rtt::String *outValue );
		void HandleJavaException() const;
		void HandleJavaExceptionUsing( lua_State *L ) const;
		void CallVoidMethod( const char * method ) const;
		void CallIntMethod( const char * method, int param ) const;
		void CallLongMethod( const char * method, long param ) const;
		void CallDoubleMethod( const char * method, double param ) const;
		void CallFloatMethod( const char * method, float param ) const;
		void CallStringMethod( const char * method, const char * param ) const;
	
	public:
		static void* JavaToNative( jpointer p );
		static NativeToJavaBridge * InitInstance( JNIEnv *env, Rtt::Runtime *runtime, jobject coronaRuntime );
		// static NativeToJaaBridge *GetInstance();
		static DictionaryRef DictionaryCreate( lua_State *L, int t, NativeToJavaBridge *bridge );

		Rtt::Runtime * GetRuntime() const;
		void SetRuntime(Rtt::Runtime *runtime);
		Rtt::AndroidPlatform * GetPlatform() const;
		// bool HasLuaErrorOccurred() const { return fHasLuaErrorOccurred; }
		DictionaryRef DictionaryCreate();
		void DictionaryDestroy( DictionaryRef dict );
	
		bool RequestSystem( lua_State *L, const char *actionName, int optionsIndex );
		void Ping();
		
		int LoadClass( lua_State *L, const char *libName, const char *className );
		int LoadFile( lua_State *L, const char *fileName );
		void OnRuntimeLoaded(lua_State *L);
		void OnRuntimeWillLoadMain();
		void OnRuntimeStarted();
		void OnRuntimeSuspended();
		void OnRuntimeResumed();
		void OnRuntimeExiting();
		
		static void OnAudioEnabled();

		static int InvokeLuaErrorHandler(lua_State *L);

		void PushLaunchArgumentsToLuaTable(lua_State *L);
		void PushApplicationOpenArgumentsToLuaTable(lua_State *L);
	
		static bool GetRawAsset( const char * assetName, Rtt::Data<char> & result );
		bool GetRawAssetExists( const char * assetName );
		bool GetCoronaResourceFileExists( const char * assetName );
		static bool GetAssetFileLocation( const char * assetName, AndroidZipFileEntry &zipFileEntry );
		bool LoadImage(
				const char *filePath, AndroidImageData& imageData, bool convertToGrayscale,
				int maxWidth, int maxHeight, bool loadImageInfoOnly);
		bool SaveImageToPhotoLibrary( const char *fileName );
		bool SaveBitmap( const Rtt::PlatformBitmap * bitmap, const char * path, int quality );
	
		bool RenderText(
				const char *text, const char *fontName, float fontSize, bool isBold,
				int wrapWidth, int clipWidth, int clipHeight, const char *alignment, AndroidImageData &result, Rtt::Real &baselineOffset);
		bool GetFonts( Rtt::StringArray & outFonts );

		Rtt::FontMetricsMap GetFontMetrics( const char *fontName, float fontSize, bool isBold );
		void SetTimer( int milliseconds );
		void CancelTimer();
	
		void LoadSound( int id, const char * name, bool eventSound );
		void PlaySound( int id, const char * name, bool loop );
		void StopSound( int id );
		void PauseSound( int id );
		void ResumeSound( int id );
		void SetVolume( int id, float volume );
		void SoundEndCallback( int id );
		float GetVolume( int id ) const;
	
		void HttpPost( const char* url, const char* key, const char* value );
	
		void PlayVideo( int id, const char * url, bool mediaControlsEnabled );
		void VideoEndCallback( int id );
	
		bool CanOpenUrl( const char* url );
		bool OpenUrl( const char * url );
		
		void SetIdleTimer( bool enabled );
		bool GetIdleTimer() const;
	
		void SetStatusBarMode( int mode );
		int GetStatusBarMode();
		int GetStatusBarHeight();
		void GetSafeAreaInsetsPixels(Rtt::Real &top, Rtt::Real &left, Rtt::Real &bottom, Rtt::Real &right);
		
		void ShowNativeAlert( const char * title, const char * msg, 
			const char ** labels, int numLabels, Rtt::LuaResource * resource );
		void CancelNativeAlert( int which );
		void AlertCallback( int which, bool cancelled );
		void ShowTrialAlert();
		void ShowNativeActivityIndicator();
		void CloseNativeActivityIndicator();
		bool HasMediaSource( int mediaSourceType );
		bool HasAccessToMediaSource( int mediaSourceType );
		void ShowImagePicker( int imageSourceType, const char *destinationFilePath );
		void ShowVideoPicker( int videoSourceType, int maxTime, int quality );
		bool CanShowPopup( const char *name );
		void ShowSendMailPopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource );
		void ShowSendSmsPopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource );
		bool ShowAppStorePopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource );
		void ShowRequestPermissionsPopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource );
		void RaisePopupClosedEvent( const char *popupName, bool wasCanceled );
	
		void DisplayUpdate();
	
		void GetManufacturerName( Rtt::String *outValue );
		void GetModel( Rtt::String *outValue );
		void GetName( Rtt::String *outValue );
		void GetUniqueIdentifier( int t, Rtt::String *outValue );
		void GetPlatformVersion( Rtt::String *outValue );
		void GetProductName( Rtt::String *outValue );
		float GetDefaultFontSize();
		int GetDefaultTextFieldPaddingInPixels();
		int GetApproximateScreenDpi();
		int PushSystemInfoToLua( lua_State *L, const char *key );
		void GetPreference( int category, Rtt::String *outValue );
		Rtt::Preference::ReadValueResult GetPreference( const char* keyName );
		Rtt::OperationResult SetPreferences( const Rtt::PreferenceCollection& collection );
		Rtt::OperationResult DeletePreferences( const char** keyNameArray, size_t keyNameCount );
		void GetSystemProperty( const char *name, Rtt::String *outValue );
		long GetUptimeInMilliseconds();
		void MakeLowerCase( Rtt::String *stringToConvert );
	
		void SetLocationAccuracy( double meters );
		void SetLocationThreshold( double meters );
		void SetAccelerometerInterval( int frequency );
		void SetGyroscopeInterval( int frequency );
		void SetEventNotification( int eventType, bool enable );
		bool HasAccelerometer();
		bool HasGyroscope();
		bool HasHeadingHardware();
		void Vibrate();
		
		void DisplayObjectDestroy( int id );
		void DisplayObjectSetVisible( int id, bool visible );
		bool DisplayObjectGetVisible( int id );
		void DisplayObjectSetAlpha( int id, float alpha );
		float DisplayObjectGetAlpha( int id );
		void DisplayObjectSetBackground( int id, bool bg );
		bool DisplayObjectGetBackground( int id );
		void DisplayObjectSetFocus( int id, bool focus );
		void DisplayObjectUpdateScreenBounds( int id, int x, int y, int width, int height );
			
		int TextFieldCreate( int id, int left, int top, int width, int height, int isSingleLine );
		void TextFieldSetReturnKey( int id, const char * imeType );
		void TextFieldSetSelection( int id, int startPosition, int endPosition );
		void TextFieldSetPlaceholder( int id, const char * placeholder );
		void TextFieldSetColor( int id, int r, int g, int b, int a );
		void TextFieldSetText( int id, const char * text );
		void TextFieldSetSize( int id, float fontSize );
		void TextFieldSetFont( int id, const char * fontName, float fontSize, bool isBold );
		void TextFieldSetAlign( int id, const char * align );
		void TextFieldSetSecure( int id, bool isSecure );
		void TextFieldSetInputType( int id, const char * inputType );
		void TextFieldSetEditable( int id, bool isEditable );
		void TextFieldGetColor( int id, int * r, int * g, int * b, int * a );
		void TextFieldGetText( int id, Rtt::String * text );
		void TextFieldGetPlaceholder( int id, Rtt::String * placeholder );
		float TextFieldGetSize( int id );
		void TextFieldGetFont( int id, Rtt::String * font, float * fontSize );
		void TextFieldGetAlign( int id, Rtt::String * align );
		bool TextFieldGetSecure( int id );
		void TextFieldGetInputType( int id, Rtt::String * inputType );
		bool TextFieldIsSingleLine( int id );
		bool TextFieldIsEditable( int id );
	
	public:
		bool RecordStart( int id, const char * file );
		void RecordStop( int id );
		bool RecordGetBytes( int id, Rtt::Data<char> & result );
		void RecordCallback( int id, int status );
		void RecordReleaseCurrentBuffer( int id );
		
		void WebViewCreate( int id, int left, int top, int width, int height, bool isPopup, bool autoCancelEnabled );
		void WebViewRequestLoadUrl( int id, const char * url );
		void WebViewRequestReload( int id );
		void WebViewRequestStop( int id );
		void WebViewRequestGoBack( int id );
		void WebViewRequestGoForward( int id );
		void WebViewRequestDeleteCookies( int id );
		bool WebPopupShouldLoadUrl( int id, const char * url );
		bool WebPopupDidFailLoadUrl( int id, const char * url, const char * msg, int code );

		void VideoViewCreate( int id, int left, int top, int width, int height );
		void VideoViewLoad( int id, const char * source );
		void VideoViewPlay( int id );
		void VideoViewPause( int id );
		void VideoViewSeek( int id, int seekTo );
		int VideoViewGetCurrentTime( int id );
		int VideoViewGetTotalTime( int id );
		bool VideoViewGetIsMuted( int id );
		void VideoViewMute( int id, bool mute);
		bool VideoViewGetIsTouchTogglesPlay( int id );
		void VideoViewTouchTogglesPlay( int id, bool toggle );
		bool VideoViewGetIsPlaying( int id );

		void MapViewCreate( int id, int left, int top, int width, int height );
		bool MapViewIsCurrentLocationVisible( int id );
		int MapViewPushCurrentLocationToLua( int id, lua_State *L );
		bool MapViewIsScrollEnabled( int id );
		void MapViewSetScrollEnabled( int id, bool enabled );
		bool MapViewIsZoomEnabled( int id );
		void MapViewSetZoomEnabled( int id, bool enabled );
		void MapViewGetType( int id, Rtt::String *mapTypeResult );
		void MapViewSetType( int id, const char *mapTypeName );
		void MapViewSetCenter( int id, double latitude, double longitude, bool isAnimated );
		void MapViewSetRegion( int id, double latitude, double longitude, double latitudeSpan, double longitudeSpan, bool isAnimated );
		int MapViewAddMarker( int id, double latitude, double longitude, const char *title, const char *subtitle, int listener, const char *fileNameWithPath );
		void MapViewRemoveMarker( int id, int markerId );
		void MapViewRemoveAllMarkers( int id );
		
		int CryptoGetDigestLength( const char * algorithm );
		void CryptoCalculateDigest( const char * algorithm, const Rtt::Data<const char> & data, U8 * digest );
		void CryptoCalculateHMAC( const char * algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, 
			U8 * digest );
	
		void ExternalizeResource( const char * assetName, Rtt::String * result );

		void ShowSplashScreen();
	
	#ifdef Rtt_FLURRY
		void FlurryInit( const char * applicationKey ) const;
		void FlurryEvent( const char * eventId ) const;
	#endif
	
	public:
		void RequestNearestAddressFromCoordinates( lua_State *L );
		int PushLocationNameCoordinatesToLua( const char *locationName, lua_State *L );
		void RequestLocationAsync( lua_State *L);

	public:
		void StoreInit( const char *storeName );
		void StorePurchase( const char *productName );
		void StoreFinishTransaction( const char *transactionStringId );
		void StoreRestoreCompletedTransactions();
		void GetAvailableStoreNames( Rtt::PtrArray<Rtt::String> &storeNames );
		void GetTargetedStoreName( Rtt::String *outValue );

	public:
		int NotificationSchedule( lua_State *L, int index );
		void NotificationCancel( int id );
		void NotificationCancelAll();
		void GooglePushNotificationsRegister( const char *projectNumber );
		void GooglePushNotificationsUnregister();
		void SetGoogleMapsAPIKey( const char *mapsKey );

	public:
		void FetchInputDevice(int coronaDeviceId);
		void FetchAllInputDevices();
		void VibrateInputDevice(int coronaDeviceId);

	public:
		void SetSystemUiVisibility( const char *visibility );
		void GetSystemUiVisibility( Rtt::String * align );

	public:
		static bool DecodeBase64( const Rtt::Data<const char> & payload, Rtt::Data<char> & data );
		static bool Check(const Rtt::Data<const char> & publicKey, const Rtt::Data<const char> & signature, const Rtt::Data<const char> & payloadData);
		static void getAudioOutputSettings(std::vector<int>& settings);

	private:
		static JavaVM *fVM;
		Rtt::Runtime *fRuntime;
		// bool fHasLuaErrorOccurred;
		Rtt::LuaResource *fAlertCallbackResource;
		Rtt::LuaResource *fPopupClosedEventResource;
		jobject fCoronaRuntime;
};


// ----------------------------------------------------------------------------

#endif // _NativeToJavaBridge_H__

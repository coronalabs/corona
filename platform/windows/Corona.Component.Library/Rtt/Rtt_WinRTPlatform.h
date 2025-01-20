//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoronaLabs\WinRT\Utf8String.h"
#include "Rtt_WinRTDevice.h"
#include "Rtt_WinRTCrypto.h"
#include <collection.h>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_MPlatform.h"
#	include "Core\Rtt_String.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs
{
	namespace Corona
	{
		namespace WinRT
		{
			ref class CoronaRuntimeEnvironment;
		}
	}
}
namespace Rtt
{
	class WinRTAudioPlayer;
	class WinRTImageProvider;
	class WinRTStoreProvider;
	class WinRTVideoPlayer;
	class WinRTVideoProvider;
	class WinRTWebPopup;
	class PlatformBitmap;
	class PlatformSurface;
	class PlatformTimer;
	class RenderingStream;
}

#pragma endregion


namespace Rtt
{

class WinRTPlatform : public MPlatform
{
	Rtt_CLASS_NO_COPIES( WinRTPlatform )

	public:
		typedef WinRTPlatform Self;

	public:
		WinRTPlatform(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment);
		virtual ~WinRTPlatform();

	public:
		virtual MPlatformDevice& GetDevice() const;
		virtual PlatformSurface* CreateScreenSurface() const;
		virtual PlatformSurface* CreateOffscreenSurface( const PlatformSurface& parent ) const;

    public:
        virtual Rtt_Allocator& GetAllocator() const;
        virtual RenderingStream* CreateRenderingStream( bool antialias ) const;
        virtual PlatformTimer* CreateTimerWithCallback( MCallback& callback ) const;
        virtual PlatformBitmap* CreateBitmap(const char* utf8FilePath, bool convertToGrayscale) const;
        virtual PlatformBitmap* CreateBitmapMask(
					const char text[], const PlatformFont& font, Real width, Real height, const char alignmentName[]) const;

        virtual void HttpPost( const char* url, const char* key, const char* value ) const;

        virtual PlatformEventSound * CreateEventSound( const ResourceHandle<lua_State> & handle, const char* filePath ) const;
        virtual void ReleaseEventSound( PlatformEventSound * soundID ) const;
        virtual void PlayEventSound( PlatformEventSound * soundID ) const;

        virtual PlatformAudioPlayer* GetAudioPlayer( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformAudioRecorder * CreateAudioRecorder(const Rtt::ResourceHandle<lua_State> & handle, const char * filePath) const;

		virtual const MCrypto& GetCrypto() const;
		virtual void GetPreference( Category category, Rtt::String * value ) const;
		virtual Preference::ReadValueResult GetPreference(const char* categoryName, const char* keyName) const;
		virtual OperationResult SetPreferences(const char* categoryName, const PreferenceCollection& collection) const;
		virtual OperationResult DeletePreferences(const char* categoryName, const char** keyNameArray, U32 keyNameCount) const;

		virtual void SetActivityIndicator( bool visible ) const;

		virtual PlatformWebPopup* GetWebPopup() const;

		virtual bool CanShowPopup( const char *name ) const;
		virtual bool ShowPopup( lua_State *L, const char *name, int optionsIndex ) const;
		virtual bool HidePopup( const char *name ) const;

		virtual PlatformDisplayObject* CreateNativeTextBox( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeTextField( const Rect& bounds ) const;
		virtual void SetKeyboardFocus( PlatformDisplayObject *textObject ) const;

		virtual PlatformDisplayObject* CreateNativeMapView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeWebView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeVideo( const Rect& bounds ) const;

		PlatformDisplayObject* GetNativeDisplayObjectById( const int objectId ) const;

		virtual Rtt_Real GetStandardFontSize() const;
		virtual S32 GetFontNames( lua_State *L, int index ) const;
		virtual PlatformFont* CreateFont( PlatformFont::SystemFont fontType, Rtt_Real size ) const;

		// Returns NULL if fontName is NULL;
		virtual PlatformFont* CreateFont( const char *fontName, Rtt_Real size ) const;

		virtual void SetTapDelay( Rtt_Real delay ) const;
		virtual Rtt_Real GetTapDelay() const;

		virtual PlatformFBConnect* GetFBConnect() const;

	public:
        virtual void RaiseError( MPlatform::Error e, const char* reason ) const;
        virtual void PathForFile(const char *filename, MPlatform::Directory baseDir, U32 flags, String &result) const;
		virtual MPlatform::Directory GetBaseDirectoryFromPath(const char *path) const;
		virtual bool FileExists( const char *filename ) const;
		virtual bool ValidateAssetFile(const char *assetFilename, const int assetSize) const;
		virtual void SetProjectResourceDirectory(const char *directoryPath);
		virtual void SetSkinResourceDirectory(const char *directoryPath);

		virtual int SetSync( lua_State* L ) const;
		virtual int GetSync( lua_State* L ) const;

	public:
        virtual void BeginRuntime( const Runtime& runtime ) const;
        virtual void EndRuntime( const Runtime& runtime ) const;

		virtual PlatformExitCallback* GetExitCallback();
		virtual bool RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const;
		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;

	public:
        virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool SaveBitmap(PlatformBitmap* bitmap, const char* filePath, float jpegQuality) const;
		virtual bool AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const;
		virtual bool OpenURL( const char* url ) const;
		virtual int CanOpenURL( const char* url ) const;

		virtual PlatformVideoPlayer* GetVideoPlayer(const Rtt::ResourceHandle<lua_State> & handle) const;
		virtual PlatformImageProvider* GetImageProvider(const Rtt::ResourceHandle<lua_State> & handle) const;
		virtual PlatformVideoProvider* GetVideoProvider(const Rtt::ResourceHandle<lua_State> & handle) const;
		virtual PlatformStoreProvider* GetStoreProvider( const ResourceHandle<lua_State>& handle ) const;

		virtual void SetStatusBarMode( MPlatform::StatusBarMode newValue ) const;
		virtual MPlatform::StatusBarMode GetStatusBarMode() const;
		virtual int GetStatusBarHeight() const;
    
        virtual int GetTopStatusBarHeightPixels()  const;
        virtual int GetBottomStatusBarHeightPixels() const;

		virtual void SetIdleTimer( bool enabled ) const;
		virtual bool GetIdleTimer() const;

		virtual NativeAlertRef ShowNativeAlert(
			const char *title,
			const char *msg,
			const char **buttonLabels,
			U32 numButtons,
			LuaResource* resource ) const;
		virtual void CancelNativeAlert( NativeAlertRef alert, S32 index ) const;

		virtual void Suspend( ) const;
		virtual void Resume( ) const;

	public:
		virtual void* CreateAndScheduleNotification( lua_State *L, int index ) const;
		virtual void ReleaseNotification( void *notificationId ) const;
		virtual void CancelNotification( void *notificationId ) const;

		virtual void FlurryInit(const char * applicationKey) const;
		virtual void FlurryEvent(const char * eventId) const;

	public:
		virtual void SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const;
		virtual int PushNativeProperty( lua_State *L, const char *key ) const;
		virtual int PushSystemInfo( lua_State *L, const char *key ) const;

	protected:
		void NetworkBaseRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *path ) const;

	public:
		virtual void NetworkRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex ) const;
		virtual void NetworkDownload( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *filename, MPlatform::Directory baseDir ) const;

		virtual PlatformReachability* NewReachability( const ResourceHandle<lua_State>& handle, PlatformReachability::PlatformReachabilityType type, const char* address ) const;
		virtual bool SupportsNetworkStatus() const;

	private:
		Rtt_Allocator* fAllocator;
		CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ fEnvironment;
		Platform::Array<CoronaLabs::WinRT::Utf8String^>^ fBaseDirectoryPaths;
		WinRTDevice fDevice;
		mutable WinRTAudioPlayer* fAudioPlayer;
		mutable WinRTVideoPlayer* fVideoPlayer;
		mutable WinRTImageProvider* fImageProvider;
		mutable WinRTVideoProvider* fVideoProvider;
		mutable WinRTWebPopup *fWebPopup;
		WinRTCrypto fCrypto;
		mutable PlatformStoreProvider *fStoreProvider;
		mutable PlatformFBConnect *fFBConnect;
};

} // namespace Rtt

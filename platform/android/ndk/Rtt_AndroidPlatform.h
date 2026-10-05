//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidPlatform_H__
#define _Rtt_AndroidPlatform_H__

#include "Rtt_AndroidDevice.h"
#include "Rtt_MPlatform.h"
#include "Rtt_AndroidCrypto.h"
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

class AndroidDisplayObjectRegistry;
class AndroidGLView;
class NativeToJavaBridge;

namespace Rtt
{

class AndroidAudioPlayer;
class AndroidImageProvider;
class AndroidStoreProvider;
class AndroidVideoPlayer;
class AndroidVideoProvider;
class AndroidWebPopup;
class PlatformBitmap;
class PlatformSurface;
class PlatformTimer;
class RenderingStream;

// ----------------------------------------------------------------------------

class AndroidPlatform : public MPlatform
{
	Rtt_CLASS_NO_COPIES( AndroidPlatform )

	public:
		typedef AndroidPlatform Self;

	public:
		AndroidPlatform(
				AndroidGLView * pView, const char * package, const char * documentsDir, const char * applicationSupportDir,
				const char * temporaryDir, const char * cachesDir, const char * systemCachesDir, const char * expansionFileDir,
				NativeToJavaBridge *ntjb);
		~AndroidPlatform();

		const String & GetPackage() const 
		{ 
			return fPackage; 
		}

	public:
		virtual MPlatformDevice& GetDevice() const;
		virtual PlatformSurface* CreateScreenSurface() const;
		virtual PlatformSurface* CreateOffscreenSurface( const PlatformSurface& parent ) const;

    public:
        virtual Rtt_Allocator& GetAllocator() const;
        virtual RenderingStream* CreateRenderingStream( bool antialias ) const;
        virtual PlatformTimer* CreateTimerWithCallback( MCallback& callback ) const;
        virtual PlatformBitmap* CreateBitmap( const char* filename, bool convertToGrayscale ) const;
        virtual PlatformBitmap* CreateBitmapMask( const char str[], const PlatformFont& font, Real w, Real h, const char alignment[] , Real & baselineOffset ) const;

        virtual PlatformEventSound * CreateEventSound( const ResourceHandle<lua_State> & handle, const char* filePath ) const;
		 virtual FontMetricsMap GetFontMetrics( const PlatformFont& font ) const;
        virtual void ReleaseEventSound( PlatformEventSound * soundID ) const;
        virtual void PlayEventSound( PlatformEventSound * soundID ) const;

        virtual PlatformAudioPlayer* GetAudioPlayer( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformAudioRecorder * CreateAudioRecorder(const Rtt::ResourceHandle<lua_State> & handle, const char * filePath) const;

		virtual const MCrypto& GetCrypto() const;
		virtual void GetPreference( Category category, Rtt::String * value ) const;
		virtual Preference::ReadValueResult GetPreference( const char* categoryName, const char* keyName ) const;
		virtual OperationResult SetPreferences( const char* categoryName, const PreferenceCollection& collection ) const;
		virtual OperationResult DeletePreferences( const char* categoryName, const char** keyNameArray, U32 keyNameCount ) const;

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
        virtual void PathForFile( const char *filename, MPlatform::Directory baseDir, U32 flags, String& result ) const;
		virtual bool FileExists( const char *filename ) const;
		virtual bool ValidateAssetFile(const char *assetFilename, const int assetSize) const;

		virtual int SetSync( lua_State* L ) const;
		virtual int GetSync( lua_State* L ) const;

	protected:
		void PathForResourceFile( const char* filename, String & result ) const;
		void PathForPluginFile( const char * filename, String & result ) const;
		void PathForFile( const char* filename, const char * baseDir, String & result ) const;

	public:
        virtual void BeginRuntime( const Runtime& runtime ) const;
        virtual void EndRuntime( const Runtime& runtime ) const;

		virtual PlatformExitCallback* GetExitCallback();
		virtual bool RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const;
		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;
		virtual void SetProjectResourceDirectory( const char* filename );
		virtual void SetSkinResourceDirectory( const char* filename );

    protected:
        char* CopyString( const char* src, bool useAllocator = true ) const;

	public:
        virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const;
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

		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const;

	public:
		virtual void* CreateAndScheduleNotification( lua_State *L, int index ) const;
		virtual void ReleaseNotification( void *notificationId ) const;
		virtual void CancelNotification( void *notificationId ) const;

#ifdef Rtt_FLURRY
		virtual void FlurryInit( const char * applicationKey ) const;
		virtual void FlurryEvent( const char * eventId ) const;
#endif

	public:
		virtual void SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const;
		virtual int PushNativeProperty( lua_State *L, const char *key ) const;
		virtual int PushSystemInfo( lua_State *L, const char *key ) const;

		virtual NativeToJavaBridge* GetNativeToJavaBridge() const;

		void Suspend( ) const;
		void Resume( ) const;

	protected:
        Rtt_Allocator* fAllocator;
        mutable AndroidAudioPlayer* fAudioPlayer;
//        mutable id fHttpPostDelegate;

	private:
		AndroidGLView* fView;
		AndroidDevice fDevice;
		mutable AndroidVideoPlayer* fVideoPlayer;
		mutable AndroidImageProvider* fImageProvider;
		mutable AndroidVideoProvider* fVideoProvider;
//		AlertViewDelegate *fDelegate;
		mutable AndroidWebPopup *fWebPopup;
		String fPackage;
		String fDocumentsDir;
		String fApplicationSupportDir;
		String fTemporaryDir;
		String fCachesDir;
		String fSystemCachesDir;
		String fExpansionFileDir;
		AndroidCrypto fCrypto;
		mutable PlatformStoreProvider *fStoreProvider;
		AndroidDisplayObjectRegistry *fDisplayObjectRegistry;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidPlatform_H__

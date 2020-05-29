//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacPlatform_H__
#define _Rtt_MacPlatform_H__

#include "Rtt_ApplePlatform.h"
#include "Rtt_MPlatformServices.h"

#include "Rtt_MacDevice.h"
#import <AppKit/NSView.h>
#import <AppKit/NSOpenGLView.h>
#include <Security/Authorization.h>

#include <sys/types.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

// ----------------------------------------------------------------------------

@class AlertDelegate;
@class GLView;

extern NSString* const kOpenFolderPath;
extern NSString* const kBuildFolderPath;
extern NSString* const kKeyStoreFolderPath;
extern NSString* const kKeyStoreFolderPathAndFile;
extern NSString* const kDstFolderPath;
extern NSString* const kImageFolderPath;
extern NSString* const kVideoFolderPath;
extern NSString* const kDidAgreeToLicense;
extern NSString* const kUserPreferenceUsersCurrentSelectedSkin;
extern NSString* const kUserPreferenceCustomBuildID;
extern NSString* const kUserPreferenceDoNotUseSkinnedWindows;
extern NSString* const kUserPreferenceScaleFactorForSkin;
extern NSString* const kUserPreferenceLastIOSCertificate;
extern NSString* const kUserPreferenceLastTVOSCertificate;
extern NSString* const kUserPreferenceLastOSXCertificate;
extern NSString* const kUserPreferenceLastAndroidKeyAlias;
extern NSString* const kUserPreferenceLastAndroidKeystore;
extern NSString* const kUserPreferenceLastAndroidTargetStore;

namespace Rtt
{

class MacConsoleDevice;
class MacFBConnect;
class MacWebPopup;
class MacVideoPlayer;
class MacViewSurface;
class MacActivityIndicator;
class PlatformSimulator;
class PlatformSurface;

// ----------------------------------------------------------------------------

class MacPlatform : public ApplePlatform
{
	public:
		typedef ApplePlatform Super;

	public:
		MacPlatform(CoronaView *view);
		virtual ~MacPlatform();

	public:
		void Initialize( GLView* pView );

		void SetResourcePath( const char resourcePath[] );

		NSString* GetSandboxPath() const { return fSandboxPath; }

		BOOL IsFilenameCaseCorrect(const char *filename, NSString *path) const;


	public:
		virtual MPlatformDevice& GetDevice() const;
		virtual PlatformSurface* CreateScreenSurface() const;

	public:
		static struct CGImage* CreateMacImage( Rtt_Allocator* pAllocator, PlatformBitmap& bitmap );

	protected:
		virtual bool SaveBitmap( PlatformBitmap* bitmap, NSURL* url ) const;
#ifdef Rtt_AUTHORING_SIMULATOR
		ValueResult<Rtt::SharedConstStdStringPtr> GetSimulatedAppPreferenceKeyFor(const char* keyName) const;
#endif

	public:
        virtual void GetPreference( Category category, Rtt::String * value ) const;
		virtual Preference::ReadValueResult GetPreference( const char* categoryName, const char* keyName ) const;
		virtual OperationResult SetPreferences( const char* categoryName, const PreferenceCollection& collection ) const;
		virtual OperationResult DeletePreferences( const char* categoryName, const char** keyNameArray, U32 keyNameCount ) const;
		virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const;
		virtual bool AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const;
		virtual bool OpenURL( const char* url ) const;
		virtual int CanOpenURL( const char* url ) const;

		virtual PlatformVideoPlayer * GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformImageProvider * GetImageProvider( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformVideoProvider * GetVideoProvider( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformStoreProvider* GetStoreProvider( const ResourceHandle<lua_State>& handle ) const;

		virtual void SetStatusBarMode( StatusBarMode newValue ) const;
		virtual StatusBarMode GetStatusBarMode() const;

		virtual void SetIdleTimer( bool enabled ) const;
		virtual bool GetIdleTimer() const;

		virtual NativeAlertRef ShowNativeAlert(
			const char *title,
			const char *msg,
			const char **buttonLabels,
			U32 numButtons,
			LuaResource* resource ) const;
		virtual void CancelNativeAlert( NativeAlertRef alert, S32 index ) const;

		virtual void SetActivityIndicator( bool visible ) const;

		virtual PlatformWebPopup* GetWebPopup() const;

		virtual PlatformDisplayObject* CreateNativeTextBox( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeTextField( const Rect& bounds ) const;
		virtual void SetKeyboardFocus( PlatformDisplayObject *textField ) const;

		virtual PlatformDisplayObject* CreateNativeMapView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeWebView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeVideo( const Rect& bounds ) const;
        virtual PlatformFBConnect* GetFBConnect() const;

		virtual S32 GetFontNames( lua_State *L, int index ) const;

		virtual void SetTapDelay( Rtt_Real delay ) const;
		virtual Rtt_Real GetTapDelay() const;

		virtual int PushSystemInfo( lua_State *L, const char *key ) const;

		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const;

	public:
		virtual NSString *PathForResourceFile( const char *filename ) const;

	protected:
		virtual NSString *PathForDocumentsFile( const char *filename ) const;
		virtual NSString *CachesParentDir() const;
		virtual NSString *PathForTmpFile( const char *filename ) const;
		virtual NSString *PathForPluginsFile( const char *filename ) const;
        virtual NSString *PathForProjectResourceFile( const char* filename ) const;
        virtual NSString *PathForSkinResourceFile( const char* filename ) const;
		virtual NSString *PathForApplicationSupportFile( const char* filename ) const;
		virtual NSString *PathForFile( const char* filename, NSString* baseDir ) const;



	public:
		virtual void BeginRuntime( const Runtime& runtime ) const;
		virtual void EndRuntime( const Runtime& runtime ) const;

		virtual PlatformExitCallback* GetExitCallback();
		virtual bool RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const;
		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;
        virtual void SetProjectResourceDirectory( const char* filename );
        virtual void SetSkinResourceDirectory( const char* filename );
#ifdef Rtt_AUTHORING_SIMULATOR
        virtual void SetCursorForRect(const char *cursorName, int x, int y, int width, int height) const;
#endif
		virtual void SetNativeProperty(lua_State *L, const char *key, int valueIndex) const;
		virtual int PushNativeProperty( lua_State *L, const char *key ) const;

		virtual void Suspend( ) const;
		virtual void Resume( ) const;

	public:
		MacConsoleDevice& GetMacDevice() const { return static_cast< MacConsoleDevice& >( GetDevice() ); }		
        GLView* GetView() const { return fView; }

	private:
		GLView* fView;
		NSString *fSandboxPath;
		MacConsoleDevice fDevice;
		mutable pthread_mutex_t fMutex;
		mutable int fMutexCount;
		AlertDelegate *fDelegate;
		mutable MacWebPopup *fWebPopup;
		mutable MacVideoPlayer *fVideoPlayer;
		mutable MacActivityIndicator* fActivityIndicator;
#if Rtt_AUTHORING_SIMULATOR
		mutable MacFBConnect *fFBConnect;
#endif // Rtr_AUTHORING_SIMULATOR	
		mutable PlatformStoreProvider *fStoreProvider;
		PlatformExitCallback* fExitCallback;
		mutable IOPMAssertionID fAssertionID;
};

// TODO: Move this to a separate file
#if !defined( Rtt_WEB_PLUGIN )

class MacGUIPlatform : public MacPlatform
{
	public:
		typedef MacPlatform Super;

	public:
		MacGUIPlatform( PlatformSimulator& simulator );

	public:
		virtual MPlatformDevice& GetDevice() const;
		virtual PlatformSurface* CreateScreenSurface() const;

	public:
		virtual void SetStatusBarMode( StatusBarMode newValue ) const;
		virtual StatusBarMode GetStatusBarMode() const;
		virtual Real GetStandardFontSize() const;
		virtual bool RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const;

	public:
		void SetAdaptiveWidth( S32 newValue ) { fAdaptiveWidth = newValue; }
		void SetAdaptiveHeight( S32 newValue ) { fAdaptiveHeight = newValue; }

	private:
		MacDevice fMacDevice;
		S32 fAdaptiveWidth;
		S32 fAdaptiveHeight;
};

// ----------------------------------------------------------------------------

class MacAppPlatform : public MacPlatform
{
	public:
		typedef MacPlatform Super;

	public:
		MacAppPlatform( DeviceOrientation::Type orientation );

	public:
		virtual MPlatformDevice& GetDevice() const;

	private:
		MacAppDevice fMacAppDevice;
};

// ----------------------------------------------------------------------------

	
#ifdef Rtt_AUTHORING_SIMULATOR

class MacPlatformServices : public MPlatformServices
{
	Rtt_CLASS_NO_COPIES( MacPlatformServices )

	public:
		typedef MacPlatformServices Self;

	public:
		MacPlatformServices( const MPlatform& platform );

	protected:
		bool RequestAdminAuthorization( const char *name ) const;

	public:
		// MPlatformServices
		virtual const MPlatform& Platform() const;
		virtual void GetPreference( const char *key, Rtt::String * value ) const;
		virtual void SetPreference( const char *key, const char *value ) const;
//		virtual bool SetGlobalPreference( const char *key, const char *value ) const;
		virtual void GetSecurePreference( const char *key, Rtt::String * value ) const;
		virtual bool SetSecurePreference( const char *key, const char *value ) const;
		virtual bool IsInternetAvailable() const;
		virtual bool IsLocalWifiAvailable() const;
		virtual void Terminate() const;

		virtual void GetLibraryPreference( const char *key, Rtt::String * value ) const;
		virtual void SetLibraryPreference( const char *key, const char *value ) const;

		virtual void Sleep( int milliseconds ) const;

	private:
		const MPlatform& fPlatform;
//		mutable AuthorizationRef fAdminAuthorization;
};

#endif // Rtt_AUTHORING_SIMULATOR
	
// ----------------------------------------------------------------------------

#endif // Rtt_WEB_PLUGIN

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacPlatform_H__

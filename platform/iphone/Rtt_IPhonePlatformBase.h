//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhonePlatformBase_H__
#define _Rtt_IPhonePlatformBase_H__

#include "Rtt_ApplePlatform.h"

#define Rtt_IPHONE_PLATFORM_STUB 1

// ----------------------------------------------------------------------------

@class AlertViewDelegate;
@class CoronaView;
@class UIViewController;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// This is a base class implementation of MPlatform.
// It's a shared implementation used by both iOS and tvOS.
// 
// NOTE: Only functionality common to both iOS/tvOS should be added here.
class IPhonePlatformBase : public ApplePlatform
{
	Rtt_CLASS_NO_COPIES( IPhonePlatformBase )

	public:
		typedef ApplePlatform Super;

	public:
		IPhonePlatformBase( CoronaView *view );
		virtual ~IPhonePlatformBase();

	public:
		/// virtual MPlatformDevice& GetDevice() const;
		CoronaView* GetView() const { return fView; }
		virtual PlatformSurface* CreateScreenSurface() const;

	public:
		PlatformTimer* CreateTimerWithCallback( MCallback& callback ) const;

	public:
		virtual bool OpenURL( const char* url ) const;
		virtual int CanOpenURL( const char* url ) const;

		virtual void SetIdleTimer( bool enabled ) const;
		virtual bool GetIdleTimer() const;

		virtual NativeAlertRef ShowNativeAlert(
			const char *title,
			const char *msg,
			const char **buttonLabels,
			U32 numButtons,
			LuaResource* resource ) const;
		virtual void CancelNativeAlert( NativeAlertRef alert, S32 index ) const;

		virtual PlatformDisplayObject* CreateNativeTextBox( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeTextField( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeVideo( const Rect& bounds ) const;

		virtual Real GetStandardFontSize() const;
		virtual S32 GetFontNames( lua_State *L, int index ) const;

		virtual int PushSystemInfo( lua_State *L, const char *key ) const;

		virtual NSString *PathForPluginsFile( const char *filename ) const;

#if Rtt_IPHONE_PLATFORM_STUB
		virtual bool SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const;
		virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const;
		virtual PlatformVideoPlayer* GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformImageProvider* GetImageProvider( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformVideoProvider* GetVideoProvider( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformStoreProvider* GetStoreProvider( const ResourceHandle<lua_State>& handle ) const;

		virtual void SetStatusBarMode( StatusBarMode newValue ) const;
		virtual StatusBarMode GetStatusBarMode() const;

		virtual void SetActivityIndicator( bool visible ) const;

		virtual PlatformWebPopup* GetWebPopup() const;

		virtual void SetKeyboardFocus( PlatformDisplayObject *textObject ) const;
		
		virtual PlatformDisplayObject* CreateNativeMapView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeWebView( const Rect& bounds ) const;

		virtual void SetTapDelay( Rtt_Real delay ) const;
		virtual Rtt_Real GetTapDelay() const;

		virtual PlatformFBConnect* GetFBConnect() const;

		virtual void* CreateAndScheduleNotification( lua_State *L, int index ) const;
		virtual void ReleaseNotification( void *notificationId ) const;
		virtual void CancelNotification( void *notificationId ) const;

		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;

		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const;
#endif

	private:
		CoronaView *fView;
		AlertViewDelegate *fDelegate;

		// Caches the state of controllerUserInteraction. We set this manually when we display popups, but do not want
		// the user to lose their current setting.
		mutable BOOL fCachedControllerUserInteractionStatus;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhonePlatformBase_H__

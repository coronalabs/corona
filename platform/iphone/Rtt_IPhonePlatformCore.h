//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhonePlatformCore_H__
#define _Rtt_IPhonePlatformCore_H__

#include "Rtt_IPhonePlatformBase.h"

#include "Rtt_IPhoneDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class IPhoneImageProvider;
class IPhoneVideoProvider;
class IPhoneVideoPlayer;

// ----------------------------------------------------------------------------

// This is the default implementation of MPlatform for use on iOS devices.
// This is used by CoronaCards.
// (Corona/Native uses the subclass iPhonePlatform).
class IPhonePlatformCore : public IPhonePlatformBase
{
	Rtt_CLASS_NO_COPIES( IPhonePlatformCore )

	public:
		// TODO: Update this to actually use the classes instead of doing a runtime check.
		// This should match apple's UIUserNotificationType enums.
		typedef enum Rtt_UIUserNotificationType  : NSUInteger  {
			Rtt_UIUserNotificationTypeNone     = 0,
			Rtt_UIUserNotificationTypeBadge    = 1 << 0,
			Rtt_UIUserNotificationTypeSound    = 1 << 1,
			Rtt_UIUserNotificationTypeAlert    = 1 << 2,
		} Rtt_UIUserNotificationType;
	
	public:
		typedef IPhonePlatformBase Super;

	public:
		IPhonePlatformCore( CoronaView *view );
		virtual ~IPhonePlatformCore();

	public:
		virtual MPlatformDevice& GetDevice() const;

	public:
		virtual int GetStatusBarHeight() const;
		virtual int GetTopStatusBarHeightPixels() const;
		virtual int GetBottomStatusBarHeightPixels() const;

	protected:
		bool SaveBitmap( PlatformBitmap* bitmap, NSString* filePath, float jpegQuality ) const;

	public:
		virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const;
		virtual bool AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const;
		/// virtual bool OpenURL( const char* url ) const;

		/// virtual void SetIdleTimer( bool enabled ) const;
		/// virtual bool GetIdleTimer() const;

		virtual Real GetStandardFontSize() const;
		/// virtual S32 GetFontNames( lua_State *L, int index ) const;

		virtual int PushSystemInfo( lua_State *L, const char *key ) const;

	public:
		void RegisterUserNotificationSettings() const;
		void RegisterUserNotificationSettings(int type) const;
	
	public:
		virtual void SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const;
		virtual int PushNativeProperty( lua_State *L, const char *key ) const;

	private:
		IPhoneDevice fDevice;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhonePlatformCore_H__

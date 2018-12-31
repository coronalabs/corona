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

#pragma once

#include "Interop\Storage\MStoredPreferences.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_String.h"
#include "Rtt_WinCrypto.h"
#include "Rtt_WinDevice.h"
#include "Rtt_WinExitCallback.h"
#include "WinString.h"
#include <memory>
#include <windows.h>


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}
namespace Rtt
{
	class PlatformBitmap;
	class PlatformSurface;
	class PlatformTimer;
	class RenderingStream;
	class WinAudioPlayer;
	class WinImageProvider;
	class WinVideoPlayer;
	class WinVideoProvider;
	class WinWebPopup;
}
class WinGLView;

#pragma endregion


#undef CreateFont    // <windows.h> defines it


namespace Rtt
{

class WinPlatform : public MPlatform
{
	Rtt_CLASS_NO_COPIES(WinPlatform)

	public:
		typedef WinPlatform Self;

		WinPlatform(Interop::RuntimeEnvironment& environment);
		virtual ~WinPlatform();

		virtual Rtt_Allocator& GetAllocator() const;
		virtual MPlatformDevice& GetDevice() const;
		virtual RenderingStream* CreateRenderingStream(bool antialias) const;
		virtual PlatformSurface* CreateScreenSurface() const;
		virtual PlatformSurface* CreateOffscreenSurface(const PlatformSurface& parent) const;
		virtual PlatformTimer* CreateTimerWithCallback(MCallback& callback) const;
		virtual PlatformBitmap* CreateBitmap(const char* filename, bool convertToGrayscale) const;
		virtual PlatformBitmap* CreateBitmapMask(const char str[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset)  const;
		virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool SaveBitmap(PlatformBitmap* bitmap, const char* filePath, float jpegQuality) const;
		virtual bool AddBitmapToPhotoLibrary(PlatformBitmap* bitmap) const;
		virtual const MCrypto& GetCrypto() const;
		virtual void GetPreference(Category category, Rtt::String * value) const;
		virtual Preference::ReadValueResult GetPreference(const char* categoryName, const char* keyName) const;
		virtual OperationResult SetPreferences(const char* categoryName, const PreferenceCollection& collection) const;
		virtual OperationResult DeletePreferences(const char* categoryName, const char** keyNameArray, U32 keyNameCount) const;
		virtual bool OpenURL(const char* url) const;
		virtual int CanOpenURL(const char* url) const;
		virtual FontMetricsMap GetFontMetrics(const PlatformFont& font ) const;
		virtual void HttpPost(const char* url, const char* key, const char* value) const;
		virtual bool HttpDownload( const char* url, String& result, String& errorMesg, const std::map<std::string, std::string>& headers  ) const;
		virtual bool HttpDownloadFile( const char* url, const char* filename, String& errorMesg, const std::map<std::string, std::string>& headers  ) const;
		virtual PlatformEventSound * CreateEventSound(const ResourceHandle<lua_State> & handle, const char* filePath) const;
		virtual void ReleaseEventSound(PlatformEventSound * soundID) const;
		virtual void PlayEventSound(PlatformEventSound * soundID) const;
		virtual PlatformAudioRecorder * CreateAudioRecorder(const ResourceHandle<lua_State> & handle, const char * filePath) const;
		virtual PlatformAudioPlayer * GetAudioPlayer(const ResourceHandle<lua_State> & handle) const;
		virtual PlatformAudioPlayer * GetAudioPlayer() const;
		virtual PlatformVideoPlayer* GetVideoPlayer(const ResourceHandle<lua_State> & handle) const;
		virtual PlatformImageProvider* GetImageProvider(const ResourceHandle<lua_State> & handle) const;
		virtual PlatformVideoProvider* GetVideoProvider(const ResourceHandle<lua_State> & handle) const;
		virtual PlatformStoreProvider* GetStoreProvider(const ResourceHandle<lua_State> & handle) const;
		virtual void SetStatusBarMode(MPlatform::StatusBarMode newValue) const;
		virtual MPlatform::StatusBarMode GetStatusBarMode() const;
		virtual int GetStatusBarHeight() const;
		virtual int GetTopStatusBarHeightPixels()  const;
		virtual int GetBottomStatusBarHeightPixels() const;
		virtual void SetIdleTimer(bool enabled) const;
		virtual bool GetIdleTimer() const;
		virtual NativeAlertRef ShowNativeAlert(
					const char *title, const char *message, const char **buttonLabels,
					U32 buttonCount, LuaResource *resourcePointer) const;
		virtual void CancelNativeAlert(NativeAlertRef alertReference, S32 buttonIndex) const;
		virtual void SetActivityIndicator(bool visible) const;
		virtual PlatformWebPopup* GetWebPopup() const;
		virtual bool CanShowPopup(const char *name) const;
		virtual bool ShowPopup(lua_State *L, const char *name, int optionsIndex) const;
		virtual bool HidePopup(const char *name) const;
		virtual PlatformDisplayObject* CreateNativeTextBox(const Rect& bounds) const;
		virtual PlatformDisplayObject* CreateNativeTextField(const Rect& bounds) const;
		virtual void SetKeyboardFocus(PlatformDisplayObject *textObject) const;
		virtual PlatformDisplayObject* CreateNativeMapView(const Rect& bounds) const;
		virtual PlatformDisplayObject* CreateNativeWebView(const Rect& bounds) const;
		virtual PlatformDisplayObject* CreateNativeVideo(const Rect& bounds) const;
		virtual Rtt_Real GetStandardFontSize() const;
		virtual S32 GetFontNames(lua_State *L, int index) const;
		virtual PlatformFont* CreateFont(PlatformFont::SystemFont fontType, Rtt_Real size) const;
		virtual PlatformFont* CreateFont(const char *fontName, Rtt_Real size) const;
		virtual void SetTapDelay(Rtt_Real delay) const;
		virtual Rtt_Real GetTapDelay() const;
		virtual PlatformFBConnect* GetFBConnect() const;
		virtual void* CreateAndScheduleNotification(lua_State *L, int index) const;
		virtual void ReleaseNotification(void *notificationId) const;
		virtual void CancelNotification(void *notificationId) const;
		virtual void FlurryInit(const char * applicationKey) const;
		virtual void FlurryEvent(const char * eventId) const;
		virtual void SetNativeProperty(lua_State *L, const char *key, int valueIndex) const;
		virtual int PushNativeProperty(lua_State *L, const char *key) const;
		virtual int PushSystemInfo(lua_State *L, const char *key) const;
		virtual void RuntimeErrorNotification(const char *errorType, const char *message, const char *stacktrace) const;
		virtual void SetProjectResourceDirectory(const char* path);
		virtual void SetSkinResourceDirectory(const char* path);
		virtual void RaiseError(MPlatform::Error e, const char* reason) const;
		virtual void PathForFile(const char* filename, MPlatform::Directory baseDir, U32 flags, String & result) const;
		virtual bool FileExists(const char * filename) const;
		virtual bool ValidateAssetFile(const char *assetFilename, const int assetSize) const;
		virtual int SetSync(lua_State* L) const;
		virtual int GetSync(lua_State* L) const;
		virtual void BeginRuntime(const Runtime& runtime) const;
		virtual void EndRuntime(const Runtime& runtime) const;
		virtual PlatformExitCallback* GetExitCallback();
		virtual bool RequestSystem(lua_State *L, const char *actionName, int optionsIndex) const;
#ifdef Rtt_AUTHORING_SIMULATOR
		virtual void SetCursorForRect(const char *cursorName, int x, int y, int width, int height) const;
#endif

		virtual void Suspend( ) const;
		virtual void Resume( ) const;
		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const;

	private:
		typedef std::shared_ptr<Interop::Storage::MStoredPreferences> SharedMStoredPreferencesPointer;

		void CopyAppNameTo(WinString& destinationString) const;
		int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) const;
		Rtt::ValueResult<SharedMStoredPreferencesPointer> GetStoredPreferencesByCategoryName(const char* categoryName) const;
		void OnSetActivityIndicator(bool visible);

		Interop::RuntimeEnvironment& fEnvironment;
		WinDevice fDevice;
		mutable WinAudioPlayer* fAudioPlayer;
		mutable WinVideoPlayer* fVideoPlayer;
		mutable WinImageProvider* fImageProvider;
		mutable WinVideoProvider* fVideoProvider;
		WinCrypto fCrypto;
		mutable WinWebPopup *fWebPopup;
		mutable PlatformFBConnect *fFBConnect;
		WinExitCallback fExitCallback;
		mutable bool fIsIdleTimerEnabled;
};

}	// namespace Rtt

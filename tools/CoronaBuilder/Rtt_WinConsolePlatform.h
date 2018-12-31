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

// ------------------------------------------------
// This is a stub to enable building CoronaBuilder
// without importing most of the Simulator code
// ------------------------------------------------

#pragma once

#include "Rtt_MPlatform.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_String.h"
#include "Rtt_WinCrypto.h"
#include "Rtt_WinConsoleDevice.h"
#include "Rtt_WinExitCallback.h"
#include "WinString.h"
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

	class WinConsolePlatform : public MPlatform
	{
		Rtt_CLASS_NO_COPIES(WinConsolePlatform)

	public:
		typedef WinConsolePlatform Self;

		WinConsolePlatform();
		virtual ~WinConsolePlatform();

		virtual Rtt_Allocator& GetAllocator() const;
		virtual MPlatformDevice& GetDevice() const;
		virtual RenderingStream* CreateRenderingStream(bool antialias) const { return NULL; };
		virtual PlatformSurface* CreateScreenSurface() const { return NULL; };
		virtual PlatformSurface* CreateOffscreenSurface(const PlatformSurface& parent) const { return NULL; };
		virtual PlatformTimer* CreateTimerWithCallback(MCallback& callback) const { return NULL; };
		virtual PlatformBitmap* CreateBitmap(const char* filename, bool convertToGrayscale) const { return NULL; };
		virtual PlatformBitmap* CreateBitmapMask(
			const char str[], const PlatformFont& font, Real w, Real h, const char alignmentStringId[]) const {
			return NULL;
		};
		virtual bool SaveImageToPhotoLibrary(const char* filePath) const { return false; };
		virtual bool SaveBitmap(PlatformBitmap* bitmap, const char* filePath, float jpegQuality) const { return false; };
		virtual bool AddBitmapToPhotoLibrary(PlatformBitmap* bitmap) const { return false; };
		virtual const MCrypto& GetCrypto() const;
		virtual void GetPreference(Category category, Rtt::String * value) const;
		virtual Preference::ReadValueResult GetPreference(const char* categoryName, const char* keyName) const;
		virtual OperationResult SetPreferences(const char* categoryName, const PreferenceCollection& collection) const;
		virtual OperationResult DeletePreferences(const char* categoryName, const char** keyNameArray, U32 keyNameCount) const;
		virtual bool OpenURL(const char* url) const { return false; };
		virtual int CanOpenURL(const char* url) const { return 0; };
		virtual void HttpPost(const char* url, const char* key, const char* value) const { return; };
		virtual PlatformEventSound * CreateEventSound(const ResourceHandle<lua_State> & handle, const char* filePath) const { return nullptr; };
		virtual void ReleaseEventSound(PlatformEventSound * soundID) const { return; };
		virtual void PlayEventSound(PlatformEventSound * soundID) const { return; };
		virtual PlatformAudioRecorder * CreateAudioRecorder(const ResourceHandle<lua_State> & handle, const char * filePath) const { return nullptr; };
		virtual PlatformAudioPlayer * GetAudioPlayer(const ResourceHandle<lua_State> & handle) const { return nullptr; };
		virtual PlatformAudioPlayer * GetAudioPlayer() const { return nullptr; };
		virtual PlatformVideoPlayer* GetVideoPlayer(const ResourceHandle<lua_State> & handle) const { return nullptr; };
		virtual PlatformImageProvider* GetImageProvider(const ResourceHandle<lua_State> & handle) const { return nullptr; };
		virtual PlatformVideoProvider* GetVideoProvider(const ResourceHandle<lua_State> & handle) const { return nullptr; };
		virtual PlatformStoreProvider* GetStoreProvider(const ResourceHandle<lua_State> & handle) const { return nullptr; };
		virtual void SetStatusBarMode(MPlatform::StatusBarMode newValue) const { return; };
		virtual MPlatform::StatusBarMode GetStatusBarMode() const;
		virtual int GetStatusBarHeight() const { return 0; };
		virtual int GetTopStatusBarHeightPixels()  const { return 0; };
		virtual int GetBottomStatusBarHeightPixels() const { return 0; };
		virtual void SetIdleTimer(bool enabled) const { return; };
		virtual bool GetIdleTimer() const { return false; };
		virtual NativeAlertRef ShowNativeAlert(
			const char *title, const char *message, const char **buttonLabels,
			U32 buttonCount, LuaResource *resourcePointer) const { return nullptr; };
		virtual void CancelNativeAlert(NativeAlertRef alertReference, S32 buttonIndex) const { return; };
		virtual void SetActivityIndicator(bool visible) const { return; };
		virtual PlatformWebPopup* GetWebPopup() const { return nullptr; };
		virtual bool CanShowPopup(const char *name) const { return false; };
		virtual bool ShowPopup(lua_State *L, const char *name, int optionsIndex) const { return false; };
		virtual bool HidePopup(const char *name) const { return false; };
		virtual PlatformDisplayObject* CreateNativeTextBox(const Rect& bounds) const { return nullptr; };
		virtual PlatformDisplayObject* CreateNativeTextField(const Rect& bounds) const { return nullptr; };
		virtual void SetKeyboardFocus(PlatformDisplayObject *textObject) const { return; };
		virtual PlatformDisplayObject* CreateNativeMapView(const Rect& bounds) const { return nullptr; };
		virtual PlatformDisplayObject* CreateNativeWebView(const Rect& bounds) const { return nullptr; };
		virtual PlatformDisplayObject* CreateNativeVideo(const Rect& bounds) const { return nullptr; };
		virtual Rtt_Real GetStandardFontSize() const { return 0.0; };
		virtual S32 GetFontNames(lua_State *L, int index) const { return 0; };
		virtual PlatformFont* CreateFont(PlatformFont::SystemFont fontType, Rtt_Real size) const { return nullptr; };
		virtual PlatformFont* CreateFont(const char *fontName, Rtt_Real size) const { return nullptr; };
		virtual void SetTapDelay(Rtt_Real delay) const { return; };
		virtual Rtt_Real GetTapDelay() const { return 0.0; };
		virtual PlatformFBConnect* GetFBConnect() const { return nullptr; };
		virtual void* CreateAndScheduleNotification(lua_State *L, int index) const { return nullptr; };
		virtual void ReleaseNotification(void *notificationId) const { return; };
		virtual void CancelNotification(void *notificationId) const { return; };
		virtual void FlurryInit(const char * applicationKey) const { return; };
		virtual void FlurryEvent(const char * eventId) const { return; };
		virtual void SetNativeProperty(lua_State *L, const char *key, int valueIndex) const { return; };
		virtual int PushNativeProperty(lua_State *L, const char *key) const { return 0; };
		virtual int PushSystemInfo(lua_State *L, const char *key) const { return 0; };
		virtual void RuntimeErrorNotification(const char *errorType, const char *message, const char *stacktrace) const;
		virtual void SetProjectResourceDirectory(const char* path) { return; };
		virtual void SetSkinResourceDirectory(const char* path) { return; };
		virtual void RaiseError(MPlatform::Error e, const char* reason) const { return; };
		virtual void PathForFile(const char* filename, MPlatform::Directory baseDir, U32 flags, String & result) const;
		virtual bool FileExists(const char * filename) const;
		virtual int SetSync(lua_State* L) const { return 0; };
		virtual int GetSync(lua_State* L) const { return 0; };
		virtual void BeginRuntime(const Runtime& runtime) const { return; };
		virtual void EndRuntime(const Runtime& runtime) const { return; };
		virtual PlatformExitCallback* GetExitCallback() { return nullptr; };
		virtual bool RequestSystem(lua_State *L, const char *actionName, int optionsIndex) const { return false; };
		virtual PlatformBitmap* CreateBitmapMask(const char str[], const PlatformFont& font, Real w, Real h, 
			const char alignment[], Real& baselineOffset) const override  { Rtt_ASSERT_MSG( 0, "Code should NOT be reached" ); return NULL; }
		virtual FontMetricsMap GetFontMetrics(const PlatformFont& font) const override;
		virtual bool HttpDownload(const char* url, String& result, String& errorMesg, const std::map<std::string, std::string>& headers) const override;
		virtual bool HttpDownloadFile(const char* url, const char* filename, String& errorMesg, const std::map<std::string, std::string>& headers) const override;
		bool HttpDownloadBuffer(const char* url, std::vector<char>& result, String& errorMesg, const std::map<std::string, std::string>& headers) const;
		virtual void Suspend() const override { Rtt_ASSERT_MSG( 0, "Code should NOT be reached" ); }
		virtual void Resume() const override { Rtt_ASSERT_MSG( 0, "Code should NOT be reached" ); }
		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const override;

#ifdef Rtt_AUTHORING_SIMULATOR
		virtual void SetCursorForRect(const char *cursorName, int x, int y, int width, int height) const { return; };
#endif

	public:
		static int RunSystemCommand(std::wstring command);
		static const std::wstring GetDirectoryPath();

	private:
		const char *GetUtf8PathFor(MPlatform::Directory baseDir) const;

		WinCrypto fCrypto;
		WinString fDirectoryPaths[MPlatform::kNumDirs];
		WinConsoleDevice *fDevice;

		/*
		void CopyAppNameTo(WinString& destinationString) const;
		int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) const;
		void OnSetActivityIndicator(bool visible);

		Interop::RuntimeEnvironment& fEnvironment;
		WinDevice fDevice;
		mutable WinAudioPlayer* fAudioPlayer;
		mutable WinVideoPlayer* fVideoPlayer;
		mutable WinImageProvider* fImageProvider;
		mutable WinVideoProvider* fVideoProvider;
		mutable WinWebPopup *fWebPopup;
		mutable PlatformFBConnect *fFBConnect;
		WinExitCallback fExitCallback;
		mutable bool fIsIdleTimerEnabled;
		*/
};

}	// namespace Rtt

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_LinuxDevice.h"
#include "Rtt_MPlatform.h"
#include "Rtt_LinuxCrypto.h"
#include "Core/Rtt_String.h"
#include "Rtt_PlatformTimer.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_LinuxRuntimeErrorDialog.h"

#undef CreateFont

namespace Rtt
{
	class LinuxAudioPlayer;
	class LinuxImageProvider;
	class LinuxStoreProvider;
	class LinuxVideoPlayer;
	class LinuxVideoProvider;
	class LinuxWebPopup;
	class PlatformBitmap;
	class PlatformSurface;
	class PlatformTimer;
	class RenderingStream;
	class LinuxScreenSurface;

	class LinuxTimer : public PlatformTimer
	{
	public:
		LinuxTimer(MCallback &callback) : PlatformTimer(callback) {};

		virtual void Start() {};
		virtual void Stop() {};
		virtual void SetInterval(U32 milliseconds) {};
		virtual bool IsRunning() const { return true; };
	};

	class LinuxPlatform : public MPlatform
	{
	Rtt_CLASS_NO_COPIES(LinuxPlatform) public :
		typedef LinuxPlatform Self;

	public:
		LinuxPlatform(const char *resourceDir, const char *documentsDir, const char *temporaryDir,
		              const char *cachesDir, const char *systemCachesDir, const char *skinDir, const char *installDir);
		virtual ~LinuxPlatform();

	public:
		virtual MPlatformDevice &GetDevice() const;
		virtual PlatformSurface *CreateScreenSurface() const;
		virtual PlatformSurface *CreateOffscreenSurface(const PlatformSurface &parent) const;

	public:
		virtual Rtt_Allocator &GetAllocator() const;
		virtual RenderingStream *CreateRenderingStream(bool antialias) const;
		virtual PlatformTimer *CreateTimerWithCallback(MCallback &callback) const;
		virtual PlatformBitmap *CreateBitmap(const char *filename, bool convertToGrayscale) const;
		virtual void HttpPost(const char *url, const char *key, const char *value) const;
		virtual PlatformEventSound *CreateEventSound(const ResourceHandle<lua_State> &handle, const char *filePath) const;
		virtual void ReleaseEventSound(PlatformEventSound *soundID) const;
		virtual void PlayEventSound(PlatformEventSound *soundID) const;
		virtual PlatformAudioPlayer *GetAudioPlayer(const ResourceHandle<lua_State> &handle) const;
		virtual PlatformAudioRecorder *CreateAudioRecorder(const Rtt::ResourceHandle<lua_State> &handle, const char *filePath) const;
		virtual const MCrypto &GetCrypto() const;
		virtual void GetPreference(Category category, Rtt::String *value) const;
		virtual void SetActivityIndicator(bool visible) const;
		virtual PlatformWebPopup *GetWebPopup() const;
		virtual bool CanShowPopup(const char *name) const;
		virtual bool ShowPopup(lua_State *L, const char *name, int optionsIndex) const;
		virtual bool HidePopup(const char *name) const;
		virtual PlatformDisplayObject *CreateNativeTextBox(const Rect &bounds) const;
		virtual PlatformDisplayObject *CreateNativeTextField(const Rect &bounds) const;
		virtual void SetKeyboardFocus(PlatformDisplayObject *textObject) const;
		virtual PlatformDisplayObject *CreateNativeMapView(const Rect &bounds) const;
		virtual PlatformDisplayObject *CreateNativeWebView(const Rect &bounds) const;
		virtual PlatformDisplayObject *CreateNativeVideo(const Rect &bounds) const;
		PlatformDisplayObject *GetNativeDisplayObjectById(const int objectId) const;
		virtual Rtt_Real GetStandardFontSize() const;
		virtual S32 GetFontNames(lua_State *L, int index) const;
		virtual PlatformFont *CreateFont(PlatformFont::SystemFont fontType, Rtt_Real size) const override;
		// Returns NULL if fontName is NULL;
		virtual PlatformFont *CreateFont(const char *fontName, Rtt_Real size) const override;
		virtual void SetTapDelay(Rtt_Real delay) const;
		virtual Rtt_Real GetTapDelay() const;
		virtual PlatformFBConnect *GetFBConnect() const;

	public:
		virtual void RaiseError(MPlatform::Error e, const char *reason) const;
		virtual void PathForFile(const char *filename, MPlatform::Directory baseDir, U32 flags, String &result) const;
		virtual void SetProjectResourceDirectory(const char *filename);
		virtual void SetSkinResourceDirectory(const char *filename);
		virtual bool FileExists(const char *filename) const;
		virtual int SetSync(lua_State *L) const;
		virtual int GetSync(lua_State *L) const;

	protected:
		void PathForFile(const char *filename, const char *baseDir, String &result) const;

	public:
		virtual void BeginRuntime(const Runtime &runtime) const;
		virtual void EndRuntime(const Runtime &runtime) const;
		virtual PlatformExitCallback *GetExitCallback();
		virtual bool RequestSystem(lua_State *L, const char *actionName, int optionsIndex) const;
		virtual void RuntimeErrorNotification(const char *errorType, const char *message, const char *stacktrace) const;

#ifdef Rtt_AUTHORING_SIMULATOR
		virtual void SetCursorForRect(const char* cursorName, int x, int y, int width, int height) const override;
#endif

	protected:
		char *CopyString(const char *src, bool useAllocator = true) const;

	public:
		virtual bool SaveImageToPhotoLibrary(const char *filePath) const;
		virtual bool SaveBitmap(PlatformBitmap *bitmap, const char *filePath, float jpegQuality) const;
		virtual bool AddBitmapToPhotoLibrary(PlatformBitmap *bitmap) const;
		virtual bool OpenURL(const char *url) const;
		virtual int CanOpenURL(const char *url) const;
		virtual PlatformVideoPlayer *GetVideoPlayer(const Rtt::ResourceHandle<lua_State> &handle) const;
		virtual PlatformImageProvider *GetImageProvider(const Rtt::ResourceHandle<lua_State> &handle) const;
		virtual PlatformVideoProvider *GetVideoProvider(const Rtt::ResourceHandle<lua_State> &handle) const;
		virtual PlatformStoreProvider *GetStoreProvider(const ResourceHandle<lua_State> &handle) const;
		virtual void SetStatusBarMode(MPlatform::StatusBarMode newValue) const;
		virtual MPlatform::StatusBarMode GetStatusBarMode() const;
		virtual int GetStatusBarHeight() const;
		virtual int GetTopStatusBarHeightPixels() const;
		virtual int GetBottomStatusBarHeightPixels() const;
		virtual void SetIdleTimer(bool enabled) const;
		virtual bool GetIdleTimer() const;

		virtual NativeAlertRef ShowNativeAlert(
		    const char *title,
		    const char *msg,
		    const char **buttonLabels,
		    U32 numButtons,
		    LuaResource *resource) const;
		virtual void CancelNativeAlert(NativeAlertRef alert, S32 index) const;

	public:
		virtual void *CreateAndScheduleNotification(lua_State *L, int index) const;
		virtual void ReleaseNotification(void *notificationId) const;
		virtual void CancelNotification(void *notificationId) const;
		virtual void FlurryInit(const char *applicationKey) const;
		virtual void FlurryEvent(const char *eventId) const;

	public:
		virtual void SetNativeProperty(lua_State *L, const char *key, int valueIndex) const;
		virtual int PushNativeProperty(lua_State *L, const char *key) const;
		virtual int PushSystemInfo(lua_State *L, const char *key) const;

	protected:
		void NetworkBaseRequest(lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *path) const;

	public:
		virtual void NetworkRequest(lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex) const;
		virtual void NetworkDownload(lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *filename, MPlatform::Directory baseDir) const;
		virtual PlatformReachability *NewReachability(const ResourceHandle<lua_State> &handle, PlatformReachability::PlatformReachabilityType type, const char *address) const;
		virtual bool SupportsNetworkStatus() const;
		const char *getInstallDir() const { return fInstallDir.GetString(); }
		void setWindow(void *ctx);
		bool fShowRuntimeErrors;

	protected:
		Rtt_Allocator *fAllocator;
		mutable LinuxAudioPlayer *fAudioPlayer;
		mutable bool isMouseCursorVisible;

	private:
		LinuxDevice fDevice;
		mutable LinuxVideoPlayer *fVideoPlayer;
		mutable LinuxImageProvider *fImageProvider;
		mutable LinuxVideoProvider *fVideoProvider;
		mutable LinuxWebPopup *fWebPopup;
		String fResourceDir;
		String fDocumentsDir;
		String fTemporaryDir;
		String fCachesDir;
		String fSystemCachesDir;
		String fInstallDir;
		String fSkinDir;
		mutable MPlatform::StatusBarMode fStatusBarMode;
		LinuxCrypto fCrypto;
		mutable PlatformStoreProvider *fStoreProvider;
		mutable PlatformFBConnect *fFBConnect;
		mutable LinuxScreenSurface *fScreenSurface;

	public:
		virtual PlatformBitmap *CreateBitmapMask(const char str[], const PlatformFont &font, Real w, Real h, const char alignment[], Real &baselineOffset) const override;
		virtual FontMetricsMap GetFontMetrics(const PlatformFont &font) const override;
		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const override;
		virtual Preference::ReadValueResult GetPreference(const char *categoryName, const char *keyName) const override;
		virtual OperationResult SetPreferences(const char *categoryName, const PreferenceCollection &collection) const override;
		virtual OperationResult DeletePreferences(const char *categoryName, const char **keyNameArray, U32 keyNameCount) const override;
		virtual void Suspend() const override;
		virtual void Resume() const override;
	};

	class LinuxGUIPlatform : public LinuxPlatform
	{
	public:
		typedef LinuxPlatform Super;

	public:
		LinuxGUIPlatform(PlatformSimulator &simulator);

	public:
		virtual MPlatformDevice &GetDevice() const;
		virtual PlatformSurface *CreateScreenSurface() const;

	public:
		virtual void SetStatusBarMode(StatusBarMode newValue) const;
		virtual StatusBarMode GetStatusBarMode() const;
		virtual Real GetStandardFontSize() const;
		virtual bool RequestSystem(lua_State *L, const char *actionName, int optionsIndex) const;

	public:
		void SetAdaptiveWidth(S32 newValue) { fAdaptiveWidth = newValue; }
		void SetAdaptiveHeight(S32 newValue) { fAdaptiveHeight = newValue; }

	private:
		LinuxDevice fMacDevice;
		S32 fAdaptiveWidth;
		S32 fAdaptiveHeight;
	};

}; // namespace Rtt

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_FileSystem.h"
#include "Rtt_GPUStream.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_Runtime.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxAudioPlayer.h"
#include "Rtt_LinuxAudioRecorder.h"
#include "Rtt_LinuxBitmap.h"
//#include "Rtt_LinuxEventSound.h"
//#include "Rtt_LinuxFBConnect.h"
#include "Rtt_LinuxFont.h"
#include "Rtt_LinuxImageProvider.h"
//#include "Rtt_LinuxMapViewObject.h"
#include "Rtt_LinuxScreenSurface.h"
//#include "Rtt_LinuxStoreProvider.h"
#include "Rtt_LinuxTextBoxObject.h"
#include "Rtt_LinuxVideoObject.h"
#include "Rtt_LinuxVideoPlayer.h"
#include "Rtt_LinuxVideoProvider.h"
#include "Rtt_LinuxWebPopup.h"
#include "Rtt_LinuxWebView.h"
#include "Rtt_LinuxContainer.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxApp.h"
#include "Rtt_PreferenceCollection.h"
#include "Rtt_Freetype.h"
#include <pwd.h>

using namespace std;

namespace Rtt
{
	LinuxPlatform::LinuxPlatform(const char* resourceDir, const char* documentsDir, const char* temporaryDir,
		const char* cachesDir, const char* systemCachesDir, const char* skinDir, const char* installDir)
		: fAllocator(Rtt_AllocatorCreate()),
		fDevice(*fAllocator),
		fAudioPlayer(NULL),
		fVideoPlayer(NULL),
		fImageProvider(NULL),
		fVideoProvider(NULL),
		fWebPopup(NULL),
		fResourceDir(fAllocator),
		fDocumentsDir(fAllocator),
		fTemporaryDir(fAllocator),
		fCachesDir(fAllocator),
		fSystemCachesDir(fAllocator),
		fInstallDir(fAllocator),
		fSkinDir(fAllocator),
		fStoreProvider(NULL),
		fFBConnect(NULL),
		fScreenSurface(NULL)
	{
		fResourceDir.Set(resourceDir);
		fDocumentsDir.Set(documentsDir);
		fTemporaryDir.Set(temporaryDir);
		fCachesDir.Set(cachesDir);
		fSystemCachesDir.Set(systemCachesDir);
		fSkinDir.Set(skinDir);
		fInstallDir.Set(installDir);
		fStatusBarMode = MPlatform::StatusBarMode::kDefaultStatusBar;
		isMouseCursorVisible = true;
	}

	LinuxPlatform::~LinuxPlatform()
	{
		Rtt_DELETE(fFBConnect);
		Rtt_DELETE(fStoreProvider);
		Rtt_DELETE(fWebPopup);
		Rtt_DELETE(fVideoPlayer);
		Rtt_DELETE(fAudioPlayer);
		Rtt_DELETE(fImageProvider);
		Rtt_DELETE(fVideoProvider);

		// it will be deleted in Rtt_Display by Rtt_DELETE( fTarget );
		//Rtt_DELETE(fScreenSurface);
	}

	MPlatformDevice& LinuxPlatform::GetDevice() const
	{
		return const_cast<LinuxDevice&>(fDevice);
	}

	PlatformSurface* LinuxPlatform::CreateScreenSurface() const
	{
		Rtt_ASSERT(fScreenSurface == NULL);
		fScreenSurface = Rtt_NEW(fAllocator, LinuxScreenSurface());
		return fScreenSurface;
	}

	PlatformSurface* LinuxPlatform::CreateOffscreenSurface(const PlatformSurface& parent) const
	{
		return Rtt_NEW(fAllocator, LinuxOffscreenSurface(parent));
	}

	int LinuxPlatform::CanOpenURL(const char* url) const
	{
		return true;
	}

	bool LinuxPlatform::OpenURL(const char* url) const
	{
		return ::OpenURL(url);
	}

	PlatformVideoPlayer* LinuxPlatform::GetVideoPlayer(const Rtt::ResourceHandle<lua_State>& handle) const
	{
		Rtt_ASSERT(0 && "todo");
		return NULL;
	}

	PlatformImageProvider* LinuxPlatform::GetImageProvider(const Rtt::ResourceHandle<lua_State>& handle) const
	{
		if (!fImageProvider)
		{
			fImageProvider = Rtt_NEW(&GetAllocator(), LinuxImageProvider(handle));
		}

		return fImageProvider;
	}

	PlatformVideoProvider* LinuxPlatform::GetVideoProvider(const Rtt::ResourceHandle<lua_State>& handle) const
	{
		if (!fVideoProvider)
		{
			int w, h;
			fScreenSurface->getWindowSize(&w, &h);

			fVideoProvider = Rtt_NEW(fAllocator, LinuxVideoProvider(handle, w, h));
		}

		return fVideoProvider;
	}

	PlatformStoreProvider* LinuxPlatform::GetStoreProvider(const ResourceHandle<lua_State>& handle) const
	{
		return NULL;
	}

	void LinuxPlatform::SetIdleTimer(bool enabled) const
	{
	}

	bool LinuxPlatform::GetIdleTimer() const
	{
		return true;
	}

	NativeAlertRef LinuxPlatform::ShowNativeAlert(const char* title, const char* msg, const char** buttonLabels, U32 numButtons, LuaResource* resource) const
	{
		app->ShowNativeAlert(title, msg, buttonLabels, numButtons, resource);
		return  (NativeAlertRef)0x1234;
	}

	void LinuxPlatform::CancelNativeAlert(NativeAlertRef alert, S32 index) const
	{
		// modal msgbox
		Rtt_ASSERT_MSG(0, "Code should NOT be reached");
	}

	Rtt_Allocator& LinuxPlatform::GetAllocator() const
	{
		return *fAllocator;
	}

	RenderingStream* LinuxPlatform::CreateRenderingStream(bool antialias) const
	{
		RenderingStream* result = Rtt_NEW(fAllocator, GPUStream(fAllocator));
		result->SetProperty(RenderingStream::kFlipHorizontalAxis, true);
		return result;
	}

	PlatformTimer* LinuxPlatform::CreateTimerWithCallback(MCallback& callback) const
	{
		return Rtt_NEW(fAllocator, LinuxTimer(callback));
	}

	PlatformBitmap* LinuxPlatform::CreateBitmap(const char* path, bool convertToGrayscale) const
	{
		PlatformBitmap* result = NULL;

		if (path)
		{
			if (convertToGrayscale)
			{
				result = Rtt_NEW(&GetAllocator(), LinuxMaskFileBitmap(GetAllocator(), path));
			}
			else
			{
				result = Rtt_NEW(&GetAllocator(), LinuxFileBitmap(GetAllocator(), path));
			}
		}

		if (result && result->GetFormat() == PlatformBitmap::kUndefined)
		{
			// failed to load bitmap
			Rtt_DELETE(result);
			result = NULL;
		}

		return result;
	}

	bool LinuxPlatform::FileExists(const char* filename) const
	{
		// Validate.
		if (Rtt_StringIsEmpty(filename))
		{
			return false;
		}

		// Determine if the given file name exists.
		bool fileExists = false;
		FILE* file = fopen(filename, "r");

		if (file)
		{
			fileExists = true;
			fclose(file);
		}

		return fileExists;
	}

	void LinuxPlatform::PathForFile(const char* filename, MPlatform::Directory baseDir, U32 flags, String& result) const
	{
		// Initialize result to an empty string in case the file was not found.
		result.Set(NULL);

		// Fetch the path for the given file.
		if (filename && strstr(filename, "://"))
		{
			// The given file name is a URL.
			// Accept it as is and ignore the base directory.
			result.Set(filename);
		}
		else
		{
			// Create an absolute path by appending the file name to the base directory.
			switch (baseDir)
			{
			case MPlatform::kResourceDir:
			{
				PathForFile(filename, fResourceDir.GetString(), result);
				String result1;
				String result2;

				if (filename != NULL && FileExists(result.GetString()) == false)
				{
					result1.Set(result);
					// look for Resources dir
					String resDir;
					resDir.Append(fInstallDir);
					resDir.Append("/Resources");
					PathForFile(filename, resDir.GetString(), result);
					result2.Set(result);
				}

				if (filename != NULL && FileExists(result.GetString()) == false)
				{
					PathForFile(filename, GetPluginsPath().c_str(), result);
					Rtt_WARN_SIM(!filename || FileExists(result.GetString()), ("WARNING: Cannot create path for resource file '%s (%s || %s || %s)'. File does not exist.\n\n", filename, result1.GetString(), result2.GetString(), result.GetString()));
				}
				break;
			}

			case MPlatform::kSystemResourceDir:
			{
				PathForFile(filename, fResourceDir.GetString(), result);

				if (filename != NULL && FileExists(result.GetString()) == false)
				{
					// look for Resources dir
					String resDir;
					resDir.Append(fInstallDir);
					resDir.Append("/Resources");
					PathForFile(filename, resDir.GetString(), result);
					Rtt_WARN_SIM(!filename || FileExists(result.GetString()), ("WARNING: Cannot create path for resource file '%s (%s)'. File does not exist.\n\n", filename, result.GetString()));
				}
				break;
			}

			case MPlatform::kCachesDir:
			{
				PathForFile(filename, fCachesDir.GetString(), result);
				break;
			}

			case MPlatform::kSystemCachesDir:
			{
				PathForFile(filename, fSystemCachesDir.GetString(), result);
				break;
			}

			case MPlatform::kTmpDir:
			{
				PathForFile(filename, fTemporaryDir.GetString(), result);
				break;
			}

			case MPlatform::kPluginsDir:
			{
				std::string pluginPath;

#ifdef Rtt_SIMULATOR
				pluginPath = GetHomePath();
				pluginPath.append("/.Solar2D/Plugins");
#else
				pluginPath = GetStartupPath(NULL);
#endif

				PathForFile(filename, pluginPath.c_str(), result);
				break;
			}

			case MPlatform::kSkinResourceDir:
			{
				PathForFile(filename, fSkinDir.GetString(), result);
				break;
			}

			case MPlatform::kDocumentsDir:
			default:
				PathForFile(filename, fDocumentsDir.GetString(), result);
				break;
			}
		}

		// Check if the file exists, if enabled.
		// Result will be set to an empty string if the file could not be found.
		if (flags & MPlatform::kTestFileExists)
		{
			if (FileExists(result.GetString()) == false)
			{
				result.Set(NULL);
			}
		}
	}

	void LinuxPlatform::SetProjectResourceDirectory(const char* filename)
	{
		fResourceDir.Set(filename);
	}

	void LinuxPlatform::SetSkinResourceDirectory(const char* filename)
	{
		fSkinDir.Set(filename);
	}

	void LinuxPlatform::PathForFile(const char* filename, const char* baseDir, String& result) const
	{
		result.Set(NULL);

		if (filename)
		{
			// Append absolute path to the given directory.
			result.Append(baseDir);

			// Append directory separator, if not already there.
			size_t npl = strlen(result.GetString());

			if (npl > 0 && result.GetString()[npl - 1] != '/')
			{
				result.Append("/");
			}

			// Append the file name.
			result.Append(filename);
		}
		else
		{
			// File name not provided. So, just provide the directory path.
			result.Set(baseDir);
		}
	}

	void LinuxPlatform::HttpPost(const char* url, const char* key, const char* value) const
	{
		// Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	PlatformEventSound* LinuxPlatform::CreateEventSound(const ResourceHandle<lua_State>& handle, const char* filePath) const
	{
		// LinuxEventSound *p = Rtt_NEW(fAllocator, LinuxEventSound(handle, *fAllocator));
		// p->Load(filePath);
		return NULL;
	}

	void LinuxPlatform::ReleaseEventSound(PlatformEventSound* soundID) const
	{
		Rtt_DELETE(soundID);
	}

	void LinuxPlatform::PlayEventSound(PlatformEventSound* soundID) const
	{
		//soundID->Play();
	}

	PlatformAudioPlayer* LinuxPlatform::GetAudioPlayer(const ResourceHandle<lua_State>& handle) const
	{
		//if (!fAudioPlayer)
		//{
		//fAudioPlayer = Rtt_NEW(fAllocator, LinuxAudioPlayer(handle, *fAllocator));
		//}
		return NULL; //fAudioPlayer;
	}

	PlatformAudioRecorder* LinuxPlatform::CreateAudioRecorder(const Rtt::ResourceHandle<lua_State>& handle, const char* filePath) const
	{
		return Rtt_NEW(fAllocator, LinuxAudioRecorder(handle, *fAllocator, filePath));
	}

	void LinuxPlatform::RaiseError(MPlatform::Error e, const char* reason) const
	{
		const char kNull[] = "(null)";

		if (!reason)
		{
			reason = kNull;
		}

		Rtt_TRACE(("MPlatformFactory error(%d): %s\n", e, kNull));
	}

	bool LinuxPlatform::SaveImageToPhotoLibrary(const char* filePath) const
	{
		return false;
	}

	bool LinuxPlatform::SaveBitmap(PlatformBitmap* bitmap, const char* filePath, float jpegQuality) const
	{
		return LinuxBaseBitmap::SaveBitmap(fAllocator, bitmap, filePath);
	}

	bool LinuxPlatform::AddBitmapToPhotoLibrary(PlatformBitmap* bitmap) const
	{
		return SaveBitmap(bitmap, NULL, 1.0f);
	}

	void LinuxPlatform::SetStatusBarMode(MPlatform::StatusBarMode newValue) const
	{
		switch (newValue)
		{
		case MPlatform::StatusBarMode::kHiddenStatusBar:
			break;

		case MPlatform::StatusBarMode::kDefaultStatusBar:
			break;

		case MPlatform::StatusBarMode::kTranslucentStatusBar:
			break;

		case MPlatform::StatusBarMode::kDarkStatusBar:
			break;

		case MPlatform::StatusBarMode::kLightTransparentStatusBar:
			break;

		case MPlatform::StatusBarMode::kDarkTransparentStatusBar:
			break;
		}

		fStatusBarMode = newValue;
	}

	MPlatform::StatusBarMode LinuxPlatform::GetStatusBarMode() const
	{
		return fStatusBarMode;
	}

	int LinuxPlatform::GetStatusBarHeight() const
	{
		return 0;
	}

	int LinuxPlatform::GetTopStatusBarHeightPixels() const
	{
		return 0;
	}

	int LinuxPlatform::GetBottomStatusBarHeightPixels() const
	{
		return 0;
	}

	int LinuxPlatform::SetSync(lua_State* L) const
	{
		return 0;
	}

	int LinuxPlatform::GetSync(lua_State* L) const
	{
		return 0;
	}

	void LinuxPlatform::BeginRuntime(const Runtime& runtime) const
	{
	}

	void LinuxPlatform::EndRuntime(const Runtime& runtime) const
	{
	}

	PlatformExitCallback* LinuxPlatform::GetExitCallback()
	{
		return NULL;
	}

	bool LinuxPlatform::RequestSystem(lua_State* L, const char* actionName, int optionsIndex) const
	{
		if (Rtt_StringCompareNoCase(actionName, "exitApplication") == 0)
		{
			//todo	app->Close(true);
			return true;
		}

		return false;
	}

	void LinuxPlatform::RuntimeErrorNotification(const char* errorType, const char* message, const char* stacktrace) const
	{
		SDL_Event e = {};
		e.type = sdl::OnRuntimeError;
		char** data = new char* [3];
		data[0] = strdup(errorType);
		data[1] = strdup(message);
		data[2] = strdup(stacktrace);
		e.user.data1 = data;
		SDL_PushEvent(&e);
	}

	const MCrypto& LinuxPlatform::GetCrypto() const
	{
		return fCrypto;
	}

	void LinuxPlatform::GetPreference(Category category, Rtt::String* value) const
	{
		// Validate.
		if (value == NULL)
		{
			return;
		}

		// Fetch the requested preference value.
		const char* resultPointer = "";
		string localeName = "En:En";		// fixme
		switch (category)
		{
		case kLocaleLanguage:
			resultPointer = localeName.substr(0, 2).c_str();
			break;
		case kLocaleCountry:
			resultPointer = localeName.substr(3, 5).c_str();
			break;
		case kLocaleIdentifier:
		case kUILanguage:
		{
			resultPointer = localeName.c_str();
			break;
		}
		case kDefaultStatusBarFile:
			break;
		case kDarkStatusBarFile:
			break;
		case kTranslucentStatusBarFile:
			break;
		case kLightTransparentStatusBarFile:
			break;
		case kDarkTransparentStatusBarFile:
			break;
		case kScreenDressingFile:
			break;
		case kSubscription:
			break;
		default:
			//Rtt_ASSERT_NOT_REACHED();
			break;
		}

		// Copy the requested preference string to the given value.
		// Set's value to NULL (an empty string) if the given preference is not supported.
		value->Set(resultPointer);
	}

	void LinuxPlatform::SetActivityIndicator(bool visible) const
	{
		app->SetActivityIndicator(visible);
	}

	PlatformWebPopup* LinuxPlatform::GetWebPopup() const
	{
		if (!fWebPopup)
		{
			fWebPopup = new LinuxWebPopup();
		}
		return fWebPopup;
	}

	bool LinuxPlatform::CanShowPopup(const char* name) const
	{
		return true;
	}

	bool LinuxPlatform::ShowPopup(lua_State* L, const char* name, int optionsIndex) const
	{
		bool result = false;

		// Validate.
		if (!L || !name || (strlen(name) <= 0))
		{
			return false;
		}

		// Display the requested window.
		if (Rtt_StringCompareNoCase(name, "mail") == 0)
		{
			// Display a "Send Mail" window.
		}
		else if (Rtt_StringCompareNoCase(name, "sms") == 0)
		{
			// nop
		}
		else if ((Rtt_StringCompareNoCase(name, "rateApp") == 0) || (Rtt_StringCompareNoCase(name, "appStore") == 0))
		{
			// nop
		}
		else
		{
			// Unknown window/popup name. Abort.
			result = false;
		}

		// Assume the window was displayed at this point.
		return result;
	}

	bool LinuxPlatform::HidePopup(const char* name) const
	{
		return true;
	}

	PlatformDisplayObject* LinuxPlatform::CreateNativeTextBox(const Rect& bounds) const
	{
		return new LinuxTextBoxObject(bounds, false);
	}

	PlatformDisplayObject* LinuxPlatform::CreateNativeTextField(const Rect& bounds) const
	{
		return new LinuxTextBoxObject(bounds, true);
	}

	void LinuxPlatform::SetKeyboardFocus(PlatformDisplayObject* textObject) const
	{
		if (textObject)
		{
			//			((LinuxDisplayObject *)textObject)->SetFocus();
		}
		else
		{
			//TODO: Remove keyboard focus from native display objects.
		}
	}

	PlatformDisplayObject* LinuxPlatform::CreateNativeMapView(const Rect& bounds) const
	{
		return NULL;
	}

	PlatformDisplayObject* LinuxPlatform::CreateNativeWebView(const Rect& bounds) const
	{
		return new LinuxWebView(bounds);
	}

	PlatformDisplayObject* LinuxPlatform::CreateNativeVideo(const Rect& bounds) const
	{
#if (wxUSE_MEDIACTRL == 1)
		return Rtt_NEW(&GetAllocator(), LinuxVideoObject(bounds));
#else
		return NULL;
#endif
	}

	PlatformDisplayObject* LinuxPlatform::GetNativeDisplayObjectById(const int objectId) const
	{
		return NULL;
	}

	Rtt_Real LinuxPlatform::GetStandardFontSize() const
	{
		return 16.0f;
	}

	S32 LinuxPlatform::GetFontNames(lua_State* L, int index) const
	{
		S32 numFonts = 0;
#if 0
		Rtt::StringArray fonts(fAllocator);

		if (NativeToJavaBridge::GetInstance()->GetFonts(fonts))
		{
			numFonts = fonts.GetLength();

			for (int i = 0; i < numFonts; i++)
			{
				lua_pushstring(L, fonts.GetElement(i));
				lua_rawseti(L, index, i + 1);
			}
		}
#endif

		return numFonts;
	}

	PlatformFont* LinuxPlatform::CreateFont(PlatformFont::SystemFont fontType, Rtt_Real size) const
	{
		return Rtt_NEW(fAllocator, LinuxFont(*fAllocator, fontType, size));
	}

	PlatformFont* LinuxPlatform::CreateFont(const char* fontName, Rtt_Real size) const
	{
		bool isBold = false;
		return Rtt_NEW(fAllocator, LinuxFont(*fAllocator, fontName, size, isBold));
	}

	void LinuxPlatform::SetTapDelay(Rtt_Real delay) const
	{
		// todo:
	}

	Rtt_Real LinuxPlatform::GetTapDelay() const
	{
		return 1.0f;
	}

	PlatformFBConnect* LinuxPlatform::GetFBConnect() const
	{
		return 0;
	}

	void* LinuxPlatform::CreateAndScheduleNotification(lua_State* L, int index) const
	{
		return NULL;
	}

	void LinuxPlatform::ReleaseNotification(void* notificationId) const
	{
	}

	void LinuxPlatform::CancelNotification(void* notificationId) const
	{
		if (notificationId == NULL)
		{
			//TODO: Cancel all notifications.
		}
		else
		{
			//TODO: Cancel the given notification by its ID.
		}
	}

	void LinuxPlatform::FlurryInit(const char* applicationKey) const
	{
	}

	void LinuxPlatform::FlurryEvent(const char* eventId) const
	{
	}

	void LinuxPlatform::SetNativeProperty(lua_State* L, const char* key, int valueIndex) const
	{
		// Validate arguments.
		if (!L || Rtt_StringIsEmpty(key) || !valueIndex)
		{
			return;
		}

		// Handle the given property value.
		if (Rtt_StringCompare(key, "windowMode") == 0)
		{
			if (lua_type(L, valueIndex) == LUA_TSTRING)
			{
				const char* windowMode = lua_tostring(L, valueIndex);

				if (windowMode != NULL)
				{
					if (Rtt_StringCompare(windowMode, "normal") == 0)
					{
						PushEvent(sdl::OnWindowNormal);
					}
					else if (Rtt_StringCompare(windowMode, "minimized") == 0)
					{
						PushEvent(sdl::OnWindowMinimized);
					}
					else if (Rtt_StringCompare(windowMode, "maximized") == 0)
					{
						PushEvent(sdl::OnWindowMaximized);
					}
					else if (Rtt_StringCompare(windowMode, "fullscreen") == 0)
					{
						PushEvent(sdl::OnWindowFullscreen);
					}
				}
			}
		}
		// window title text
		else if (Rtt_StringCompare(key, "windowTitleText") == 0)
		{
			if (lua_type(L, valueIndex) == LUA_TSTRING)
			{
				const char* windowTitle = lua_tostring(L, valueIndex);
				if (windowTitle != NULL)
				{
					app->SetTitle(windowTitle);
				}
			}
		}
		// mouse cursor
		else if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
		{
			if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
			{
				SDL_Event e = {};
				e.type = sdl::OnMouseCursorVisible;
				e.user.code = lua_toboolean(L, valueIndex);
				SDL_PushEvent(&e);
			}
		}
		// mouse cursor type
		else if (Rtt_StringCompare(key, "mouseCursor") == 0)
		{
			if (lua_type(L, valueIndex) == LUA_TSTRING)
			{
				const char* cursorName = lua_tostring(L, valueIndex);
				SDL_Event e = {};
				e.type = sdl::OnSetCursor;
				e.user.data1 = strdup(cursorName);
				SDL_PushEvent(&e);
			}
		}
		else
		{
			Rtt_LogException("SetNativeProperty %s is not implemented\n", key);
		}
	}

	int LinuxPlatform::PushNativeProperty(lua_State* L, const char* key) const
	{
		// Validate.
		if (!L)
		{
			return 0;
		}

		int pushedValues = 0;

		// Push the requested native property information to Lua.
		if (Rtt_StringCompare(key, "windowTitleText") == 0)
		{
			lua_pushstring(L, app->GetTitle().c_str());
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "windowMode") == 0)
		{
			bool isFullScreen = app->IsFullScreen();
			bool isMinimized = app->IsIconized();
			bool isMaximized = app->IsMaximized();
			bool isNormal = !isFullScreen || !isMinimized || !isMaximized;
			const char* windowMode = NULL;

			if (isFullScreen)
			{
				windowMode = "fullscreen";
			}
			else if (isMinimized)
			{
				windowMode = "minimized";
			}
			else if (isMaximized)
			{
				windowMode = "maximized";
			}
			else if (isNormal)
			{
				windowMode = "normal";
			}

			lua_pushstring(L, windowMode);
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
		{
			lua_pushboolean(L, isMouseCursorVisible);
			pushedValues = 1;
		}
		else
		{
			// The given key is unknown. Log a warning.
			if (Rtt_StringIsEmpty(key))
			{
				Rtt_LogException("native.getProperty() was given a nil or empty string key.");
			}
			else
			{
				Rtt_LogException("native.getProperty() was given unknown key: %s", key);
			}
		}

		// Push nil if given a key that is unknown on this platform.
		if (pushedValues <= 0)
		{
			lua_pushnil(L);
			pushedValues = 1;
		}

		// Return the number of values pushed into Lua.
		return pushedValues;
	}

	int LinuxPlatform::PushSystemInfo(lua_State* L, const char* key) const
	{
		// Validate.
		if (!L)
		{
			return 0;
		}

		// Push the requested system information to Lua.
		if (Rtt_StringCompare(key, "appName") == 0)
		{
			// Fetch the application's name.
			lua_pushstring(L, app->GetAppName().c_str());
		}
		else if (Rtt_StringCompare(key, "appVersionString") == 0)
		{
			// Return an empty version string since it is unknown by the simulator.
			lua_pushstring(L, ""); // todo
		}
		else if (Rtt_StringCompare(key, "bundleID") == 0)
		{
			lua_pushstring(L, ""); // todo
		}
		else if (Rtt_StringCompare(key, "isoCountryCode") == 0)
		{
			// Fetch the ISO 3166-1 country code.
			lua_pushstring(L, ""); // todo
		}
		else if (Rtt_StringCompare(key, "isoLanguageCode") == 0)
		{
			// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
			// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
			//       For example, this can happen with the Hawaiian language, which will return "haw".
			lua_pushstring(L, ""); // todo
		}
		else
		{
			// Push nil if given a key that is unknown on this platform.
			lua_pushnil(L);
		}

		// Return the number of values pushed into Lua.
		return 1;
	}

	void LinuxPlatform::NetworkBaseRequest(lua_State* L, const char* url, const char* method, LuaResource* listener, int paramsIndex, const char* path) const
	{
	}

	void LinuxPlatform::NetworkRequest(lua_State* L, const char* url, const char* method, LuaResource* listener, int paramsIndex) const
	{
		NetworkBaseRequest(L, url, method, listener, paramsIndex, NULL);
	}

	void LinuxPlatform::NetworkDownload(lua_State* L, const char* url, const char* method, LuaResource* listener, int paramsIndex, const char* filename, MPlatform::Directory baseDir) const
	{
		Runtime& runtime = *LuaContext::GetRuntime(L);
		const MPlatform& platform = runtime.Platform();

		String filePath(runtime.GetAllocator());
		platform.PathForFile(filename, baseDir, MPlatform::kDefaultPathFlags, filePath);
		NetworkBaseRequest(L, url, method, listener, paramsIndex, filePath.GetString());
	}

	PlatformReachability* LinuxPlatform::NewReachability(const ResourceHandle<lua_State>& handle, PlatformReachability::PlatformReachabilityType type, const char* address) const
	{
		return Rtt_NEW(fAllocator, PlatformReachability(handle, type, address));
	}

	bool LinuxPlatform::SupportsNetworkStatus() const
	{
		return false;
	}

	PlatformBitmap* LinuxPlatform::CreateBitmapMask(const char str[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset) const
	{
		return Rtt_NEW(&GetAllocator(), LinuxTextBitmap(GetAllocator(), str, font, (int)(w + 0.5f), (int)(h + 0.5f), alignment, baselineOffset));
	}

	FontMetricsMap LinuxPlatform::GetFontMetrics(const PlatformFont& font) const
	{
		FontMetricsMap ret;
		float ascent = 0;
		float descent = 0;
		float height = 0;
		float leading = 0;

		glyph_freetype_provider* gp = getGlyphProvider();
		if (gp)
		{
			gp->getMetrics(font.Name(), font.Size(), &ascent, &descent, &height, &leading);
		}

		ret["ascent"] = ascent;
		ret["descent"] = descent;
		ret["leading"] = leading;
		ret["height"] = height;
		return ret;
	}

	void LinuxPlatform::GetSafeAreaInsetsPixels(Rtt_Real& top, Rtt_Real& left, Rtt_Real& bottom, Rtt_Real& right) const
	{
		top = left = bottom = right = 0;
	}

	Preference::ReadValueResult LinuxPlatform::GetPreference(const char* categoryName, const char* keyName) const
	{
		// add category name as prefix
		std::string key(categoryName);

		if (strlen(keyName) > 0)
		{
			key += '.';
			key += keyName;
		}
		char val[1024];

		// maybe it would be better to use sqlite3 ??
		std::string path = fSystemCachesDir.GetString();
		path += '/';
		path += key;
		FILE* f = fopen(path.c_str(), "rb");
		bool rc = false;

		if (f)
		{
			int n = fread(val, 1, sizeof(val) - 1, f);
			val[n] = '\0';
			fclose(f);
			rc = true;
		}

		return rc == false ? Preference::ReadValueResult::kPreferenceNotFound : Preference::ReadValueResult::SucceededWith(val);
	}

	OperationResult LinuxPlatform::SetPreferences(const char* categoryName, const PreferenceCollection& preferences) const
	{
		bool rc = true;

		for (int index = preferences.GetCount() - 1; index >= 0 && rc == true; index--)
		{
			// Fetch the next preference to write to the database.
			auto preferencePointer = preferences.GetByIndex(index);

			if (preferencePointer != NULL)
			{
				// add category name as prefix
				std::string key(categoryName);
				key += '.';
				key += preferencePointer->GetKeyName();

				PreferenceValue pval = preferencePointer->GetValue();

				// Insert the preference value as string.
				PreferenceValue::StringResult strval = pval.ToString();
				const char* val = strval.GetValue()->c_str();

				// maybe it would be better to use sqlite3 ??
				std::string path = fSystemCachesDir.GetString();
				path += '/';
				path += key;
				FILE* f = fopen(path.c_str(), "wb");

				if (f)
				{
					fwrite(val, 1, strlen(val), f);
					fclose(f);
					rc = true;
				}
			}
		}
		return rc == false ? OperationResult::FailedWith("SetPreferences failed") : Rtt::OperationResult::kSucceeded;
	}

	OperationResult LinuxPlatform::DeletePreferences(const char* categoryName, const char** keyNameArray, U32 keyNameCount) const
	{
		bool rc = true;

		for (int i = 0; i < keyNameCount && keyNameArray && rc; i++)
		{
			// Fetch the next preference key, add category name as prefix
			std::string key(categoryName);
			key += '.';
			key += keyNameArray[i];

			// maybe it would be better to use sqlite3 ??
			std::string path = fSystemCachesDir.GetString();
			path += '/';
			path += key;
			int n = Rtt_DeleteFile(path.c_str());
			rc = n == 0 ? true : false;
		}
		return rc == false ? OperationResult::FailedWith("DeletePreferences failed") : Rtt::OperationResult::kSucceeded;
	}

	void LinuxPlatform::Suspend() const
	{
	}

	void LinuxPlatform::Resume() const
	{
	}

	void LinuxPlatform::setWindow(void* ctx)
	{
		Rtt_ASSERT(fScreenSurface);
		fScreenSurface->setWindow(ctx);
	}

#ifdef Rtt_AUTHORING_SIMULATOR
	void LinuxPlatform::SetCursorForRect(const char* cursorName, int x, int y, int width, int height) const
	{
		if (Rtt_StringCompareNoCase(cursorName, "arrow") == 0)
		{
		}
		else if (Rtt_StringCompareNoCase(cursorName, "closedHand") == 0)
		{
		}
		else if (Rtt_StringCompareNoCase(cursorName, "openHand") == 0)
		{
		}
		else if (Rtt_StringCompareNoCase(cursorName, "crosshair") == 0)
		{
		}
		else if (Rtt_StringCompareNoCase(cursorName, "notAllowed") == 0)
		{
		}
		else if (Rtt_StringCompareNoCase(cursorName, "pointingHand") == 0)
		{
		}
		else
		{
			// Remove any rect with these bounds...
			return;
		}

		// Store the rectangle within the control's client area that the give mouse cursor should be displayed.
	}
#endif

}; // namespace Rtt

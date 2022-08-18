//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_GPUStream.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_Runtime.h"
#include "Rtt_EmscriptenPlatform.h"
#include "Rtt_EmscriptenAudioPlayer.h"
#include "Rtt_EmscriptenAudioRecorder.h"
#include "Rtt_EmscriptenBitmap.h"
#include "Rtt_EmscriptenEventSound.h"
#include "Rtt_EmscriptenFBConnect.h"
#include "Rtt_EmscriptenFont.h"
#include "Rtt_EmscriptenImageProvider.h"
#include "Rtt_EmscriptenMapViewObject.h"
#include "Rtt_EmscriptenScreenSurface.h"
#include "Rtt_EmscriptenStoreProvider.h"
#include "Rtt_EmscriptenTextBoxObject.h"
#include "Rtt_EmscriptenVideoObject.h"
#include "Rtt_EmscriptenVideoPlayer.h"
#include "Rtt_EmscriptenVideoProvider.h"
#include "Rtt_EmscriptenWebPopup.h"
#include "Rtt_EmscriptenWebViewObject.h"
#include "Rtt_EmscriptenContainer.h"
#include "Rtt_PreferenceCollection.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc

extern "C"
{

	// alert ==> Lua callback
	void EMSCRIPTEN_KEEPALIVE jsAlertCallback(int selectedButtonID, Rtt::LuaResource* resource)
	{
		// Invoke the Lua listener.
		if (resource)
		{
			// Invoke the Lua listener.
			Rtt::LuaResource *pLuaResource = (Rtt::LuaResource *)resource;
			bool wasCanceled = selectedButtonID >= 0 ? false : true;
			Rtt::LuaLibNative::AlertComplete(*pLuaResource, selectedButtonID, wasCanceled);

			// Delete the Lua resource.
			Rtt_DELETE(pLuaResource);
		}
	}

	// JS API
		extern const char* jsGetLocaleLanguage();
		extern const char* jsGetLocaleCountry();
		extern const char* jsGetLanguage();
		extern bool jsOpenURL(const char* url);
		extern int jsShowAlert(const char* title, const char* msg, const char **buttonLabels, U32 numButtons, Rtt::LuaResource* resource, int pointerSize);
		extern void	jsSetActivityIndicator(bool visible);
		extern bool jsGetPreference(const char* key, char* val);
		extern bool jsSetPreference(const char* key, const char* val);
		extern bool jsDeletePreference(const char* key);

		const char* jsGetLocaleLanguageStatic() {
			static std::string ret(jsGetLocaleLanguage());
			return ret.c_str();
		}
		const char* jsGetLocaleCountryStatic() {
			static std::string ret(jsGetLocaleCountry());
			return ret.c_str();
		}
		const char* jsGetLanguageStatic() {
			static std::string ret(jsGetLanguage());
			return ret.c_str();
		}

}
#else
extern "C"
{
		extern const char* jsGetLocaleLanguageStatic() { return ""; }
		extern const char* jsGetLocaleCountryStatic() { return ""; }
		extern const char* jsGetLanguageStatic() { return ""; }
		bool jsOpenURL(const char* url) { return false; }
		int jsShowAlert(const char* title, const char* msg, const char **buttonLabels, U32 numButtons, Rtt::LuaResource* resource, int pointerSize) { return 0; }
		void	jsSetActivityIndicator(bool visible) {}
		bool jsGetPreference(const char* key, char* val) { return false; }
		bool jsSetPreference(const char* key, const char* val) { return false; }
		bool jsDeletePreference(const char* key) { return false; }
}
#endif

#if !defined( Rtt_CUSTOM_CODE )
Rtt_EXPORT const luaL_Reg* Rtt_GetCustomModulesList()
{
	return NULL;
}
#endif

namespace Rtt
{

	EmscriptenPlatform::EmscriptenPlatform(const char *resourceDir, const char *documentsDir, const char *temporaryDir,
		const char *cachesDir, const char *systemCachesDir)
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
		fStoreProvider(NULL),
		fFBConnect(NULL),
		fScreenSurface(NULL)
	{
		fResourceDir.Set(resourceDir);
		fDocumentsDir.Set(documentsDir);
		fTemporaryDir.Set(temporaryDir);
		fCachesDir.Set(cachesDir);
		fSystemCachesDir.Set(systemCachesDir);
	}

	EmscriptenPlatform::EmscriptenPlatform(int width, int height)
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
		fStoreProvider(NULL),
		fFBConnect(NULL),
		fScreenSurface(NULL)
	{
		fResourceDir.Set("");
		fDocumentsDir.Set("");
		fTemporaryDir.Set("");
		fCachesDir.Set("");
		fSystemCachesDir.Set("");
	}


	EmscriptenPlatform::~EmscriptenPlatform()
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

	MPlatformDevice& EmscriptenPlatform::GetDevice() const
	{
		return const_cast<EmscriptenDevice&>(fDevice);
	}

	PlatformSurface* EmscriptenPlatform::CreateScreenSurface() const
	{
		Rtt_ASSERT(fScreenSurface == NULL);
		fScreenSurface = Rtt_NEW(fAllocator, EmscriptenScreenSurface());
		return fScreenSurface;
	}

	PlatformSurface* EmscriptenPlatform::CreateOffscreenSurface(const PlatformSurface& parent) const
	{
		return Rtt_NEW(fAllocator, EmscriptenOffscreenSurface(parent));
	}

	int EmscriptenPlatform::CanOpenURL(const char* url) const
	{
		return true;
	}

	bool EmscriptenPlatform::OpenURL(const char* url) const
	{
		return jsOpenURL(url);
	}

	PlatformVideoPlayer* EmscriptenPlatform::GetVideoPlayer(const Rtt::ResourceHandle<lua_State> & handle) const
	{
		if (!fVideoPlayer)
		{
			int w, h;
			fScreenSurface->getWindowSize(&w, &h);

			fVideoPlayer = Rtt_NEW(fAllocator, EmscriptenVideoPlayer(handle, *fAllocator, w, h));
		}
		return fVideoPlayer;
	}

	PlatformImageProvider* EmscriptenPlatform::GetImageProvider(const Rtt::ResourceHandle<lua_State> & handle) const
	{
		if (!fImageProvider)
		{
			int w, h;
			fScreenSurface->getWindowSize(&w, &h);

			fImageProvider = Rtt_NEW(fAllocator, EmscriptenImageProvider(handle, w, h));
		}
		return fImageProvider;
	}

	PlatformVideoProvider* EmscriptenPlatform::GetVideoProvider(const Rtt::ResourceHandle<lua_State> & handle) const
	{
		if (!fVideoProvider)
		{
			int w, h;
			fScreenSurface->getWindowSize(&w, &h);

			fVideoProvider = Rtt_NEW(fAllocator, EmscriptenVideoProvider(handle, w, h));
		}
		return fVideoProvider;
	}

	PlatformStoreProvider* EmscriptenPlatform::GetStoreProvider(const ResourceHandle<lua_State>& handle) const
	{
		if (!fStoreProvider)
		{
			fStoreProvider = Rtt_NEW(Allocator(), EmscriptenStoreProvider(handle));
		}
		return fStoreProvider;
	}

	void EmscriptenPlatform::SetIdleTimer(bool enabled) const
	{
	}

	bool EmscriptenPlatform::GetIdleTimer() const
	{
		return true;
	}


	NativeAlertRef EmscriptenPlatform::ShowNativeAlert(const char *title, const char *msg, const char **buttonLabels, U32 numButtons, LuaResource* resource) const
	{
		jsShowAlert( title, msg, buttonLabels, numButtons, resource, sizeof(void*));
		return NULL; // (NativeAlertRef) 0x1234;
	}

	void EmscriptenPlatform::CancelNativeAlert(NativeAlertRef alert, S32 index) const
	{
		// modal msgbox
		Rtt_ASSERT_MSG(0, "Code should NOT be reached");
	}

	Rtt_Allocator& EmscriptenPlatform::GetAllocator() const
	{
		return *fAllocator;
	}

	RenderingStream* EmscriptenPlatform::CreateRenderingStream(bool antialias) const
	{
		RenderingStream* result = Rtt_NEW(fAllocator, GPUStream(fAllocator));
		result->SetProperty(RenderingStream::kFlipHorizontalAxis, true);
		return result;
	}

	PlatformTimer* EmscriptenPlatform::CreateTimerWithCallback(MCallback& callback) const
	{
		return Rtt_NEW(fAllocator, EmscriptenTimer(callback));
	}

	PlatformBitmap* EmscriptenPlatform::CreateBitmap(const char* path, bool convertToGrayscale) const
	{
		PlatformBitmap *result = NULL;
		if (path)
		{
			if (convertToGrayscale)
			{
				result = Rtt_NEW(&GetAllocator(), EmscriptenMaskFileBitmap(GetAllocator(), path));
			}
			else
			{
				result = Rtt_NEW(&GetAllocator(), EmscriptenFileBitmap(GetAllocator(), path));
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

	bool EmscriptenPlatform::FileExists(const char *filename) const
	{
		// Validate.
		if (Rtt_StringIsEmpty(filename))
		{
			return false;
		}

		// Determine if the given file name exists.
		bool fileExists = false;
		FILE *file = fopen(filename, "r");
		if (file)
		{
			fileExists = true;
			fclose(file);
		}
		return fileExists;
	}

	void EmscriptenPlatform::PathForFile(const char *filename, MPlatform::Directory baseDir, U32 flags, String& result) const
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
			case MPlatform::kSystemResourceDir:
				PathForFile(filename, fResourceDir.GetString(), result);

				Rtt_WARN_SIM(
					!filename || FileExists(result.GetString()),
					("WARNING: Cannot create path for resource file '%s (%s)'. File does not exist.\n\n", filename, result.GetString()));
				break;

			case MPlatform::kCachesDir:
				PathForFile(filename, fCachesDir.GetString(), result);
				break;

			case MPlatform::kSystemCachesDir:
				PathForFile(filename, fSystemCachesDir.GetString(), result);
				break;

			case MPlatform::kTmpDir:
				PathForFile(filename, fTemporaryDir.GetString(), result);
				break;

			case MPlatform::kPluginsDir:
				break;

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

	void EmscriptenPlatform::SetProjectResourceDirectory(const char* filename)
	{
//		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	void EmscriptenPlatform::SetSkinResourceDirectory(const char* filename)
	{
//		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	void EmscriptenPlatform::PathForFile(const char * filename, const char * baseDir, String & result) const
	{
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

	PlatformEventSound * EmscriptenPlatform::CreateEventSound(const ResourceHandle<lua_State> & handle, const char* filePath) const
	{
		EmscriptenEventSound *p = Rtt_NEW(fAllocator, EmscriptenEventSound(handle, *fAllocator));
		p->Load(filePath);
		return p;
	}

	void EmscriptenPlatform::ReleaseEventSound(PlatformEventSound * soundID) const
	{
		Rtt_DELETE(soundID);
	}

	void EmscriptenPlatform::PlayEventSound(PlatformEventSound * soundID) const
	{
		soundID->Play();
	}

	PlatformAudioPlayer * EmscriptenPlatform::GetAudioPlayer(const ResourceHandle<lua_State> & handle) const
	{
		if (!fAudioPlayer)
		{
			fAudioPlayer = Rtt_NEW(fAllocator, EmscriptenAudioPlayer(handle, *fAllocator));
		}
		return fAudioPlayer;
	}

	PlatformAudioRecorder * EmscriptenPlatform::CreateAudioRecorder(const Rtt::ResourceHandle<lua_State> & handle, const char * filePath) const
	{
		return Rtt_NEW(fAllocator, EmscriptenAudioRecorder(handle, *fAllocator, filePath));
	}

	void EmscriptenPlatform::RaiseError(MPlatform::Error e, const char * reason) const
	{
		const char kNull[] = "(null)";
		if (!reason) { reason = kNull; }
		Rtt_TRACE(("MPlatformFactory error(%d): %s\n", e, kNull));
	}

	bool EmscriptenPlatform::SaveImageToPhotoLibrary(const char* filePath) const
	{
		return false;
	}

	bool EmscriptenPlatform::SaveBitmap(PlatformBitmap * bitmap, const char * filePath, float jpegQuality) const
	{
		return EmscriptenBaseBitmap::SaveBitmap(fAllocator, bitmap, filePath);
	}

	bool EmscriptenPlatform::AddBitmapToPhotoLibrary(PlatformBitmap* bitmap) const
	{
		return SaveBitmap(bitmap, NULL, 1.0f);
	}

	void EmscriptenPlatform::SetStatusBarMode(MPlatform::StatusBarMode newValue) const
	{
	}

	MPlatform::StatusBarMode EmscriptenPlatform::GetStatusBarMode() const
	{
		return MPlatform::kDefaultStatusBar;
	}

	int EmscriptenPlatform::GetStatusBarHeight() const
	{
		return 0;
	}

	int EmscriptenPlatform::GetTopStatusBarHeightPixels()  const
	{
		return 0;
	}

	int EmscriptenPlatform::GetBottomStatusBarHeightPixels() const
	{
		return 0;
	}

	int EmscriptenPlatform::SetSync(lua_State* L) const
	{
		return 0;
	}

	int EmscriptenPlatform::GetSync(lua_State* L) const
	{
		return 0;
	}

	void EmscriptenPlatform::BeginRuntime(const Runtime& runtime) const
	{
	}

	void EmscriptenPlatform::EndRuntime(const Runtime& runtime) const
	{
	}

	PlatformExitCallback* EmscriptenPlatform::GetExitCallback()
	{
		return NULL;
	}

	bool EmscriptenPlatform::RequestSystem(lua_State *L, const char *actionName, int optionsIndex) const
	{
		return false;
	}

	void EmscriptenPlatform::RuntimeErrorNotification(const char *errorType, const char *message, const char *stacktrace) const
	{
	}

	const MCrypto& EmscriptenPlatform::GetCrypto() const
	{
		return fCrypto;
	}

	void EmscriptenPlatform::GetPreference(Category category, Rtt::String * value) const
	{
		// Validate.
		if (value == NULL)
		{
			return;
		} 

		// Fetch the requested preference value.
		const char *resultPointer = "";
		switch (category)
		{
		case kLocaleLanguage:
			resultPointer = jsGetLocaleLanguageStatic();
			break;
		case kLocaleCountry:
			resultPointer = jsGetLocaleCountryStatic();
			break;
		case kLocaleIdentifier:
		case kUILanguage:
		{
			resultPointer = jsGetLanguageStatic();
			break;
		}
		case kDefaultStatusBarFile:
			// todo
			break;
		case kDarkStatusBarFile:
			// todo
			break;
		case kTranslucentStatusBarFile:
			// todo
			break;
		case kLightTransparentStatusBarFile:
			// todo
			break;
		case kDarkTransparentStatusBarFile:
			// todo
			break;
		case kScreenDressingFile:
			// todo
			break;
		case kSubscription:
			// todo
			break;
		default:
			//			Rtt_ASSERT_NOT_REACHED();
			break;
		}

		// Copy the requested preference string to the given value.
		// Set's value to NULL (an empty string) if the given preference is not supported.
		value->Set(resultPointer);
	}

	void EmscriptenPlatform::SetActivityIndicator(bool visible) const
	{
		jsSetActivityIndicator(visible);
	}

	PlatformWebPopup* EmscriptenPlatform::GetWebPopup() const
	{
		if (!fWebPopup)
		{
			fWebPopup = Rtt_NEW(&GetAllocator(), EmscriptenWebPopup());
		}
		return fWebPopup;
	}

	bool EmscriptenPlatform::CanShowPopup(const char *name) const
	{
		return true;
	}

	bool EmscriptenPlatform::ShowPopup(lua_State *L, const char *name, int optionsIndex) const
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
			// Display a "Send SMS" window.
		}
		else if ((Rtt_StringCompareNoCase(name, "rateApp") == 0) || (Rtt_StringCompareNoCase(name, "appStore") == 0))
		{
			// Display the app store window.
		}
		else
		{
			// Unknown window/popup name. Abort.
			result = false;
		}

		// Assume the window was displayed at this point.
		return result;
	}

	bool EmscriptenPlatform::HidePopup(const char *name) const
	{
		return true;
	}

	PlatformDisplayObject* EmscriptenPlatform::CreateNativeTextBox(const Rect& bounds) const
	{
		return Rtt_NEW(&GetAllocator(), EmscriptenTextBoxObject(bounds, false));
	}

	PlatformDisplayObject * EmscriptenPlatform::CreateNativeTextField(const Rect & bounds) const
	{
		return Rtt_NEW(&GetAllocator(), EmscriptenTextBoxObject(bounds, true));
	}

	void EmscriptenPlatform::SetKeyboardFocus(PlatformDisplayObject * textObject) const
	{
		if (textObject)
		{
			((EmscriptenDisplayObject*)textObject)->SetFocus();
		}
		else
		{
			//TODO: Remove keyboard focus from native display objects.
		}
	}

	PlatformDisplayObject * EmscriptenPlatform::CreateNativeMapView(const Rect& bounds) const
	{
		return Rtt_NEW(&GetAllocator(), EmscriptenMapViewObject(bounds));
	}

	PlatformDisplayObject * EmscriptenPlatform::CreateNativeWebView(const Rect& bounds) const
	{
		return Rtt_NEW(&GetAllocator(), EmscriptenWebViewObject(bounds));
	}

	PlatformDisplayObject * EmscriptenPlatform::CreateNativeVideo(const Rect& bounds) const
	{
		return Rtt_NEW(&GetAllocator(), EmscriptenVideoObject(bounds));
	}

	PlatformDisplayObject * EmscriptenPlatform::GetNativeDisplayObjectById(const int objectId) const
	{
		return NULL;
	}

	Rtt_Real EmscriptenPlatform::GetStandardFontSize() const
	{
		return 32.0f;
	}

	S32 EmscriptenPlatform::GetFontNames(lua_State *L, int index) const
	{
		S32 numFonts = 0;
#if 0
		Rtt::StringArray fonts(fAllocator);
		if (NativeToJavaBridge::GetInstance()->GetFonts(fonts)) {
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

	PlatformFont * EmscriptenPlatform::CreateFont(PlatformFont::SystemFont fontType, Rtt_Real size) const
	{
		return Rtt_NEW(fAllocator, EmscriptenFont(*fAllocator, fontType, size));
	}

	PlatformFont * EmscriptenPlatform::CreateFont(const char *fontName, Rtt_Real size) const
	{
		bool isBold = false;
		return Rtt_NEW(fAllocator, EmscriptenFont(*fAllocator, fontName, size, isBold));
	}

	void EmscriptenPlatform::SetTapDelay(Rtt_Real delay) const
	{
	}

	Rtt_Real EmscriptenPlatform::GetTapDelay() const
	{
		return 1.0f;
	}

	PlatformFBConnect* EmscriptenPlatform::GetFBConnect() const
	{
		if (NULL == fFBConnect)
		{
			fFBConnect = Rtt_NEW(fAllocator, EmscriptenFBConnect);
		}
		return fFBConnect;
	}

	void* EmscriptenPlatform::CreateAndScheduleNotification(lua_State *L, int index) const
	{
		return NULL;
	}

	void EmscriptenPlatform::ReleaseNotification(void *notificationId) const
	{
	}

	void EmscriptenPlatform::CancelNotification(void *notificationId) const
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

	void EmscriptenPlatform::FlurryInit(const char * applicationKey) const
	{
	}

	void EmscriptenPlatform::FlurryEvent(const char * eventId) const
	{
	}

	// native.setProperty(key, val)
	void EmscriptenPlatform::SetNativeProperty(lua_State *L, const char *key, int valueIndex) const
	{
		// Validate arguments.
		if (!L || Rtt_StringIsEmpty(key) || !valueIndex)
		{
			return;
		}

		if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
		{
			// Show/hide the mouse cursor. (Only supported when running in desktop app mode.)
			if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
			{
				SDL_ShowCursor(lua_toboolean(L, valueIndex) ? SDL_ENABLE : SDL_DISABLE);
			}
			else
			{
				CoronaLuaWarning(L, "native.setProperty(\"%s\") was given an invalid value type.", key);
			}
		}
		else
		{
			CoronaLuaWarning(L, "native.setProperty(\"%s\") is not supported on HTML5", key);
		}
	}

	int EmscriptenPlatform::PushNativeProperty(lua_State *L, const char *key) const
	{
		// Validate.
		if (!L)
		{
			return 0;
		}

		int pushedValues = 0;

		if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
		{
			int val = SDL_ShowCursor(SDL_QUERY);
			lua_pushboolean(L, val == SDL_ENABLE ? true : false);
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
				Rtt_LogException("native.getProperty() was given unsupported key: %s", key);
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

	int EmscriptenPlatform::PushSystemInfo(lua_State *L, const char *key) const
	{
		// Validate.
		if (!L)
		{
			return 0;
		}

		// Push the requested system information to Lua.
		int pushedValues = 0;
		if (Rtt_StringCompare(key, "appName") == 0)
		{
			// Fetch the application's name.
			lua_pushstring(L, "appName");		// todo
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "appVersionString") == 0)
		{
			// Return an empty version string since it is unknown by the simulator.
			lua_pushstring(L, "appVersionString");		// todo
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "bundleID") == 0)
		{
			lua_pushstring(L, "bundleID");		// todo
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "isoCountryCode") == 0)
		{
			// Fetch the ISO 3166-1 country code.
			lua_pushstring(L, "isoCountryCode");	// todo
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "isoLanguageCode") == 0)
		{
			// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
			// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
			//       For example, this can happen with the Hawaiian language, which will return "haw".
			lua_pushstring(L, "isoLanguageCode");	// todo
			pushedValues = 1;
		}
		else
		{
			// Push nil if given a key that is unknown on this platform.
			lua_pushnil(L);
			pushedValues = 1;
		}

		// Return the number of values pushed into Lua.
		return pushedValues;
	}

	void EmscriptenPlatform::NetworkBaseRequest(lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *path) const
	{
	}

	void EmscriptenPlatform::NetworkRequest(lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex) const
	{
		NetworkBaseRequest(L, url, method, listener, paramsIndex, NULL);
	}

	void EmscriptenPlatform::NetworkDownload(lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *filename, MPlatform::Directory baseDir) const
	{
		Runtime& runtime = *LuaContext::GetRuntime(L);
		const MPlatform& platform = runtime.Platform();

		String filePath(runtime.GetAllocator());
		platform.PathForFile(filename, baseDir, MPlatform::kDefaultPathFlags, filePath);
		NetworkBaseRequest(L, url, method, listener, paramsIndex, filePath.GetString());
	}


	PlatformReachability* EmscriptenPlatform::NewReachability(const ResourceHandle<lua_State>& handle, PlatformReachability::PlatformReachabilityType type, const char* address) const
	{
		return Rtt_NEW(fAllocator, PlatformReachability(handle, type, address));
	}

	bool EmscriptenPlatform::SupportsNetworkStatus() const
	{
		return false;
	}

	PlatformBitmap* EmscriptenPlatform::CreateBitmapMask(const char str[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset) const
	{
		return Rtt_NEW(&GetAllocator(), EmscriptenTextBitmap(GetAllocator(), str, font, (int)(w + 0.5f), (int)(h + 0.5f), alignment, baselineOffset));
	}


	FontMetricsMap EmscriptenPlatform::GetFontMetrics(const PlatformFont& font) const
	{
		FontMetricsMap ret;
		ret["ascent"] = font.Size(); //TTF_FontAscent(f);
		ret["descent"] = 0; //TTF_FontDescent(f);
		ret["leading"] = 0; //TTF_FontLineSkip(f) - TTF_FontHeight(f);
		ret["height"] = 0; //TTF_FontHeight(f);
		Rtt_LogException("GetFontMetrics is not implemented yet");
		return ret;
	}

	void EmscriptenPlatform::GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const
	{
		top = left = bottom = right = 0;
	}

	Preference::ReadValueResult EmscriptenPlatform::GetPreference(const char* categoryName, const char* keyName) const
	{
		// add category name as prefix
		std::string key(categoryName);
		key += '.';
		key += keyName;
		char val[1024] = {0};
		bool rc = jsGetPreference(key.c_str(), val);
		return rc == false ? Preference::ReadValueResult::kPreferenceNotFound : Preference::ReadValueResult::SucceededWith(val);
	}

	OperationResult EmscriptenPlatform::SetPreferences(const char* categoryName, const PreferenceCollection& preferences) const
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
				rc = jsSetPreference(key.c_str(), val);
			}
		}
		return rc == false ? OperationResult::FailedWith("This API is not supported on this platform.") : Rtt::OperationResult::kSucceeded;
	}

	OperationResult EmscriptenPlatform::DeletePreferences(const char* categoryName, const char** keyNameArray, U32 keyNameCount) const
	{
		bool rc = true;
		for (int i = 0; i < keyNameCount && keyNameArray && rc; i++)
		{
			// Fetch the next preference key, add category name as prefix
			std::string key(categoryName);
			key += '.';
			key += keyNameArray[i];
			rc = jsDeletePreference(key.c_str());
		}
		return rc == false ? OperationResult::FailedWith("This API is not supported on this platform.") : Rtt::OperationResult::kSucceeded;
	}


	void EmscriptenPlatform::Suspend() const
	{
	}

	void EmscriptenPlatform::Resume() const
	{
	}

	void EmscriptenPlatform::setWindow(SDL_Window *window, DeviceOrientation::Type orientation)
	{
		Rtt_ASSERT(fScreenSurface);
		fScreenSurface->setWindow(window, orientation); 
	}

	// ----------------------------------------------------------------------------

	} // namespace Rtt

// ----------------------------------------------------------------------------

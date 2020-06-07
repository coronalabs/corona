//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinPlatform.h"
#include "Core\Rtt_Build.h"
#include "Interop\Graphics\FontSizeConverter.h"
#include "Interop\Graphics\HorizontalAlignment.h"
#include "Interop\Ipc\CommandLineRunner.h"
#include "Interop\Storage\MStoredPreferences.h"
#include "Interop\Storage\RegistryStoredPreferences.h"
#include "Interop\Storage\StoredPreferencesProxy.h"
#include "Interop\UI\CoronaTaskDialogHandler.h"
#include "Interop\UI\RenderSurfaceControl.h"
#include "Interop\UI\Window.h"
#include "Interop\ApplicationServices.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "CoronaLua.h"
#include "Rtt_Allocator.h"
#include "Rtt_FileSystem.h"
#include "Rtt_GPUStream.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_NativeWindowMode.h"
#include "Rtt_PlatformInAppStore.h"
#include "Rtt_Runtime.h"
#include "Rtt_String.h"
#include "Rtt_WinAudioPlayer.h"
#include "Rtt_WinAudioRecorder.h"
#include "Rtt_WinBitmap.h"
#include "Rtt_WinCrypto.h"
#include "Rtt_WinEventSound.h"
#include "Rtt_WinFBConnect.h"
#include "Rtt_WinFont.h"
#include "Rtt_WinImageProvider.h"
#include "Rtt_WinScreenSurface.h"
#include "Rtt_WinTextBoxObject.h"
#include "Rtt_WinTimer.h"
#include "Rtt_WinVideoObject.h"
#include "Rtt_WinVideoPlayer.h"
#include "Rtt_WinVideoProvider.h"
#include "Rtt_WinWebPopup.h"
#include "Rtt_WinWebViewObject.h"
#include "WinString.h"
#include <algorithm>
#include <Gdiplus.h>
#include <Gdipluscolor.h>
#include <io.h>
#include <shellapi.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <WinInet.h>


#if !defined( Rtt_CUSTOM_CODE )
Rtt_EXPORT const luaL_Reg* Rtt_GetCustomModulesList()
{
	return nullptr;
}
#endif


namespace Rtt
{

	WinPlatform::WinPlatform(Interop::RuntimeEnvironment& environment)
		: fEnvironment(environment),
		fDevice(environment),
		fAudioPlayer(nullptr),
		fVideoPlayer(nullptr),
		fImageProvider(nullptr),
		fVideoProvider(nullptr),
		fWebPopup(nullptr),
		fFBConnect(nullptr),
		fExitCallback(environment),
		fIsIdleTimerEnabled(true)
	{
		// Set up a plugin DLL lookup path if one was provided and it doesn't reference the EXE's directory.
		auto pluginsDirectoryPath = fEnvironment.GetUtf16PathFor(kPluginsDir);
		if (pluginsDirectoryPath && (pluginsDirectoryPath[0] != L'\0') &&
			wcscmp(pluginsDirectoryPath, Interop::ApplicationServices::GetDirectoryPath()))
		{
			// Add the plugins directory to the look up path via the SetDllDirectory() Win32 call.
			// Note: We only do this for Corona apps such as the Corona Simulator or Corona Shell.
			//       It's not appropriate for the library version of Corona to call SetDllDirectory() since it can
			//       only be set to one path for the entire application. This API is for the EXE developer to use.
			if (Interop::ApplicationServices::IsCoronaSdkApp())
			{
				static bool sWasSetDllDirectoryCalled = false;
				if (!sWasSetDllDirectoryCalled)
				{
					::SetDllDirectoryW(pluginsDirectoryPath);
					sWasSetDllDirectoryCalled = true;
				}
			}

			// Add the plugins directory to the beginning of the PATH environment variable.
			// Note: We also need to do this for Corona apps because some systems (for whatever reason)
			//       ignore the path set by the SetDllDirectory() call up above.
			std::wstring newUtf16Paths(pluginsDirectoryPath);
			newUtf16Paths += L';';
			bool canSetPath = true;
			auto pathsLength = ::GetEnvironmentVariableW(L"PATH", nullptr, 0);
			if (pathsLength > 0)
			{
				pathsLength++;
				auto oldUtf16Paths = new wchar_t[pathsLength];
				oldUtf16Paths[0] = L'\0';
				::GetEnvironmentVariableW(L"PATH", oldUtf16Paths, pathsLength);
				canSetPath = (wcsstr(oldUtf16Paths, newUtf16Paths.c_str()) == nullptr);
				if (canSetPath)
				{
					// The plugins directory has not been assigned to the PATH environment variable before.
					// Concatenate the two strings, to be assigned to PATH down below.
					newUtf16Paths += oldUtf16Paths;
				}
				delete[] oldUtf16Paths;
			}
			if (canSetPath)
			{
				::SetEnvironmentVariableW(L"PATH", newUtf16Paths.c_str());
			}
		}
	}

	WinPlatform::~WinPlatform()
	{
		// If the idle timer was disabled then re-enable it to allow Windows to sleep when idle.
		if (GetIdleTimer() == false)
		{
			SetIdleTimer(true);
		}

		// Clear the activity indicator, if shown.
		SetActivityIndicator(false);

		// Delete platform's owned objects.
		Rtt_DELETE(fFBConnect);
		Rtt_DELETE(fVideoPlayer);
		Rtt_DELETE(fAudioPlayer);
		Rtt_DELETE(fWebPopup);
	}

	MPlatformDevice& WinPlatform::GetDevice() const
	{
		return const_cast<WinDevice&>(fDevice);
	}

	PlatformSurface* WinPlatform::CreateScreenSurface() const
	{
		return Rtt_NEW(&GetAllocator(), WinScreenSurface(fEnvironment));
	}

	PlatformSurface* WinPlatform::CreateOffscreenSurface(const PlatformSurface& parent) const
	{
		return nullptr;
	}

	/// Saves the given bitmap to Window's "My Pictures\Corona Simulator" directory.
	/// @param bitmap The bitmap object to be saved. Cannot be NULL.
	/// @return Returns TRUE if successfully saved given bitmap to file.
	///         Returns FALSE if unable to save or was given an invalid bitmap object.
	bool WinPlatform::AddBitmapToPhotoLibrary(PlatformBitmap* bitmap) const
	{
		WinString appName;
		wchar_t utf16FileName[MAX_PATH];
		wchar_t utf16PathName[MAX_PATH];
		HRESULT hr;
		int result;
		int index;

		// Validate.
		if (nullptr == bitmap)
		{
			return false;
		}

		// Fetch the app name to be used for the directory path below.
		if (Interop::ApplicationServices::IsCoronaSdkApp())
		{
			appName.SetUTF16(L"Corona Simulator");
		}
		else
		{
			CopyAppNameTo(appName);
		}

		// Create a directory under "My Pictures" for the simulator to save pictures to, if not done already.
		utf16PathName[0] = L'\0';
		hr = ::SHGetFolderPathW(nullptr, CSIDL_MYPICTURES, nullptr, 0, utf16PathName);
		if (hr != S_OK)
		{
			return false;
		}
		if (appName.IsEmpty() == false)
		{
			wcscat_s(utf16PathName, MAX_PATH, L"\\");
			wcscat_s(utf16PathName, MAX_PATH, appName.GetUTF16());
		}
		result = ::SHCreateDirectoryExW(nullptr, utf16PathName, nullptr);

		// Create a unique file name for this bitmap in the destination directory.
		utf16FileName[0] = L'\0';
		for (index = 1;; index++)
		{
			swprintf_s(utf16FileName, MAX_PATH, L"%s\\Picture %d.png", utf16PathName, index);
			if (_waccess_s(utf16FileName, 0) == ENOENT)
			{
				// Generated file name is unique. Break and proceed to save bitmap to file.
				break;
			}
			if (index >= 10000)
			{
				// Give up trying to find a unique file name.
				return false;
			}
		}

		// Save the bitmap to file.
		WinString finalFilePath(utf16FileName);
		return SaveBitmap(bitmap, finalFilePath.GetUTF8(), 1.0f);
	}

	/// Determines if the given URL can be opened by the OpenURL() method.
	/// @param url A WEB address, e-mail address, or file to be opened by the default application.
	///            Open WEB browser to URL:                "http://www.google.com"
	///            Open e-mail window with correspondence: "mailto:johndoe@domain.com"
	///            Open file with default application:     "C:\My Folder\MyFile.pdf"
	/// @return Returns 0 if it cannot be opened. Returns 1 if it can be opened. Returns -1 if unknown.
	int WinPlatform::CanOpenURL(const char* url) const
	{
		// Can't open a null or empty string.
		if (!url || ('\0' == url[0]))
		{
			return 0;
		}

		// Copy the given URL.
		WinString updatedUrl(url);

		// Check if the given string is actually a URL and validate its URL scheme.
		DWORD urlSchemeLength = INTERNET_MAX_SCHEME_LENGTH;
		wchar_t urlScheme[INTERNET_MAX_SCHEME_LENGTH];
		urlScheme[0] = L'\0';
		::UrlGetPartW(updatedUrl.GetUTF16(), urlScheme, &urlSchemeLength, URL_PART_SCHEME, 0);
		if ((urlSchemeLength > 0) && (urlScheme[0] != L'\0'))
		{
			_wcslwr_s(urlScheme, INTERNET_MAX_SCHEME_LENGTH);
			if (wcscmp(urlScheme, L"file") == 0)
			{
				// The URL is a "file:" path. Attempt to convert it to a Win32 "C:\" style DOS path.
				const size_t kMaxUtf16FilePathSize = 1024;
				DWORD utf16FilePathSize = kMaxUtf16FilePathSize;
				wchar_t utf16FilePath[kMaxUtf16FilePathSize];
				utf16FilePath[0] = L'\0';
				::PathCreateFromUrlW(updatedUrl.GetUTF16(), utf16FilePath, &utf16FilePathSize, 0);
				if (L'\0' == utf16FilePath[0])
				{
					// The "file:" URL string is invalid.
					return 0;
				}

				// Update our local URL with the file path.
				// Next, we need to verify that it has a file association down below.
				updatedUrl.SetUTF16(utf16FilePath);
			}
			else
			{
				// Verify that the URL scheme is defined in the registry.
				HKEY keyHandle = nullptr;
				::RegOpenKeyExW(HKEY_CLASSES_ROOT, urlScheme, 0, KEY_READ, &keyHandle);
				if (!keyHandle)
				{
					return 0;
				}
				DWORD valueType = REG_NONE;
				auto queryResult = ::RegQueryValueExW(keyHandle, L"URL Protocol", nullptr, &valueType, nullptr, nullptr);
				::RegCloseKey(keyHandle);
				bool isUrlSchemeValid = ((ERROR_SUCCESS == queryResult) || (REG_SZ == valueType));
				return isUrlSchemeValid ? 1 : 0;
			}
		}

		// At this point, the URL is assumed to be a DOS style "C:\" path.
		// Verify that the path is valid and references an existing file or directory.
		if (::PathFileExistsW(updatedUrl.GetUTF16()) == FALSE)
		{
			return 0;
		}

		// If the path references a directory, then we can open the path via Windows Explorer.
		if (::PathIsDirectoryW(updatedUrl.GetUTF16()))
		{
			return 1;
		}

		// Attempt to fetch the file extension.
		// If it doesn't have an extension, then it cannot be opened. (File associations are established via extensions.)
		updatedUrl.MakeLowerCase();
		auto utf16FileExtensionPointer = ::PathFindExtensionW(updatedUrl.GetUTF16());
		if (!utf16FileExtensionPointer || (L'\0' == utf16FileExtensionPointer[0]))
		{
			return 0;
		}

		// Don't allow executables or batch files to be opened.
		if (!wcscmp(utf16FileExtensionPointer, L".exe") || !wcscmp(utf16FileExtensionPointer, L".bat"))
		{
			return 0;
		}

		// Do not continue if there is no application associated with the given file extension.
		{
			const size_t kMaxUtf16FilePathSize = 1024;
			DWORD utf16FilePathSize = kMaxUtf16FilePathSize;
			wchar_t utf16FilePath[kMaxUtf16FilePathSize];
			utf16FilePath[0] = L'\0';
			::AssocQueryStringW(
				ASSOCF_INIT_DEFAULTTOSTAR, ASSOCSTR_EXECUTABLE, utf16FileExtensionPointer,
				L"open", utf16FilePath, &utf16FilePathSize);
			if (L'\0' == utf16FilePath)
			{
				return 0;
			}
			if (::PathFileExistsW(utf16FilePath) == FALSE)
			{
				return 0;
			}
		}

		// We can open the file referenced by the URL.
		return 1;
	}

	/// Opens the given URL using the default web browser or application.
	/// @param url A WEB address, e-mail address, or file to be opened by the default application.
	///            Open WEB browser to URL:                "http://www.google.com"
	///            Open e-mail window with correspondence: "mailto:johndoe@domain.com"
	///            Open file with default application:     "C:\My Folder\MyFile.pdf"
	/// @return Returns true if displayed. Returns false if not or if given an invalid URL.
	bool WinPlatform::OpenURL(const char* url) const
	{
		// Validate.
		if ((nullptr == url) || ('\0' == url[0]))
		{
			return false;
		}

		// Don't allow executables or batch files to be opened.
		WinString stringBuffer;
		stringBuffer.SetUTF8(url);
		stringBuffer.MakeLowerCase();
		if (stringBuffer.EndsWith(".exe") || stringBuffer.EndsWith(".bat"))
		{
			return false;
		}

		// Execute the given URL. This will display the given web page or file.
		bool wasOpened = false;
		try
		{
			// Copy the given URL again since our current string was made lowercase up above.
			// Note: Internet URLs are case sensitive.
			stringBuffer.SetUTF8(url);

			// Attempt to execute the URL via the Windows shell.
			auto result = ::ShellExecuteW(nullptr, L"open", stringBuffer.GetUTF16(), nullptr, nullptr, SW_SHOWNORMAL);
			wasOpened = ((int)result > 32);
		}
		catch (...) {}
		return wasOpened;
	}

	FontMetricsMap
		WinPlatform::GetFontMetrics(const PlatformFont& font) const
	{
		auto winFont = (const WinFont&)font;
		Interop::Graphics::FontSettings fontSettings;
		fontSettings.CopyFrom(winFont, Gdiplus::UnitPixel);
		auto gdiFontPointer = fEnvironment.GetFontServices().LoadUsing(fontSettings);
		if (!gdiFontPointer)
		{
			Rtt_ASSERT(0);
			return FontMetricsMap();
		}
		auto metrics = FontMetricsMap();
		auto fontFamilyPointer = fEnvironment.GetFontServices().GetFamilyFrom(*gdiFontPointer.get());
		if (fontFamilyPointer)
		{
			auto fontStyle = gdiFontPointer->GetStyle();
			auto emHeight = fontFamilyPointer->GetEmHeight(fontStyle);
			if (emHeight > 0)
			{
				auto emScale = gdiFontPointer->GetSize() / (Gdiplus::REAL)emHeight;
				auto ascentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellAscent(fontStyle) * emScale;
				auto descentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellDescent(fontStyle) * emScale;
				auto lineSpaceing = (Gdiplus::REAL)fontFamilyPointer->GetLineSpacing(fontStyle) * emScale;
				auto fontHeight = ascentHeight + descentHeight;
				metrics["ascent"] = ascentHeight;
				metrics["descent"] = -descentHeight;
				metrics["leading"] = lineSpaceing - ascentHeight - descentHeight;
				metrics["height"] = fontHeight;
			}
		}
		return metrics;
	}

	PlatformVideoPlayer* WinPlatform::GetVideoPlayer(const ResourceHandle<lua_State>& handle) const
	{
		if (!fVideoPlayer)
		{
			fVideoPlayer = Rtt_NEW(&GetAllocator(), WinVideoPlayer(handle));
		}
		return fVideoPlayer;
	}

	PlatformImageProvider* WinPlatform::GetImageProvider(const ResourceHandle<lua_State>& handle) const
	{
		if (!fImageProvider)
		{
			fImageProvider = Rtt_NEW(&GetAllocator(), WinImageProvider(handle));
		}
		return fImageProvider;
	}

	PlatformVideoProvider*
		WinPlatform::GetVideoProvider(const ResourceHandle<lua_State>& handle) const
	{
		if (!fVideoProvider)
		{
			fVideoProvider = Rtt_NEW(&GetAllocator(), WinVideoProvider(handle));
		}
		return fVideoProvider;
	}

	PlatformStoreProvider* WinPlatform::GetStoreProvider(const ResourceHandle<lua_State>& handle) const
	{
		return nullptr;
	}

	void WinPlatform::SetIdleTimer(bool enabled) const
	{
		EXECUTION_STATE state = ES_CONTINUOUS;
		if (!enabled)
		{
			// States used to inform the system to not idle to sleep.
			state = ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED;
		}
		fIsIdleTimerEnabled = enabled;
		::SetThreadExecutionState(state);
	}

	bool WinPlatform::GetIdleTimer() const
	{
		return fIsIdleTimerEnabled;
	}

	NativeAlertRef WinPlatform::ShowNativeAlert(
		const char* title, const char* message,
		const char** buttonLabels, U32 buttonCount, LuaResource* resourcePointer) const
	{
		// Make sure we do not display multiple alerts at the same time for simulated devices that don't support it.
		auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
		if (deviceSimulatorServicesPointer && !deviceSimulatorServicesPointer->AreMultipleAlertsSupported())
		{
			if (Interop::UI::CoronaTaskDialogHandler::HasPendingHandlersFor(fEnvironment))
			{
				lua_State* luaStatePointer = resourcePointer ? resourcePointer->L() : nullptr;
				::CoronaLuaWarning(
					luaStatePointer, "Device \"%s\" cannot display multiple alerts at the same time.",
					deviceSimulatorServicesPointer->GetModelName());
				return nullptr;
			}
		}

		// Display a native Win32 dialog.
		Interop::UI::CoronaTaskDialogHandler::ShowSettings settings{};
		settings.RuntimeEnvironmentPointer = &fEnvironment;
		settings.Title = title;
		settings.Message = message;
		settings.ButtonLabels = buttonLabels;
		settings.ButtonLabelCount = buttonCount;
		settings.LuaResourcePointer = resourcePointer;
		if (deviceSimulatorServicesPointer && deviceSimulatorServicesPointer->IsBackKeySupported())
		{
			// Always show a [x] close/cancel button when simulating a device that supports a back key.
			// This is because these device always have the ability to back out of an alert dialog.
			settings.IsCancelButtonEnabled = true;
		}
		auto showResult = Interop::UI::CoronaTaskDialogHandler::ShowUsing(settings);

		// Do not continue if failed to display the dialog.
		if (showResult.HasFailed() || !showResult.GetValue())
		{
			WinString stringTranscoder(showResult.GetMessage());
			if (stringTranscoder.IsEmpty())
			{
				stringTranscoder.SetUTF8("Failed to display native alert. Reason: Unknown");
			}
			lua_State* luaStatePointer = resourcePointer ? resourcePointer->L() : nullptr;
			::CoronaLuaError(luaStatePointer, stringTranscoder.GetUTF8());
			return nullptr;
		}

		// Return a reference to the dialog.
		// This allows Lua to close the dialog via the Lua native.cancelAlert() function.
		return showResult.GetValue()->GetNativeAlertReference();
	}

	void WinPlatform::CancelNativeAlert(NativeAlertRef alertReference, S32 buttonIndex) const
	{
		// Fetch the referenced native Win32 dialog.
		auto dialogHandlerPointer = Interop::UI::CoronaTaskDialogHandler::FetchShownDialogFor(alertReference);
		if (!dialogHandlerPointer)
		{
			return;
		}

		// Convert the given button index from a Lua one-based index to a zero-based index.
		buttonIndex--;

		// Close the dialog.
		bool wasClosed = false;
		if (buttonIndex >= 0)
		{
			// Close it by simulating a button press.
			wasClosed = dialogHandlerPointer->CloseWithButtonIndex(buttonIndex);
		}
		if (false == wasClosed)
		{
			// Cancel out of the dialog.
			dialogHandlerPointer->Close();
		}
	}

	Rtt_Allocator& WinPlatform::GetAllocator() const
	{
		return fEnvironment.GetAllocator();
	}

	RenderingStream* WinPlatform::CreateRenderingStream(bool antialias) const
	{
		Rtt_Allocator& allocator = GetAllocator();
		RenderingStream* streamPointer = Rtt_NEW(&allocator, GPUStream(&allocator));
		streamPointer->SetProperty(RenderingStream::kFlipHorizontalAxis, true);
		return streamPointer;
	}

	PlatformTimer* WinPlatform::CreateTimerWithCallback(MCallback& callback) const
	{
		HWND windowHandle = nullptr;
		auto renderSurfacePointer = fEnvironment.GetRenderSurface();
		if (renderSurfacePointer)
		{
			windowHandle = renderSurfacePointer->GetWindowHandle();
		}
		if (!windowHandle && fEnvironment.GetMainWindow())
		{
			auto windowPointer = fEnvironment.GetMainWindow();
			if (windowPointer)
			{
				windowHandle = windowPointer->GetWindowHandle();
			}
		}
		return Rtt_NEW(&GetAllocator(), WinTimer(callback, windowHandle));
	}

	PlatformBitmap* WinPlatform::CreateBitmap(const char* path, bool convertToGrayscale) const
	{
		PlatformBitmap* bitmapPointer = nullptr;

		if (path)
		{
			if (convertToGrayscale)
			{
				bitmapPointer = Rtt_NEW(&GetAllocator(), WinFileGrayscaleBitmap(path, GetAllocator()));
			}
			else
			{
				bitmapPointer = Rtt_NEW(&GetAllocator(), WinFileBitmap(path, GetAllocator()));
			}
		}
		return bitmapPointer;
	}

	PlatformBitmap* WinPlatform::CreateBitmapMask(
		const char str[], const PlatformFont& font, Real w, Real h, const char alignmentStringId[], Real& baselineOffset) const
	{
		int integerWidth = (int)(Rtt_RealToFloat(w) + 0.5f);
		int integerHeight = (int)(Rtt_RealToFloat(h) + 0.5f);
		auto alignmentPointer = Interop::Graphics::HorizontalAlignment::FromCoronaStringId(alignmentStringId);
		if (!alignmentPointer)
		{
			alignmentPointer = &Interop::Graphics::HorizontalAlignment::kLeft;
		}
		return Rtt_NEW(&GetAllocator(), WinTextBitmap(
			fEnvironment, str, (const WinFont&)font, integerWidth, integerHeight, *alignmentPointer, baselineOffset));
	}

	PlatformEventSound* WinPlatform::CreateEventSound(const ResourceHandle<lua_State>& handle, const char* filePath) const
	{
		WinEventSound* player = new WinEventSound(fEnvironment, handle);
		player->Load(filePath);
		return player;
	}

	void WinPlatform::ReleaseEventSound(PlatformEventSound* soundID) const
	{
		delete soundID;
	}

	void WinPlatform::PlayEventSound(PlatformEventSound* soundID) const
	{
		soundID->Play();
	}

	PlatformAudioRecorder* WinPlatform::CreateAudioRecorder(
		const ResourceHandle<lua_State>& handle, const char* filePath) const
	{
		return Rtt_NEW(&GetAllocator(), WinAudioRecorder(handle, GetAllocator(), filePath));
	}

	PlatformAudioPlayer* WinPlatform::GetAudioPlayer(const ResourceHandle<lua_State>& handle) const
	{
		if (!fAudioPlayer)
		{
			fAudioPlayer = Rtt_NEW(&GetAllocator(), WinAudioPlayer(fEnvironment, handle));
		}
		return fAudioPlayer;
	}

	// This version just returns an existing player for appSoundNotify()
	PlatformAudioPlayer* WinPlatform::GetAudioPlayer() const
	{
		return fAudioPlayer;
	}

	void WinPlatform::RaiseError(MPlatform::Error e, const char* reason) const
	{
		const char kNull[] = "(null)";

		if (!reason)
		{
			reason = kNull;
		}
		Rtt_TRACE(("MPlatformFactory error(%d): %s\n", e, kNull));
	}

	bool WinPlatform::FileExists(const char* filename) const
	{
		return (Rtt_FileExists(filename) != 0);
	}

	bool WinPlatform::ValidateAssetFile(const char* assetFilename, const int assetSize) const
	{
		return true;
	}

	void WinPlatform::PathForFile(const char* filename, MPlatform::Directory baseDir, U32 flags, String& result) const
	{
		// Default to the "Documents" directory if given an invalid base directory type.
		if ((baseDir < 0) || (baseDir >= MPlatform::kNumDirs))
		{
			baseDir = MPlatform::kDocumentsDir;
		}

		// Initialize result to an empty string in case the file was not found.
		result.Set(nullptr);

		// Fetch the absolute path for the given base directory type.
		const char* directoryPath = fEnvironment.GetUtf8PathFor(baseDir);

		// Always check for file existence if referencing a resource file.
		if ((MPlatform::kResourceDir == baseDir) || (MPlatform::kSystemResourceDir == baseDir))
		{
			// Only check for existance if given a file name.
			// Note: If a file name was not provided, then the caller is fetching the path for the given base directory.
			if (Rtt_StringIsEmpty(filename) == false)
			{
				flags |= MPlatform::kTestFileExists;
			}
		}

		// Set the "result" to an absolute path for the given file.
		if (directoryPath)
		{
			result.Set(directoryPath);
			if (filename && (strlen(filename) > 0))
			{
				result.Append("\\");
				result.Append(filename);
			}
		}

		// Check if the file exists, if enabled.
		// Result will be set to an empty string if the file could not be found.
		WinString coronaResourceDirectoryPath;
		if (flags & MPlatform::kTestFileExists)
		{
			// Check if the given file name exists.
			bool doesFileExist = FileExists(result.GetString());
			//TODO: Need a solution for widget resources. It might be better to embed these files within the Corona library.
			#if 1 //defined(Rtt_AUTHORING_SIMULATOR)
			if ((false == doesFileExist) && ((MPlatform::kResourceDir == baseDir) || (MPlatform::kSystemResourceDir)))
			{
				// File not found. Since it is a resource file, check if it is installed under the Corona Simulator directory.
				coronaResourceDirectoryPath.SetUTF16(Interop::ApplicationServices::GetDirectoryPath());
				coronaResourceDirectoryPath.Append(L"\\Resources\\Corona");
				WinString coronaResourceFilePath(coronaResourceDirectoryPath.GetUTF16());
				coronaResourceFilePath.Append(L'\\');
				coronaResourceFilePath.Append(filename);
				doesFileExist = FileExists(coronaResourceFilePath.GetUTF8());
				if (doesFileExist)
				{
					result.Set(coronaResourceFilePath.GetUTF8());
					directoryPath = coronaResourceDirectoryPath.GetUTF8();
				}
				else
				{
					// Not found in the Resource directory, try the plugins directory
					WinString pluginDirectoryPath;

					#if defined(Rtt_AUTHORING_SIMULATOR)
					pluginDirectoryPath.SetUTF16(fEnvironment.GetUtf16PathFor(MPlatform::kPluginsDir));
					#else // Win32 desktop app
					pluginDirectoryPath.SetUTF16(Interop::ApplicationServices::GetDirectoryPath());
					pluginDirectoryPath.Append(L"\\corona-plugins");
					#endif

					WinString pluginFilePath;

					pluginFilePath.SetUTF16(pluginDirectoryPath.GetUTF16());
					pluginFilePath.Append(L'\\');
					pluginFilePath.Append(filename);

					doesFileExist = FileExists(pluginFilePath.GetUTF8());

					if (doesFileExist)
					{
						result.Set(pluginFilePath.GetUTF8());
						directoryPath = pluginDirectoryPath.GetUTF8();
					}
				}
			}
			#endif

			if (false == doesFileExist)
			{
				// File not found. Return a null path.
				result.Set(nullptr);
			}
		}

		// On the Simulator, make sure that the file is being requested with the same case characters as the file
		// on disk so we emulate the behavior of the mobile OSes which have case sensitive file systems
		if (!result.IsEmpty() && fEnvironment.GetDeviceSimulatorServices())
		{
			// Check that the existing file has the same character case in its name for device interoperability
			if (!Rtt_FileExistsWithSameCase(result.GetString(), directoryPath, NULL))
			{
				Rtt_TRACE_SIM(("WARNING: case of filename '%s' differs on disk\n", result.GetString()));
			}
		}
	}

	void WinPlatform::CopyAppNameTo(WinString& destinationString) const
	{
		destinationString.Clear();
		if (Interop::ApplicationServices::IsCoronaSdkApp())
		{
			const size_t kMaxCharacters = 1024;
			wchar_t utf16Path[kMaxCharacters];
			utf16Path[0] = L'\0';
			wcscpy_s(utf16Path, kMaxCharacters, fEnvironment.GetUtf16PathFor(kResourceDir));
			::PathStripPathW(utf16Path);
			destinationString.SetUTF16(utf16Path);
		}
		else
		{
			destinationString.SetUTF16(Interop::ApplicationServices::GetProductName());
			if (destinationString.IsEmpty())
			{
				destinationString.SetUTF16(Interop::ApplicationServices::GetExeFileNameWithoutExtension());
			}
		}
	}

	// Protected function used to fetch the Windows class ID for the given image format.
	// Note: This function was based on sample code from MSDN.
	// Argument "format" must be set to "image/bmp", "image/jpeg", "image/gif", "image/tiff", or "image/png".
	// Argument "pClsid" must be a pointer to an exist CLSID type. It will be assigned an ID if the encoder was found.
	// Returns a value greater than zero if the specified encoder was found and argment "pClsid" was assigned an ID.
	// Returns -1 if the specified encoder was not found.
	int WinPlatform::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) const
	{
		Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;
		UINT size = 0;
		UINT encoderCount = 0;
		UINT index;

		// Allocate memory to hold all encoders supported by the operating system.
		Gdiplus::GetImageEncodersSize(&encoderCount, &size);
		if (size <= 0)
		{
			return -1;
		}
		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (nullptr == pImageCodecInfo)
		{
			return -1;
		}

		// Fetch all image encoders.
		Gdiplus::GetImageEncoders(encoderCount, size, pImageCodecInfo);

		// Fetch the class ID belonging to the specified image format.
		for (index = encoderCount - 1; index >= 0; index--)
		{
			if (wcscmp(pImageCodecInfo[index].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[index].Clsid;
				break;
			}
		}
		free(pImageCodecInfo);
		return index;
	}

	bool WinPlatform::SaveImageToPhotoLibrary(const char* filePath) const
	{
		PlatformBitmap* bitmap = CreateBitmap(filePath, false);
		if (bitmap)
		{
			AddBitmapToPhotoLibrary(bitmap);
			Rtt_DELETE(bitmap);
		}
		return true;
	}

	// Saves the given bitmap to file.
	// Returns true if bitmap was saved successfully. Returns false if not.
	bool WinPlatform::SaveBitmap(PlatformBitmap* bitmap, const char* filePath, float jpegQuality) const
	{
		WinString stringBuffer;
		CLSID encoderId;
		Gdiplus::Color color;
		WCHAR* encoderName;
		int bytesPerPixel;
		int alphaIndex;
		int redIndex;
		int greenIndex;
		int blueIndex;
		int xIndex;
		int yIndex;

		// Validate.
		if ((nullptr == bitmap) || (nullptr == filePath))
		{
			return false;
		}
		if ((bitmap->Width() <= 0) || (bitmap->Height() <= 0))
		{
			return false;
		}

		// Fetch the given bitmap's bits.
		U8* bits = (U8*)(bitmap->GetBits(&GetAllocator()));
		if (nullptr == bits)
		{
			return false;
		}

		// Fetch the byte indexes for each color channel according to given image format and machine endianness.
		PlatformBitmap::GetColorByteIndexesFor(bitmap->GetFormat(), &alphaIndex, &redIndex, &greenIndex, &blueIndex);
		bytesPerPixel = PlatformBitmap::BytesPerPixel(bitmap->GetFormat());

		Gdiplus::PixelFormat pixel_format;
		if (bytesPerPixel == 4)
		{
			pixel_format = PixelFormat32bppARGB;
		}
		else
		{
			pixel_format = PixelFormat24bppRGB;
		}

		// Create a 24-bit GDI+ image object to store the given bitmap data.
		Gdiplus::Bitmap targetImage(bitmap->Width(), bitmap->Height(), pixel_format);
		for (yIndex = 0; yIndex < (int)bitmap->Height(); yIndex++)
		{
			for (xIndex = 0; xIndex < (int)bitmap->Width(); xIndex++)
			{
				// Fetch the indexed pixel's color.
				if (bytesPerPixel == 3)
				{
					// Given image has a 3 channel color format. Fetch its color channel values.
					color = Gdiplus::Color(bits[redIndex], bits[greenIndex], bits[blueIndex]);
				}
				else if (bytesPerPixel == 4)
				{
					// Given image has a 4 channel color format. Fetch its color channel values.
					color = Gdiplus::Color(bits[alphaIndex], bits[redIndex], bits[greenIndex], bits[blueIndex]);
				}
				else if (bitmap->GetFormat() == PlatformBitmap::kMask)
				{
					// Given image is a 1 channel grayscale bitmap. Convert it to a 3 channel RGB color value.
					color = Gdiplus::Color((BYTE)(*bits / 0.30), (BYTE)(*bits / 0.59), (BYTE)(*bits / 0.11));
				}
				else
				{
					// Given image format is not supported. Abort.
					return false;
				}

				// Copy the pixel's color to the image buffer to be saved.
				targetImage.SetPixel(xIndex, yIndex, color);

				// Point to the next pixel byte(s) in the bitmap buffer.
				bits += bytesPerPixel;
			}
		}

		// Determine which image format we should save to based on the file extension.
		stringBuffer.SetUTF8(filePath);
		stringBuffer.MakeLowerCase();
		if (stringBuffer.EndsWith(".bmp"))
		{
			encoderName = L"image/bmp";
		}
		else if (stringBuffer.EndsWith(".gif"))
		{
			encoderName = L"image/gif";
		}
		else if (stringBuffer.EndsWith(".tiff") || stringBuffer.EndsWith(".tif"))
		{
			encoderName = L"image/tiff";
		}
		else if (stringBuffer.EndsWith(".png"))
		{
			encoderName = L"image/png";
		}
		else
		{
			encoderName = L"image/jpeg";
		}
		GetEncoderClsid(encoderName, &encoderId);

		// Save the image to file.
		stringBuffer.SetUTF8(filePath);
		targetImage.Save(stringBuffer.GetTCHAR(), &encoderId, nullptr);
		return true;
	}

	void WinPlatform::SetStatusBarMode(MPlatform::StatusBarMode newValue) const
	{
		#ifdef Rtt_AUTHORING_SIMULATOR
		auto serviceSimulator = fEnvironment.GetDeviceSimulatorServices();
		if (serviceSimulator)
		{
			serviceSimulator->SetStatusBar(newValue);
		}
		#endif
	}

	MPlatform::StatusBarMode WinPlatform::GetStatusBarMode() const
	{
		#ifdef Rtt_AUTHORING_SIMULATOR
		auto serviceSimulator = fEnvironment.GetDeviceSimulatorServices();
		if (serviceSimulator)
		{
			serviceSimulator->GetStatusBar();
		}
		#endif
		return MPlatform::kHiddenStatusBar;
	}

	int WinPlatform::GetStatusBarHeight() const
	{
		if (fEnvironment.GetDeviceSimulatorServices())
		{
			return 20;
		}
		return 0;
	}
	int WinPlatform::GetTopStatusBarHeightPixels()  const
	{
		return GetStatusBarHeight();
	}

	int WinPlatform::GetBottomStatusBarHeightPixels() const
	{
		return 0;
	}

	int WinPlatform::SetSync(lua_State* L) const
	{
		return 0;
	}

	int WinPlatform::GetSync(lua_State* L) const
	{
		return 0;
	}

	void WinPlatform::BeginRuntime(const Runtime& runtime) const
	{
	}

	void WinPlatform::EndRuntime(const Runtime& runtime) const
	{
	}

	PlatformExitCallback* WinPlatform::GetExitCallback()
	{
		return &fExitCallback;
	}

	bool WinPlatform::RequestSystem(lua_State* L, const char* actionName, int optionsIndex) const
	{
		// Validate.
		if (Rtt_StringIsEmpty(actionName))
		{
			return false;
		}

		// Attempt to execute the requested operation.
		bool wasExecuted = false;
		if (Rtt_StringCompare("exitApplication", actionName) == 0)
		{
			// Exit/close the app gracefully.
			auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
			if (deviceSimulatorServicesPointer)
			{
				// We're simulating a device via the Corona Simulator.
				// Request the simulator to terminate the Corona runtime, if supported by the simulated platform.
				if (deviceSimulatorServicesPointer->AreExitRequestsSupported())
				{
					::CoronaLog("native.requestExit() called - application will close\r\n");

					deviceSimulatorServicesPointer->RequestTerminate();
					wasExecuted = true;
				}
				else
				{
					::CoronaLuaError(
						L, "native.requestExit() is not supported on device \"%s\".\r\n",
						deviceSimulatorServicesPointer->GetModelName());
				}
			}
			else
			{
				// The runtime is running in Win32 desktop app mode.
				// Attempt to close the window that is hosting its rendering surface, if provided.
				auto windowPointer = fEnvironment.GetMainWindow();
				if (windowPointer && windowPointer->GetWindowHandle())
				{
					// If the window's close [x] button is currently disabled, then re-enable it.
					HMENU menuHandle = ::GetSystemMenu(windowPointer->GetWindowHandle(), FALSE);
					if (menuHandle)
					{
						auto menuItemState = ::GetMenuState(menuHandle, SC_CLOSE, MF_BYCOMMAND);
						if (menuItemState & MF_DISABLED)
						{
							::EnableMenuItem(menuHandle, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
						}
					}

					// Post a close message to the window.
					::PostMessageW(windowPointer->GetWindowHandle(), WM_CLOSE, 0, 0);
					wasExecuted = true;
				}
				else
				{
					::CoronaLuaError(L, "native.requestExit() is unable to close the window because its HWND handle was not provided.\r\n");
				}
			}
		}
		else if (Rtt_StringCompare("executeUntilExit", actionName) == 0)
		{
			// Execute a command line string containing an EXE or shell command.
			// Blocks until command line execution exits. Only suitable for short lived processes.
			// Note: This is a secret feature that only Windows supports. It's needed to run console apps
			//       or shell commands invisibly in the background, which Lua's os.execute() can't do.
			if (lua_istable(L, optionsIndex))
			{
				Interop::Ipc::CommandLineRunner commandLineRunner;

				// Fetch the command line string to execute. (This is required.)
				std::string commandLineString;
				lua_getfield(L, optionsIndex, "commandLine");
				if (lua_type(L, -1) == LUA_TSTRING)
				{
					commandLineString = lua_tostring(L, -1);
				}
				lua_pop(L, 1);

				// Determine if the window/console should be shown or not.
				commandLineRunner.SetHideWindowEnabled(false);
				lua_getfield(L, optionsIndex, "isVisible");
				if (lua_type(L, -1) == LUA_TBOOLEAN)
				{
					commandLineRunner.SetHideWindowEnabled(lua_toboolean(L, -1) ? false : true);
				}
				lua_pop(L, 1);

				// Enable/disable logging the process' stdout and stderr.
				commandLineRunner.SetLogOutputEnabled(false);
				lua_getfield(L, optionsIndex, "isLogOutputEnabled");
				if (lua_type(L, -1) == LUA_TBOOLEAN)
				{
					commandLineRunner.SetLogOutputEnabled(lua_toboolean(L, -1) ? true : false);
				}
				lua_pop(L, 1);

				// Determine if this is a shell command (ex: "dir", "copy", etc.) or a reference to an EXE file.
				commandLineRunner.SetIsShellCommand(false);
				lua_getfield(L, optionsIndex, "isShellCommand");
				if (lua_type(L, -1) == LUA_TBOOLEAN)
				{
					commandLineRunner.SetIsShellCommand(lua_toboolean(L, -1) ? true : false);
				}
				lua_pop(L, 1);

				// Run the given command line string until it exits.
				auto runResult = commandLineRunner.RunUntilExit(commandLineString.c_str());
				if (runResult.GetExitCode() == 0)
				{
					wasExecuted = true;
				}
			}
		}
		else
		{
			// The requested action is unknown.
			::CoronaLuaError(L, "system.request() was given unknown command \"%s\".\r\n", actionName);
		}

		// Return whether or not the requested action was accepted/executed.
		return wasExecuted;
	}

	#ifdef Rtt_AUTHORING_SIMULATOR
	void WinPlatform::SetCursorForRect(const char* cursorName, int x, int y, int width, int height) const
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

	const MCrypto& WinPlatform::GetCrypto() const
	{
		return fCrypto;
	}

	void WinPlatform::GetPreference(Category category, Rtt::String* value) const
	{
		// Validate.
		if (nullptr == value)
		{
			return;
		}

		// Fetch the requested preference value.
		Interop::MDeviceSimulatorServices* deviceSimulatorServicesPointer;
		char stringBuffer[MAX_PATH];
		const char* resultPointer = stringBuffer;
		DWORD dwValue = 0;
		stringBuffer[0] = '\0';
		switch (category)
		{
			case kLocaleLanguage:
				::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, stringBuffer, MAX_PATH);
				break;
			case kLocaleCountry:
				::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, stringBuffer, MAX_PATH);
				break;
			case kLocaleIdentifier:
			case kUILanguage:
			{
				// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
				// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
				//       For example, this can happen with the Hawaiian language, which will return "haw".
				::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SPARENT, stringBuffer, MAX_PATH);
				if (stringBuffer[0] != '\0')
				{
					// Special handling for older OS versions.
					// Replace non-standard Chinese "CHS" and "CHT" script names with respective ISO 15924 codes.
					if (_stricmp(stringBuffer, "zh-chs") == 0)
					{
						strncpy_s(stringBuffer, MAX_PATH, "zh-Hans", MAX_PATH);
					}
					else if (_stricmp(stringBuffer, "zh-cht") == 0)
					{
						strncpy_s(stringBuffer, MAX_PATH, "zh-Hant", MAX_PATH);
					}
				}
				else
				{
					// Use an older API that only fetches the ISO 639 language code if the above API call fails.
					::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, stringBuffer, MAX_PATH);
				}

				// Fetch and append the ISO 3166 country code string to the language code string.
				// This is appended with an underscore '_' to be consistent with Apple and Android platforms.
				char countryCode[16];
				countryCode[0] = '\0';
				::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, countryCode, MAX_PATH);
				if (countryCode[0] != '\0')
				{
					auto stringBufferLength = strlen(stringBuffer);
					stringBuffer[stringBufferLength] = '_';
					stringBufferLength++;
					stringBuffer[stringBufferLength] = '\0';
					strcat_s(stringBuffer + stringBufferLength, MAX_PATH - stringBufferLength, countryCode);
				}
				break;
			}
			case kDefaultStatusBarFile:
				deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
				if (deviceSimulatorServicesPointer)
				{
					resultPointer = deviceSimulatorServicesPointer->GetStatusBarImageFilePathFor(MPlatform::kDefaultStatusBar);
				}
				break;
			case kDarkStatusBarFile:
				deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
				if (deviceSimulatorServicesPointer)
				{
					resultPointer = deviceSimulatorServicesPointer->GetStatusBarImageFilePathFor(MPlatform::kDarkStatusBar);
				}
				break;
			case kTranslucentStatusBarFile:
				deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
				if (deviceSimulatorServicesPointer)
				{
					resultPointer = deviceSimulatorServicesPointer->GetStatusBarImageFilePathFor(MPlatform::kTranslucentStatusBar);
				}
				break;
			case kLightTransparentStatusBarFile:
				deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
				if (deviceSimulatorServicesPointer)
				{
					resultPointer = deviceSimulatorServicesPointer->GetStatusBarImageFilePathFor(MPlatform::kLightTransparentStatusBar);
				}
				break;
			case kDarkTransparentStatusBarFile:
				deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
				if (deviceSimulatorServicesPointer)
				{
					resultPointer = deviceSimulatorServicesPointer->GetStatusBarImageFilePathFor(MPlatform::kDarkTransparentStatusBar);
				}
				break;
			case kScreenDressingFile:
				deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
				if (deviceSimulatorServicesPointer)
				{
					resultPointer = deviceSimulatorServicesPointer->GetStatusBarImageFilePathFor((Rtt::MPlatform::StatusBarMode)kScreenDressingFile);
				}
				break;
			case kSubscription:
				resultPointer = "Solar2D";
				break;
			default:
				resultPointer = nullptr;
				//			Rtt_ASSERT_NOT_REACHED();
				break;
		}

		// Copy the requested preference string to the given value.
		// Set's value to NULL (an empty string) if the given preference is not supported.
		value->Set(resultPointer);
	}

	Rtt::ValueResult<WinPlatform::SharedMStoredPreferencesPointer> WinPlatform::GetStoredPreferencesByCategoryName(
		const char* categoryName) const
	{
		// Create a preferences accessing interface for the given category.
		SharedMStoredPreferencesPointer sharedStoredPreferencesPointer;
		if (Rtt_StringCompare(categoryName, Rtt::Preference::kCategoryNameApp) == 0)
		{
			// Fetch the application's preferences interface.
			sharedStoredPreferencesPointer = fEnvironment.GetStoredPreferences();
		}
		else if (Rtt_StringCompare(categoryName, Rtt::Preference::kCategoryNameSimulator) == 0)
		{
			// Fetch an interface to the Corona Simulator's preferences in the registry.
			sharedStoredPreferencesPointer = Interop::Storage::RegistryStoredPreferences::ForSimulatorPreferences();
		}
		else if (Rtt_StringCompareNoCase(categoryName, "win32:registry:32bit") == 0)
		{
			// Create an interface which accesses the 32-bit section of the registry.
			// Caller must use fully qualified paths for preference key names, starting with the registry hive name.
			Interop::Storage::RegistryStoredPreferences::CreationSettings settings{};
			settings.IsUsingForwardSlashAsPathSeparator = false;
			settings.Wow64ViewType = Interop::Storage::RegistryStoredPreferences::Wow64ViewType::k32Bit;
			sharedStoredPreferencesPointer = std::make_shared<Interop::Storage::RegistryStoredPreferences>(settings);
		}
		else if (Rtt_StringCompareNoCase(categoryName, "win32:registry:64bit") == 0)
		{
			// Create an interface which accesses the 64-bit section of the registry.
			// Caller must use fully qualified paths for preference key names, starting with the registry hive name.
			Interop::Storage::RegistryStoredPreferences::CreationSettings settings{};
			settings.IsUsingForwardSlashAsPathSeparator = false;
			settings.Wow64ViewType = Interop::Storage::RegistryStoredPreferences::Wow64ViewType::k64Bit;
			sharedStoredPreferencesPointer = std::make_shared<Interop::Storage::RegistryStoredPreferences>(settings);
		}

		// Return an error if given an unkonwn/unsupported category name.
		if (!sharedStoredPreferencesPointer)
		{
			std::string message("Given unknown preference category name \"");
			message += categoryName ? categoryName : "";
			message += "\"";
			return Rtt::ValueResult<SharedMStoredPreferencesPointer>::FailedWith(message.c_str());
		}

		// Return a shared pointer to the preferences interface acquired above.
		return Rtt::ValueResult<SharedMStoredPreferencesPointer>::SucceededWith(sharedStoredPreferencesPointer);
	}

	Preference::ReadValueResult WinPlatform::GetPreference(const char* categoryName, const char* keyName) const
	{
		// Fetch a shared pointer to a preferences accessing interface for the given category.
		auto storedPreferencesResult = GetStoredPreferencesByCategoryName(categoryName);
		if (storedPreferencesResult.GetValue() == nullptr)
		{
			return Preference::ReadValueResult::FailedWith(storedPreferencesResult.GetUtf8MessageAsSharedPointer());
		}

		// Fetch and return the requested preference value.
		return storedPreferencesResult.GetValue()->Fetch(keyName);
	}

	OperationResult WinPlatform::SetPreferences(const char* categoryName, const PreferenceCollection& collection) const
	{
		// Fetch a shared pointer to a preferences accessing interface for the given category.
		auto storedPreferencesResult = GetStoredPreferencesByCategoryName(categoryName);
		if (storedPreferencesResult.GetValue() == nullptr)
		{
			return OperationResult::FailedWith(storedPreferencesResult.GetUtf8MessageAsSharedPointer());
		}

		// Write the given preferences to storage and return the result.
		return storedPreferencesResult.GetValue()->UpdateWith(collection);
	}

	OperationResult WinPlatform::DeletePreferences(
		const char* categoryName, const char** keyNameArray, U32 keyNameCount) const
	{
		// Fetch a shared pointer to a preferences accessing interface for the given category.
		auto storedPreferencesResult = GetStoredPreferencesByCategoryName(categoryName);
		if (storedPreferencesResult.GetValue() == nullptr)
		{
			return OperationResult::FailedWith(storedPreferencesResult.GetUtf8MessageAsSharedPointer());
		}

		// Delete the given preferences from storage and return the result.
		return storedPreferencesResult.GetValue()->Delete(keyNameArray, (int)keyNameCount);
	}

	void WinPlatform::SetActivityIndicator(bool visible) const
	{
		const_cast<WinPlatform*>(this)->OnSetActivityIndicator(visible);
	}

	void WinPlatform::OnSetActivityIndicator(bool visible)
	{
		// Show/hide the simulated device's activity indicator, if applicable.
		auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
		if (deviceSimulatorServicesPointer)
		{
			deviceSimulatorServicesPointer->SetActivityIndicatorVisible(visible);
		}

		// Show/hide the Windows mouse wait cursor.
		// Note: Corona's native UI objects, such as text boxes, will block user input only if the wait cursor is enabled.
		//       So, it's important to set this for both the Corona Simulator and Win32 desktop apps.
		WinInputDeviceManager& inputDeviceManager = (WinInputDeviceManager&)(fDevice.GetInputDeviceManager());
		inputDeviceManager.SetWaitCursorEnabled(visible);
	}

	PlatformWebPopup* WinPlatform::GetWebPopup() const
	{
		if (!fWebPopup)
		{
			fWebPopup = Rtt_NEW(&GetAllocator(), WinWebPopup(fEnvironment));
		}
		return fWebPopup;
	}

	bool WinPlatform::CanShowPopup(const char* name) const
	{
		return false;
	}

	bool WinPlatform::ShowPopup(lua_State* L, const char* name, int optionsIndex) const
	{
		return false;
	}

	bool WinPlatform::HidePopup(const char* name) const
	{
		return false;
	}

	PlatformDisplayObject* WinPlatform::CreateNativeTextBox(const Rect& bounds) const
	{
		bool isSingleLine = false;
		return Rtt_NEW(&GetAllocator(), WinTextBoxObject(fEnvironment, bounds, isSingleLine));
	}

	PlatformDisplayObject* WinPlatform::CreateNativeTextField(const Rect& bounds) const
	{
		bool isSingleLine = true;
		return Rtt_NEW(&GetAllocator(), WinTextBoxObject(fEnvironment, bounds, isSingleLine));
	}

	void WinPlatform::SetKeyboardFocus(PlatformDisplayObject* displayObjectPointer) const
	{
		if (displayObjectPointer)
		{
			// Set the focus to the given native display object.
			((WinDisplayObject*)displayObjectPointer)->SetFocus();
		}
		else
		{
			// Set the focus to the render surface control.
			auto renderSurfacePointer = fEnvironment.GetRenderSurface();
			if (renderSurfacePointer)
			{
				renderSurfacePointer->SetFocus();
			}
		}
	}

	PlatformDisplayObject* WinPlatform::CreateNativeMapView(const Rect& bounds) const
	{
		Rtt_TRACE_SIM(("WARNING: Map views are not supported in the simulator. Please build for device.\n"));
		return nullptr;
	}

	PlatformDisplayObject* WinPlatform::CreateNativeWebView(const Rect& bounds) const
	{
		return Rtt_NEW(&GetAllocator(), WinWebViewObject(fEnvironment, bounds));
	}

	PlatformDisplayObject* WinPlatform::CreateNativeVideo(const Rect& bounds) const
	{
		Rtt_TRACE_SIM(("WARNING: Native video objects are not supported in the simulator. Please build for device.\n"));
		return Rtt_NEW(&GetAllocator(), WinVideoObject(fEnvironment, bounds));
	}

	Rtt_Real WinPlatform::GetStandardFontSize() const
	{
		// Acquire the system's default font size.
		double fontSize = 0;
		auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
		if (deviceSimulatorServicesPointer)
		{
			// *** Fetch the simulated device's default font size. ***
			fontSize = deviceSimulatorServicesPointer->GetDefaultFontSize();
		}
		else
		{
			// *** Fetch the default font size from the Windows desktop. ***

			// Fetch the system's default font metrics.
			NONCLIENTMETRICSW metrics{};
			metrics.cbSize = sizeof(metrics);
			OSVERSIONINFOW versionInfo{};
			versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
			::GetVersionExW(&versionInfo);
			if (versionInfo.dwMajorVersion < 6)
			{
				// According to Microsoft's documentation, we must subtract off this field
				// from the total struct size for OS versions older than Windows Vista.
				metrics.cbSize -= sizeof(metrics.iPaddedBorderWidth);
			}
			::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);

			// Convert the default font size to pixels.
			if (metrics.lfMessageFont.lfHeight != 0)
			{
				HWND windowHandle = nullptr;
				auto renderSurfacePointer = fEnvironment.GetRenderSurface();
				if (renderSurfacePointer)
				{
					windowHandle = renderSurfacePointer->GetWindowHandle();
				}
				if (!windowHandle)
				{
					auto windowPointer = fEnvironment.GetMainWindow();
					if (windowPointer)
					{
						windowHandle = windowPointer->GetWindowHandle();
					}
				}
				if (windowHandle)
				{
					HDC deviceContextHandle = ::GetDC(windowHandle);
					if (deviceContextHandle)
					{
						Interop::Graphics::FontSizeConverter fontSizeConverter;
						fontSizeConverter.SetSizeUsing(deviceContextHandle, metrics.lfMessageFont);
						fontSizeConverter.ConvertTo(Gdiplus::UnitPixel);
						fontSize = (double)fontSizeConverter.GetSize();
						::ReleaseDC(windowHandle, deviceContextHandle);
					}
				}
			}
		}

		// If we've failed to acquire a default font size, then use a hard coded one.
		if (fontSize < 1.0)
		{
			fontSize = 16.0;
		}

		// Return a default font size as an Rtt_Real type.
		return Rtt_FloatToReal((float)fontSize);
	}

	struct fontInfo
	{
		lua_State* L;
		int index;
		S32 fontCount;
	};

	static int CALLBACK EnumFontFamExProc(
		const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
	{
		fontInfo* f = (fontInfo*)lParam;
		lua_State* L = f->L;
		WinString stringConverter;
		stringConverter.SetUTF16(lpelfe->lfFaceName);
		lua_pushstring(L, stringConverter.GetUTF8());
		lua_rawseti(L, f->index, ++f->fontCount);
		return 1;
	}

	S32 WinPlatform::GetFontNames(lua_State* L, int index) const
	{
		// Attempt to fetch a device context to the runtime's window.
		// If Corona is not rendering to a window, then we'll use the screen's device context instead via a null HWND.
		HWND windowHandle = nullptr;
		auto renderSurfacePointer = fEnvironment.GetRenderSurface();
		if (renderSurfacePointer)
		{
			windowHandle = renderSurfacePointer->GetWindowHandle();
		}
		if (!windowHandle)
		{
			auto windowPointer = fEnvironment.GetMainWindow();
			if (windowPointer)
			{
				windowHandle = windowPointer->GetWindowHandle();
			}
		}
		HDC deviceContextHandle = ::GetDC(windowHandle);
		if (!deviceContextHandle)
		{
			return 0;
		}

		// Push the family name of all installed system fonts into Lua.
		S32 fontCount = 0;
		{
			LOGFONT logFont;
			logFont.lfCharSet = ANSI_CHARSET;
			logFont.lfFaceName[0] = 0;
			logFont.lfPitchAndFamily = 0;

			fontInfo info;
			info.L = L;
			info.fontCount = 0;
			info.index = index;

			::EnumFontFamiliesExW(deviceContextHandle, &logFont, EnumFontFamExProc, (LPARAM)&info, 0);
			fontCount = (S32)info.fontCount;
		}

		// Release the device context.
		::ReleaseDC(nullptr, deviceContextHandle);

		// Return the number of font family names pushed into Lua.
		return fontCount;
	}

	PlatformFont* WinPlatform::CreateFont(PlatformFont::SystemFont fontType, Rtt_Real size) const
	{
		auto fontPointer = Rtt_NEW(&GetAllocator(), WinFont(fEnvironment));
		fontPointer->SetSize(size);
		switch (fontType)
		{
			case PlatformFont::kSystemFontBold:
				fontPointer->SetBold(true);
				break;
		}
		return fontPointer;
	}

	PlatformFont* WinPlatform::CreateFont(const char* fontName, Rtt_Real size) const
	{
		auto fontPointer = Rtt_NEW(&GetAllocator(), WinFont(fEnvironment));
		fontPointer->SetName(fontName);
		fontPointer->SetSize(size);
		return fontPointer;
	}

	void WinPlatform::SetTapDelay(Rtt_Real delay) const
	{
	}

	Rtt_Real WinPlatform::GetTapDelay() const
	{
		return Rtt_REAL_1;
	}

	PlatformFBConnect* WinPlatform::GetFBConnect() const
	{
		if (!fFBConnect)
		{
			fFBConnect = Rtt_NEW(&GetAllocator(), WinFBConnect);
		}
		return fFBConnect;
	}

	void* WinPlatform::CreateAndScheduleNotification(lua_State* L, int index) const
	{
		return nullptr;
	}

	void WinPlatform::ReleaseNotification(void* notificationId) const
	{
	}

	void WinPlatform::CancelNotification(void* notificationId) const
	{
	}

	void WinPlatform::FlurryInit(const char* applicationKey) const
	{
	}

	void WinPlatform::FlurryEvent(const char* eventId) const
	{
	}

	void WinPlatform::SetNativeProperty(lua_State* L, const char* key, int valueIndex) const
	{
		// Validate.
		if (!L || !key)
		{
			return;
		}

		// Set the requested native property.
		if (Rtt_StringCompare(key, "windowTitleText") == 0)
		{
			// Update the main window's title bar text, if given access to the window.
			auto windowPointer = fEnvironment.GetMainWindow();
			HWND windowHandle = windowPointer ? windowPointer->GetWindowHandle() : nullptr;
			if (windowHandle && (lua_type(L, valueIndex) == LUA_TSTRING))
			{
				WinString stringConverter;
				stringConverter.SetUTF8(lua_tostring(L, valueIndex));
				::SetWindowTextW(windowHandle, stringConverter.GetUTF16());
			}
		}
		else if (Rtt_StringCompare(key, "windowSize") == 0)
		{
			// Change the window size.
			auto windowPointer = fEnvironment.GetMainWindow();
			if (windowPointer && (lua_type(L, valueIndex) == LUA_TTABLE))
			{
				// Get the current window size and update its values.
				auto currentClientSize = windowPointer->GetNormalModeClientSize();
				lua_getfield(L, -1, "width");
				if (lua_type(L, -1) == LUA_TNUMBER)
				{
					currentClientSize.cx = (S32)lua_tointeger(L, -1);
				}
				lua_pop(L, 1);

				lua_getfield(L, -1, "height");
				if (lua_type(L, -1) == LUA_TNUMBER)
				{
					currentClientSize.cy = (S32)lua_tointeger(L, -1);
				}
				lua_pop(L, 1);

				// Update the window size.
				windowPointer->SetNormalModeClientSize(currentClientSize);
			}
		}
		else if (Rtt_StringCompare(key, "windowMode") == 0)
		{
			// Change the window mode to normal, minimized, maximized, fullscreen, etc.
			auto windowPointer = fEnvironment.GetMainWindow();
			if (windowPointer && (lua_type(L, valueIndex) == LUA_TSTRING))
			{
				// Fetch the requested window mode from Lua.
				auto modeName = lua_tostring(L, valueIndex);
				auto windowModePointer = NativeWindowMode::FromStringId(modeName);
				if (!windowModePointer)
				{
					Rtt_LogException("native.setProperty() was given an invalid \"windowMode\" value name: %s", modeName);
					return;
				}

				// Update the window mode.
				windowPointer->SetWindowMode(*windowModePointer);
			}
		}
		else if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
		{
			// Show/hide the mouse cursor. (Only supported when running in desktop app mode.)
			auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
			if (!deviceSimulatorServicesPointer)
			{
				if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
				{
					WinInputDeviceManager& inputDeviceManager =
						(WinInputDeviceManager&)const_cast<Rtt::WinDevice&>(fDevice).GetInputDeviceManager();
					inputDeviceManager.SetCursorVisible(lua_toboolean(L, valueIndex) ? true : false);
				}
				else
				{
					CoronaLuaWarning(L, "native.setProperty(\"%s\") was given an invalid value type.", key);
				}
			}
			else
			{
				CoronaLuaWarning(
					L, "native.setProperty(\"%s\") is not supported on device: \"%s\"",
					key, deviceSimulatorServicesPointer->GetModelName());
			}
		}
		else if (Rtt_StringCompare(key, "mouseCursor") == 0)
		{
			if (lua_type(L, valueIndex) == LUA_TSTRING)
			{
				WinInputDeviceManager& inputDeviceManager =
					(WinInputDeviceManager&)const_cast<Rtt::WinDevice&>(fDevice).GetInputDeviceManager();
				WinString requestedStyle = lua_tostring(L, valueIndex);
				WinInputDeviceManager::CursorStyle style = WinInputDeviceManager::CursorStyle::kDefaultArrow;

				if (Rtt_StringCompare(requestedStyle, "appStarting") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kAppStarting;
				}
				else if (Rtt_StringCompare(requestedStyle, "arrow") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kDefaultArrow;
				}
				else if (Rtt_StringCompare(requestedStyle, "crosshair") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kCrosshair;
				}
				else if (Rtt_StringCompare(requestedStyle, "pointingHand") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kPointingHand;
				}
				else if (Rtt_StringCompare(requestedStyle, "beam") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kIBeam;
				}
				else if (Rtt_StringCompare(requestedStyle, "notAllowed") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kSlashedCircle;
				}
				else if (Rtt_StringCompare(requestedStyle, "resizeAll") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kMove;
				}
				else if (Rtt_StringCompare(requestedStyle, "resizeNorthEastSouthWest") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kSizeNorthEastSouthWest;
				}
				else if (Rtt_StringCompare(requestedStyle, "resizeUpDown") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kSizeNorthSouth;
				}
				else if (Rtt_StringCompare(requestedStyle, "resizeNorthWestSouthEast") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kSizeNorthWestSouthEast;
				}
				else if (Rtt_StringCompare(requestedStyle, "resizeLeftRight") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kSizeWestEast;
				}
				else if (Rtt_StringCompare(requestedStyle, "upArrow") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kUpArrow;
				}
				else if (Rtt_StringCompare(requestedStyle, "hourglass") == 0)
				{
					style = WinInputDeviceManager::CursorStyle::kHourGlass;
				}

				if (inputDeviceManager.GetCursor() == style)
				{
					return;
				}

				inputDeviceManager.SetCursor(style);
			}
			else
			{
				CoronaLuaWarning(L, "native.setProperty(\"%s\") was given an invalid cursor style.", key);
			}
		}
	}

	int WinPlatform::PushNativeProperty(lua_State* L, const char* key) const
	{
		// Validate.
		if (!L)
		{
			return 0;
		}

		// Push the requested native property information to Lua.
		int pushedValues = 0;
		if (Rtt_StringCompare(key, "windowTitleText") == 0)
		{
			// Fetch the window's title bar text.
			auto windowPointer = fEnvironment.GetMainWindow();
			HWND windowHandle = windowPointer ? windowPointer->GetWindowHandle() : nullptr;
			if (windowHandle)
			{
				// Get the title text in UTF-16 form and push it as a UTF-8 string.
				auto characterCount = ::GetWindowTextLengthW(windowHandle);
				if (characterCount > 0)
				{
					characterCount++;
					wchar_t* utf16String = new wchar_t[characterCount];
					utf16String[0] = L'\0';
					::GetWindowTextW(windowHandle, utf16String, characterCount);
					auto utf8String = lua_create_utf8_string_from(utf16String);
					if (utf8String)
					{
						lua_pushstring(L, utf8String);
						lua_destroy_utf8_string(utf8String);
						pushedValues = 1;
					}
					delete[] utf16String;
				}

				// If we've failed to acquire and push the title text above, then push an empty string.
				if (pushedValues <= 0)
				{
					lua_pushstring(L, "");
					pushedValues = 1;
				}
			}
		}
		else if (Rtt_StringCompare(key, "windowMode") == 0)
		{
			// Fetch the window's current mode such as "normal", "maximized", "fullscreen", etc.
			auto windowPointer = fEnvironment.GetMainWindow();
			if (windowPointer)
			{
				lua_pushstring(L, windowPointer->GetWindowMode().GetStringId());
				pushedValues = 1;
			}
		}
		else if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
		{
			// Fetch the mouse cursor's current visibility state.
			auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
			if (!deviceSimulatorServicesPointer)
			{
				// We're in Win32 desktop app mode.
				WinInputDeviceManager& inputDeviceManager =
					(WinInputDeviceManager&)const_cast<Rtt::WinDevice&>(fDevice).GetInputDeviceManager();
				lua_pushboolean(L, inputDeviceManager.IsCursorVisible() ? 1 : 0);
				pushedValues = 1;
			}
			else if (deviceSimulatorServicesPointer->IsMouseSupported())
			{
				// Only Android supports a mouse and that OS does not allow the cursor to be hidden.
				lua_pushboolean(L, 1);
				pushedValues = 1;
			}
			else
			{
				// No other device supports this API.
				CoronaLuaWarning(
					L, "native.getProperty(\"%s\") is not supported on device: \"%s\"",
					key, deviceSimulatorServicesPointer->GetModelName());
			}
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

	int WinPlatform::PushSystemInfo(lua_State* L, const char* key) const
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
			WinString appName;
			CopyAppNameTo(appName);
			lua_pushstring(L, appName.GetUTF8());
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "appVersionString") == 0)
		{
			// Return an empty version string since it is unknown by the simulator.
			if (Interop::ApplicationServices::IsCoronaSdkApp())
			{
				lua_pushstring(L, "");
			}
			else
			{
				WinString stringConverter(Interop::ApplicationServices::GetFileVersionString());
				lua_pushstring(L, stringConverter.GetUTF8());
			}
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "bundleID") == 0)
		{
			lua_pushstring(L, "com.coronalabs.NewApp");
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "isoCountryCode") == 0)
		{
			// Fetch the ISO 3166-1 country code.
			char stringBuffer[16];
			stringBuffer[0] = '\0';
			::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, stringBuffer, sizeof(stringBuffer));
			lua_pushstring(L, stringBuffer);
			pushedValues = 1;
		}
		else if (Rtt_StringCompare(key, "isoLanguageCode") == 0)
		{
			// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
			// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
			//       For example, this can happen with the Hawaiian language, which will return "haw".
			WinString languageCode;
			const size_t kUtf16StringBufferMaxLength = 128;
			wchar_t utf16StringBuffer[kUtf16StringBufferMaxLength];
			utf16StringBuffer[0] = L'\0';
			::GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SPARENT, utf16StringBuffer, kUtf16StringBufferMaxLength);
			if (utf16StringBuffer[0] != L'\0')
			{
				// Special handling for older OS versions.
				// Replace non-standard Chinese "chs" and "cht" script names with respective ISO 15924 codes.
				languageCode.SetUTF16(utf16StringBuffer);
				languageCode.MakeLowerCase();
				if (languageCode.Equals("zh-chs"))
				{
					languageCode.SetUTF16(L"zh-hans");
				}
				else if (languageCode.Equals("zh-cht"))
				{
					languageCode.SetUTF16(L"zh-hant");
				}
			}
			else
			{
				// Use an older API that only fetches the ISO 639 language code if the above API call fails.
				::GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, utf16StringBuffer, kUtf16StringBufferMaxLength);
				languageCode.SetUTF16(utf16StringBuffer);
			}
			languageCode.MakeLowerCase();
			lua_pushstring(L, languageCode.GetUTF8());
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

	void WinPlatform::RuntimeErrorNotification(const char* errorType, const char* message, const char* stacktrace) const
	{
		// Determine if this is a Corona Simulator project.
		bool isSimulatingDevice = (fEnvironment.GetDeviceSimulatorServices() != nullptr);

		// Set up the error message to be displayed to the user.
		WinString strMessage;
		WinString strResourcePath;
		strMessage.SetUTF8(message);
		if (stacktrace && (strlen(stacktrace) > 0))
		{
			strMessage.Append(stacktrace);
		}
		strResourcePath.SetUTF8(fEnvironment.GetUtf8PathFor(kResourceDir));
		strResourcePath.Append("\\");
		strMessage.Replace(strResourcePath.GetUTF8(), "");
		if (isSimulatingDevice)
		{
			strMessage.Append("\n\nDo you want to relaunch the project?");
		}

		// Suspend the Corona runtime.
		fEnvironment.RequestSuspend();

		// Fetch a handle to the window hosting the Corona runtime.
		// It'll be used as a parent window for the error message box below.
		// If Corona is not rendering to a window, then use a null handle, which means no parent window.
		HWND windowHandle = nullptr;
		auto renderSurfacePointer = fEnvironment.GetRenderSurface();
		if (renderSurfacePointer)
		{
			windowHandle = renderSurfacePointer->GetWindowHandle();
		}
		if (!windowHandle)
		{
			auto windowPointer = fEnvironment.GetMainWindow();
			if (windowPointer)
			{
				windowHandle = windowPointer->GetWindowHandle();
			}
		}

		// Display an error message box.
		// Note: This is a blocking multi-tasking function call.
		UINT buttonType = isSimulatingDevice ? MB_YESNO : MB_OK;
		int buttonIndex = ::MessageBoxW(
			windowHandle, strMessage.GetTCHAR(), L"Corona Runtime Error", MB_ICONERROR | buttonType);
		if (isSimulatingDevice)
		{
			// Relaunch the Corona Simulator project if the user clicked the "yes" button.
			if (IDYES == buttonIndex && fEnvironment.GetDeviceSimulatorServices() != NULL)
			{
				fEnvironment.GetDeviceSimulatorServices()->RequestRestart();
			}
		}
		else
		{
			//TODO: We should probably terminate the Corona runtime for desktop apps.
		}
	}

	void WinPlatform::SetProjectResourceDirectory(const char* path)
	{
		WinString stringTranscoder(path);
		fEnvironment.SetPathForProjectResourceDirectory(stringTranscoder.GetUTF16());
	}

	void WinPlatform::SetSkinResourceDirectory(const char* path)
	{
		// This path should not be allowed to change dynamically.
		// Should assign this directory path via the SimulatorRuntimeEnvironment::CreateUsing() function instead.
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	void
		WinPlatform::Suspend() const
	{
	}

	void
		WinPlatform::Resume() const
	{
	}

	void WinPlatform::GetSafeAreaInsetsPixels(Rtt_Real& top, Rtt_Real& left, Rtt_Real& bottom, Rtt_Real& right) const
	{
		top = left = bottom = right = 0;
		auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
		if (deviceSimulatorServicesPointer)
		{
			deviceSimulatorServicesPointer->GetSafeAreaInsetsPixels(top, left, bottom, right);
		}
	}

}	// namespace Rtt

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
#include "Rtt_LinuxConsolePlatform.h"
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
#include "Rtt_PreferenceCollection.h"
#include "Rtt_Freetype.h"
#include <pwd.h>

using namespace std;

namespace Rtt
{
	LinuxConsolePlatform::LinuxConsolePlatform(const char *resourceDir, const char *documentsDir, const char *temporaryDir,
	        const char *cachesDir, const char *systemCachesDir, const char *skinDir, const char *installDir)
		: fAllocator(Rtt_AllocatorCreate()),
		  fDevice(*fAllocator),
		  fResourceDir(fAllocator),
		  fDocumentsDir(fAllocator),
		  fTemporaryDir(fAllocator),
		  fCachesDir(fAllocator),
		  fSystemCachesDir(fAllocator),
		  fInstallDir(fAllocator),
		  fSkinDir(fAllocator)
	{
		fResourceDir.Set(resourceDir);
		fDocumentsDir.Set(documentsDir);
		fTemporaryDir.Set(temporaryDir);
		fCachesDir.Set(cachesDir);
		fSystemCachesDir.Set(systemCachesDir);
		fSkinDir.Set(skinDir);
		fInstallDir.Set(installDir);
	}

	LinuxConsolePlatform::~LinuxConsolePlatform()
	{
	}

	FontMetricsMap LinuxConsolePlatform::GetFontMetrics(const PlatformFont &font) const
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

	void LinuxConsolePlatform::GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const
	{
		top = left = bottom = right = 0;
	}

	Rtt_Allocator &LinuxConsolePlatform::GetAllocator() const
	{
		return *fAllocator;
	}

	MPlatformDevice &LinuxConsolePlatform::GetDevice() const
	{
		return const_cast<LinuxDevice &>(fDevice);
	}

	const MCrypto &LinuxConsolePlatform::GetCrypto() const
	{
		return fCrypto;
	}

	MPlatform::StatusBarMode LinuxConsolePlatform::GetStatusBarMode() const
	{
		return MPlatform::kDefaultStatusBar;
	}

	bool LinuxConsolePlatform::FileExists(const char *filename) const
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

	void LinuxConsolePlatform::PathForFile(const char *filename, MPlatform::Directory baseDir, U32 flags, String &result) const
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
					pluginPath = GetPluginsPath();
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

	void LinuxConsolePlatform::PathForFile(const char *filename, const char *baseDir, String &result) const
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

	void LinuxConsolePlatform::GetPreference(Category category, Rtt::String *value) const
	{
		// Validate.
		if (value == NULL)
		{
			return;
		}

		// Fetch the requested preference value.
		const char *resultPointer = "";
		string localeName = "en_US"; // hack

		switch (category)
		{
			case kLocaleLanguage:
				resultPointer = localeName.substr(0, 2).c_str();
				break;
			case kLocaleCountry:
				resultPointer = localeName.substr(3, 2).c_str();
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

	Preference::ReadValueResult LinuxConsolePlatform::GetPreference(const char *categoryName, const char *keyName) const
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
		FILE *f = fopen(path.c_str(), "rb");
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

	OperationResult LinuxConsolePlatform::SetPreferences(const char *categoryName, const PreferenceCollection &preferences) const
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
				const char *val = strval.GetValue()->c_str();

				// maybe it would be better to use sqlite3 ??
				std::string path = fSystemCachesDir.GetString();
				path += '/';
				path += key;
				FILE *f = fopen(path.c_str(), "wb");

				if (f)
				{
					fwrite(val, 1, strlen(val), f);
					fclose(f);
					rc = true;
				}
			}
		}

		return rc == false ? OperationResult::FailedWith("This API is not supported on this platform.") : Rtt::OperationResult::kSucceeded;
	}

	OperationResult LinuxConsolePlatform::DeletePreferences(const char *categoryName, const char **keyNameArray, U32 keyNameCount) const
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

		return rc == false ? OperationResult::FailedWith("This API is not supported on this platform.") : Rtt::OperationResult::kSucceeded;
	}

	int LinuxConsolePlatform::PushSystemInfo(lua_State *L, const char *key) const
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
			//lua_pushstring(L, solarApp->GetContext()->GetAppName().c_str());
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

	void LinuxConsolePlatform::RuntimeErrorNotification(const char *errorType, const char *message, const char *stacktrace) const
	{
	}

	int LinuxConsolePlatform::RunSystemCommand(string command) const
	{
		//	Rtt_Log("Running: %S\n", command.c_str());

		// system(3) on Win32 wants the whole command in double quotes
		string quotedCommand("\"");
		quotedCommand.append(command);
		quotedCommand.append("\"");

		return system(quotedCommand.c_str());
	}

	const string LinuxConsolePlatform::GetDirectoryPath() const
	{
		static string coronaSDKPath = getenv("CORONA_PATH");

		return coronaSDKPath;
	}
}; // namespace Rtt

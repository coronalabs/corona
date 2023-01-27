//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_ProjectSettings.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_FileSystem.h"
#include "Display/Rtt_Display.h"
#include "Rtt_Archive.h"
#include "Rtt_LuaFile.h"
#include "Rtt_NativeWindowMode.h"
#include "Rtt_Runtime.h"
#include <algorithm>
#include <string>
extern "C"
{
#	include "lua.h"
}


namespace Rtt
{

ProjectSettings::ProjectSettings()
{
	ResetBuildSettings();
	ResetConfigLuaSettings();
}

ProjectSettings::~ProjectSettings()
{
}

bool ProjectSettings::LoadFromDirectory(const char* directoryPath)
{
	bool wasBuildSettingsFound = false;
	bool wasConfigLuaFound = false;

	// Validate argument.
	if (Rtt_StringIsEmpty(directoryPath))
	{
		return false;
	}

	// Check if the directory exists.
	if (!Rtt_FileExists(directoryPath))
	{
		return false;
	}

	// Create a new Lua state to load the "build.settings" and "config.lua" into.
	lua_State* luaStatePointer = luaL_newstate();
	if (!luaStatePointer)
	{
		return false;
	}
	luaL_openlibs(luaStatePointer);

	// Make a directory path with a trailing slash.
	std::string directoryPathWithSlash(directoryPath);
	char lastCharacter = directoryPathWithSlash.c_str()[directoryPathWithSlash.size() - 1];
#ifdef Rtt_WIN_ENV
	if ((lastCharacter != '\\') && (lastCharacter != '/'))
	{
		directoryPathWithSlash += '\\';
	}
#else
	if (lastCharacter != '/')
	{
		directoryPathWithSlash += '/';
	}
#endif

	// Attempt to load the Corona project's "build.settings" and "config.lua" files into the Lua state as globals.
	std::string filePath = directoryPathWithSlash;
	filePath += "resource.car";
	if (Rtt_FileExists(filePath.c_str()))
	{
		// Load the "build.settings" and "config.lua" from the directory's "resource.car" file.
		Rtt_Allocator* allocatorPointer = Rtt_AllocatorCreate();
		if (allocatorPointer)
		{
			{
				Rtt::Archive archive(*allocatorPointer, filePath.c_str());
				archive.DoResource(luaStatePointer, "build.settings", 0);

				// archive.DoResource(luaStatePointer, Rtt_LUA_OBJECT_FILE("config"), 0);
				// Sometimes config.lua uses display.* APIs. It will fail here: not a big deal
				// It will be loaded again in Rtt::Runtime (similar logic implemented below)
				int status = archive.LoadResource(luaStatePointer, Rtt_LUA_OBJECT_FILE("config"));
				if (!status)
				{
					status = lua_pcall(luaStatePointer, 0, 0, 0);
				}
			}
			Rtt_AllocatorDestroy(allocatorPointer);
		}
	}
	else
	{
		// *** A "resource.car" file was not found. ***

		// Attempt to load the "build.settings" file.
		filePath = directoryPathWithSlash;
		filePath += "build.settings";
		if (Rtt_FileExists(filePath.c_str()))
		{
			Rtt::Lua::DoFile(luaStatePointer, filePath.c_str(), 0, true);
		}

		// Attempt to load the "config.lua" file.
		// Note: This could fail if the script contains Corona's Lua APIs.
		//       In this case, we have to load this configuration via the Rtt::Runtime instead.
		filePath = directoryPathWithSlash;
		filePath += "config.lua";
		if (Rtt_FileExists(filePath.c_str()))
		{
			int status = luaL_loadfile(luaStatePointer, filePath.c_str());
			if (!status)
			{
				status = lua_pcall(luaStatePointer, 0, 0, 0);
			}
		}
	}

	// Fetch the "build.settings" information, if successfully loaded into Lua up above.
	lua_getglobal(luaStatePointer, "settings");
	if (lua_istable(luaStatePointer, -1))
	{
		// Reset "build.settings" related info and flag that this file was found.
		ResetBuildSettings();
		fHasBuildSettings = true;
		wasBuildSettingsFound = true;

		// Fetch the project's orientation settings.
		lua_getfield(luaStatePointer, -1, "orientation");
		if (lua_istable(luaStatePointer, -1))
		{
			// Fetch the default orientation.
			lua_getfield(luaStatePointer, -1, "default");
			if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
			{
				fDefaultOrientation = Rtt::DeviceOrientation::TypeForString(lua_tostring(luaStatePointer, -1));
			}
			lua_pop(luaStatePointer, 1);

			// Fetch the supported orientations.
			lua_getfield(luaStatePointer, -1, "supported");
			if (lua_istable(luaStatePointer, -1))
			{
				auto tableItemCount = lua_objlen(luaStatePointer, -1);
				if (tableItemCount > 0)
				{
					int luaArrayIndex = lua_gettop(luaStatePointer);
					for (lua_pushnil(luaStatePointer);
					     lua_next(luaStatePointer, luaArrayIndex) != 0;
					     lua_pop(luaStatePointer, 1))
					{
						if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
						{
							auto orientation = Rtt::DeviceOrientation::TypeForString(lua_tostring(luaStatePointer, -1));
							if (orientation != Rtt::DeviceOrientation::kUnknown)
							{
								fOrientationsSupportedSet.insert(orientation);
							}
						}
					}
				}
			}
			lua_pop(luaStatePointer, 1);
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the project's window settings.
		lua_getfield(luaStatePointer, -1, "window");
		if (lua_istable(luaStatePointer, -1))
		{
			// Fetch the window's default window mode such as "normal", "maximized", "fullscreen", etc.
			lua_getfield(luaStatePointer, -1, "defaultMode");
			if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
			{
				fDefaultWindowModePointer = Rtt::NativeWindowMode::FromStringId(lua_tostring(luaStatePointer, -1));
			}
			lua_pop(luaStatePointer, 1);

            // Fetch the window's resizable setting.
            lua_getfield(luaStatePointer, -1, "resizable");
            if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
            {
                fIsWindowResizable = lua_toboolean(luaStatePointer, -1) ? true : false;
            }
            lua_pop(luaStatePointer, 1);
            
            // Fetch the window's suspend when minimized setting.
            lua_getfield(luaStatePointer, -1, "suspendWhenMinimized");
            if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
            {
                fSuspendWhenMinimized = lua_toboolean(luaStatePointer, -1) ? true : false;
            }
            lua_pop(luaStatePointer, 1);
            
			// Fetch the minimum width/height the window's client/view area can be resized to.
			lua_getfield(luaStatePointer, -1, "minViewWidth");
			if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
			{
				fMinWindowViewWidth = (int)lua_tointeger(luaStatePointer, -1);
				if (fMinWindowViewWidth < 0)
				{
					fMinWindowViewWidth = 0;
				}
			}
			lua_pop(luaStatePointer, 1);
			lua_getfield(luaStatePointer, -1, "minViewHeight");
			if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
			{
				fMinWindowViewHeight = (int)lua_tointeger(luaStatePointer, -1);
				if (fMinWindowViewHeight < 0)
				{
					fMinWindowViewHeight = 0;
				}
			}
			lua_pop(luaStatePointer, 1);

			// Fetch the width/height the window's client/view area should default to on startup.
			lua_getfield(luaStatePointer, -1, "defaultViewWidth");
			if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
			{
				fDefaultWindowViewWidth = (int)lua_tointeger(luaStatePointer, -1);
				if (fDefaultWindowViewWidth < 0)
				{
					fDefaultWindowViewWidth = 0;
				}
			}
			lua_pop(luaStatePointer, 1);
			lua_getfield(luaStatePointer, -1, "defaultViewHeight");
			if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
			{
				fDefaultWindowViewHeight = (int)lua_tointeger(luaStatePointer, -1);
				if (fDefaultWindowViewHeight < 0)
				{
					fDefaultWindowViewHeight = 0;
				}
			}
			lua_pop(luaStatePointer, 1);

			// Fetch the window close, minimize, and maximize button enable settings.
			lua_getfield(luaStatePointer, -1, "enableCloseButton");
			if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
			{
				fIsWindowCloseButtonEnabled = lua_toboolean(luaStatePointer, -1) ? true : false;
			}
			lua_pop(luaStatePointer, 1);
			lua_getfield(luaStatePointer, -1, "enableMinimizeButton");
			if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
			{
				fIsWindowMinimizeButtonEnabled = lua_toboolean(luaStatePointer, -1) ? true : false;
			}
			lua_pop(luaStatePointer, 1);
			lua_getfield(luaStatePointer, -1, "enableMaximizeButton");
			if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
			{
				fIsWindowMaximizeButtonEnabled = lua_toboolean(luaStatePointer, -1) ? true : false;
			}
			lua_pop(luaStatePointer, 1);
			// Fetch the window title shown setting.
			lua_getfield(luaStatePointer, -1, "showWindowTitle");
			if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
			{
				fIsWindowTitleShown = lua_toboolean(luaStatePointer, -1) ? true : false;
			}
			lua_pop(luaStatePointer, 1);
			
			// Fetch the window's title bar text.
			lua_getfield(luaStatePointer, -1, "titleText");
			if (lua_istable(luaStatePointer, -1))
			{
				// A localized string table was provided.
				// The keys are expected to be the ISO "language-country" codes and values are the localized strings.
				int luaTableIndex = lua_gettop(luaStatePointer);
				for (lua_pushnil(luaStatePointer);
				     lua_next(luaStatePointer, luaTableIndex) != 0;
				     lua_pop(luaStatePointer, 1))
				{
					// Validate the key/value pair.
					// Note: Value can be an empty string, but not null.
					if ((lua_type(luaStatePointer, -1) != LUA_TSTRING) || (lua_type(luaStatePointer, -2) != LUA_TSTRING))
					{
						continue;
					}
					auto stringKey = lua_tostring(luaStatePointer, -2);
					auto stringValue = lua_tostring(luaStatePointer, -1);
					if (Rtt_StringIsEmpty(stringKey) || !stringValue)
					{
						continue;
					}

					// Convert the ASCII string key to lowercase.
					std::string lowercaseStringKey(stringKey);
					std::transform(
							lowercaseStringKey.begin(), lowercaseStringKey.end(), lowercaseStringKey.begin(), ::tolower);

					// Add the localized text to the dictionary.
					std::pair<std::string, std::string> pair(lowercaseStringKey, std::string(stringValue));
					fLocalizedWindowTitleTextMap.insert(pair);
				}
			}
			else if (lua_isstring(luaStatePointer, -1))
			{
				// Only one string was provided. Use it as the default, regardless of current system locale.
				auto titleText = lua_tostring(luaStatePointer, -1);
				if (titleText)
				{
					std::pair<std::string, std::string> pair(std::string("default"), std::string(titleText));
					fLocalizedWindowTitleTextMap.insert(pair);
				}
			}
			lua_pop(luaStatePointer, 1);
		}
		lua_pop(luaStatePointer, 1);

		// If a valid default orientation was assigned, then make sure it is in the "supported" collection.
		if (Rtt::DeviceOrientation::IsInterfaceOrientation(fDefaultOrientation))
		{
			fOrientationsSupportedSet.insert(fDefaultOrientation);
		}
	}
	lua_pop(luaStatePointer, 1);

	// Fetch the "config.lua" information, if successfully loaded into Lua up above.
	lua_getglobal(luaStatePointer, "application");
	if (lua_istable(luaStatePointer, -1))
	{
		// Reset "config.lua" related info and flag that this file was found.
		ResetConfigLuaSettings();
		fHasConfigLua = true;
		wasConfigLuaFound = true;

		// Fetch the project's transparency setting.
		lua_getfield(luaStatePointer, -1, "isTransparent");
		if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
		{
			fIsWindowTransparent = lua_toboolean(luaStatePointer, -1) ? true : false;
		}
		lua_pop(luaStatePointer, 1);
		
		lua_getfield(luaStatePointer, -1, "backend");
		if (lua_isstring(luaStatePointer, -1))
		{
			const char * backend = lua_tostring(luaStatePointer, -1);

			if (strcmp(backend, "wantVulkan") == 0 || strcmp(backend, "requireVulkan") == 0)
			{
				fBackend = backend;
			}
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the project's content scaling settings.
		lua_getfield(luaStatePointer, -1, "content");
		if (lua_istable(luaStatePointer, -1))
		{
			// Fetch the scale mode.
			Rtt::Display::ScaleMode scaleMode = Rtt::Display::kNone;
			lua_getfield(luaStatePointer, -1, "scale");
			if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
			{
				scaleMode = Rtt::Display::ScaleModeFromString(lua_tostring(luaStatePointer, -1));
			}
			lua_pop(luaStatePointer, 1);

			// Fetch the content width and height if content scaling is enabled.
			if ((scaleMode != Rtt::Display::kNone) && (scaleMode != Rtt::Display::kAdaptive))
			{
				lua_getfield(luaStatePointer, -1, "width");
				if (lua_isnumber(luaStatePointer, -1))
				{
					fContentWidth = (int)lua_tointeger(luaStatePointer, -1);
				}
				lua_pop(luaStatePointer, 1);
				lua_getfield(luaStatePointer, -1, "height");
				if (lua_isnumber(luaStatePointer, -1))
				{
					fContentHeight = (int)lua_tointeger(luaStatePointer, -1);
				}
				lua_pop(luaStatePointer, 1);
			}

			// Fetch image suffix scales if content scaling is enabled.
			if (scaleMode != Rtt::Display::kNone)
			{
				lua_getfield(luaStatePointer, -1, "imageSuffix");
				if (lua_istable(luaStatePointer, -1))
				{
					int luaTableIndex = lua_gettop(luaStatePointer);
					for (lua_pushnil(luaStatePointer);
					     lua_next(luaStatePointer, luaTableIndex) != 0;
					     lua_pop(luaStatePointer, 1))
					{
						if (lua_isnumber(luaStatePointer, -1))
						{
							double scale = lua_tonumber(luaStatePointer, -1);
							if (scale > 0)
							{
								fImageSuffixScaleSet.insert(scale);
							}
						}
					}
				}
				lua_pop(luaStatePointer, 1);
			}
		}
		lua_pop(luaStatePointer, 1);
	}
	lua_pop(luaStatePointer, 1);

	// Let a derived version of this class load its custom fields, but only if at least 1 of the files were found.
	if (wasBuildSettingsFound || wasConfigLuaFound)
	{
		OnLoadedFrom(luaStatePointer);
	}

	// Destroy the Lua state created up above.
	lua_close(luaStatePointer);

	// Return an error result if no project files were found.
	if (!wasBuildSettingsFound && !wasConfigLuaFound)
	{
		return false;
	}

	// Load was successful.
	return true;
}

bool ProjectSettings::LoadFrom(const Rtt::Runtime& runtime)
{
	// Reset the "config.lua" related info.
	ResetConfigLuaSettings();

	// Flag that the "config.lua" file exists.
	// We're guarranteed to have config-like settings from the Corona runtime object we were given.
	fHasConfigLua = true;

	// Check if content scaling is enabled. If not, then the rest of the "config.lua" settings are irrelevant.
	const Rtt::Display& display = runtime.GetDisplay();
	if (display.GetScaleMode() == Rtt::Display::kNone)
	{
		return true;
	}

	// Fetch the runtime's content width and height.
	// Note: If currently in landscape mode, then the width and height will be swapped.
	if (Rtt::DeviceOrientation::IsUpright(display.GetContentOrientation()))
	{
		fContentWidth = display.ContentWidth();
		fContentHeight = display.ContentHeight();
	}
	else
	{
		fContentWidth = display.ContentHeight();
		fContentHeight = display.ContentWidth();
	}

	// Fetch the Lua state. (Should always exist, but check just in case.)
	lua_State* luaStatePointer = display.GetL();
	if (!luaStatePointer)
	{
		return true;
	}

	// Fetch the runtime's image suffix scales.
	bool wasPushed = display.PushImageSuffixTable();
	if (wasPushed)
	{
		if (lua_istable(luaStatePointer, -1))
		{
			int luaArrayIndex = lua_gettop(luaStatePointer);
			for (lua_pushnil(luaStatePointer); lua_next(luaStatePointer, luaArrayIndex) != 0; lua_pop(luaStatePointer, 1))
			{
				if (lua_istable(luaStatePointer, -1))
				{
					lua_getfield(luaStatePointer, -1, "scale");
					if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
					{
						double scale = lua_tonumber(luaStatePointer, -1);
						if (scale > 0)
						{
							fImageSuffixScaleSet.insert(scale);
						}
					}
					lua_pop(luaStatePointer, 1);
				}
			}
		}
		lua_pop(luaStatePointer, 1);
	}

	// Load was successful.
	return true;
}

void ProjectSettings::ResetBuildSettings()
{
	fHasBuildSettings = false;
	fDefaultOrientation = Rtt::DeviceOrientation::kUnknown;
	fOrientationsSupportedSet.clear();
	fDefaultWindowModePointer = NULL;
	fIsWindowResizable = false;
	fSuspendWhenMinimized = false;
	fMinWindowViewWidth = 0;
	fMinWindowViewHeight = 0;
	fDefaultWindowViewWidth = 0;
	fDefaultWindowViewHeight = 0;
	fIsWindowCloseButtonEnabled = true;
	fIsWindowMinimizeButtonEnabled = true;
	fIsWindowMaximizeButtonEnabled = false;
	fLocalizedWindowTitleTextMap.clear();
	fIsWindowTitleShown = true;
	fIsWindowTransparent = false;
	fBackend = "gl";
}

void ProjectSettings::ResetConfigLuaSettings()
{
	fHasConfigLua = false;
	fContentWidth = 0;
	fContentHeight = 0;
	fImageSuffixScaleSet.clear();
}

bool ProjectSettings::HasBuildSettings() const
{
	return fHasBuildSettings;
}

bool ProjectSettings::HasConfigLua() const
{
	return fHasConfigLua;
}

Rtt::DeviceOrientation::Type ProjectSettings::GetDefaultOrientation() const
{
	return fDefaultOrientation;
}

void ProjectSettings::SetDefaultOrientation(Rtt::DeviceOrientation::Type value)
{
	if (Rtt::DeviceOrientation::IsInterfaceOrientation(value) || (Rtt::DeviceOrientation::kUnknown == value))
	{
		fDefaultOrientation = value;
		if (value != Rtt::DeviceOrientation::kUnknown)
		{
			fOrientationsSupportedSet.insert(value);
		}
	}
}

bool ProjectSettings::IsSupported(Rtt::DeviceOrientation::Type value) const
{
	auto iter = fOrientationsSupportedSet.find(value);
	return (iter != fOrientationsSupportedSet.end());
}

bool ProjectSettings::IsPortraitSupported() const
{
	for (auto iter = fOrientationsSupportedSet.begin(); iter != fOrientationsSupportedSet.end(); iter++)
	{
		if (Rtt::DeviceOrientation::IsUpright(*iter))
		{
			return true;
		}
	}
	return false;
}

bool ProjectSettings::IsLandscapeSupported() const
{
	for (auto iter = fOrientationsSupportedSet.begin(); iter != fOrientationsSupportedSet.end(); iter++)
	{
		if (Rtt::DeviceOrientation::IsSideways(*iter))
		{
			return true;
		}
	}
	return false;
}

int ProjectSettings::GetSupportedOrientationsCount() const
{
	return (int)fOrientationsSupportedSet.size();
}

Rtt::DeviceOrientation::Type ProjectSettings::GetSupportedOrientationByIndex(int index) const
{
	auto orientation = Rtt::DeviceOrientation::kUnknown;
	if ((index >= 0) && (index < GetSupportedOrientationsCount()))
	{
		for (auto iter = fOrientationsSupportedSet.begin(); iter != fOrientationsSupportedSet.end(); iter++)
		{
			if (0 == index)
			{
				orientation = *iter;
				break;
			}
			index--;
		}
	}
	return orientation;
}

const Rtt::NativeWindowMode* ProjectSettings::GetDefaultWindowMode() const
{
	return fDefaultWindowModePointer;
}

bool ProjectSettings::IsWindowResizable() const
{
    return fIsWindowResizable;
}

bool ProjectSettings::SuspendWhenMinimized() const
{
    return fSuspendWhenMinimized;
}

int ProjectSettings::GetMinWindowViewWidth() const
{
	return fMinWindowViewWidth;
}

int ProjectSettings::GetMinWindowViewHeight() const
{
	return fMinWindowViewHeight;
}

int ProjectSettings::GetDefaultWindowViewWidth() const
{
	return fDefaultWindowViewWidth;
}

int ProjectSettings::GetDefaultWindowViewHeight() const
{
	return fDefaultWindowViewHeight;
}

bool ProjectSettings::IsWindowCloseButtonEnabled() const
{
	return fIsWindowCloseButtonEnabled;
}

bool ProjectSettings::IsWindowMinimizeButtonEnabled() const
{
	return fIsWindowMinimizeButtonEnabled;
}

bool ProjectSettings::IsWindowMaximizeButtonEnabled() const
{
	return fIsWindowMaximizeButtonEnabled;
}

const char* ProjectSettings::GetWindowTitleTextForLocale(
	const char* languageCode, const char* countryCode) const
{
	// Optimization: Do not continue if there are no localized strings available.
	if (fLocalizedWindowTitleTextMap.size() <= 0)
	{
		return NULL;
	}

	// Attempt to fetch a localized string in the following order:
	// 1) By language and country.
	// 2) By language. (Supports all countries.)
	// 3) Fallback to the "default" string if all else fails.
	const char* titleText = NULL;
	if (!Rtt_StringIsEmpty(countryCode))
	{
		titleText = GetWindowTitleTextForLocaleWithoutFallback(languageCode, countryCode);
	}
	if (!titleText)
	{
		titleText = GetWindowTitleTextForLocaleWithoutFallback(languageCode, NULL);
	}
	if (!titleText)
	{
		titleText = GetWindowTitleTextForLocaleWithoutFallback("default", NULL);
	}

	// Return the localized string.
	return titleText;
}

const char* ProjectSettings::GetWindowTitleTextForLocaleWithoutFallback(
	const char* languageCode, const char* countryCode) const
{
	// Do not continue if given an invalid language code. Only the country code is optional.
	if (Rtt_StringIsEmpty(languageCode))
	{
		return NULL;
	}

	// Create a lowercase locale string key with the given ISO language code and country code.
	std::string stringKey(languageCode);
	if (!Rtt_StringIsEmpty(countryCode))
	{
		stringKey.append("-");
		stringKey.append(countryCode);
	}
	std::transform(stringKey.begin(), stringKey.end(), stringKey.begin(), ::tolower);

	// Attempt to fetch the localized string.
	const char* titleText = NULL;
	auto iter = fLocalizedWindowTitleTextMap.find(stringKey);
	if (iter != fLocalizedWindowTitleTextMap.end())
	{
		titleText = iter->second.c_str();
	}
	return titleText;
}

int ProjectSettings::GetContentWidth() const
{
	return fContentWidth;
}

int ProjectSettings::GetContentHeight() const
{
	return fContentHeight;
}

int ProjectSettings::GetImageSuffixScaleCount() const
{
	return (int)fImageSuffixScaleSet.size();
}

double ProjectSettings::GetImageSuffixScaleByIndex(int index) const
{
	double scale = 0.0;
	if ((index >= 0) && (index < GetImageSuffixScaleCount()))
	{
		for (auto iter = fImageSuffixScaleSet.begin(); iter != fImageSuffixScaleSet.end(); iter++)
		{
			if (0 == index)
			{
				scale = *iter;
				break;
			}
			index--;
		}
	}
	return scale;
}

bool ProjectSettings::IsWindowTitleShown() const
{
	return fIsWindowTitleShown;
}

bool ProjectSettings::IsWindowTransparent() const
{
	return fIsWindowTransparent;
}

const std::string & ProjectSettings::Backend() const
{
	return fBackend;
}

void ProjectSettings::OnLoadedFrom(lua_State* luaStatePointer)
{
}

}	// namespace Rtt

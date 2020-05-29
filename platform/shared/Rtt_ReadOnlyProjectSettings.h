//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ReadOnlyProjectSettings_H__
#define _Rtt_ReadOnlyProjectSettings_H__

#include "Core/Rtt_Macros.h"
#include "Rtt_DeviceOrientation.h"


namespace Rtt
{

class NativeWindowMode;
class ProjectSettings;

/**
 * Read-only interface to a mutable ProjectSettings object.
 *
 * Provides information about a Corona project's "build.settings" and "config.lua" information.
 */
class ReadOnlyProjectSettings
{
	Rtt_CLASS_NO_COPIES(ReadOnlyProjectSettings)

	public:
		/**
		 * Creates a read-only wrapper around the given project settings.
		 * @param settings The project settings to be wrapped.
		 */
		ReadOnlyProjectSettings(const ProjectSettings& settings);

		/** Destroys this object. */
		virtual ~ReadOnlyProjectSettings();

		/**
		 * Determines if a "build.settings" file was successfully loaded by this object.
		 * @return
		 * Returns true if the object has successfully loaded the Corona project's "build.settings" file.
		 *
		 * Returns false if a "build.settings" file was not found.
		 */
		bool HasBuildSettings() const;

		/**
		 * Determines if a "config.lua" file was successfully loaded by this object.
		 * @return
		 * Returns true if the object has successfully loaded the Corona project's "config.lua" file.
		 *
		 * Returns false if a "config.lua" file was not found.
		 */
		bool HasConfigLua() const;

		/**
		 * Gets the Corona project's default orientation.
		 * @return
		 * Returns the Corona project's default orientation.
		 *
		 * Returns kUnknown if a default has not been assigned.
		 */
		Rtt::DeviceOrientation::Type GetDefaultOrientation() const;

		/**
		 * Determines if the given orientation type is supported by the Corona project.
		 * @param value The orientation type such as kUpright, kSidewaysRight, etc.
		 * @return Returns true if the Corona project supports the given orientation. Returns false if not.
		 */
		bool IsSupported(Rtt::DeviceOrientation::Type value) const;

		/**
		 * Determines if a portrait orientation is supported.
		 * @return
		 * Returns true if a portrait or portrait upside-down orientation is supported by the Corona project.
		 *
		 * Returns false if only landscape is supported or if no supported orientations were specified.
		 */
		bool IsPortraitSupported() const;

		/**
		 * Determines if a landscape orientation is supported.
		 * @return
		 * Returns true if a landscape-right or landscape-left orientation is supported by the Corona project.
		 *
		 * Returns false if only a portrait orientation is supported or if no supported orientations were specified.
		 */
		bool IsLandscapeSupported() const;

		/**
		 * Fetches the number of orientations supported by the Corona project.
		 * Intended to be used in conjunction with the GetSupportedOrientationByIndex() method in a for loop.
		 * @return
		 * Returns the number of orientations supported by the Corona project.
		 *
		 * Returns zero if a "build.settings" file was not found or if it has not specified any supported orientations.
		 */
		int GetSupportedOrientationsCount() const;

		/**
		 * Fetches the Corona project's next supported orientation.
		 * @param index
		 * Zero based index to a supported orientation.
		 *
		 * Given index must be less than the count returned by the GetSupportedOrientationCount() method.
		 * @return
		 * Returns the indexed orientation supported by the Corona project.
		 *
		 * Returns kUnknown if given an invalid index.
		 */
		Rtt::DeviceOrientation::Type GetSupportedOrientationByIndex(int index) const;

		/**
		 * Gets the default mode the window should be launch as such as kNormal, kMaximized, kFullscreen, etc.
		 * @return
		 * Returns a pointer to a window mode constant such as kNormal, kMaximized, kFullscreen, etc.
		 *
		 * Returns null if a default mode was not provided or if the "build.settings" file was not loaded.
		 */
		const Rtt::NativeWindowMode* GetDefaultWindowMode() const;

		/**
		 * Determines if the window can be resized by the end-user.
		 * @return
		 * Returns true if the window can be resized by the end-user.
		 *
		 * Returns false if not set up to be resizable.
		 */
		bool IsWindowResizable() const;

		/**
		 * Gets the minimum width in pixels the window's client/view area is allowed to be resized to.
		 * This is the region within the window's borders that Corona will render to.
		 * @return
		 * Returns the minimum width in pixels the window's client/view area is allowed to be.
		 *
		 * Returns zero if a minimum was not provided.
		 */
		int GetMinWindowViewWidth() const;

		/**
		 * Gets the minimum height in pixels the window's client/view area is allowed to be resized to.
		 * This is the region within the window's borders that Corona will render to.
		 * @return
		 * Returns the minimum height in pixels the window's client/view area is allowed to be.
		 *
		 * Returns zero if a minimum was not provided.
		 */
		int GetMinWindowViewHeight() const;

		/**
		 * Gets the default width in pixels the window's client/view area should be on startup.
		 * This is the region within the window's borders that Corona will render to.
		 * @return
		 * Returns the default width in pixels the window's client/view are should be on startup.
		 *
		 * Returns zero if a default was not provided.
		 */
		int GetDefaultWindowViewWidth() const;

		/**
		 * Gets the default height in pixels the window's client/view area should be on startup.
		 * This is the region within the window's borders that Corona will render to.
		 * @return
		 * Returns the default height in pixels the window's client/view are should be on startup.
		 *
		 * Returns zero if a default was not provided.
		 */
		int GetDefaultWindowViewHeight() const;

		/**
		 * Determines if the close button should be displayed by the window hosting the Corona runtime.
		 * @return
		 * Returns true if the close button should be displayed by the window.
		 *
		 * Returns false if it should be hidden.
		 */
		bool IsWindowCloseButtonEnabled() const;

		/**
		 * Determines if the minimize button should be displayed by the window hosting the Corona runtime.
		 * @return
		 * Returns true if the minimize button should be displayed by the window.
		 *
		 * Returns false if it should be hidden.
		 */
		bool IsWindowMinimizeButtonEnabled() const;

		/**
		 * Determines if the maximize button should be displayed by the window hosting the Corona runtime.
		 * @return
		 * Returns true if the maximize button should be displayed by the window.
		 *
		 * Returns false if it should be hidden.
		 */
		bool IsWindowMaximizeButtonEnabled() const;

		/**
		 * Fetches the UTF-8 encoded localized window title bar text for the given ISO language and country codes.
		 *
		 * If localized text was not found for the given country code, then this method fetches by language code.
		 *
		 * If localized text was not found by either language or country codes, then the default non-localized
		 * text is retrieved.
		 * @param languageCode
		 * The 2 character ISO 639-1 language code such as "en" for English, "fr" for French, etc.
		 *
		 * Can be null or empty, in which case, the "default" non-localized text will be retrieved.
		 * @param countryCode
		 * The 2 character ISO 3166-1 country code such as "us" for United States, "gb" for Great Britain, etc.
		 *
		 * Can be null or empty, in which case, only the language code is looked up.
		 * @return
		 * Returns the localized string for the given ISO langugae and/or country codes.
		 *
		 * Returns the "default" non-localized text if the given locale was not in the localization string table.
		 *
		 * Returns null if the "build.settings" file was not loaded or if it did not provide any title bar text.
		 */
		const char* GetWindowTitleTextForLocale(const char* languageCode, const char* countryCode) const;
		
		/**
		 * Fetches the UTF-8 encoded localized window title bar text for the given ISO language and country codes.
		 *
		 * This method will not fallback to language code only strings or a default non-localized text if the
		 * given language and country code combination was not found in the localization table.
		 * @param languageCode
		 * The 2 character ISO 639-1 language code such as "en" for English, "fr" for French, etc.
		 *
		 * Cannot be null or empty.
		 * @param countryCode
		 * The 2 character ISO 3166-1 country code such as "us" for United States, "gb" for Great Britain, etc.
		 *
		 * Can be null or empty, in which case, only the language code is looked up.
		 * @return
		 * Returns the localized string for the given ISO langugae and country codes.
		 *
		 * Returns null if a localized string was not found for the given arguments.
		 */
		const char* GetWindowTitleTextForLocaleWithoutFallback(const char* languageCode, const char* countryCode) const;

		/**
		 * Gets the content width provided by the "config.lua" file.
		 * @return
		 * Returns the "config.lua" file's content width.
		 *
		 * Returns zero if the "config.lua" file was not found or if it is not set up for content scaling.
		 */
		int GetContentWidth() const;

		/**
		 * Gets the content height provided by the "config.lua" file.
		 * @return
		 * Returns the "config.lua" file's content height.
		 *
		 * Returns zero if the "config.lua" file was not found or if it is not set up for content scaling.
		 */
		int GetContentHeight() const;

		/**
		 * Fetches the number of image suffix scales provided by the project's "config.lua" file.
		 *
		 * Intended to be used in conjunction with the GetImageSuffixScaleByIndex() method in a for loop.
		 * @return
		 * Returns the number of image suffix scales provided by the "config.lua" file.
		 *
		 * Returns zero if the "config.lua" file was not found or if it did not provide any image suffix scales.
		 */
		int GetImageSuffixScaleCount() const;

		/**
		 * Fetches the Corona project's next image suffix scale.
		 * @param index
		 * Zero based index to an image suffix scale.
		 *
		 * Given index must be less than the count returned by the GetImageSuffixScaleCount() method.
		 * @return
		 * Returns the indexed image suffix scale.
		 *
		 * Returns zero if given an invalid index.
		 */
		double GetImageSuffixScaleByIndex(int index) const;

	private:
		/** Reference to the mutable project settings wrapped by this read-only container. */
		const ProjectSettings& fSettings;
};

} // namespace Rtt

#endif // _Rtt_ReadOnlyProjectSettings_H__

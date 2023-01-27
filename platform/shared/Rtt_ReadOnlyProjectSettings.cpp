//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_ReadOnlyProjectSettings.h"
#include "Rtt_ProjectSettings.h"
#include "Rtt_NativeWindowMode.h"


namespace Rtt
{

ReadOnlyProjectSettings::ReadOnlyProjectSettings(const ProjectSettings& settings)
:	fSettings(settings)
{
}

ReadOnlyProjectSettings::~ReadOnlyProjectSettings()
{
}

bool ReadOnlyProjectSettings::HasBuildSettings() const
{
	return fSettings.HasBuildSettings();
}

bool ReadOnlyProjectSettings::HasConfigLua() const
{
	return fSettings.HasConfigLua();
}

Rtt::DeviceOrientation::Type ReadOnlyProjectSettings::GetDefaultOrientation() const
{
	return fSettings.GetDefaultOrientation();
}

bool ReadOnlyProjectSettings::IsSupported(Rtt::DeviceOrientation::Type value) const
{
	return fSettings.IsSupported(value);
}

bool ReadOnlyProjectSettings::IsPortraitSupported() const
{
	return fSettings.IsPortraitSupported();
}

bool ReadOnlyProjectSettings::IsLandscapeSupported() const
{
	return fSettings.IsLandscapeSupported();
}

int ReadOnlyProjectSettings::GetSupportedOrientationsCount() const
{
	return fSettings.GetSupportedOrientationsCount();
}

Rtt::DeviceOrientation::Type ReadOnlyProjectSettings::GetSupportedOrientationByIndex(int index) const
{
	return fSettings.GetSupportedOrientationByIndex(index);
}

const Rtt::NativeWindowMode* ReadOnlyProjectSettings::GetDefaultWindowMode() const
{
	return fSettings.GetDefaultWindowMode();
}

bool ReadOnlyProjectSettings::IsWindowResizable() const
{
	return fSettings.IsWindowResizable();
}

int ReadOnlyProjectSettings::GetMinWindowViewWidth() const
{
	return fSettings.GetMinWindowViewWidth();
}

int ReadOnlyProjectSettings::GetMinWindowViewHeight() const
{
	return fSettings.GetMinWindowViewHeight();
}

int ReadOnlyProjectSettings::GetDefaultWindowViewWidth() const
{
	return fSettings.GetDefaultWindowViewWidth();
}

int ReadOnlyProjectSettings::GetDefaultWindowViewHeight() const
{
	return fSettings.GetDefaultWindowViewHeight();
}

bool ReadOnlyProjectSettings::IsWindowCloseButtonEnabled() const
{
	return fSettings.IsWindowCloseButtonEnabled();
}

bool ReadOnlyProjectSettings::IsWindowMinimizeButtonEnabled() const
{
	return fSettings.IsWindowMinimizeButtonEnabled();
}

bool ReadOnlyProjectSettings::IsWindowMaximizeButtonEnabled() const
{
	return fSettings.IsWindowMaximizeButtonEnabled();
}

const char* ReadOnlyProjectSettings::GetWindowTitleTextForLocale(
	const char* languageCode, const char* countryCode) const
{
	return fSettings.GetWindowTitleTextForLocale(languageCode, countryCode);
}

const char* ReadOnlyProjectSettings::GetWindowTitleTextForLocaleWithoutFallback(
	const char* languageCode, const char* countryCode) const
{
	return fSettings.GetWindowTitleTextForLocaleWithoutFallback(languageCode, countryCode);
}

const char * ReadOnlyProjectSettings::Backend() const
{
	return fSettings.Backend().c_str();
}

int ReadOnlyProjectSettings::GetContentWidth() const
{
	return fSettings.GetContentWidth();
}

int ReadOnlyProjectSettings::GetContentHeight() const
{
	return fSettings.GetContentHeight();
}

int ReadOnlyProjectSettings::GetImageSuffixScaleCount() const
{
	return fSettings.GetImageSuffixScaleCount();
}

double ReadOnlyProjectSettings::GetImageSuffixScaleByIndex(int index) const
{
	return fSettings.GetImageSuffixScaleByIndex(index);
}

}	// namespace Rtt

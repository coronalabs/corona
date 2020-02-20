//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaRuntimeLaunchSettings.h"
#include "CoronaRuntimeEnvironment.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Constructors/Destructors
CoronaRuntimeLaunchSettings::CoronaRuntimeLaunchSettings()
{
	// Initialize the directories to their defaults.
	this->ResourceDirectoryPath = CoronaRuntimeEnvironment::DefaultResourceDirectoryPath;
	this->DocumentsDirectoryPath = CoronaRuntimeEnvironment::DefaultDocumentsDirectoryPath;
	this->TemporaryDirectoryPath = CoronaRuntimeEnvironment::DefaultTemporaryDirectoryPath;
	this->CachesDirectoryPath = CoronaRuntimeEnvironment::DefaultCachesDirectoryPath;
	this->InternalDirectoryPath = CoronaRuntimeEnvironment::DefaultInternalDirectoryPath;

	// Set the launch file to null, which tells Corona to load a "main.lua" or "resource.car" from the
	// root of the resource directory by default. This is the behavior most Corona developers expect.
	this->LaunchFilePath = nullptr;
}

#pragma endregion


#pragma region Public Methods/Properties
void CoronaRuntimeLaunchSettings::CopyFrom(CoronaRuntimeLaunchSettings^ settings)
{
	// Do not continue if given null or a reference to this object.
	if ((nullptr == settings) || (this == settings))
	{
		return;
	}

	// Copy the given settings.
	this->ResourceDirectoryPath = settings->ResourceDirectoryPath;
	this->DocumentsDirectoryPath = settings->DocumentsDirectoryPath;
	this->TemporaryDirectoryPath = settings->TemporaryDirectoryPath;
	this->CachesDirectoryPath = settings->CachesDirectoryPath;
	this->InternalDirectoryPath = settings->InternalDirectoryPath;
	this->LaunchFilePath = settings->LaunchFilePath;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT

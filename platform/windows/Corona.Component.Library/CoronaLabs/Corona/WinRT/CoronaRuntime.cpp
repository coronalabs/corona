//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaRuntime.h"
#include "CoronaRuntimeEnvironment.h"
#include "CoronaRuntimeEventArgs.h"
#include "CoronaRuntimeLaunchSettings.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "CoronaLabs\WinRT\OperationResult.h"
#include "Interop\CoronaInteropSettings.h"
#include <ppltasks.h>


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Constructors/Destructors
CoronaRuntime::CoronaRuntime(Interop::CoronaInteropSettings^ settings)
:	fDefaultInteropSettings(settings),
	fEnvironment(nullptr)
{
	if (nullptr == settings)
	{
		throw ref new Platform::NullReferenceException(L"settings");
	}
}

CoronaRuntime::~CoronaRuntime()
{
	// Destroy the runtime, if not done already.
	Terminate();
	fEnvironment = nullptr;
}

#pragma endregion


#pragma region Public Member Methods/Properties
CoronaRuntimeState CoronaRuntime::State::get()
{
	return fEnvironment ? fEnvironment->RuntimeState : CoronaRuntimeState::NotStarted;
}

CoronaLabs::WinRT::IOperationResult^ CoronaRuntime::Run()
{
	auto settings = ref new CoronaRuntimeLaunchSettings();
	return RunUsing(settings);
}

CoronaLabs::WinRT::IOperationResult^ CoronaRuntime::Run(Platform::String^ filePath)
{
	// Validate.
	if (filePath->IsEmpty())
	{
		throw ref new Platform::NullReferenceException("filePath");
	}

	// Start the Corona runtime with the given file.
	auto settings = ref new CoronaRuntimeLaunchSettings();
	settings->LaunchFilePath = filePath;
	return RunUsing(settings);
}

CoronaLabs::WinRT::IOperationResult^ CoronaRuntime::RunInDirectory(Platform::String^ resourceDirectoryPath)
{
	// Validate.
	if (resourceDirectoryPath->IsEmpty())
	{
		throw ref new Platform::NullReferenceException("resourceDirectoryPath");
	}

	// Start the Corona runtime using the given path as the resource directory.
	auto settings = ref new CoronaRuntimeLaunchSettings();
	settings->ResourceDirectoryPath = resourceDirectoryPath;
	return RunUsing(settings);
}

CoronaLabs::WinRT::IOperationResult^ CoronaRuntime::RunUsing(CoronaRuntimeLaunchSettings^ settings)
{
	// Validate argument.
	if (nullptr == settings)
	{
		throw ref new Platform::NullReferenceException("settings");
	}
	
	// Start setting up the Corona environment settings.
	// Note: The launch settings are copied here because they will likely be modified down below.
	CoronaRuntimeEnvironment::CreationSettings environmentSettings;
	environmentSettings.InteropSettings = fDefaultInteropSettings;
	environmentSettings.LaunchSettings = ref new CoronaRuntimeLaunchSettings();
	environmentSettings.LaunchSettings->CopyFrom(settings);

	/// <summary>
	///  <para>Macro used to validate a Corona sandboxed directory and create its directory tree.</para>
	///  <para>Will return out of the RunUsing() method if failed to create the given directory.</para>
	/// </summary>
	/// <param name="directoryPathProperty">The directory path property to get and set.</param>
	/// <param name="defaultDirectoryPath">
	///  <para>Default directory to use if the "directoryPathProperty" is not set.</para>
	///  <para>The default directory path cannot be null/empty or else an exception will be thrown.</para>
	/// </param>
	#define CORONA_RUNTIME_WINRT_SET_UP_DIRECTORY(directoryPathProperty, defaultDirectoryPath) \
	{ \
		CoronaRuntime::SetUpDirectoryResult^ directoryResult; \
		directoryResult = SetUpDirectory(##directoryPathProperty, ##defaultDirectoryPath); \
		if (directoryResult->HasSucceeded) \
		{ \
			##directoryPathProperty = directoryResult->UpdatedPath; \
		} \
		else \
		{ \
			return directoryResult; \
		} \
	}

	// Set up all directories to be used by the Corona runtime.
	// Validate the given launch settings' custom directory paths if provided and create the directory trees.
	CORONA_RUNTIME_WINRT_SET_UP_DIRECTORY(
			environmentSettings.LaunchSettings->ResourceDirectoryPath,
			CoronaRuntimeEnvironment::DefaultResourceDirectoryPath);
	CORONA_RUNTIME_WINRT_SET_UP_DIRECTORY(
			environmentSettings.LaunchSettings->DocumentsDirectoryPath,
			CoronaRuntimeEnvironment::DefaultDocumentsDirectoryPath);
	CORONA_RUNTIME_WINRT_SET_UP_DIRECTORY(
			environmentSettings.LaunchSettings->TemporaryDirectoryPath,
			CoronaRuntimeEnvironment::DefaultTemporaryDirectoryPath);
	CORONA_RUNTIME_WINRT_SET_UP_DIRECTORY(
			environmentSettings.LaunchSettings->CachesDirectoryPath,
			CoronaRuntimeEnvironment::DefaultCachesDirectoryPath);
	CORONA_RUNTIME_WINRT_SET_UP_DIRECTORY(
			environmentSettings.LaunchSettings->InternalDirectoryPath,
			CoronaRuntimeEnvironment::DefaultInternalDirectoryPath);

	// Verify that we have a file for the Corona runtime to execute.
	if (environmentSettings.LaunchSettings->LaunchFilePath->IsEmpty() == false)
	{
		// A custom file path was given. Check that the file exists.
		if (this->FileExists(environmentSettings.LaunchSettings->LaunchFilePath) == false)
		{
			auto message =
					L"Corona could not find specified launch file: " +
					environmentSettings.LaunchSettings->LaunchFilePath;
			return CoronaLabs::WinRT::OperationResult::FailedWith(message);
		}
	}
	else
	{
		// A file path was not given. Attempt to locate a "resource.car" or "main.lua" file.
		environmentSettings.LaunchSettings->LaunchFilePath =
				environmentSettings.LaunchSettings->ResourceDirectoryPath + L"\\main.lua";
		if (this->FileExists(environmentSettings.LaunchSettings->LaunchFilePath) == false)
		{
			environmentSettings.LaunchSettings->LaunchFilePath =
					environmentSettings.LaunchSettings->ResourceDirectoryPath + L"\\resource.car";
			if (this->FileExists(environmentSettings.LaunchSettings->LaunchFilePath) == false)
			{
				return CoronaLabs::WinRT::OperationResult::FailedWith(
							L"Corona could not find a 'resource.car' or 'main.lua' file to launch.");
			}
		}
	}

	// Set up event handlers to listen in to the CoronaRuntimeEnvironment's events.
	// Note: Event handlers must be passed into the CoronaRuntimeEnvironment's constructor since it might
	//       raise a Loaded or Started events within the constructor.
	environmentSettings.LoadedEventHandler = ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntime::OnLoaded);
	environmentSettings.StartedEventHandler = ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntime::OnStarted);
	environmentSettings.SuspendedEventHandler = ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntime::OnSuspended);
	environmentSettings.ResumedEventHandler = ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntime::OnResumed);
	environmentSettings.TerminatingEventHandler = ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntime::OnTerminating);

	// Terminate the previous Corona runtime, if running.
	Terminate();

	// Create and run a new Corona environment.
	try
	{
		fEnvironment = ref new CoronaRuntimeEnvironment(environmentSettings);
	}
	catch (Platform::Exception^ exception)
	{
		return CoronaLabs::WinRT::OperationResult::FailedWith(exception->Message);
	}

	// We've successfully started the Corona runtime. Return a success result.
	return CoronaLabs::WinRT::OperationResult::Succeeded;
}

void CoronaRuntime::Suspend()
{
	if (fEnvironment)
	{
		fEnvironment->RequestSuspend();
	}
}

void CoronaRuntime::Resume()
{
	if (fEnvironment)
	{
		fEnvironment->RequestResume();
	}
}

void CoronaRuntime::Terminate()
{
	if (fEnvironment)
	{
		fEnvironment->Terminate();
	}
}

#pragma endregion


#pragma region Private Methods
CoronaRuntime::SetUpDirectoryResult^ CoronaRuntime::SetUpDirectory(
	Platform::String^ directoryPath, Platform::String^ defaultDirectoryPath)
{
	// If the given directory path is empty, then use the default director's path.
	if (directoryPath->IsEmpty())
	{
		if (defaultDirectoryPath->IsEmpty())
		{
			throw ref new Platform::NullReferenceException(L"defaultDirectoryPath");
		}
		directoryPath = defaultDirectoryPath;
	}

	// Remove any trailing slashes from the end of the path.
	const wchar_t* utf16String = directoryPath->Data();
	int subStringCharacterCount = 0;
	for (int index = directoryPath->Length() - 1; index > 0; index--)
	{
		wchar_t nextCharacter = utf16String[index];
		if ((nextCharacter != L'\\') && (nextCharacter != L'/') && (nextCharacter != L' '))
		{
			subStringCharacterCount = index + 1;
			break;
		}
	}
	if (subStringCharacterCount <= 0)
	{
		auto message = L"The given directory path is invalid: " + directoryPath;
		return ref new CoronaRuntime::SetUpDirectoryResult(false, nullptr, message);
	}
	else if (subStringCharacterCount < (int)directoryPath->Length())
	{
		auto trimmedDirectoryPath = ref new Platform::String(utf16String, subStringCharacterCount);
		directoryPath = trimmedDirectoryPath;
	}

	// Create the directory tree if it doesn't already exist.
	bool wasCreated = CreateDirectoryTreeFor(directoryPath);
	if (false == wasCreated)
	{
		auto message = L"Failed to access or create a directory for: " + directoryPath;
		return ref new CoronaRuntime::SetUpDirectoryResult(false, nullptr, message);
	}

	// The given directory was successfully set up.
	// Return a success result with an updated directory path in case the path was changed up above.
	return ref new CoronaRuntime::SetUpDirectoryResult(true, directoryPath, nullptr);
}

bool CoronaRuntime::CreateDirectoryTreeFor(Platform::String^ directoryPath)
{
	// Validate argument.
	if ((nullptr == directoryPath) || (directoryPath->Length() < 1))
	{
		return false;
	}
	
	// Do not continue if the given path references an existing directory.
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
	BOOL result = GetFileAttributesEx(directoryPath->Data(), GetFileExInfoStandard, &fileAttributes);
	if (result)
	{
		return true;
	}

	// Get a path to the given directory's parent directory.
	Platform::String^ parentDirectoryPath = nullptr;
	const wchar_t* utf16String = directoryPath->Data();
	int subStringCharacterCount = 0;
	for (int index = directoryPath->Length() - 1; index > 0; index--)
	{
		wchar_t nextCharacter = utf16String[index];
		if ((L'\\' == nextCharacter) || (L'/' == nextCharacter))
		{
			subStringCharacterCount = index;
			break;
		}
	}
	if (subStringCharacterCount > 0)
	{
		parentDirectoryPath = ref new Platform::String(utf16String, subStringCharacterCount);
	}

	// Create the parent directory if it doesn't already exist.
	// Warning: This is a recursive function call.
	if (parentDirectoryPath && (parentDirectoryPath->Length() > 0))
	{
		CreateDirectoryTreeFor(parentDirectoryPath);
	}

	// The given path's parent directories should exist at this point.
	// Attempt to create a directory for the given path.
	result = CreateDirectory(directoryPath->Data(), nullptr);
	return result ? true : false;
}

bool CoronaRuntime::FileExists(Platform::String^ filePath)
{
	FILE *fileHandle = nullptr;
	bool doesExist = false;

	// Do not continue if given a null or empty string.
	if (filePath->IsEmpty())
	{
		return false;
	}

	// Check for file existence by attempting to open it.
	::_wfopen_s(&fileHandle, filePath->Data(), L"r");
	if (fileHandle)
	{
		::fclose(fileHandle);
		doesExist = true;
	}
	return doesExist;
}

void CoronaRuntime::OnLoaded(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Relay the event to this object's event handlers.
	this->Loaded(this, ref new CoronaRuntimeEventArgs(safe_cast<CoronaRuntimeEnvironment^>(sender)));
}

void CoronaRuntime::OnStarted(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Relay the event to this object's event handlers.
	this->Started(this, ref new CoronaRuntimeEventArgs(safe_cast<CoronaRuntimeEnvironment^>(sender)));
}

void CoronaRuntime::OnSuspended(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Relay the event to this object's event handlers.
	this->Suspended(this, ref new CoronaRuntimeEventArgs(safe_cast<CoronaRuntimeEnvironment^>(sender)));
}

void CoronaRuntime::OnResumed(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Relay the event to this object's event handlers.
	this->Resumed(this, ref new CoronaRuntimeEventArgs(safe_cast<CoronaRuntimeEnvironment^>(sender)));
}

void CoronaRuntime::OnTerminating(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Relay the event to this object's event handlers.
	this->Terminating(this, ref new CoronaRuntimeEventArgs(safe_cast<CoronaRuntimeEnvironment^>(sender)));
}

#pragma endregion


#pragma region Private SetUpDirectoryResult Class
CoronaRuntime::SetUpDirectoryResult::SetUpDirectoryResult(
	bool hasSucceeded, Platform::String^ updatedPath, Platform::String^ message)
{
	fHasSucceeded = hasSucceeded;
	fUpdatedPath = updatedPath;
	fMessage = message;
}

bool CoronaRuntime::SetUpDirectoryResult::HasSucceeded::get()
{
	return fHasSucceeded;
}

bool CoronaRuntime::SetUpDirectoryResult::HasFailed::get()
{
	return !fHasSucceeded;
}

Platform::String^ CoronaRuntime::SetUpDirectoryResult::Message::get()
{
	return fMessage;
}

Platform::String^ CoronaRuntime::SetUpDirectoryResult::UpdatedPath::get()
{
	return fUpdatedPath;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTPlatform.h"
#include "Rtt_WinRTAudioPlayer.h"
#include "Rtt_WinRTAudioRecorder.h"
#include "Rtt_WinRTBitmap.h"
#include "Rtt_WinRTEventSound.h"
#include "Rtt_WinRTFBConnect.h"
#include "Rtt_WinRTFont.h"
#include "Rtt_WinRTImageProvider.h"
#include "Rtt_WinRTMapViewObject.h"
#include "Rtt_WinRTScreenSurface.h"
#include "Rtt_WinRTStoreProvider.h"
#include "Rtt_WinRTTimer.h"
#include "Rtt_WinRTTextBoxObject.h"
#include "Rtt_WinRTVideoObject.h"
#include "Rtt_WinRTVideoPlayer.h"
#include "Rtt_WinRTVideoProvider.h"
#include "Rtt_WinRTWebPopup.h"
#include "Rtt_WinRTWebViewObject.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\FontSettings.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\HorizontalAlignment.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IImageServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IImageDecoder.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\ImageDecoderSettings.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\ITextRenderer.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\PixelFormat.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\PixelFormatSet.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\TextRendererSettings.h"
#include "CoronaLabs\Corona\WinRT\Interop\Storage\IResourceServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\CoronaMessageBoxHandler.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\ICoronaControlAdapter.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IPage.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IUserInterfaceServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\MessageBoxSettings.h"
#include "CoronaLabs\Corona\WinRT\Interop\IApplicationServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\InteropServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\ITimerServices.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
#include "CoronaLabs\WinRT\IOperationResult.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include "CoronaLabs\WinRT\Utf8String.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <winnls.h>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Core\Rtt_FileSystem.h"
#	include "Core\Rtt_String.h"
#	include "Display\Rtt_PlatformBitmap.h"
#	include "Rtt_GPUStream.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaLibNative.h"
#	include "Rtt_PlatformExitCallback.h"
#	include "Rtt_PlatformInAppStore.h"
#	include "Rtt_PlatformInAppStore.h"
#	include "Rtt_PreferenceCollection.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#if !defined( Rtt_CUSTOM_CODE )
Rtt_EXPORT const luaL_Reg* Rtt_GetCustomModulesList()
{
	return NULL;
}
#endif


namespace Rtt
{

WinRTPlatform::WinRTPlatform(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment)
:	fAllocator( Rtt_AllocatorCreate() ),
	fEnvironment(environment),
	fDevice(*fAllocator, environment),
	fAudioPlayer(NULL),
	fVideoPlayer(NULL),
	fImageProvider(NULL),
	fVideoProvider(NULL),
	fWebPopup(NULL),	
	fStoreProvider(NULL),
	fFBConnect(NULL)
{
	fBaseDirectoryPaths = ref new Platform::Array<CoronaLabs::WinRT::Utf8String^>(MPlatform::kNumDirs);
	auto utf8DocumentsDirectoryPath = CoronaLabs::WinRT::Utf8String::From(environment->DocumentsDirectoryPath);
	for (int index = 0; index < (int)fBaseDirectoryPaths->Length; index++)
	{
		fBaseDirectoryPaths[index] = utf8DocumentsDirectoryPath;
	}
	fBaseDirectoryPaths[MPlatform::kResourceDir] = CoronaLabs::WinRT::Utf8String::From(environment->ResourceDirectoryPath);
	fBaseDirectoryPaths[MPlatform::kSystemResourceDir] = fBaseDirectoryPaths[MPlatform::kResourceDir];
	fBaseDirectoryPaths[MPlatform::kTmpDir] = CoronaLabs::WinRT::Utf8String::From(environment->TemporaryDirectoryPath);
	fBaseDirectoryPaths[MPlatform::kCachesDir] = CoronaLabs::WinRT::Utf8String::From(environment->CachesDirectoryPath);
	fBaseDirectoryPaths[MPlatform::kSystemCachesDir] = CoronaLabs::WinRT::Utf8String::From(environment->InternalDirectoryPath);
	fBaseDirectoryPaths[MPlatform::kPluginsDir] = CoronaLabs::WinRT::Utf8String::From(
			Windows::ApplicationModel::Package::Current->InstalledLocation->Path);
}

WinRTPlatform::~WinRTPlatform()
{
	Rtt_DELETE(fFBConnect);
	Rtt_DELETE(fStoreProvider);
	Rtt_DELETE(fWebPopup);
	Rtt_DELETE(fVideoPlayer);
	Rtt_DELETE(fAudioPlayer);
	Rtt_DELETE(fImageProvider);
	Rtt_DELETE(fVideoProvider);
}

MPlatformDevice& WinRTPlatform::GetDevice() const
{
	return const_cast<WinRTDevice&>(fDevice);
}

PlatformSurface* WinRTPlatform::CreateScreenSurface() const
{
	auto coronaControl = fEnvironment->CoronaControlAdapter;
	if (nullptr == coronaControl)
	{
		return nullptr;
	}
	return Rtt_NEW(fAllocator, WinRTScreenSurface(fEnvironment));
}

PlatformSurface* WinRTPlatform::CreateOffscreenSurface( const PlatformSurface& parent ) const
{
	return Rtt_NEW(fAllocator, WinRTOffscreenSurface(parent));
}

int WinRTPlatform::CanOpenURL(const char *url) const
{
	// Validate argument.
	if (Rtt_StringIsEmpty(url))
	{
		return 0;
	}

	// Fetch the Corona control. Needed to acquire the parent page for navigation.
	auto coronaControl = fEnvironment->CoronaControlAdapter;
	if (nullptr == coronaControl)
	{
		return 0;
	}

	// Convert the URL string to a URI object.
	Windows::Foundation::Uri^ uri = nullptr;
	try
	{
		uri = ref new Windows::Foundation::Uri(CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(url));
	}
	catch (...)
	{
		return 0;
	}

	// Determine if we can navigate to the given URL's location.
	return coronaControl->ParentPageProxy->CanNavigateTo(uri) ? 1 : 0;
}

bool WinRTPlatform::OpenURL(const char *url) const
{
	// Validate argument.
	if (Rtt_StringIsEmpty(url))
	{
		return false;
	}

	// Fetch the Corona control. Needed to acquire the parent page for navigation.
	auto coronaControl = fEnvironment->CoronaControlAdapter;
	if (nullptr == coronaControl)
	{
		return false;
	}

	// Convert the URL string to a URI object.
	Windows::Foundation::Uri^ uri = nullptr;
	try
	{
		uri = ref new Windows::Foundation::Uri(CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(url));
	}
	catch (...)
	{
		return false;
	}

	// Navigate to the given URL's appropriate application page.
	return coronaControl->ParentPageProxy->NavigateTo(uri);
}

PlatformVideoPlayer* WinRTPlatform::GetVideoPlayer(const Rtt::ResourceHandle<lua_State> & handle) const
{
	if ( ! fVideoPlayer )
	{
		fVideoPlayer = Rtt_NEW( fAllocator, WinRTVideoPlayer( handle, *fAllocator ) );
	}
	return fVideoPlayer;
}

PlatformImageProvider* WinRTPlatform::GetImageProvider(const Rtt::ResourceHandle<lua_State> & handle) const
{
	if ( ! fImageProvider )
	{
		fImageProvider = Rtt_NEW( fAllocator, WinRTImageProvider(handle) );
	}
	return fImageProvider;	
}

PlatformVideoProvider* WinRTPlatform::GetVideoProvider(const Rtt::ResourceHandle<lua_State> & handle) const
{
	if ( ! fVideoProvider )
	{
		fVideoProvider = Rtt_NEW( fAllocator, WinRTVideoProvider(handle) );
	}
	return fVideoProvider;	
}

PlatformStoreProvider* WinRTPlatform::GetStoreProvider( const ResourceHandle<lua_State>& handle ) const
{
	if (!fStoreProvider)
	{
		fStoreProvider = Rtt_NEW( Allocator(), WinRTStoreProvider( handle ) );
	}
	return fStoreProvider;
}

void WinRTPlatform::SetIdleTimer( bool enabled ) const
{
	fEnvironment->InteropServices->ApplicationServices->LockScreenWhenIdleEnabled = enabled;
}

bool WinRTPlatform::GetIdleTimer() const
{
	return fEnvironment->InteropServices->ApplicationServices->LockScreenWhenIdleEnabled;
}

NativeAlertRef WinRTPlatform::ShowNativeAlert(
	const char *title, const char *msg, const char **buttonLabels, U32 numButtons, LuaResource* resource ) const
{
	// Create the native message box handler.
	auto messageBoxHandler = ref new CoronaLabs::Corona::WinRT::Interop::UI::CoronaMessageBoxHandler(fEnvironment);
	if (messageBoxHandler->IsSupported == false)
	{
		Rtt_LogException("Native alerts are not supported.");
		return nullptr;
	}

	// Show the message box using the given settings.
	auto settings = ref new CoronaLabs::Corona::WinRT::Interop::UI::MessageBoxSettings();
	settings->Title = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(title);
	settings->Message = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(msg);
	if (buttonLabels)
	{
		for (U32 index = 0; index < numButtons; index++)
		{
			settings->ButtonLabels->Append(CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(buttonLabels[index]));
		}
	}
	auto result = messageBoxHandler->ShowUsing(settings, resource);
	if (result->HasFailed)
	{
		if (result->Message->IsEmpty())
		{
			Rtt_LogException("Failed to display the native alert.");
		}
		else
		{
			auto utf8ErrorMessage = CoronaLabs::WinRT::NativeStringServices::Utf8From(result->Message);
			Rtt_LogException(utf8ErrorMessage->Data);
		}
		return nullptr;
	}

	// Return the new message box's reference.
	// Can be used by Lua to close this message box programmatically via the Lua native.cancelAlert() function.
	return messageBoxHandler->NativeAlertReference;
}

void WinRTPlatform::CancelNativeAlert(NativeAlertRef alert, S32 index) const
{
	// Fetch the referenced message box.
	auto messageBoxHandler = CoronaLabs::Corona::WinRT::Interop::UI::CoronaMessageBoxHandler::FetchShownMessageBoxBy(alert);
	if (nullptr == messageBoxHandler)
	{
		return;
	}

	// Convert the given button index from a Lua 1 based index to a zero based index.
	index--;

	// Close the message box.
	bool wasClosed = false;
	if (index >= 0)
	{
		// Close it by simulating a button press.
		wasClosed = messageBoxHandler->CloseWithButtonIndex((int)index);
	}
	if (false == wasClosed)
	{
		// Cancel out of the message box. Equivalent to pressing the back key.
		messageBoxHandler->Close();
	}
}

Rtt_Allocator& WinRTPlatform::GetAllocator() const
{
    return *fAllocator;
}

RenderingStream* WinRTPlatform::CreateRenderingStream( bool antialias ) const
{
	RenderingStream* result = Rtt_NEW( fAllocator, GPUStream( fAllocator ) );
	result->SetProperty( RenderingStream::kFlipHorizontalAxis, true );
	return result;
}

PlatformTimer* WinRTPlatform::CreateTimerWithCallback(MCallback& callback) const
{
	return Rtt_NEW(fAllocator, WinRTTimer(callback, fEnvironment->InteropServices->TimerServices));
}

PlatformBitmap* WinRTPlatform::CreateBitmap(const char* utf8FilePath, bool convertToGrayscale) const
{
	// Convert the given file path to UTF-16.
	Platform::String^ utf16FilePath = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8FilePath);
	if (nullptr == utf16FilePath)
	{
		return nullptr;
	}

	// Set up the image decoder.
	auto imageServices = fEnvironment->InteropServices->ImageServices;
	if (nullptr == imageServices)
	{
		Rtt_LogException("Error: Attempted to load an image without providing an 'IImageServices' object to the Corona runtime.");
		return nullptr;
	}
	auto imageDecoder = imageServices->CreateImageDecoder();
	if (nullptr == imageDecoder)
	{
		Rtt_LogException("Error: The IImageServices.CreateImageDecoder() method failed to create an image decoder.");
	}
	int maxTextureSize = (int)Rtt::Display::GetMaxTextureSize();
	imageDecoder->Settings->MaxPixelWidth = maxTextureSize;
	imageDecoder->Settings->MaxPixelHeight = maxTextureSize;
	imageDecoder->Settings->PremultiplyAlphaEnabled = true;
	if (convertToGrayscale)
	{
		imageDecoder->Settings->PreferredPixelFormat = CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Grayscale;
		imageDecoder->Settings->SupportedPixelFormats->Clear();
		imageDecoder->Settings->SupportedPixelFormats->Add(imageDecoder->Settings->PreferredPixelFormat);
	}
	else
	{
		imageDecoder->Settings->PreferredPixelFormat = CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::BGRA;
		imageDecoder->Settings->SupportedPixelFormats->Clear();
		imageDecoder->Settings->SupportedPixelFormats->Add(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::RGBA);
		imageDecoder->Settings->SupportedPixelFormats->Add(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::BGRA);
	}

	// Create and return a new object which will load a bitmap from the given image file.
	return Rtt_NEW(&GetAllocator(), WinRTImageFileBitmap(GetAllocator(), utf16FilePath, imageDecoder));
}

PlatformBitmap* WinRTPlatform::CreateBitmapMask(
	const char text[], const PlatformFont& font, Real width, Real height, const char alignmentName[]) const
{
	// Get the horizontal alignment type.
	// Default to "Left" if the given alignment string is null, empty, or invalid.
	auto horizontalAlignment =
			CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment::FromCoronaStringId(alignmentName);
	if (!horizontalAlignment)
	{
		horizontalAlignment = CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment::Left;
	}

	// Set up a new renderer used to draw text to a bitmap.
	auto imageServices = fEnvironment->InteropServices->ImageServices;
	if (nullptr == imageServices)
	{
		Rtt_LogException("Error: Attempted to load an image without providing an 'IImageServices' object to the Corona runtime.");
		return nullptr;
	}
	auto textRenderer = imageServices->CreateTextRenderer();
	if (nullptr == textRenderer)
	{
		Rtt_LogException("Error: The IImageServices.CreateTextRenderer() method failed to create a text renderer.");
	}
	int maxTextureSize = (int)Rtt::Display::GetMaxTextureSize();
	textRenderer->Settings->ClipWidth = maxTextureSize;
	textRenderer->Settings->ClipHeight = maxTextureSize;
	if ((height > 0) && (height < (float)maxTextureSize))
	{
		textRenderer->Settings->ClipHeight = (int)(height + 0.5f);
	}
	textRenderer->Settings->BlockWidth = (int)(width + 0.5f);
	textRenderer->Settings->BlockHeight = (int)(height + 0.5f);
	textRenderer->Settings->HorizontalAlignment = horizontalAlignment;
	textRenderer->Settings->Text = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(text);
	textRenderer->Settings->FontSettings->CopyFrom(((WinRTFont&)font).GetSettings());

	// Create and return a new object which will render text to a bitmap.
	return Rtt_NEW(&GetAllocator(), WinRTTextBitmap(GetAllocator(), textRenderer));
}

bool WinRTPlatform::FileExists(const char *filename) const
{
	// Stores a static dictionary of all files found under the read-only installation directory for fast lookup.
	static Platform::Collections::Map<Platform::String^, bool> sInstallationFileSet;
	static CoronaLabs::WinRT::Utf8String sUtf8InstallationPath(
				Windows::ApplicationModel::Package::Current->InstalledLocation->Path);

	// Validate.
	if (Rtt_StringIsEmpty(filename))
	{
		return false;
	}

	// Determine if the given file exists.
	bool wasFound = false;
	if (Rtt_StringStartsWith(filename, sUtf8InstallationPath.Data))
	{
		// The given file is under the installation directory, which is a read-only directory.
		// It is safe to assume that a file under this directory will never be changed or deleted.
		// For best performance, check if this file has been found before via our static dictionary.
		auto utf16FilePath = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(filename);
		if (sInstallationFileSet.HasKey(utf16FilePath))
		{
			// File was found in our static dictionary. This means the file was found to exist via the "else" block below.
			wasFound = true;
		}
		else
		{
			// File not found in our static dictionary. Check for file existence via the file system.
			WIN32_FILE_ATTRIBUTE_DATA attributes;
			wasFound = (GetFileAttributesExW(utf16FilePath->Data(), GetFileExInfoStandard, &attributes) != 0);
			if (wasFound)
			{
				// File found. Cache its path to a dictionary for fast file existence checks later.
				sInstallationFileSet.Insert(utf16FilePath, true);
			}
		}
	}
	else
	{
		// The given file is under a writeable directory. Always check for file existence.
		wasFound = (Rtt_FileExists(filename) != 0);
	}
	return wasFound;
}

bool ValidateAssetFile(const char *assetFilename, const int assetSize) const
{
	return true;
}

void WinRTPlatform::PathForFile(const char *filename, MPlatform::Directory baseDir, U32 flags, String &result) const
{
	// Default to the "Documents" directory if given an invalid base directory type.
	if ((baseDir < 0) || (baseDir >= MPlatform::kNumDirs))
	{
		baseDir = MPlatform::kDocumentsDir;
	}

	// Initialize result to an empty string in case the file was not found.
	result.Set(nullptr);

	// Set "result" to the absolute path for the given file.
	if (filename && strstr(filename, "://"))
	{
		// The given file name is a URL.
		// Accept it as is and ignore the base directory.
		result.Set(filename);
	}
	else
	{
		// Fetch the absolute path for the given base directory type.
		const char *directoryPath =
						CoronaLabs::WinRT::NativeStringServices::GetStringPointerFrom(fBaseDirectoryPaths[baseDir]);

		// Always check for file existence if referencing a resource file.
		// This matches the Corona Simulator's behavior.
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
	}
	
	// Check if the file exists, if enabled.
	// Result will be set to an empty string if the file could not be found.
	if (flags & MPlatform::kTestFileExists)
	{
		// Check if the given file name exists.
		bool doesFileExist = FileExists(result.GetString());
		if ((false == doesFileExist) && ((MPlatform::kResourceDir == baseDir) || (MPlatform::kSystemResourceDir)))
		{
			// File not found. Since it is a resource file, check if it is an embedded resource within the DLL.
			auto utf16FileName = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(filename);
			doesFileExist = fEnvironment->InteropServices->ResourceServices->ContainsFile(utf16FileName);
			if (doesFileExist)
			{
				result.Set(filename);
			}
		}
		if (false == doesFileExist)
		{
			// File not found. Return a null path.
			result.Set(nullptr);
		}
	}
}

MPlatform::Directory WinRTPlatform::GetBaseDirectoryFromPath(const char *path) const
{
	// Validate.
	if (Rtt_StringIsEmpty(path))
	{
		return MPlatform::kUnknownDir;
	}

	// Traverse all supported Corona base directories, looking for a match.
	auto pathLength = strlen(path);
	for (int index = 0; index < MPlatform::kNumDirs; index++)
	{
		// Fetch the next base directory path and length.
		auto baseDirectoryPath = fBaseDirectoryPaths[index];
		auto baseDirectoryPathLength = strlen(baseDirectoryPath->Data);

		// The given path must be larger than the base directory path. Skip it if not.
		if (pathLength < baseDirectoryPathLength)
		{
			continue;
		}

		// Do a case insensitive base directory comparison.
		if (_strnicmp(path, baseDirectoryPath->Data, baseDirectoryPathLength) == 0)
		{
			// Found it! Return the base directory enum value.
			return (MPlatform::Directory)index;
		}
	}

	// Base directory match not found. Return the unknown type.
	return MPlatform::kUnknownDir;
}

void WinRTPlatform::SetProjectResourceDirectory(const char *directoryPath)
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void WinRTPlatform::SetSkinResourceDirectory(const char *directoryPath)
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void WinRTPlatform::HttpPost( const char* url, const char* key, const char* value ) const
{
}

PlatformEventSound * WinRTPlatform::CreateEventSound( const ResourceHandle<lua_State> & handle, const char* filePath ) const
{
	WinRTEventSound *p = Rtt_NEW( fAllocator, WinRTEventSound( handle, *fAllocator ) );
	p->Load( filePath );
	return p;
}

void WinRTPlatform::ReleaseEventSound( PlatformEventSound * soundID ) const
{
	Rtt_DELETE( soundID );
}

void WinRTPlatform::PlayEventSound( PlatformEventSound * soundID ) const
{
	soundID->Play();
}

PlatformAudioPlayer * WinRTPlatform::GetAudioPlayer( const ResourceHandle<lua_State> & handle ) const
{
    if ( ! fAudioPlayer )
    {
        fAudioPlayer = Rtt_NEW( fAllocator, WinRTAudioPlayer( handle, *fAllocator ) );
    }
    return fAudioPlayer;
}

PlatformAudioRecorder * WinRTPlatform::CreateAudioRecorder(const Rtt::ResourceHandle<lua_State> & handle, const char * filePath) const
{
    return Rtt_NEW( fAllocator, WinRTAudioRecorder( handle, *fAllocator, filePath ) );
}

void WinRTPlatform::RaiseError( MPlatform::Error e, const char * reason ) const
{
    const char kNull[] = "(null)";
    if ( ! reason ) { reason = kNull; }
    Rtt_TRACE( ( "MPlatformFactory error(%d): %s\n", e, kNull ) );
}

bool WinRTPlatform::SaveImageToPhotoLibrary(const char* filePath) const
{
	return false;
}

bool WinRTPlatform::SaveBitmap(PlatformBitmap * bitmap, const char * filePath, float jpegQuality) const
{
	return false;
}

bool WinRTPlatform::AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const
{
	return SaveBitmap( bitmap, NULL, 1.0f );
}

void WinRTPlatform::SetStatusBarMode( MPlatform::StatusBarMode newValue ) const
{
}

MPlatform::StatusBarMode WinRTPlatform::GetStatusBarMode() const
{
	return MPlatform::kDefaultStatusBar;
}

int WinRTPlatform::GetStatusBarHeight() const
{
	return 0;
}
    
int WinRTPlatform::GetTopStatusBarHeightPixels()  const
{
	return 0;
}
    
int WinRTPlatform::GetBottomStatusBarHeightPixels() const
{
    return 0;
}

int WinRTPlatform::SetSync( lua_State* L ) const
{
	return 0;
}

int WinRTPlatform::GetSync( lua_State* L ) const
{
	return 0;
}

void WinRTPlatform::BeginRuntime( const Runtime& runtime ) const
{
}

void WinRTPlatform::EndRuntime( const Runtime& runtime ) const
{
}

PlatformExitCallback* WinRTPlatform::GetExitCallback()
{
	return NULL;
}

bool WinRTPlatform::RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const
{
	// Validate.
	if (Rtt_StringIsEmpty(actionName))
	{
		return false;
	}

	// Fetch the page that contains the Corona control.
	auto coronaControl = fEnvironment->CoronaControlAdapter;
	if (nullptr == coronaControl)
	{
		return false;
	}
	auto pageProxy = coronaControl->ParentPageProxy;

	// Attempt to execute the requested operation.
	if (Rtt_StringCompare("exitApplication", actionName) == 0)
	{
		// Back out of the page that is hosting Corona's rendering surface.
		pageProxy->NavigateBack();
	}
	else
	{
		// The requested action is unknown.
		return false;
	}

	// Return true to indicate that the requested action was accepted.
	return true;
}

void WinRTPlatform::RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const
{
}

const MCrypto& WinRTPlatform::GetCrypto() const
{
	return fCrypto;
}

void WinRTPlatform::GetPreference(Category category, Rtt::String *value) const
{
	// Validate.
	if (nullptr == value)
	{
		return;
	}

	// Fetch the requested preference value as a UTF-16 string.
	wchar_t utf16StringBuffer[MAX_PATH];
	utf16StringBuffer[0] = L'\0';
	switch (category)
	{
		case kLocaleIdentifier:
			::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, utf16StringBuffer, MAX_PATH);
			break;
		case kLocaleLanguage:
			::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SISO639LANGNAME, utf16StringBuffer, MAX_PATH);
			break;
		case kLocaleCountry:
			::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, utf16StringBuffer, MAX_PATH);
			break;
		case kUILanguage:
			::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SLOCALIZEDDISPLAYNAME, utf16StringBuffer, MAX_PATH);
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	// Copy the preference value to the given "value" argument.
	// Sets value to NULL (an empty string) if the given preference is not supported.
	if (utf16StringBuffer[0] != L'\0')
	{
		auto utf8String = CoronaLabs::WinRT::Utf8String::From(utf16StringBuffer);
		value->Set(utf8String->Data);
	}
	else
	{
		value->Set(nullptr);
	}
}

Preference::ReadValueResult WinRTPlatform::GetPreference(const char* categoryName, const char* keyName) const
{
	std::string message;
	message = "Category name '";
	message += categoryName ? categoryName : "";
	message += "' is not supported on this platform.";
	return Preference::ReadValueResult::FailedWith(message.c_str());
}

OperationResult WinRTPlatform::SetPreferences(const char* categoryName, const PreferenceCollection& collection) const
{
	return OperationResult::FailedWith("This API is not supported on this platform.");
}

OperationResult WinRTPlatform::DeletePreferences(
	const char* categoryName, const char** keyNameArray, U32 keyNameCount) const
{
	return OperationResult::FailedWith("This API is not supported on this platform.");
}

void WinRTPlatform::SetActivityIndicator(bool visible) const
{
}

PlatformWebPopup* WinRTPlatform::GetWebPopup() const
{
	if (!fWebPopup)
	{
		fWebPopup = Rtt_NEW( & GetAllocator(), WinRTWebPopup() );
	}
	return fWebPopup;
}

bool WinRTPlatform::CanShowPopup( const char *name ) const
{
	return false;
}

bool WinRTPlatform::ShowPopup( lua_State *L, const char *name, int optionsIndex ) const
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

bool WinRTPlatform::HidePopup( const char *name ) const
{
	return false;
}

PlatformDisplayObject* WinRTPlatform::CreateNativeTextBox( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), WinRTTextBoxObject( bounds, false ) );
}

PlatformDisplayObject * WinRTPlatform::CreateNativeTextField( const Rect & bounds ) const
{
	return Rtt_NEW( & GetAllocator(), WinRTTextBoxObject( bounds, true ) );
}

void WinRTPlatform::SetKeyboardFocus( PlatformDisplayObject * textObject ) const
{
	if (textObject)
	{
		((WinRTDisplayObject*)textObject)->SetFocus();
	}
	else
	{
		//TODO: Remove keyboard focus from native display objects.
	}
}

PlatformDisplayObject * WinRTPlatform::CreateNativeMapView( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), WinRTMapViewObject( bounds ) );
}

PlatformDisplayObject * WinRTPlatform::CreateNativeWebView( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), WinRTWebViewObject( bounds ) );
}

PlatformDisplayObject * WinRTPlatform::CreateNativeVideo( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), WinRTVideoObject( bounds ) );
}

PlatformDisplayObject * WinRTPlatform::GetNativeDisplayObjectById( const int objectId ) const
{
	return NULL;
}

Rtt_Real WinRTPlatform::GetStandardFontSize() const
{
	// This is the default font size matching .NET silverlight resource constant "PhoneFontSizeMediumLarge".
	// Note: We should fetch this value from .NET in case it changes in the future.
	double fontSize = 25.333;

	// Acquire the device's native scaling factor.
	double deviceScale = 1.0;
	auto userInterfaceServices = fEnvironment->InteropServices->UserInterfaceServices;
	if (userInterfaceServices)
	{
		deviceScale = userInterfaceServices->ScaleFactor;
	}

	// Return the default font size with the device's native scaling factor already applied to it.
	fontSize *= deviceScale;
	return Rtt_FloatToReal((float)fontSize);
}

S32 WinRTPlatform::GetFontNames( lua_State *L, int index ) const
{
	S32 numFonts = 0;
#if 0
	Rtt::StringArray fonts( fAllocator );
	if ( NativeToJavaBridge::GetInstance()->GetFonts( fonts ) ) {
		numFonts = fonts.GetLength();
		
		for ( int i = 0; i < numFonts; i++ )
		{
			lua_pushstring( L, fonts.GetElement( i ) );
			lua_rawseti( L, index, i + 1 );
		}
	}
#endif
	return numFonts;
}

PlatformFont* WinRTPlatform::CreateFont(PlatformFont::SystemFont fontType, Rtt_Real size) const
{
	auto fontPointer = Rtt_NEW(fAllocator, WinRTFont(fEnvironment, nullptr));
	fontPointer->GetSettings()->PointSize = Rtt_RealToFloat(size);
	fontPointer->GetSettings()->IsBold = (PlatformFont::kSystemFontBold == fontType);
	return fontPointer;
}

PlatformFont* WinRTPlatform::CreateFont(const char *fontName, Rtt_Real size) const
{
	auto fontPointer = Rtt_NEW(fAllocator, WinRTFont(fEnvironment, fontName));
	fontPointer->GetSettings()->PointSize = Rtt_RealToFloat(size);
	return fontPointer;
}

void WinRTPlatform::SetTapDelay( Rtt_Real delay ) const
{
}

Rtt_Real WinRTPlatform::GetTapDelay() const
{
	return Rtt_REAL_1;
}

PlatformFBConnect* WinRTPlatform::GetFBConnect() const
{
	if ( NULL == fFBConnect )
	{
		fFBConnect = Rtt_NEW( fAllocator, WinRTFBConnect );
	}
	return fFBConnect;
}

void* WinRTPlatform::CreateAndScheduleNotification( lua_State *L, int index ) const
{
	return NULL;
}

void WinRTPlatform::ReleaseNotification( void *notificationId ) const
{
}

void WinRTPlatform::CancelNotification( void *notificationId ) const
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

void WinRTPlatform::FlurryInit(const char *applicationKey) const
{
}

void WinRTPlatform::FlurryEvent(const char *eventId) const
{
}

void WinRTPlatform::SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const
{
	// Validate arguments.
	if (!L || Rtt_StringIsEmpty(key) || !valueIndex)
	{
		return;
	}

	// Handle the given property value.
	if (Rtt_StringCompare(key, "applicationIconBadgeNumber") == 0)
	{
		// Clear all notifications if the application badge number was set to zero.
		// Note: This is to match iOS' behavior.
		if (lua_type(L, valueIndex) == LUA_TNUMBER)
		{
			int badgeNumber = lua_tointeger(L, valueIndex);
			if (badgeNumber <= 0)
			{
				//TODO: Cancel all notifications.
			}
			//TODO: Update the badge count on the Windows Phone tile???
		}
	}
}

int WinRTPlatform::PushNativeProperty( lua_State *L, const char *key ) const
{
	return 0;
}

int WinRTPlatform::PushSystemInfo(lua_State *L, const char *key) const
{
	// Validate.
	if (!L)
	{
		return 0;
	}

	// Push the requested system information to Lua.
	int pushedValues = 0;
	if (Rtt_StringCompare(key, "isoCountryCode") == 0)
	{
		wchar_t utf16StringBuffer[16];
		utf16StringBuffer[0] = L'\0';
		::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, utf16StringBuffer, MAX_PATH);
		auto utf8String = CoronaLabs::WinRT::Utf8String::From(utf16StringBuffer);
		lua_pushstring(L, utf8String->Data);
		pushedValues = 1;
	}
	else if (Rtt_StringCompare(key, "isoLanguageCode") == 0)
	{
		// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
		// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
		//       For example, this can happen with the Hawaiian language, which will return "haw".
		wchar_t utf16StringBuffer[128];
		utf16StringBuffer[0] = L'\0';
		::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SPARENT, utf16StringBuffer, MAX_PATH);
		auto utf8String = CoronaLabs::WinRT::Utf8String::From(utf16StringBuffer);
		lua_pushstring(L, utf8String->Data);
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

void WinRTPlatform::NetworkBaseRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *path ) const
{
}

void WinRTPlatform::NetworkRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex ) const
{
	NetworkBaseRequest( L, url, method, listener, paramsIndex, NULL );
}

void WinRTPlatform::NetworkDownload( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *filename, MPlatform::Directory baseDir ) const
{
	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();

	String filePath( runtime.GetAllocator() );
	platform.PathForFile( filename, baseDir, MPlatform::kDefaultPathFlags, filePath );
	NetworkBaseRequest( L, url, method, listener, paramsIndex, filePath.GetString() );
}


PlatformReachability* WinRTPlatform::NewReachability( const ResourceHandle<lua_State>& handle, PlatformReachability::PlatformReachabilityType type, const char* address ) const
{
	return Rtt_NEW( fAllocator, PlatformReachability( handle, type, address ) );
}

bool WinRTPlatform::SupportsNetworkStatus() const
{
	return false;
}

void
WinRTPlatform::Suspend( ) const
{
}

void
WinRTPlatform::Resume( ) const
{
}

} // namespace Rtt

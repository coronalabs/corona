//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Time.h"
#include "Core/Rtt_String.h"
#include "Corona/CoronaLua.h"

#define TEMPORARY_HACK 1
// TODO: Remove need for this. See also "TEMPORARY_HACK" in code
#if TEMPORARY_HACK
	#include "librtt/Rtt_GPUStream.h"	
#endif

#include "Display/Rtt_Display.h"
#include "librtt/Display/Rtt_Display.h"
#include "librtt/Display/Rtt_Scene.h"
#include "librtt/Display/Rtt_StageObject.h"
#include "librtt/Rtt_Runtime.h"	
#include "librtt/Rtt_Event.h"
#include "librtt/Rtt_DeviceOrientation.h"
#include "librtt/Rtt_LuaContext.h"
#include "librtt/Rtt_LuaResource.h"
#include "librtt/Rtt_PlatformInAppStore.h"
#include "librtt/Input/Rtt_InputAxisType.h"
#include "librtt/Input/Rtt_InputDeviceConnectionState.h"
#include "librtt/Input/Rtt_InputDeviceType.h"
#include "librtt/Input/Rtt_PlatformInputAxis.h"
#include "librtt/Input/Rtt_ReadOnlyInputDeviceCollection.h"
#include "Rtt_AndroidAudioRecorder.h"
#include "Rtt_AndroidDisplayObject.h"
#include "Rtt_AndroidImageProvider.h"
#include "Rtt_AndroidInputDeviceManager.h"
#include "Rtt_AndroidInputDevice.h"
#include "Rtt_AndroidMapViewObject.h"
#include "Rtt_AndroidPlatform.h"
#include "Rtt_AndroidRuntimeDelegate.h"
#include "Rtt_AndroidStore.h"
#include "Rtt_AndroidStoreProvider.h"
#include "Rtt_AndroidStoreTransaction.h"
#include "Rtt_AndroidSystemOpenEvent.h"
#include "Rtt_AndroidTextFieldObject.h"
#include "Rtt_AndroidVideoObject.h"
#include "Rtt_AndroidVideoProvider.h"
#include "Rtt_AndroidWebPopup.h"
#include "Rtt_AndroidWebViewObject.h"

#include "JavaToNativeBridge.h"
#include "AndroidImageData.h"
#include "AndroidGLView.h"
#include "NativeToJavaBridge.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <android/bitmap.h>
#include <android/log.h>

#include "importgl.h"

#if defined( Rtt_SUPPORTS_NOOK ) && defined( Rtt_SUPPORTS_KINDLE )
	#define Rtt_ENTERPRISE 1
#endif

// ----------------------------------------------------------------------------

// TODO: goal is to expose Runtime* through theWrapper singleton.
// Not clear this is the ideal way to do it architecturally...

JavaToNativeBridge::JavaToNativeBridge()
:	fView( NULL ),
	fPlatform( NULL ),
	fRuntime( NULL ),
	fRuntimeDelegate( NULL ),
	fIsResourcesLoaded( false ),
	fCoronaRuntime( NULL ),
	fNativeToJavaBridge( NULL )
{
	fGravityAccel[0] = fGravityAccel[1] = fGravityAccel[2] = 0.0f;
	fInstantAccel[0] = fInstantAccel[1] = fInstantAccel[2] = 0.0f;
	fPreviousShakeTime = 0.0f;
	fMultitouchEventCount = 0;
	fMultitouchEventBuffer = new Rtt::TouchEvent[kMaxMultitouchPointerEvents];
}

JavaToNativeBridge::~JavaToNativeBridge()
{
	delete[] fMultitouchEventBuffer;
}

void 
JavaToNativeBridge::Init(
	JNIEnv * env, jstring packageJ, jstring documentsDirJ, jstring applicationSupportDirJ, jstring temporaryDirJ, jstring cachesDirJ,
	jstring systemCachesDirJ, jstring expansionFileDirJ, int width, int height, int orientation, bool isCoronaKit )
{
	NativeTrace trace( "JavaToNativeBridge::Init" );
	if ( fView != NULL )
	{
		Rtt::DeviceOrientation::Type lastOrientation = fView->GetOrientation();
		fView->SetOrientation( (Rtt::DeviceOrientation::Type)orientation );
		fView->Resize(width, height);

		Rtt::Display& display = fRuntime->GetDisplay();

		// TEMPORARY_HACK (begin)
		// TODO: Remove need to access Renderer directly
		Rtt::RenderingStream& stream = display.GetStream();
		// TEMPORARY_HACK (end)

		if (!isCoronaKit &&
			Rtt::DeviceOrientation::IsSideways(lastOrientation) !=
		    Rtt::DeviceOrientation::IsSideways(fView->GetOrientation()))
		{
			stream.SwapContentSize();
			stream.SwapContentAlign();
		}
		stream.UpdateContentScale(width, height);

		fRuntime->RestartRenderer((Rtt::DeviceOrientation::Type)orientation);
		display.GetScene().Invalidate();
		display.GetStage()->Invalidate( Rtt::DisplayObject::kRenderDirty );
		ReloadResources();
	}
	else
	{
		fView = new AndroidGLView();

		fView->CreateFramebuffer( width, height, (Rtt::DeviceOrientation::Type)orientation );

		jstringResult packageName( env, packageJ );
		jstringResult documentsDir( env, documentsDirJ );
		jstringResult applicationSupportDir( env, applicationSupportDirJ );
		jstringResult temporaryDir( env, temporaryDirJ );
		jstringResult cachesDir( env, cachesDirJ );
		jstringResult systemCachesDir( env, systemCachesDirJ );
		jstringResult expansionFileDir( env, expansionFileDirJ );
		
		packageName.setNotLocal();
		documentsDir.setNotLocal();
		applicationSupportDir.setNotLocal();
		temporaryDir.setNotLocal();
		cachesDir.setNotLocal();
		systemCachesDir.setNotLocal();
		expansionFileDir.setNotLocal();
		fNativeToJavaBridge = NativeToJavaBridge::InitInstance( env, fRuntime, fCoronaRuntime );
		fPlatform = new Rtt::AndroidPlatform(
							fView, packageName.getUTF8(), documentsDir.getUTF8(), applicationSupportDir.getUTF8(),
							temporaryDir.getUTF8(), cachesDir.getUTF8(), systemCachesDir.getUTF8(), expansionFileDir.getUTF8(),
							fNativeToJavaBridge );

		fRuntime = Rtt_NEW( & fPlatform->GetAllocator(), Rtt::Runtime( * fPlatform ) );

		fNativeToJavaBridge->SetRuntime( fRuntime );
		
		fView->SetNativeToJavaBridge( fNativeToJavaBridge );

		fRuntimeDelegate = Rtt_NEW( & fPlatform->GetAllocator(), Rtt::AndroidRuntimeDelegate(fNativeToJavaBridge, isCoronaKit) );
		fRuntime->SetDelegate(fRuntimeDelegate);

		((Rtt::AndroidDevice&)fPlatform->GetDevice()).SetOrientation( (Rtt::DeviceOrientation::Type)orientation );
		fNativeToJavaBridge->FetchAllInputDevices();
#ifdef Rtt_ENTERPRISE
		fRuntime->SetProperty( Rtt::Runtime::kIsEnterpriseFeature, true );
#endif

		fRuntime->SetProperty( Rtt::Runtime::kIsCoronaKit, isCoronaKit );
		if (isCoronaKit)
		// if (true)		// for Tegra debugging
		{
			fRuntime->SetProperty(Rtt::Runtime::kIsApplicationNotArchived, true);
			fRuntime->SetProperty(Rtt::Runtime::kIsLuaParserAvailable, true);
		}

		// Load the Corona application, starting with the "config.lua" and "shell.lua".
		Rtt::Runtime::LoadApplicationReturnCodes retCode = fRuntime->LoadApplication(	Rtt::Runtime::kLaunchDeviceShell, 
																						(Rtt::DeviceOrientation::Type)orientation);		

		if (Rtt::Runtime::kSuccess == retCode)
		{
			// Load was successful. Start running the Corona application.
			fIsResourcesLoaded = true;
			fView->SetCallback( fRuntime );
			fRuntime->BeginRunLoop();

#if defined( Rtt_TRIAL )
			// Display a nag window for trial builds.
			fNativeToJavaBridge->ShowTrialAlert();
#endif
		}
		else
		{
			Rtt_LogException("This application failed to load and execute main.lua");
		}

		// else
		// {
		// 	// Failed to load the "main.lua" file.
		// 	if (fNativeToJavaBridge->HasLuaErrorOccurred())
		// 	{
		// 		// An unhandled Lua runtime error occurred.
		// 		// These errors are automatically displayed in an alert dialog. So, do nothing.
		// 	}
		// 	else
		// 	{
		// 		// One of the following has occurred:
		// 		// - The runtime failed to find the "resource.car" file or it is corrupted.
		// 		// - A Lua runtime error occurred in the "main.lua", but has been overriden.
		// 		fNativeToJavaBridge->ShowNativeAlert(
		// 				"Error", "This application encountered a Lua error (see logs) or has been corrupted.",
		// 				NULL, 0, NULL );
		// 	}
		// }
	}
}

void
JavaToNativeBridge::SetCoronaRuntime(jobject runtime)
{
	fCoronaRuntime = runtime;
}

jobject
JavaToNativeBridge::GetCoronaRuntime()
{
	return fCoronaRuntime;
}

void 
JavaToNativeBridge::ReinitializeRenderer()
{
	NativeTrace trace( "JavaToNativeBridge::ReinitializeRenderer" );

	if ( fView )
	{
		fView->ReinitializeRenderingStream();
	}
}

void
JavaToNativeBridge::UnloadResources()
{
	if (fRuntime && fIsResourcesLoaded)
	{
		NativeTrace trace( "JavaToNativeBridge::UnloadResources" );
		fRuntime->UnloadResources();
		fIsResourcesLoaded = false;
	}
}

void
JavaToNativeBridge::ReloadResources()
{
	if (fRuntime && (false == fIsResourcesLoaded))
	{
		NativeTrace trace( "JavaToNativeBridge::ReloadResources" );
		fRuntime->GetDisplay().GetScene().Invalidate();
		fRuntime->ReloadResources();
		fIsResourcesLoaded = true;
	}
}

void 
JavaToNativeBridge::Deinit()
{
	NativeTrace trace( "JavaToNativeBridge::Deinit" );

	if (fNativeToJavaBridge == NULL)
	{
		return;
	}
	
	// Notify the application that the runtime is being terminated.
	fNativeToJavaBridge->OnRuntimeExiting();
	
	// Delete in opposite order of instantiation
	// Per this thread: http://groups.google.com/group/android-ndk/browse_thread/thread/3e0661379875c7ca
	// We need to be able to NULL-out variables during shutdown since NDK statics persist across app launches!
	// So we NULL out the member variables of the persistent static instance.
	Rtt_DELETE( fRuntime );
	fRuntime = NULL;
	
	Rtt_DELETE( fRuntimeDelegate );
	fRuntimeDelegate = NULL;
	
	Rtt_DELETE( fPlatform );
	fPlatform = NULL;

	if ( fView )
	{
		fView->DestroyFramebuffer();
	}
	Rtt_DELETE( fView );
	fView = NULL;

	Rtt_DELETE( fNativeToJavaBridge );
	fNativeToJavaBridge = NULL;
}

size_t
JavaToNativeBridge::GetMaxTextureSize()
{
	return Rtt::Display::GetMaxTextureSize();
}

int
JavaToNativeBridge::GetHorizontalMarginInPixels()
{
	S32 x = 0;
	S32 y = 0;
	S32 width = 0;
	S32 height = 0;
	
	if (fRuntime)
	{
		fRuntime->GetDisplay().ContentToPixels(x, y, width, height);
	}
	return x;
}

int
JavaToNativeBridge::GetVerticalMarginInPixels()
{
	S32 x = 0;
	S32 y = 0;
	S32 width = 0;
	S32 height = 0;
	
	if (fRuntime)
	{
		fRuntime->GetDisplay().ContentToPixels(x, y, width, height);
	}
	return y;
}

int
JavaToNativeBridge::GetContentWidthInPixels()
{
	S32 x = 0;
	S32 y = 0;
	S32 width = 0;
	S32 height = 0;
	
	if (fRuntime)
	{
		width = fRuntime->GetDisplay().ContentWidth();
		fRuntime->GetDisplay().ContentToPixels(x, y, width, height);
	}
	return width;
}

int
JavaToNativeBridge::GetContentHeightInPixels()
{
	S32 x = 0;
	S32 y = 0;
	S32 width = 0;
	S32 height = 0;
	
	if (fRuntime)
	{
		height = fRuntime->GetDisplay().ContentHeight();
		fRuntime->GetDisplay().ContentToPixels(x, y, width, height);
	}
	return height;
}

void
JavaToNativeBridge::ConvertCoronaPointToAndroidPoint(int& x, int& y)
{
	using namespace Rtt;

	const Display& display = fRuntime->GetDisplay();
	int w = 0, h = 0;
	display.ContentToPixels( x, y, w, h );

Rtt_ASSERT_NOT_IMPLEMENTED();
/*
	if ( stream.IsProperty(RenderingStream::kFlipVerticalAxis) )
	{
		x = fRuntime->Surface().Width() - x;
	}
	if ( stream.IsProperty(RenderingStream::kFlipHorizontalAxis) )
	{
		y = fRuntime->Surface().Height() - y;
	}
*/
}

bool
JavaToNativeBridge::CopyBitmapInfo(
	JNIEnv *env, jlong nativeImageMemoryAddress, int width, int height,
	float downscaleFactor, int rotationInDegrees)
{
	// Validate.
	if (!nativeImageMemoryAddress)
	{
		return false;
	}

	// Copy the loaded image information to the given image object.
	AndroidImageData *imagePointer = (AndroidImageData*)nativeImageMemoryAddress;
	imagePointer->SetWidth((width >= 0) ? (U32)width : 0);
	imagePointer->SetHeight((height >= 0) ? (U32)height : 0);
	imagePointer->SetScale(Rtt_FloatToReal((float)downscaleFactor));
	imagePointer->SetOrientationInDegrees(rotationInDegrees);
	return true;
}

bool
JavaToNativeBridge::CopyBitmap(
	JNIEnv *env, jlong nativeImageMemoryAddress, jobject bitmap, float downscaleFactor,
	int rotationInDegrees, bool convertToGrayscale)
{
	// Validate.
	if (!nativeImageMemoryAddress)
	{
		return false;
	}

	// Fetch the Java bitmap's information such as width, height, pixel format, and stride.
	AndroidBitmapInfo bitmapInfo;
	int resultValue = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
	if ((resultValue < 0) || (bitmapInfo.width <= 0) || (bitmapInfo.height <= 0))
	{
		return false;
	}

	// Determine the Java bitmap's pixel size in bytes.
	int sourcePixelSizeInBytes;
	switch (bitmapInfo.format)
	{
		case ANDROID_BITMAP_FORMAT_RGBA_8888:
			sourcePixelSizeInBytes = 4;
			break;
		case ANDROID_BITMAP_FORMAT_RGB_565:
		case ANDROID_BITMAP_FORMAT_RGBA_4444:
			sourcePixelSizeInBytes = 2;
			break;
		case ANDROID_BITMAP_FORMAT_A_8:
			sourcePixelSizeInBytes = 1;
			break;
		default:
			Rtt_LogException(
					"Failed to copy Java bitmap because it uses unknown pixel format '%d'.",
					bitmapInfo.format);
			return false;
	}
	int rowPaddingInBytes = bitmapInfo.stride - (bitmapInfo.width * sourcePixelSizeInBytes);
	if (rowPaddingInBytes < 0)
	{
		rowPaddingInBytes = 0;
	}

	// Create the byte buffer to copy the Java bitmap's pixels to.
	AndroidImageData *imagePointer = (AndroidImageData*)nativeImageMemoryAddress;
	imagePointer->SetWidth(bitmapInfo.width);
	imagePointer->SetHeight(bitmapInfo.height);
	imagePointer->SetScale(Rtt_FloatToReal((float)downscaleFactor));
	imagePointer->SetOrientationInDegrees(rotationInDegrees);
	if (convertToGrayscale)
	{
		imagePointer->SetPixelFormatToGrayscale();
	}
	else
	{
		imagePointer->SetPixelFormatToRGBA();
	}

	// Create the target image buffer.
	imagePointer->CreateImageByteBuffer();
	char *targetByteBufferPointer = (char*)imagePointer->GetImageByteBuffer();
	if (!targetByteBufferPointer)
	{
		Rtt_LogException("Failed to copy Java bitmap due to buffer not created.");
		return false;
	}

	// Fetch a pointer to the Java bitmap's byte buffer.
	char *sourceByteBufferPointer = NULL;
	resultValue = AndroidBitmap_lockPixels(env, bitmap, (void**)&sourceByteBufferPointer);
	if (resultValue < 0)
	{
		Rtt_LogException("Unable to access the Java bitmap's bytes.");
		return false;
	}

	// Copy the Java bitmap's pixels to the C/C++ bitmap buffer.
	if ((rowPaddingInBytes <= 0) &&
	    ((convertToGrayscale && (ANDROID_BITMAP_FORMAT_A_8 == bitmapInfo.format)) ||
	     (!convertToGrayscale && (ANDROID_BITMAP_FORMAT_RGBA_8888 == bitmapInfo.format))))
	{
		// The format of the source bitmap exactly matches the target bitmap. No pixel conversion is needed.
		// In this case, it is much faster to do a memcpy().
		size_t byteBufferLength = (size_t)bitmapInfo.width * (size_t)bitmapInfo.height;
		byteBufferLength *= (size_t)sourcePixelSizeInBytes;
		memcpy(targetByteBufferPointer, sourceByteBufferPointer, byteBufferLength);
	}
	else
	{
		// Copy one pixel at a time, converting them to the target's expected color format.
		for (int rowIndex = 0; rowIndex < bitmapInfo.height; rowIndex++)
		{
			for (int columnIndex = 0; columnIndex < bitmapInfo.width; columnIndex++)
			{
				if (convertToGrayscale)
				{
					char alpha = 0;
					switch (bitmapInfo.format)
					{
						case ANDROID_BITMAP_FORMAT_A_8:
							alpha = *sourceByteBufferPointer;
							break;
						case ANDROID_BITMAP_FORMAT_RGBA_8888:
							alpha = (char)(
								(0.30f * sourceByteBufferPointer[0]) +
								(0.59f * sourceByteBufferPointer[1]) +
								(0.11f * sourceByteBufferPointer[2]));
							break;
						case ANDROID_BITMAP_FORMAT_RGB_565:
						{
							uint16_t pixel = (uint16_t)sourceByteBufferPointer[0];
							pixel |= ((uint16_t)sourceByteBufferPointer[1]) << 8;
							float fractionalAlpha = 0.11f * ((pixel & 0x1F) * 8);
							pixel >>= 5;
							fractionalAlpha += 0.59f * ((pixel & 0x3F) * 4);
							pixel >>= 6;
							fractionalAlpha += 0.30f * ((pixel & 0x1F) * 8);
							alpha = (char)fractionalAlpha;
							break;
						}
						case ANDROID_BITMAP_FORMAT_RGBA_4444:
						{
							float fractionalAlpha = 0.30f * (((sourceByteBufferPointer[1] >> 4) & 0xF) * 17);
							fractionalAlpha += 0.59f * ((sourceByteBufferPointer[1] & 0xF) * 17);
							fractionalAlpha += 0.11f * (((sourceByteBufferPointer[0] >> 4) & 0xF) * 17);
							alpha = (char)fractionalAlpha;
							break;
						}
					}
					*targetByteBufferPointer = alpha;
					targetByteBufferPointer++;
				}
				else
				{
					char red = 0;
					char green = 0;
					char blue = 0;
					char alpha = 255;
					switch (bitmapInfo.format)
					{
						case ANDROID_BITMAP_FORMAT_RGBA_8888:
							red = sourceByteBufferPointer[0];
							green = sourceByteBufferPointer[1];
							blue = sourceByteBufferPointer[2];
							alpha = sourceByteBufferPointer[3];
							break;
						case ANDROID_BITMAP_FORMAT_RGB_565:
						{
							uint16_t pixel = (uint16_t)sourceByteBufferPointer[0];
							pixel |= ((uint16_t)sourceByteBufferPointer[1]) << 8;
							blue = pixel & 0x1F;
							blue = (blue << 3) | (blue >> 2);
							pixel >>= 5;
							green = pixel & 0x3F;
							green = (green << 2) | (green >> 4);
							pixel >>= 6;
							red = pixel & 0x1F;
							red = (red << 3) | (red >> 2);
							break;
						}
						case ANDROID_BITMAP_FORMAT_RGBA_4444:
							alpha = (sourceByteBufferPointer[0] & 0xF);
							alpha |= alpha << 4;
							blue = ((sourceByteBufferPointer[0] >> 4) & 0xF);
							blue |= blue << 4;
							green = (sourceByteBufferPointer[1] & 0xF);
							green |= green << 4;
							red = ((sourceByteBufferPointer[1] >> 4) & 0xF);
							red |= red << 4;
							break;
						case ANDROID_BITMAP_FORMAT_A_8:
							alpha = *sourceByteBufferPointer;
							red = alpha;
							green = alpha;
							blue = alpha;
							break;
					}
					targetByteBufferPointer[0] = red;
					targetByteBufferPointer[1] = green;
					targetByteBufferPointer[2] = blue;
					targetByteBufferPointer[3] = alpha;
					targetByteBufferPointer += 4;
				}
				sourceByteBufferPointer += sourcePixelSizeInBytes;
			}
			sourceByteBufferPointer += rowPaddingInBytes;
		}
	}

	// Release the Java bitmap's byte buffer.
	// This allows it to be garbage collected.
	AndroidBitmap_unlockPixels(env, bitmap);
	return true;
}

void
JavaToNativeBridge::UpdateInputDevice(
	JNIEnv * env, jint coronaDeviceId, jint androidDeviceId, jint deviceTypeId, jstring permanentStringId,
	jstring productName, jstring displayName, jboolean canVibrate, jint playerNumber, jint connectionStateId)
{
	// Validate.
	if (NULL == fPlatform)
	{
		return;
	}

	// Fetch the input device from the device manager.
	Rtt::AndroidInputDeviceManager &deviceManager =
				(Rtt::AndroidInputDeviceManager&)fPlatform->GetDevice().GetInputDeviceManager();
	Rtt::AndroidInputDevice *devicePointer = deviceManager.GetByCoronaDeviceId((int)coronaDeviceId);

	// If the device was not found, then add it to the device manager.
	if (NULL == devicePointer)
	{
		// Fetch the device type.
		Rtt::InputDeviceType::ConversionResult result;
		result = Rtt::InputDeviceType::FromIntegerId(&fPlatform->GetAllocator(), (int)deviceTypeId);
		const Rtt::InputDeviceType *deviceTypePointer = result.GetDeviceType();
		if (NULL == deviceTypePointer)
		{
			return;
		}

		// Add the device and set its Corona device ID.
		devicePointer = (Rtt::AndroidInputDevice*)deviceManager.Add(*deviceTypePointer);
		if (NULL == devicePointer)
		{
			return;
		}
		devicePointer->SetCoronaDeviceId((int)coronaDeviceId);
	}

	// Update the device's information.
	jstringResult permanentStringIdConverter(env, permanentStringId);
	jstringResult productNameConverter(env, productName);
	jstringResult displayNameConverter(env, displayName);
	devicePointer->SetAndroidDeviceId((int)androidDeviceId);
	devicePointer->SetPermanentStringId(permanentStringIdConverter.getUTF8());
	devicePointer->SetProductName(productNameConverter.getUTF8());
	devicePointer->SetDisplayName(displayNameConverter.getUTF8());
	devicePointer->SetCanVibrate(canVibrate);
	devicePointer->SetPlayerNumber((int)playerNumber);
	devicePointer->SetConnectionState(Rtt::InputDeviceConnectionState::FromIntegerId((int)connectionStateId));
}

void
JavaToNativeBridge::ClearInputDeviceAxes(int coronaDeviceId)
{
	// Validate.
	if (NULL == fPlatform)
	{
		return;
	}

	// Fetch the input device from the device manager.
	Rtt::AndroidInputDeviceManager &deviceManager =
				(Rtt::AndroidInputDeviceManager&)fPlatform->GetDevice().GetInputDeviceManager();
	Rtt::AndroidInputDevice *devicePointer = deviceManager.GetByCoronaDeviceId((int)coronaDeviceId);
	if (NULL == devicePointer)
	{
		return;
	}

	// Remove all axis inputs from the device.
	devicePointer->RemoveAllAxes();
}

void
JavaToNativeBridge::AddInputDeviceAxis(
	int coronaDeviceId, int axisTypeId, float minValue, float maxValue, float accuracy, bool isAbsolute)
{
	// Validate.
	if (NULL == fPlatform)
	{
		return;
	}

	// Fetch the input device from the device manager.
	Rtt::AndroidInputDeviceManager &deviceManager =
				(Rtt::AndroidInputDeviceManager&)fPlatform->GetDevice().GetInputDeviceManager();
	Rtt::AndroidInputDevice *devicePointer = deviceManager.GetByCoronaDeviceId((int)coronaDeviceId);
	if (NULL == devicePointer)
	{
		return;
	}

	// Fetch the axis type.
	Rtt::InputAxisType::ConversionResult result;
	result = Rtt::InputAxisType::FromIntegerId(&fPlatform->GetAllocator(), axisTypeId);
	const Rtt::InputAxisType *axisTypePointer = result.GetAxisType();
	if (NULL == axisTypePointer)
	{
		return;
	}
	
	// Add the given axis information to the input device.
	Rtt::PlatformInputAxis *axisPointer = devicePointer->AddAxis();	
	if (NULL == axisPointer)
	{
		return;
	}
	axisPointer->SetType(*axisTypePointer);
	axisPointer->SetMinValue(minValue);
	axisPointer->SetMaxValue(maxValue);
	axisPointer->SetAccuracy(accuracy);
	axisPointer->SetIsAbsolute(isAbsolute);
}

void
JavaToNativeBridge::InputDeviceStatusEvent(
	int coronaDeviceId, bool hasConnectionStatusChanged, bool wasReconfigured)
{
	// Validate.
	if (NULL == fRuntime || NULL == fPlatform || fNativeToJavaBridge == NULL)
	{
		return;
	}

	// Fetch the changes made to the input device.
	// This updates the information stored in the InputDeviceManager.
	fNativeToJavaBridge->FetchInputDevice(coronaDeviceId);

	// Fetch the input device object this event is associated with.
	Rtt::AndroidInputDeviceManager &deviceManager =
				(Rtt::AndroidInputDeviceManager&)fPlatform->GetDevice().GetInputDeviceManager();
	Rtt::AndroidInputDevice *devicePointer = deviceManager.GetByCoronaDeviceId((int)coronaDeviceId);
	if (NULL == devicePointer)
	{
		return;
	}

	// Raise the event.
	Rtt::InputDeviceStatusEvent event(devicePointer, hasConnectionStatusChanged, wasReconfigured);
	fRuntime->DispatchEvent(event);
}

void
JavaToNativeBridge::UseDefaultLuaErrorHandler()
{
	Rtt::Lua::SetErrorHandler(NULL);
}

static int
JavaLuaErrorHandler(lua_State* L)
{
	return NativeToJavaBridge::InvokeLuaErrorHandler(L);
}

void
JavaToNativeBridge::UseJavaLuaErrorHandler()
{
	Rtt::Lua::SetErrorHandler(JavaLuaErrorHandler);
}

void 
JavaToNativeBridge::Render()
{
	if ( fView == NULL )
		return;

	// Re-load resources such as textures and audio data back into memory, if not done already.
	ReloadResources();

	// Render to the display.
	fView->Render();
}

void
JavaToNativeBridge::Pause()
{
	NativeTrace trace( "JavaToNativeBridge::Pause" );
	if ( fRuntime == NULL || fNativeToJavaBridge == NULL)
		return;

	// Suspend the Corona runtime.
	fRuntime->Suspend();
	fNativeToJavaBridge->OnRuntimeSuspended();

	// Free up memory by deleting loaded resources such as bitmaps and audio data. This will help prevent this
	// app from being terminated by the operating system if there is not enough memory to load the next activity.
	UnloadResources();
}

void 
JavaToNativeBridge::Resume()
{
	NativeTrace trace( "JavaToNativeBridge::Resume" );
	if ( fRuntime == NULL || fNativeToJavaBridge == NULL)
		return;
	
	// Fetch all available input devices in case some have been connected/disconnected since the being suspended.
	fNativeToJavaBridge->FetchAllInputDevices();

	// Resume the Corona runtime.
	fRuntime->Resume();
	fNativeToJavaBridge->OnRuntimeResumed();
}

void 
JavaToNativeBridge::DispatchEventInLua()
{
	NativeTrace trace( "JavaToNativeBridge::DispatchEventInLua" );
	if ( fRuntime == NULL )
		return;
	
	Corona::Lua::RuntimeDispatchEvent(fRuntime->VMContext().L(), -1);
}

void
JavaToNativeBridge::ApplicationOpenEvent()
{
	NativeTrace trace( "JavaToNativeBridge::ApplicationOpenEvent" );
	if ( fRuntime == NULL )
		return;
	
	Rtt::AndroidSystemOpenEvent e(fNativeToJavaBridge);
	fRuntime->DispatchEvent(e);
}

void
JavaToNativeBridge::TapEvent(int x, int y, int count)
{
	NativeTrace trace( "JavaToNativeBridge::TapEvent" );
	if ( fRuntime == NULL )
		return;
	
	Rtt::TapEvent e( Rtt_FloatToReal( x ), Rtt_FloatToReal( y ), count );
	
	fRuntime->DispatchEvent( e );
}

// TODO: Remove this function since there is an identical one in Rtt::TouchEvent
static Rtt::TouchEvent::Phase
phaseForType( int touchType )
{
	Rtt::TouchEvent::Phase result;
	
	switch( touchType )
	{
	case 0:
		result = Rtt::TouchEvent::kBegan;
		break;
	case 1:
		result = Rtt::TouchEvent::kMoved;
		break;
	case 2:
		result = Rtt::TouchEvent::kStationary;
		break;
	case 3:
		result = Rtt::TouchEvent::kEnded;
		break;
	case 4:
	default:
		result = Rtt::TouchEvent::kCancelled;
		break;
	}
	
	return result;
}

/// Converts a Java SystemClock uptime timestamp to Corona absolute time.
/// @param time Timestsamp provided by the Java SystemClock.uptimeMillis() method.
/// @return Returns a Corona absolute time timestamp compatible with Rtt::Runtime.GetElapsedMS().
static double
absoluteTimeFromSystemClockUptime(Rtt::Runtime *runtimePointer, long time, NativeToJavaBridge* bridge)
{
	if (!runtimePointer)
	{
		return 0;
	}

	// Get the current time from both time systems. These are considered equivalent.
	long currentUptime = bridge->GetUptimeInMilliseconds();
	double currentAbsoluteTime = runtimePointer->GetElapsedMS();

	// Calculate the delta time between the given timestamp and current time.
	double deltaTimeInMilliseconds = (double)(currentUptime - time);

	// Apply delta time to the current absolute time and return it.
	return currentAbsoluteTime - deltaTimeInMilliseconds;
}

// TODO: call Rtt::TouchEvent::phaseForType when above function is removed
void 
JavaToNativeBridge::TouchEvent(int x, int y, int xStart, int yStart, int touchType, long timestamp, int touchId, float pressure)
{
// 	NativeTrace trace( "JavaToNativeBridge::TouchEvent" );

	if ( fRuntime == NULL || fNativeToJavaBridge == NULL)
		return;

	Rtt::TouchEvent e( Rtt_FloatToReal( x ), Rtt_FloatToReal( y ), xStart, yStart, phaseForType(touchType), Rtt_FloatToReal( pressure ) );
	e.SetId( (void*)touchId );
	e.SetTime( absoluteTimeFromSystemClockUptime(fRuntime, timestamp, fNativeToJavaBridge) );
	
	fRuntime->DispatchEvent( e );
}

void
JavaToNativeBridge::MouseEvent(
	int x, int y, int scrollX, int scrollY, long timestamp,
	bool isPrimaryButtonDown, bool isSecondaryButtonDown, bool isMiddleButtonDown)
{
	// Validate.
	if ( NULL == fRuntime || fNativeToJavaBridge == NULL)
	{
		return;
	}

	// Convert x/y scroll deltas from pixels to Corona content coordinates.
	Rtt_Real contentScrollX = Rtt_RealMul(Rtt_IntToReal(scrollX), fRuntime->GetDisplay().GetSxUpright());
	Rtt_Real contentScrollY = Rtt_RealMul(Rtt_IntToReal(scrollY), fRuntime->GetDisplay().GetSyUpright());

	// Dispatch a "mouse" event to Lua.
	Rtt::MouseEvent event(
			Rtt::MouseEvent::kGeneric,
			Rtt_IntToReal(x), Rtt_IntToReal(y),
			contentScrollX, contentScrollY, 0,
			isPrimaryButtonDown, isSecondaryButtonDown, isMiddleButtonDown,
			false, false, false, false); // TODO: get modifier keys
	event.SetTime(absoluteTimeFromSystemClockUptime(fRuntime, timestamp, fNativeToJavaBridge));
	
	fRuntime->DispatchEvent(event);
}

bool 
JavaToNativeBridge::KeyEvent(
	int coronaDeviceId, int phase, const char *keyName, int keyCode,
	bool isShiftDown, bool isAltDown, bool isCtrlDown, bool isCommandDown)
{
	// Validate.
	if (NULL == fRuntime || NULL == fPlatform || fNativeToJavaBridge == NULL)
	{
		return false;
	}

	// Fetch the device that this input event came from.
	Rtt::AndroidInputDeviceManager &deviceManager =
				(Rtt::AndroidInputDeviceManager&)fPlatform->GetDevice().GetInputDeviceManager();
	Rtt::PlatformInputDevice *devicePointer = deviceManager.GetByCoronaDeviceId(coronaDeviceId);
	if (NULL == devicePointer)
	{
		fNativeToJavaBridge->FetchInputDevice(coronaDeviceId);
		devicePointer = deviceManager.GetByCoronaDeviceId(coronaDeviceId);
	}

	// Send the key event.
	Rtt::KeyEvent event(
			devicePointer, (Rtt::KeyEvent::Phase)phase, keyName, keyCode,
			isShiftDown, isAltDown, isCtrlDown, isCommandDown);
	fRuntime->DispatchEvent( event );
	return event.GetResult();
}

void
JavaToNativeBridge::AxisEvent(int coronaDeviceId, int axisIndex, float rawValue)
{
	// Validate.
	if (NULL == fRuntime || NULL == fPlatform || fNativeToJavaBridge == NULL)
	{
		return;
	}

	// Fetch the device that this input event came from.
	Rtt::AndroidInputDeviceManager &deviceManager =
				(Rtt::AndroidInputDeviceManager&)fPlatform->GetDevice().GetInputDeviceManager();
	Rtt::PlatformInputDevice *devicePointer = deviceManager.GetByCoronaDeviceId(coronaDeviceId);
	if (NULL == devicePointer)
	{
		fNativeToJavaBridge->FetchInputDevice(coronaDeviceId);
		devicePointer = deviceManager.GetByCoronaDeviceId(coronaDeviceId);
		if (NULL == devicePointer)
		{
			return;
		}
	}

	// Fetch this event's axis information.
	Rtt::PlatformInputAxis *axisPointer = devicePointer->GetAxes().GetByIndex(axisIndex);
	if (NULL == axisPointer)
	{
		return;
	}

	// Send the axis event.
	Rtt::AxisEvent event(devicePointer, axisPointer, rawValue);
	fRuntime->DispatchEvent(event);
}

void 
JavaToNativeBridge::AccelerometerEvent(double rawX, double rawY, double rawZ, double deltaTime)
{
	if ( NULL == fRuntime )
	{
		return;
	}
	
	const double kFilteringFactor = 0.1;
	
	double rawAccel[3];
	rawAccel[0] = rawX;
	rawAccel[1] = rawY;
	rawAccel[2] = rawZ;
	
	double x = fGravityAccel[0];
	double y = fGravityAccel[1];
	double z = fGravityAccel[2];
	
	// Use a basic low-pass filter to keep only the gravity component of each axis.
	x = (rawX * kFilteringFactor) + (x * (1.0 - kFilteringFactor));
	y = (rawY * kFilteringFactor) + (y * (1.0 - kFilteringFactor));
	z = (rawZ * kFilteringFactor) + (z * (1.0 - kFilteringFactor));
	fGravityAccel[0] = x;
	fGravityAccel[1] = y;
	fGravityAccel[2] = z;
	
	// Subtract the low-pass value from the current value to get a simplified high-pass filter
	x = rawX - x;
	y = rawY - y;
	z = rawZ - z;
	fInstantAccel[0] = x;
	fInstantAccel[1] = y;
	fInstantAccel[2] = z;
	
	
	// Compute the magnitude of the current acceleration 
	// and if above a given threshold, it's a shake
	bool isShake = false;
	const float kShakeAccelSq = 4.0;
	float accelSq = x*x + y*y + z*z;
	if ( accelSq >= kShakeAccelSq )
	{
		const Rtt_AbsoluteTime kMinShakeInterval = 0.5;
		
		Rtt_AbsoluteTime t = Rtt_GetAbsoluteTime();
		isShake = ( t > fPreviousShakeTime + kMinShakeInterval );
		if ( isShake )
		{
			fPreviousShakeTime = t;
		}
	}
	
	Rtt::AccelerometerEvent event( fGravityAccel, fInstantAccel, rawAccel, isShake, deltaTime );
	fRuntime->DispatchEvent( event );
}

void
JavaToNativeBridge::GyroscopeEvent(double x, double y, double z, double deltaTime)
{
	if ( NULL == fRuntime )
	{
		return;
	}
	
	Rtt::GyroscopeEvent event(x, y, z, deltaTime);
	fRuntime->DispatchEvent(event);
}

void 
JavaToNativeBridge::LocationEvent(double lat, double lon, double altitude, double accuracy, double speed, double bearing, double time)
{
	if ( NULL == fRuntime )
	{
		return;
	}
	
	if ( time < 1.0 ) {
		Rtt::HeadingEvent headingEvent( -1.0f, bearing );
		fRuntime->DispatchEvent( headingEvent );
	}
	else {
		Rtt::LocationEvent event( lat, lon, altitude, accuracy, speed, bearing, time );
		fRuntime->DispatchEvent( event );
	}
}

void 
JavaToNativeBridge::OrientationChanged( int newOrientation, int oldOrientation )
{
	NativeTrace trace( "JavaToNativeBridge::OrientationChanged" );
	
	if (( NULL == fRuntime ) || ( NULL == fPlatform ))
	{
		return;
	}
	
	((Rtt::AndroidDevice&)fPlatform->GetDevice()).SetOrientation( (Rtt::DeviceOrientation::Type)newOrientation );
	Rtt::OrientationEvent e( (Rtt::DeviceOrientation::Type)newOrientation, (Rtt::DeviceOrientation::Type)oldOrientation );
	fRuntime->DispatchEvent( e );
}

void 
JavaToNativeBridge::ResizeEvent()
{
	NativeTrace trace( "JavaToNativeBridge::ResizeEvent" );
	
	if ( NULL == fRuntime )
	{
		return;
	}
	
	Rtt::ResizeEvent event;
	fRuntime->DispatchEvent( event );
}

void
JavaToNativeBridge::AlertCallback( int which, bool cancelled )
{
	if (fNativeToJavaBridge != NULL)
	{
		fNativeToJavaBridge->AlertCallback(which, cancelled);
	}
}

void
JavaToNativeBridge::SoundEndCallback( long id )
{
	if (fNativeToJavaBridge != NULL)
	{
		fNativeToJavaBridge->SoundEndCallback((uintptr_t)id);
	}
}

void
JavaToNativeBridge::VideoEndCallback( long id )
{
	if (fNativeToJavaBridge != NULL)
	{
		fNativeToJavaBridge->VideoEndCallback((uintptr_t)id);
	}
}

void 
JavaToNativeBridge::RecordCallback( long id, int status )
{
	if (fNativeToJavaBridge != NULL)
	{
		fNativeToJavaBridge->RecordCallback((uintptr_t)id, status);
	}
}

void
JavaToNativeBridge::SetAudioRecorderState( long id, bool isRecording )
{
	if (fPlatform && fRuntime)
	{
		Rtt::AndroidAudioRecorder *audioRecorderPointer = (Rtt::AndroidAudioRecorder*) (uintptr_t)id;
		if (audioRecorderPointer)
		{
			audioRecorderPointer->SetRunningState( isRecording );
		}
	}
}

void
JavaToNativeBridge::TextEvent( int id, bool hasFocus, bool isDone )
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidTextFieldObject *textField = (Rtt::AndroidTextFieldObject *)fPlatform->GetNativeDisplayObjectById(id);
	if (!textField)
	{
		return;
	}
	
	// Send the event.
	Rtt::UserInputEvent::Phase phase = Rtt::UserInputEvent::kEnded;
	if ( hasFocus )
	{
		phase = Rtt::UserInputEvent::kBegan;
	}
	else if ( isDone )
	{
		phase = Rtt::UserInputEvent::kSubmitted;
	}
	Rtt::UserInputEvent e( phase );
	textField->DispatchEventWithTarget( e );
}

void
JavaToNativeBridge::TextEditingEvent( JNIEnv *env, int id, int startPos, int numDeleted, jstring newCharacters, jstring oldString, jstring newString )
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidTextFieldObject *textField = (Rtt::AndroidTextFieldObject *)fPlatform->GetNativeDisplayObjectById(id);
	if (!textField)
	{
		return;
	}
	
	// Send the event.
	// Add 1 to the start position to convert the zero-based index to a one-based index that is compatible with Lua.
	jstringResult newchars( env, newCharacters );
	jstringResult oldstr( env, oldString );
	jstringResult newstr( env, newString );
	Rtt::UserInputEvent e( startPos+1, numDeleted, newchars.getUTF8(), oldstr.getUTF8(), newstr.getUTF8() );
	textField->DispatchEventWithTarget( e );
}

void
JavaToNativeBridge::MultitouchEventBegin()
{
	fMultitouchEventCount = 0;
}

void
JavaToNativeBridge::MultitouchEventAdd(
	JNIEnv * env, int xLast, int yLast, int xStart, int yStart, int phaseType, long timestamp, int touchId, float pressure )
{
	// Do not add another touch event to the multitouch buffer if the maximum has been reached.
	if (fMultitouchEventCount >= kMaxMultitouchPointerEvents)
	{
		return;
	}

	// Create the touch event.
	Rtt::TouchEvent event(xLast, yLast, xStart, yStart, (Rtt::TouchEvent::Phase)phaseType, Rtt_FloatToReal( pressure ) );
	if (touchId > 0)
	{
		event.SetId((void*)touchId);
	}
	event.SetTime(absoluteTimeFromSystemClockUptime(fRuntime, timestamp, fNativeToJavaBridge));

	// Add the touch event to the buffer.
	fMultitouchEventBuffer[fMultitouchEventCount] = event;
	fMultitouchEventCount++;
}

void
JavaToNativeBridge::MultitouchEventEnd()
{
	if (fRuntime && (fMultitouchEventCount > 0))
	{
		Rtt::MultitouchEvent event(fMultitouchEventBuffer, fMultitouchEventCount);
		fRuntime->DispatchEvent(event);
	}
}

void 
JavaToNativeBridge::WebViewShouldLoadUrl( JNIEnv * env, int id, jstring urlJ, int sourceType )
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidWebViewObject *view = (Rtt::AndroidWebViewObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	// Raise the event.
	jstringResult url(env, urlJ);
	url.setNotLocal();
	if (view->IsPopup())
	{
		// This web view belongs to a web popup.
		// Make sure that the popup is still referencing the web view in case it just closed it.
		Rtt::AndroidWebPopup *popup = (Rtt::AndroidWebPopup*)(fPlatform->GetWebPopup());
		if (popup && (popup->GetWebViewId() == view->GetId()))
		{
			// If the Lua listener returns false, then close the popup.
			// Also, the "event.type" property is not supported by web popup.
			bool wasCloseRequested = !(popup->ShouldLoadUrl(url.getUTF8()));
			if (wasCloseRequested)
			{
				popup->Close();
			}
		}
	}
	else
	{
		// This web view is a display object.
		Rtt::UrlRequestEvent e(url.getUTF8(), (Rtt::UrlRequestEvent::Type)sourceType);
		view->DispatchEventWithTarget(e);
	}
}

void 
JavaToNativeBridge::WebViewFinishedLoadUrl( JNIEnv * env, int id, jstring urlJ )
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidWebViewObject *view = (Rtt::AndroidWebViewObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	//TODO: Raise an event to notify Lua that the page has finished loading.
	jstringResult url(env, urlJ);
	url.setNotLocal();
	Rtt::UrlRequestEvent e( url.getUTF8(), Rtt::UrlRequestEvent::kLoaded );
	view->DispatchEventWithTarget( e );
}

void 
JavaToNativeBridge::WebViewDidFailLoadUrl( JNIEnv * env, int id, jstring urlJ, jstring msgJ, int code )
{
	// Fetch the display object by ID.
	Rtt::AndroidWebViewObject *view = (Rtt::AndroidWebViewObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	// Raise the event.
	jstringResult url(env, urlJ);
	url.setNotLocal();
	jstringResult message(env, msgJ);
	message.setNotLocal();
	if (view->IsPopup())
	{
		// This web view belongs to a web popup.
		// Make sure that the popup is still referencing the web view in case it just closed it.
		Rtt::AndroidWebPopup *popup = (Rtt::AndroidWebPopup*)(fPlatform->GetWebPopup());
		if (popup && (popup->GetWebViewId() == view->GetId()))
		{
			// If the Lua listener returns false, then close the popup.
			bool wasCloseRequested = !(popup->DidFailLoadUrl(url.getUTF8(), message.getUTF8(), code));
			if (wasCloseRequested)
			{
				popup->Close();
			}
		}
	}
	else
	{
		// The web view is a display object.
		Rtt::UrlRequestEvent e(url.getUTF8(), message.getUTF8(), code);
		view->DispatchEventWithTarget(e);
	}
}

void 
JavaToNativeBridge::WebViewHistoryUpdated( JNIEnv * env, int id, jboolean canGoBack, jboolean canGoForward )
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidWebViewObject *view = (Rtt::AndroidWebViewObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	// Update the web view's navigation history.
	view->SetCanGoBack(canGoBack);
	view->SetCanGoForward(canGoForward);
}

void
JavaToNativeBridge::WebViewClosed( JNIEnv * env, int id )
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidWebViewObject *view = (Rtt::AndroidWebViewObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	// If the web view belongs to a web popup object, then close it.
	if (view->IsPopup())
	{
		// Make sure that the popup is still referencing the given web view.
		Rtt::AndroidWebPopup *popup = (Rtt::AndroidWebPopup*)(fPlatform->GetWebPopup());
		if (popup && (popup->GetWebViewId() == view->GetId()))
		{
			popup->Close();
		}
	}
}

void
JavaToNativeBridge::AdsRequestEvent( bool isError )
{
}

void
JavaToNativeBridge::ImagePickerEvent( JNIEnv *env, jstring selectedImageFileName )
{
	if (fPlatform && fRuntime && env)
	{
		Rtt::AndroidImageProvider *imageProviderPointer = (Rtt::AndroidImageProvider*)(fPlatform->GetImageProvider(fRuntime->VMContext().LuaState()));
		if (imageProviderPointer)
		{
			jstringResult selectedImageFileNameResult( env, selectedImageFileName );
			imageProviderPointer->CloseWithResult(selectedImageFileNameResult.getUTF8());
		}
	}
}

void
JavaToNativeBridge::AbortShowingImageProvider()
{
	if (fPlatform && fRuntime)
	{
		Rtt::AndroidImageProvider *imageProviderPointer = (Rtt::AndroidImageProvider*)(fPlatform->GetImageProvider(fRuntime->VMContext().LuaState()));
		if (imageProviderPointer)
		{
			imageProviderPointer->Abort();
		}
	}
}

void
JavaToNativeBridge::VideoPickerEvent( JNIEnv *env, jstring selectedVideoFileName, jint duration, jlong size )
{
	if (fPlatform && fRuntime && env)
	{
		Rtt::AndroidVideoProvider *videoProviderPointer = (Rtt::AndroidVideoProvider*)(fPlatform->GetVideoProvider(fRuntime->VMContext().LuaState()));
		if (videoProviderPointer)
		{
			jstringResult selectedVideoFileNameResult( env, selectedVideoFileName );
			videoProviderPointer->CloseWithResult(selectedVideoFileNameResult.getUTF8(), duration, size);
		}
	}
}

void
JavaToNativeBridge::AbortShowingVideoProvider()
{
	if (fPlatform && fRuntime)
	{
		Rtt::AndroidVideoProvider *videoProviderPointer = (Rtt::AndroidVideoProvider*)(fPlatform->GetVideoProvider(fRuntime->VMContext().LuaState()));
		if (videoProviderPointer)
		{
			videoProviderPointer->Abort();
		}
	}
}

void
JavaToNativeBridge::MemoryWarningEvent()
{
	if (fRuntime)
	{
		Rtt::MemoryWarningEvent event;
		fRuntime->DispatchEvent(event);
		fRuntime->Collect();
	}
}

void
JavaToNativeBridge::PopupClosedEvent( JNIEnv *env, jstring popupName, jboolean wasCanceled )
{
	if (fNativeToJavaBridge != NULL)
	{
		jstringResult popupNameJavaString(env, popupName);
		fNativeToJavaBridge->RaisePopupClosedEvent(popupNameJavaString.getUTF8(), wasCanceled);
	}
}

void
JavaToNativeBridge::StoreTransactionEvent(
	JNIEnv *env, jint state, jint errorType, jstring errorMessage, jstring productId, jstring signature,
	jstring receipt, jstring transactionId, jstring transactionTime,
	jstring originalReceipt, jstring originalTransactionId, jstring originalTransactionTime)
{
	// Validate.
	if (!fRuntime || !fPlatform)
	{
		return;
	}
	
	// Fetch the store interface.
	Rtt::PlatformStoreProvider *storeProviderPointer = fPlatform->GetStoreProvider(fRuntime->VMContext().LuaState());
	if (!storeProviderPointer)
	{
		return;
	}
	Rtt::PlatformStore *storePointer = storeProviderPointer->GetActiveStore();
	if (!storePointer)
	{
		return;
	}
	
	// Ignore the event if a Lua listener has not been set up.
	if (storePointer->GetTransactionNotifier().HasListener() == false)
	{
		return;
	}
	
	// Get the Java string objects for the given Java references.
	jstringResult errorMessageJavaString(env, errorMessage);
	jstringResult productIdJavaString(env, productId);
	jstringResult signatureJavaString(env, signature);
	jstringResult receiptJavaString(env, receipt);
	jstringResult transactionIdJavaString(env, transactionId);
	jstringResult transactionTimeJavaString(env, transactionTime);
	jstringResult originalReceiptJavaString(env, originalReceipt);
	jstringResult originalTransactionIdJavaString(env, originalTransactionId);
	jstringResult originalTransactionTimeJavaString(env, originalTransactionTime);
	
	// Create a store transaction object. Will be deleted by the Lua garbage collector.
	Rtt_Allocator &allocator = fPlatform->GetAllocator();
	Rtt::AndroidStoreTransaction *transactionPointer = Rtt_NEW(&allocator, Rtt::AndroidStoreTransaction(&allocator));
	transactionPointer->SetState((Rtt::PlatformStoreTransaction::State)state);
	transactionPointer->SetErrorType((Rtt::PlatformStoreTransaction::ErrorType)errorType);
	transactionPointer->SetErrorString(errorMessageJavaString.getUTF8());
	transactionPointer->SetProductIdentifier(productIdJavaString.getUTF8());
	transactionPointer->SetSignature(signatureJavaString.getUTF8());
	transactionPointer->SetReceipt(receiptJavaString.getUTF8());
	transactionPointer->SetIdentifier(transactionIdJavaString.getUTF8());
	transactionPointer->SetDate(transactionTimeJavaString.getUTF8());
	transactionPointer->SetOriginalReceipt(originalReceiptJavaString.getUTF8());
	transactionPointer->SetOriginalIdentifier(originalTransactionIdJavaString.getUTF8());
	transactionPointer->SetOriginalDate(originalTransactionTimeJavaString.getUTF8());
	
	// Raise the store transaction event.
	// The event object will be automatically deleted by the dispatcher.
	Rtt::StoreTransactionEvent *eventPointer = Rtt_NEW(&allocator, Rtt::StoreTransactionEvent(transactionPointer));
	storePointer->GetTransactionNotifier().ScheduleDispatch(eventPointer);
}

void
JavaToNativeBridge::MapAddressReceivedEvent(
	JNIEnv *env, jstring street, jstring streetDetails, jstring city, jstring cityDetails,
	jstring region, jstring regionDetails, jstring postalCode, jstring country, jstring countryCode)
{
	// Validate.
	if (!fRuntime)
	{
		return;
	}
	
	// Get the Java string objects for the given Java references.
	jstringResult streetJavaString(env, street);
	jstringResult streetDetailsJavaString(env, streetDetails);
	jstringResult cityJavaString(env, city);
	jstringResult cityDetailsJavaString(env, cityDetails);
	jstringResult regionJavaString(env, region);
	jstringResult regionDetailsJavaString(env, regionDetails);
	jstringResult postalCodeJavaString(env, postalCode);
	jstringResult countryJavaString(env, country);
	jstringResult countryCodeJavaString(env, countryCode);

	// Raise the event.
	Rtt::MapAddressEvent event(
			streetJavaString.getUTF8(), streetDetailsJavaString.getUTF8(), cityJavaString.getUTF8(),
			cityDetailsJavaString.getUTF8(), regionJavaString.getUTF8(), regionDetailsJavaString.getUTF8(),
			postalCodeJavaString.getUTF8(), countryJavaString.getUTF8(), countryCodeJavaString.getUTF8());
	fRuntime->DispatchEvent(event);
}

void
JavaToNativeBridge::MapAddressRequestFailedEvent(JNIEnv *env, jstring errorMessage)
{
	if (fRuntime)
	{
		jstringResult errorMessageJavaString(env, errorMessage);
		int errorCode = 0;
		Rtt::MapAddressEvent event(errorMessageJavaString.getUTF8(), errorCode);
		fRuntime->DispatchEvent(event);
	}
}

void 
JavaToNativeBridge::MapRequestLocationFailedEvent(JNIEnv *env, jint listenerId, jstring errorMessage, jstring originalRequest)
{
	lua_State* L = fRuntime->VMContext().L();
	if (L)
	{
		jstringResult errorMessageJavaString(env, errorMessage);
		jstringResult originalRequestJavaString(env, originalRequest);

		// LuaResource check to see if the listener is a table or a function
		Rtt::LuaResource* resource = Rtt_NEW( Rtt::LuaContext::GetAllocator( L ),
										Rtt::LuaResource( Rtt::LuaContext::GetContext( L )->LuaState(), LUA_NOREF ) );
		resource->SetRef(listenerId);
		Rtt::MapLocationEvent e(errorMessageJavaString.getUTF8(), 0, originalRequestJavaString.getUTF8(), Rtt::MapLocationEvent::kRequestType);
		resource->DispatchEvent(e);
		luaL_unref(L, LUA_REGISTRYINDEX, listenerId);
	}
}

void 
JavaToNativeBridge::MapRequestLocationEvent(JNIEnv *env, jint listenerId, jdouble latitude, jdouble longitude, jstring originalRequest)
{
	lua_State* L = fRuntime->VMContext().L();
	if (L)
	{
		jstringResult originalRequestJavaString(env, originalRequest);

		// LuaResource check to see if the listener is a table or a function
		Rtt::LuaResource* resource = Rtt_NEW( Rtt::LuaContext::GetAllocator( L ),
										Rtt::LuaResource( Rtt::LuaContext::GetContext( L )->LuaState(), LUA_NOREF ) );
		resource->SetRef(listenerId);
		Rtt::MapLocationEvent e(latitude, longitude, originalRequestJavaString.getUTF8());
		resource->DispatchEvent(e);
		luaL_unref(L, LUA_REGISTRYINDEX, listenerId);
	}
}

void
JavaToNativeBridge::MapMarkerEvent(jint markerId, jint listenerId, jdouble latitude, jdouble longitude)
{
	lua_State* L = fRuntime->VMContext().L();
	if (L)
	{
		// LuaResource check to see if the listener is a table or a function
		Rtt::LuaResource* resource = Rtt_NEW( Rtt::LuaContext::GetAllocator( L ),
										Rtt::LuaResource( Rtt::LuaContext::GetContext( L )->LuaState(), LUA_NOREF ) );
		resource->SetRef(listenerId);
		Rtt::MapMarkerEvent e( markerId, latitude, longitude );
		resource->DispatchEvent(e);
	}
}

void
JavaToNativeBridge::MapTappedEvent(jint id, jdouble latitude, jdouble longitude)
{
	Rtt::AndroidMapViewObject *view = (Rtt::AndroidMapViewObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	// Raise the event.
	Rtt::MapLocationEvent e(latitude, longitude);
	view->DispatchEventWithTarget(e);
}

void
JavaToNativeBridge::VideoViewPreparedEvent(jint id)
{
// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidVideoObject *view = (Rtt::AndroidVideoObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	Rtt::VideoEvent e( Rtt::VideoEvent::kReady );
	view->DispatchEventWithTarget( e );
}

void
JavaToNativeBridge::VideoViewEndedEvent(jint id)
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}
	
	// Fetch the display object by ID.
	Rtt::AndroidVideoObject *view = (Rtt::AndroidVideoObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}
	
	Rtt::VideoEvent e( Rtt::VideoEvent::kEnded );
	view->DispatchEventWithTarget( e );
}

void
JavaToNativeBridge::VideoViewFailedEvent(jint id)
{
	// Validate.
	if (!fPlatform)
	{
		return;
	}

	// Fetch the display object by ID.
	Rtt::AndroidVideoObject *view = (Rtt::AndroidVideoObject*)(fPlatform->GetNativeDisplayObjectById(id));
	if (!view)
	{
		return;
	}

	Rtt::VideoEvent e( Rtt::VideoEvent::kFailed );
	view->DispatchEventWithTarget( e );
}
const char*
JavaToNativeBridge::GetBuildId()
{
	if(fRuntime)
		return fRuntime->GetBuildId();
	return NULL;
}

// ----------------------------------------------------------------------------


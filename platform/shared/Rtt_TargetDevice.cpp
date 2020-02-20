//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"


#include "Core/Rtt_String.h"
#include "Rtt_LuaFile.h"
#include "Rtt_String.h"
#ifdef Rtt_AUTHORING_SIMULATOR
#include "CoronaLua.h"
#endif
#include <string.h>
#include <stdlib.h>

#ifndef PATH_MAX
#define PATH_MAX	4096
#endif

#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// The following finalizer class and static member variable will automatically delete this class' data on app exit.
class StaticTargetDeviceFinalizer
{
	public:
		StaticTargetDeviceFinalizer() {}
		~StaticTargetDeviceFinalizer()
		{
			TargetDevice::ReleaseAllSkins();
		}
};
static StaticTargetDeviceFinalizer sTargetDeviceFinalizer;


#if defined( Rtt_WIN_ENV )
const char *kDefaultSkinName = "Samsung Galaxy S5";
#else
const char *kDefaultSkinName = "iPhone 5";
#endif
    
TargetDevice::SkinSpec **TargetDevice::fSkins = NULL;
int TargetDevice::fSkinCount = 0;
TargetDevice::Skin TargetDevice::fDefaultSkinID = TargetDevice::kUnknownSkin;

#ifdef Rtt_AUTHORING_SIMULATOR
static int compar_SkinSpec(const void *item1, const void *item2)
{
    TargetDevice::SkinSpec *skin1 = *(TargetDevice::SkinSpec **) item1;
    TargetDevice::SkinSpec *skin2 = *(TargetDevice::SkinSpec **) item2;
    const char *borderless = "borderless-";
    const size_t borderlessLen = strlen(borderless);
    
    // printf("Comparing %s and %s\n", skin1->GetName(), skin2->GetName());
    
    Rtt_ASSERT( skin1 != NULL );
    Rtt_ASSERT( skin2 != NULL );
    
    // Sort first by device type and then by name within that, then by device width, then by height
    // with the exception that device types of "borderless-*" sort to the end

    if (strncmp(skin1->GetDeviceType(), borderless, borderlessLen) != 0 &&
        strncmp(skin2->GetDeviceType(), borderless, borderlessLen) == 0)
    {
        return -1;
    }

    if (strncmp(skin1->GetDeviceType(), borderless, borderlessLen) == 0 &&
        strncmp(skin2->GetDeviceType(), borderless, borderlessLen) != 0)
    {
        return 1;
    }

    int devTypeComp = strcmp(skin1->GetDeviceType(), skin2->GetDeviceType());
    
    if (devTypeComp == 0)
    {
        int skinNameComp = strcmp(skin1->GetName(), skin2->GetName());

        if (skinNameComp == 0)
        {
            int skinWidthComp = skin1->GetWidth() - skin2->GetWidth();

            if (skinWidthComp == 0)
            {
                return skin1->GetHeight() - skin2->GetHeight();
            }
            else
            {
                return skinWidthComp;
            }
        }
        else
        {
            return skinNameComp;
        }
    }
    else
    {
        return devTypeComp;
    }
}
#endif // Rtt_AUTHORING_SIMULATOR

bool
TargetDevice::Initialize( char **skinFiles, const int skinFileCount )
{
#ifdef Rtt_AUTHORING_SIMULATOR
	ReleaseAllSkins();
    if ((fSkins = (SkinSpec **) calloc(sizeof(SkinSpec*), skinFileCount)) == NULL)
    {
        Rtt_TRACE_SIM(("ERROR: Cannot allocate memory for skins in TargetDevice::Initialize()\n"));
        
        return false;
    }
    
    fSkinCount = 0;
    for (int i = 0; i < skinFileCount; i++ )
    {
        int status = 0;
        char *skinName = NULL;
        char *skinDevice = NULL;
        int skinWidth = 0;
        int skinHeight = 0;
        lua_State *L = CoronaLuaNew( kCoronaLuaFlagNone );
        status = CoronaLuaDoFile( L, skinFiles[i], 0, false );
        
        if ( 0 == status )
        {
            lua_getglobal( L, "simulator" );
            
            if ( lua_istable( L, -1 ) )
            {
                lua_getfield( L, -1, "windowTitleBarName" );
                skinName = (char *) luaL_optstring( L, -1, "Untitled Skin" );
                lua_pop( L, 1 );

                lua_getfield( L, -1, "device" );
                skinDevice = (char *) luaL_optstring( L, -1, "Untitled Skin" );
                lua_pop( L, 1 );

                lua_getfield( L, -1, "screenWidth" );
                skinWidth = luaL_optint( L, -1, 0 );
                lua_pop( L, 1 );

                lua_getfield( L, -1, "screenHeight" );
                skinHeight = luaL_optint( L, -1, 0 );
                lua_pop( L, 1 );
            }

            fSkins[fSkinCount] = new SkinSpec(skinName, skinFiles[i], skinDevice, skinWidth, skinHeight);
            ++fSkinCount;
        }
        else
        {
            CoronaLuaError(L, "invalid Lua in skin file '%s'", skinFiles[i]);
        }

        CoronaLuaDelete( L );
    }

    qsort(fSkins, fSkinCount, sizeof(TargetDevice::SkinSpec *), compar_SkinSpec);
    
	for (int i = 0; i < fSkinCount; i++)
    {
		// Rtt_TRACE_SIM(("TargetDevice::Initialize: skin: %d: %s = %s\n", i, fSkins[i]->GetDeviceType(), fSkins[i]->GetName()));
        
        // Remember the index of the platform's default skin in case we want a default later
        if (strcmp(fSkins[i]->GetName(), kDefaultSkinName) == 0)
        {
            fDefaultSkinID = (Skin) i;
            break;
        }
    }
#endif // Rtt_AUTHORING_SIMULATOR
    
    return true;
}

void
TargetDevice::ReleaseAllSkins()
{
#ifdef Rtt_AUTHORING_SIMULATOR
	// Do not continue if a skin collection has not been allocated.
	if (!fSkins)
	{
		return;
	}

	// First delete all of the SkinSpec objects in the collection.
	for (int index = 0; index < fSkinCount; index++)
	{
		TargetDevice::SkinSpec *skinPointer = fSkins[index];
		if (skinPointer)
		{
			delete skinPointer;
		}
	}

	// Delete the skin collection.
	free(fSkins);

	// Reset the skin collection's static variables.
	fSkins = NULL;
	fSkinCount = 0;
	fDefaultSkinID = TargetDevice::kUnknownSkin;
#endif
}

static const char kAndroidPlatformString[] = "Android";
static const char kIOSPlatformString[] = "iOS";
static const char kKindlePlatformString[] = "Amazon/Kindle";
static const char kNookPlatformString[] = "Nook";
static const char kWebPlatformString[] = "HTML5";
static const char kLinuxPlatformString[] = "LINUX";
static const char kWin32PlatformString[] = "Win32";
static const char kOSXPlatformString[] = "OSX";
static const char kWinPhoneSilverlightPlatformString[] = "WinPhoneSilverlight";
static const char kTVOSPlatformString[] = "tvOS";

const char*
TargetDevice::StringForPlatform( TargetDevice::Platform platform )
{
	const char *result = kIOSPlatformString;

	switch ( platform )
	{
		case kIPhonePlatform:
			result = kIOSPlatformString;
			break;
		case kAndroidPlatform:
			result = kAndroidPlatformString;
			break;
		case kKindlePlatform:
			result = kKindlePlatformString;
			break;
		case kNookPlatform:
			result = kNookPlatformString;
			break;
		case kWebPlatform:
			result = kWebPlatformString;
			break;
		case kLinuxPlatform:
			result = kLinuxPlatformString;
			break;
		case kWin32Platform:
			result = kWin32PlatformString;
			break;
		case kOSXPlatform:
			result = kOSXPlatformString;
			break;
		case kWinPhoneSilverlightPlatform:
			result = kWinPhoneSilverlightPlatformString;
			break;
		case kTVOSPlatform:
			result = kTVOSPlatformString;
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

TargetDevice::Platform
TargetDevice::PlatformForString( const char *str )
{
	TargetDevice::Platform result = kUnknownPlatform;

	if ( str )
	{
		if ( 0 == Rtt_StringCompareNoCase( str, kAndroidPlatformString ) )
		{
			result = kAndroidPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kKindlePlatformString ) )
		{
			result = kKindlePlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kNookPlatformString ) )
		{
			result = kNookPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kWebPlatformString ) )
		{
			result = kWebPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kLinuxPlatformString ) )
		{
			result = kLinuxPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kIOSPlatformString ) )
		{
			result = kIPhonePlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kWin32PlatformString ) )
		{
			result = kWin32Platform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kOSXPlatformString ) )
		{
			result = kOSXPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kWinPhoneSilverlightPlatformString ) )
		{
			result = kWinPhoneSilverlightPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kTVOSPlatformString ) )
		{
			result = kTVOSPlatform;
		}
	}

	return result;
}

static const char kAndroidPlatformTag[] = "android";
static const char kIOSPlatformTag[] = "ios";
static const char kKindlePlatformTag[] = "kindle";
static const char kNookPlatformTag[] = "nook";
static const char kWebPlatformTag[] = "html5";
static const char kLinuxPlatformTag[] = "linux";
static const char kWin32PlatformTag[] = "win32";
static const char kOSXPlatformTag1[] = "osx";
static const char kOSXPlatformTag2[] = "macos";
static const char kWinPhoneSilverlightPlatformTag[] = "winphonesilverlight";
static const char kTVOSPlatformTag[] = "tvos";

const char*
TargetDevice::TagForPlatform( TargetDevice::Platform platform )
{
	const char *result = kIOSPlatformTag;

	switch ( platform )
	{
		case kIPhonePlatform:
			result = kIOSPlatformTag;
			break;
		case kAndroidPlatform:
			result = kAndroidPlatformTag;
			break;
		case kKindlePlatform:
			result = kKindlePlatformTag;
			break;
		case kNookPlatform:
			result = kNookPlatformTag;
			break;
		case kWebPlatform:
			result = kWebPlatformTag;
			break;
		case kLinuxPlatform:
			result = kLinuxPlatformTag;
			break;
		case kWin32Platform:
			result = kWin32PlatformTag;
			break;
		case kOSXPlatform:
			result = kOSXPlatformTag2;
			break;
		case kWinPhoneSilverlightPlatform:
			result = kWinPhoneSilverlightPlatformTag;
			break;
		case kTVOSPlatform:
			result = kTVOSPlatformTag;
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}

TargetDevice::Platform
TargetDevice::PlatformForTag( const char *str )
{
	TargetDevice::Platform result = kUnknownPlatform;

	if ( str )
	{
		if ( 0 == Rtt_StringCompareNoCase( str, kAndroidPlatformTag ) )
		{
			result = kAndroidPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kKindlePlatformTag ) )
		{
			result = kKindlePlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kNookPlatformTag ) )
		{
			result = kNookPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kWebPlatformTag ) )
		{
			result = kWebPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kLinuxPlatformTag ) )
		{
			result = kLinuxPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kIOSPlatformTag ) )
		{
			result = kIPhonePlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kWin32PlatformTag ) )
		{
			result = kWin32Platform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kOSXPlatformTag1 ) ||
				  0 == Rtt_StringCompareNoCase( str, kOSXPlatformTag2 ) )
		{
			result = kOSXPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kWinPhoneSilverlightPlatformTag ) )
		{
			result = kWinPhoneSilverlightPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kTVOSPlatformTag ) )
		{
			result = kTVOSPlatform;
		}
	}

	return result;
}

// TODO: Load these values from JSON file
TargetDevice::Version
TargetDevice::VersionForPlatform( Platform platform )
{
	Version result = kUnknownVersion;

	switch ( platform )
	{
		case kAndroidPlatform:
		case kKindlePlatform:
		case kNookPlatform:
			result = kAndroidOS4_0_3;
			break;
		case kOSXPlatform:
		case kIPhonePlatform:
		case kTVOSPlatform: // As of Xcode 7.1, it seems like iOS/tvOS versions are in sync
			result = kIPhoneOS9_0;
			break;
		default:
			break;
	}

	return result;
}

const char *
TargetDevice::LuaObjectFileFromSkin( int skinID )
{
    if (skinID >= fSkinCount)
    {
        return NULL;
    }
    
    if (skinID < 0)
    {
        skinID = fDefaultSkinID;
    }
    
    return fSkins[skinID]->GetPath();
}

const char *
TargetDevice::SkinSpec::GenerateLabel( const char *path )
{
    // Find the filename part at the end of the path (sans extension)

    static char buf[PATH_MAX];
    char *retVal = NULL;

#if defined( Rtt_WIN_ENV )
    const char *lastSlash = strrchr(path, '\\');
#else
    const char *lastSlash = strrchr(path, '/');
#endif

	Rtt_ASSERT( lastSlash != NULL ); // we require full paths

    if (lastSlash != NULL)
    {
#if defined( Rtt_WIN_ENV )
        strncpy_s(buf, PATH_MAX, lastSlash+1, strlen(lastSlash+1));
#else
        strncpy(buf, lastSlash+1, PATH_MAX);
#endif

        char *lastPeriod = strrchr(buf, '.');

		Rtt_ASSERT( lastPeriod != NULL ); // we require extensions (i.e. ".lua")

        if (lastPeriod != NULL)
        {
            *lastPeriod = '\0';

            retVal = buf;
        }
    }

    return retVal;
}

const char *
TargetDevice::NameForSkin( int skinID )
{
    // Value of "windowTitleBarName"
    // This is used to iterate through all the skins
    if (skinID >= fSkinCount)
    {
        return NULL;
    }

    if (skinID < 0)
    {
        skinID = fDefaultSkinID;
    }
    
    return fSkins[skinID]->GetName();
}

const int
TargetDevice::WidthForSkin( int skinID )
{
    if (skinID < 0 || skinID >= fSkinCount)
    {
        skinID = fDefaultSkinID;
    }

    return fSkins[skinID]->GetWidth();
}

const int
TargetDevice::HeightForSkin( int skinID )
{
    if (skinID < 0 || skinID >= fSkinCount)
    {
        skinID = fDefaultSkinID;
    }

    return fSkins[skinID]->GetHeight();
}
    
const char *
TargetDevice::DeviceTypeForSkin( int skinID )
{
    if (skinID < 0 || skinID >= fSkinCount)
    {
        skinID = fDefaultSkinID;
    }

    return fSkins[skinID]->GetDeviceType();
}

TargetDevice::Skin
TargetDevice::SkinForLabel( const char* skinLabel )
{
    int result = kUnknownSkin;

	if (skinLabel != NULL)
	{
		for (int i = 0; i < fSkinCount; i++)
		{
#if defined( Rtt_WIN_ENV )
			if (_stricmp(fSkins[i]->GetLabel(), skinLabel) == 0)
#else
			if (strcasecmp(fSkins[i]->GetLabel(), skinLabel) == 0)
#endif
			{
				result = i;
				break;
			}
		}
	}

    if (result == kUnknownSkin)
    {
        Rtt_TRACE_SIM(("Warning: unknown skin label '%s'\n", skinLabel));
        
        result = fDefaultSkinID;
    }
    
	return (TargetDevice::Skin) result;
}

const char *
TargetDevice::LabelForSkin( int skinID )
{
    if (skinID < 0 || skinID >= fSkinCount)
    {
        skinID = fDefaultSkinID;
    }

    return fSkins[skinID]->GetLabel();
}

#if defined(Rtt_LINUX_ENV) && !defined(_WIN32)
static char* strcasestrForLinux(const char* s1, const char* s2)
{
	return strcasecmp(s1, s2) == 0 ? (char*) s1 : NULL;
}
#endif

TargetDevice::Platform
TargetDevice::PlatformForDeviceType( const char *typeName )
{
	Platform platformType = kUnknownPlatform;

	if (Rtt_StringIsEmpty(typeName) == false)
	{
#	ifdef Rtt_WIN_ENV
		// Convert the given string to lower case.
		char stringBuffer[64];
		const size_t stringBufferSize = sizeof(stringBuffer);
		size_t typeNameLength = strlen(typeName);
		if (typeNameLength < stringBufferSize)
		{
			strncpy_s(stringBuffer, stringBufferSize, typeName, _TRUNCATE);
			_strlwr_s(stringBuffer, stringBufferSize);
			typeName = stringBuffer;
		}

		// Fetch a callback to a substring search function.
		// Note: Windows does not have a case insensitive version of this function,
		//       which is why we convert the given string to lower-case up above first.
		const char*(*substringSearchCallback)(const char*, const char*) = &strstr;
#elif defined(Rtt_LINUX_ENV) && !defined(_WIN32)
		char*(*substringSearchCallback)(const char*, const char*) = &strcasestrForLinux;
#else
		// Fetch a callback to a case insensitive substring search function.
		char*(*substringSearchCallback)(const char*, const char*) = &strcasestr;
#endif
		if (substringSearchCallback(typeName, "android"))
		{
			platformType = kAndroidPlatform;
		}
		else if (substringSearchCallback(typeName, "ios"))
		{
			platformType = kIPhonePlatform;
		}
		else if (substringSearchCallback(typeName, "tvos"))
		{
			platformType = kTVOSPlatform;
		}
		else if (substringSearchCallback(typeName, "kindle"))
		{
			platformType = kKindlePlatform;
		}
		else if (substringSearchCallback(typeName, "macos"))
		{
			platformType = kOSXPlatform;
		}
		else if (substringSearchCallback(typeName, "nook"))
		{
			platformType = kNookPlatform;
		}
		else if (substringSearchCallback(typeName, "win32"))
		{
			platformType = kWin32Platform;
		}
		else if (substringSearchCallback(typeName, "winphone"))
		{
			platformType = kWinPhoneSilverlightPlatform;
		}
	}

	return platformType;
}

// Given skin, returns true if the skin's device runs using platform's OS; false otherwise.
bool
TargetDevice::IsSkinForPlatform( int skinID, Platform platform )
{
    if (skinID < 0 || skinID >= fSkinCount)
    {
        skinID = fDefaultSkinID;
    }
	return (PlatformForDeviceType(fSkins[skinID]->GetDeviceType()) == platform);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

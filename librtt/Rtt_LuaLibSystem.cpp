//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibSystem.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_TextureFactory.h"
#include "Rtt_LuaContext.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"

#include "Rtt_Runtime.h"
#include "Rtt_GPU.h"
#include "Rtt_GPUStream.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_PlatformInAppStore.h"
#include "Rtt_PreferenceCollection.h"
#include "Core/Rtt_String.h"
#include "Input/Rtt_PlatformInputDeviceManager.h"
#include "Input/Rtt_PlatformInputDevice.h"
#include "Input/Rtt_InputDeviceCollection.h"
#include "Input/Rtt_ReadOnlyInputDeviceCollection.h"
#include "Renderer/Rtt_Geometry_Renderer.h"

#include <locale>
#include <locale.h>
#include <string.h>
#include <vector>

#include "Rtt_Lua.h"
#include "CoronaLibrary.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kNotificationMetatable[] = "notification";

#if defined(Rtt_NXS_ENV) 
extern "C"
{
	extern const char* sSaveMountPoint;
	extern const char* sTmpMountPoint;
	int is_mounted(const char* path);
}
#endif

static void PushStringOrNil( lua_State *L, const char *str )
{
    if ( str )
    {
        lua_pushstring( L, str );
    }
    else
    {
        lua_pushnil( L );
    }
}

int
LuaLibSystem::getInfo( lua_State *L )
{
    int result = 1;

    const char* key = lua_tostring( L, 1 );
    if ( ! key )
    {
        CoronaLuaError(L, "system.getInfo() takes a string as its first parameter" );
        return 0;
    }
    
    const MPlatform& platform = LuaContext::GetPlatform( L );
    MPlatformDevice& device = platform.GetDevice();
    if ( Rtt_StringCompare( key, "name" ) == 0 )
    {
        lua_pushstring( L, device.GetName() );
    }
    else if ( Rtt_StringCompare( key, "manufacturer" ) == 0 )
    {
        lua_pushstring( L, device.GetManufacturer() );
    }
    else if ( Rtt_StringCompare( key, "model" ) == 0 )
    {
        lua_pushstring( L, device.GetModel() );
    }
    else if ( Rtt_StringCompare( key, "deviceID" ) == 0 )
    {
        PushStringOrNil( L, device.GetUniqueIdentifier( MPlatformDevice::kDeviceIdentifier ) );
    }
    else if ( Rtt_StringCompare( key, "hardwareId" ) == 0 )
    {
        PushStringOrNil( L, device.GetUniqueIdentifier( MPlatformDevice::kHardwareIdentifier ) );
    }
    else if ( Rtt_StringCompare( key, "osId" ) == 0 )
    {
        PushStringOrNil( L, device.GetUniqueIdentifier( MPlatformDevice::kOSIdentifier ) );
    }
    else if ( Rtt_StringCompare( key, "macId" ) == 0 )
    {
        PushStringOrNil( L, device.GetUniqueIdentifier( MPlatformDevice::kMacIdentifier ) );
    }
    else if ( Rtt_StringCompare( key, "udid" ) == 0 )
    {
        PushStringOrNil( L, device.GetUniqueIdentifier( MPlatformDevice::kUdidIdentifier ) );
    }
    else if ( Rtt_StringCompare( key, "iosAdvertisingIdentifier" ) == 0 )
    {
        PushStringOrNil( L, device.GetUniqueIdentifier( MPlatformDevice::kIOSAdvertisingIdentifier ) );
    }
    else if ( Rtt_StringCompare( key, "iosIdentifierForVendor" ) == 0 )
    {
        PushStringOrNil( L, device.GetUniqueIdentifier( MPlatformDevice::kIOSIdentifierForVendor ) );
    }
    else if ( Rtt_StringCompare( key, "environment" ) == 0 )
    {
        MPlatformDevice::EnvironmentType t = device.GetEnvironment();

        const char *env = NULL;
        switch( t )
        {
            case MPlatformDevice::kSimulatorEnvironment:
                env = "simulator";
                break;
            case MPlatformDevice::kDeviceEnvironment:
                env = "device";
                break;
            case MPlatformDevice::kBrowserEnvironment:
                env = "browser";
                break;
            default:
                // Should never hit this case
                Rtt_ASSERT_NOT_REACHED();
                break;
        }

        Rtt_ASSERT( env );

		lua_pushstring( L, env );
	}
	else if ( Rtt_StringCompare( key, "platformName" ) == 0 )
	{
		lua_pushstring( L, device.GetPlatformName() );
	}
	else if ( Rtt_StringCompare( key, "platform" ) == 0 )
	{
		lua_pushstring( L, device.GetPlatform() );
	}
	else if ( Rtt_StringCompare( key, "platformVersion" ) == 0 )
	{
		lua_pushstring( L, device.GetPlatformVersion() );
	}
	else if ( Rtt_StringCompare( key, "graphicsPipelineVersion" ) == 0 )
	{
		lua_pushstring( L, "2.0" );
	}
	else if (Rtt_StringCompare( key, "graphicsBackend" ) == 0 )
	{
		Runtime *runtime = LuaContext::GetRuntime( L );
		lua_pushstring( L, runtime->GetBackend() );
	}
	else if ( Rtt_StringCompare( key, "architectureInfo" ) == 0 )
	{
		lua_pushstring( L, device.GetArchitectureInfo() );
	}
	else if ( Rtt_StringCompare( key, "textureMemoryUsed" ) == 0 )
	{
		TextureFactory& factory = LuaContext::GetRuntime( L )->GetDisplay().GetTextureFactory();
		lua_pushinteger( L, factory.GetTextureMemoryUsed() );
	}
	else if ( Rtt_StringCompare( key, "maxTextureSize" ) == 0 )
	{
		Runtime *runtime = LuaContext::GetRuntime( L );
		lua_pushinteger( L, runtime->GetDisplay().GetMaxTextureSize() );
	}
	
#ifdef OLD_GRAPHICS
    else if ( Rtt_StringCompare( key, "maxTextureUnits" ) == 0 )
    {
        lua_pushinteger( L, GPUStream::GetMaxTextureUnits() );
    }
#endif
    else if ( Rtt_StringCompare( key, "supportsScreenCapture" ) == 0 )
    {
#ifdef OLD_GRAPHICS
        bool supportsScreenCapture = GPU::CheckIfContextSupportsExtension( "GL_OES_framebuffer_object" )
            || GPU::CheckIfContextSupportsExtension( "GL_EXT_framebuffer_object" )
            || GPU::CheckIfContextSupportsExtension( "GL_ARB_framebuffer_object" );
#else
        Rtt_ASSERT_NOT_IMPLEMENTED();
        bool supportsScreenCapture = false;
#endif
        lua_pushboolean( L, supportsScreenCapture );
    }
    else if ( Rtt_StringCompare( key, "targetAppStore" ) == 0 )
    {
        Runtime *runtime = LuaContext::GetRuntime( L );
        PlatformStoreProvider *storeProvider = runtime->Platform().GetStoreProvider(runtime->VMContext().LuaState());
        const char *storeName = storeProvider ? storeProvider->GetTargetedStoreName() : NULL;
        if (!storeName)
        {
            storeName = "none";
        }
        lua_pushstring( L, storeName );
    }
    else if ( Rtt_StringCompare( key, "version" ) == 0 )
    {
        lua_pushstring( L,  Rtt_STRING_VERSION );
    }
    else if ( Rtt_StringCompare( key, "build" ) == 0 )
    {
        lua_pushstring( L,  Rtt_STRING_BUILD );
    }
    else if ( Rtt_StringCompare( key, "credits" ) == 0 )
    {
        lua_pushstring( L,  Rtt_STRING_CREDITS );
    }
    else if ( Rtt_StringCompare( key, "copyright" ) == 0 )
    {
        lua_pushstring( L, "Portions " Rtt_STRING_COPYRIGHT );
    }
    else if( ( Rtt_StringCompare( key, "GL_VENDOR" ) == 0 ) ||
                ( Rtt_StringCompare( key, "GL_RENDERER" ) == 0 ) ||
                ( Rtt_StringCompare( key, "GL_VERSION" ) == 0 ) ||
                ( Rtt_StringCompare( key, "GL_SHADING_LANGUAGE_VERSION" ) == 0 ) ||
                ( Rtt_StringCompare( key, "GL_EXTENSIONS" ) == 0 ) )
    {
        Runtime *runtime = LuaContext::GetRuntime( L );
        lua_pushstring( L, runtime->GetDisplay().GetGlString( key ) );
    }
    else if ( Rtt_StringCompare( key, "gpuSupportsHighPrecisionFragmentShaders" ) == 0 )
    {
        Runtime *runtime = LuaContext::GetRuntime( L );
        lua_pushboolean( L, runtime->GetDisplay().GetGpuSupportsHighPrecisionFragmentShaders() );
    }
	else if ( Rtt_StringCompare( key, "gpuSupportsScaledCaptures" ) == 0 )
	{
		Runtime *runtime = LuaContext::GetRuntime( L );
		bool canScale = false;
		lua_pushboolean( L, runtime->GetDisplay().HasFramebufferBlit( &canScale ) && canScale );
	}
    else if ( Rtt_StringCompare( key, "maxUniformVectorsCount" ) == 0 )
    {
        Runtime *runtime = LuaContext::GetRuntime( L );
        lua_pushnumber( L, runtime->GetDisplay().GetMaxUniformVectorsCount() );
    }
    else if ( Rtt_StringCompare( key, "maxVertexTextureUnits" ) == 0 )
    {
        Runtime *runtime = LuaContext::GetRuntime( L );
        size_t n = runtime->GetDisplay().GetMaxVertexTextureUnits();
        lua_pushnumber( L, n > 2U ? 2U : 0U ); // more than 2 requires new paint types
    }
    else if ( Rtt_StringCompare( key, "maxVertexAttributes" ) == 0 || Rtt_StringCompare( key, "instancingSupport" ) == 0 )
    {
        VertexAttributeSupport support;
        Runtime *runtime = LuaContext::GetRuntime( L );
        runtime->GetDisplay().GetVertexAttributes( support );
        if (Rtt_StringCompare( key, "maxVertexAttributes" ) == 0)
        {
            lua_pushnumber( L, support.maxCount );
        }
        else
        {
            if (!support.hasInstancing)
            {
                lua_pushboolean( L, 0 );
            }
            
            else
            {
                lua_createtable( L, 0, 2 );
                if (support.hasDivisors)
                {
                    lua_pushliteral( L, "multiInstance" );
                }
                else if (support.hasPerInstance)
                {
                    lua_pushliteral( L, "singleInstance" );
                }
                else
                {
                    lua_pushliteral( L, "none" );
                }
                lua_setfield( L, -2, "vertexReplication" );
                lua_pushboolean( L, NULL != support.suffix );
                lua_setfield( L, -2, "hasInstanceID" );
            }
        }
    }
    else
    {
        // This is a place where we can add system.getInfo() categories that return arbitrary types
        result = platform.PushSystemInfo( L, key );
    }
    
    return result;
}

    
static int
canOpenURL( lua_State *L )
{
    int result = -1;
    
    if (lua_isstring(L, 1))
    {
        const char* url = lua_tostring( L, 1 );
        if (url == NULL || *url == 0)
        {
            CoronaLuaWarning(L, "system.canOpenURL() called with empty URL");
        }
        else
        {
            result = LuaContext::GetPlatform( L ).CanOpenURL( url );
        }
    }
    else
    {
        CoronaLuaWarning(L, "system.canOpenURL() expects a string URL as argument #1 (got %s)",
                         lua_typename(L, lua_type(L, 1)));
    }
    
    if (result == -1)
    {
        lua_pushnil( L );
    }
    else
    {
        lua_pushboolean( L, result );
    }
        
    return 1;
}
    
    
static int
openURL( lua_State *L )
{
    bool result = false;
    
    if (lua_isstring(L, 1))
    {
        const char* url = lua_tostring( L, 1 );
        if (url == NULL || *url == 0)
        {
            CoronaLuaWarning(L, "system.openURL() called with empty URL");
        }
        else
        {
            result = LuaContext::GetPlatform( L ).OpenURL( url );
            
            if (! result)
            {
                CoronaLuaWarning(L, "system.openURL() failed to open URL: %s", url);
            }
        }
    }
    else
    {
        CoronaLuaWarning(L, "system.openURL() expects a string URL as argument #1 (got %s)",
                         lua_typename(L, lua_type(L, 1)));
    }
    
    lua_pushboolean( L, result );
    
    return 1;
}

static int
getTimer( lua_State *L )
{
    lua_pushnumber( L, LuaContext::GetRuntime( L )->GetElapsedMS() );
    return 1;
}

static int
vibrate( lua_State *L )
{
	LuaContext::GetPlatform( L ).GetDevice().Vibrate();
	return 0;
}

static int
setIdleTimer( lua_State *L )
{
    LuaContext::GetPlatform( L ).SetIdleTimer( lua_toboolean( L, 1 ) );
    return 0;
}

static int
getIdleTimer( lua_State *L )
{
    lua_pushboolean( L, LuaContext::GetPlatform( L ).GetIdleTimer() );
    return 1;
}

// system.getPreference( categoryName, keyName [,valueType] )
int
LuaLibSystem::getPreference( lua_State *L )
{
    const char* kApiName = "system.getPreference()";

    // Validate.
    if ( !L )
    {
        return 0;
    }

    // Fetch the preference category name argument.
    const char *categoryName = NULL;
    if ( lua_type( L, 1 ) == LUA_TSTRING )
    {
        categoryName = lua_tostring( L, 1 );
    }
    if ( Rtt_StringIsEmpty( categoryName ) )
    {
        CoronaLuaError( L, "%s - category name argument must be set to a non-empty string", kApiName );
        lua_pushnil( L );
        return 1;
    }

    // Fetch the preference key name argument.
    const char *keyName = NULL;
    if ( lua_type( L, 2 ) == LUA_TSTRING )
    {
        keyName = lua_tostring( L, 2 );
    }
    if ( Rtt_StringIsEmpty( keyName ) )
    {
        CoronaLuaError( L, "%s - key name argument must be set to a non-empty string", kApiName );
        lua_pushnil( L );
        return 1;
    }

    // Determine if the category name matches Corona's legacy "locale" or "ui" category types.
    // These preferences are read-only and only support strings.
    MPlatform::Category categoryType = MPlatform::kUnknownCategory;
    if ( 0 == strcmp( categoryName, Rtt::Preference::kCategoryNameLocale ) )
    {
        if ( 0 == strcmp( keyName, "identifier" ) )
        {
            categoryType = MPlatform::kLocaleIdentifier;
        }
        else if ( 0 == strcmp( keyName, "language" ) )
        {
            categoryType = MPlatform::kLocaleLanguage;
        }
        else if ( 0 == strcmp( keyName, "country" ) )
        {
            categoryType = MPlatform::kLocaleCountry;
        }
    }
    else if ( 0 == strcmp( categoryName, Rtt::Preference::kCategoryNameUI ) )
    {
        if ( 0 == strcmp( keyName, "language" ) )
        {
            categoryType = MPlatform::kUILanguage;
        }
    }

    // Fetch the optional value type argument.
    // This is the type that the stored preference value must be returned as.
    PreferenceValue::Type requestedValueType = PreferenceValue::kTypeString;
    {
        const int luaArgumentType = lua_type( L, 3 );
        if ( luaArgumentType == LUA_TSTRING )
        {
            const char* valueTypeName = lua_tostring( L, 3 );
            if ( !valueTypeName )
            {
                valueTypeName = "";
            }
            if ( Rtt_StringCompare( valueTypeName, "boolean" ) == 0 )
            {
                requestedValueType = PreferenceValue::kTypeBoolean;
            }
            else if ( Rtt_StringCompare(valueTypeName, "number") == 0 )
            {
                requestedValueType = PreferenceValue::kTypeFloatDouble;
            }
            else if ( Rtt_StringCompare(valueTypeName, "string") == 0 )
            {
                requestedValueType = PreferenceValue::kTypeString;
            }
            else
            {
                CoronaLuaError(
                        L, "%s argument 3 was set to unknown/unsupported value type name '%s'.",
                        kApiName, valueTypeName );
                lua_pushnil( L );
                return 1;
            }
        }
        else if ( ( luaArgumentType != LUA_TNONE ) && ( luaArgumentType != LUA_TNIL ) )
        {
            CoronaLuaError( L, "%s argument 3 must be set to a string", kApiName );
            lua_pushnil( L );
            return 1;
        }
    }

    // Attempt to read the given preference.
    Preference::ReadValueResult readPreferenceResult = Preference::ReadValueResult::FailedWith( NULL );
    if ( categoryType != MPlatform::kUnknownCategory )
    {
        Rtt::String value( LuaContext::GetAllocator( L ) );
        LuaContext::GetPlatform( L ).GetPreference( categoryType, &value );
        if ( value.IsEmpty() )
        {
            readPreferenceResult = Preference::ReadValueResult::kPreferenceNotFound;
        }
        else
        {
            readPreferenceResult = Preference::ReadValueResult::SucceededWith( PreferenceValue( value.GetString() ) );
        }
    }
    else
    {
        readPreferenceResult = LuaContext::GetPlatform( L ).GetPreference( categoryName, keyName );
    }

    // Return nil to Lua if we've failed to acquire the preference value.
    // Note: Only log an error message if we've failed to access preferences from storage.
    if ( readPreferenceResult.HasFailed() )
    {
        if ( readPreferenceResult.HadPreferenceNotFoundError() == false )
        {
            CoronaLuaError( L, "%s - %s", kApiName, readPreferenceResult.GetUtf8Message() );
        }
        lua_pushnil( L );
        return 1;
    }

    // If the preference was found, convert its value to match the requested type.
    // This guarantees that we'll return a Lua compatible value type.
    PreferenceValue preferenceValue = readPreferenceResult.GetValue();
    {
        ValueResult<PreferenceValue> conversionResult = ValueResult<PreferenceValue>::FailedWith( NULL );
        if (( PreferenceValue::kTypeString == requestedValueType ) &&
            ( preferenceValue.GetType() != PreferenceValue::kTypeString ))
        {
            // Special case for converting numeric value types to string.
            // We must always return this string in localized form to Lua using the C language's current locale.
            // The reason is because Lua's string conversion functions, such as tonumber(), are localized by default.
            PreferenceValue::StringResult stringResult = PreferenceValue::StringResult::FailedWith( NULL );
            const char* localeName = setlocale( LC_ALL, NULL );
            if ( Rtt_StringIsEmpty( localeName ) )
            {
                stringResult = preferenceValue.ToString();
            }
            else
            {
                stringResult = preferenceValue.ToStringWithLocale( std::locale( localeName ) );
            }
            if ( stringResult.HasSucceeded() )
            {
                conversionResult = ValueResult<PreferenceValue>::SucceededWith(
                        PreferenceValue( stringResult.GetValue() ) );
            }
            else
            {
                conversionResult = ValueResult<PreferenceValue>::FailedWith(
                        stringResult.GetUtf8MessageAsSharedPointer() );
            }
        }
        else
        {
            // Attempt to convert to the requested value type.
            conversionResult = preferenceValue.ToValueType( requestedValueType );
        }
        if ( conversionResult.HasFailed() )
        {
            CoronaLuaError( L, "%s - %s", kApiName, conversionResult.GetUtf8Message() );
            lua_pushnil( L );
            return 1;
        }
        preferenceValue = conversionResult.GetValue();
    }

    // Success! Return the preference value to Lua.
    switch ( preferenceValue.GetType() )
    {
        case PreferenceValue::kTypeBoolean:
        {
            lua_pushboolean( L, preferenceValue.ToBoolean().GetValue() ? 1 : 0 );
            break;
        }
        case PreferenceValue::kTypeFloatDouble:
        {
            lua_pushnumber( L, preferenceValue.ToFloatDouble().GetValue() );
            break;
        }
        default:
        {
            Rtt::SharedConstStdStringPtr sharedStringPointer = preferenceValue.ToString().GetValue();
            if ( sharedStringPointer.NotNull() )
            {
                lua_pushlstring( L, sharedStringPointer->c_str(), sharedStringPointer->length() );
            }
            else
            {
                lua_pushstring( L, "" );
            }
            break;
        }
    }
    return 1;
}

static bool
IsPreferenceCategoryReadOnly(const char* categoryName)
{
    if ( Rtt_StringIsEmpty( categoryName ) == false )
    {
        const char* kReadOnlyCategoryNames[] =
        {
            Rtt::Preference::kCategoryNameLocale,
            Rtt::Preference::kCategoryNameUI,
            NULL
        };
        for ( int index = 0; kReadOnlyCategoryNames[index] != NULL; index++ )
        {
            if ( Rtt_StringCompare( categoryName, kReadOnlyCategoryNames[index] ) == 0 )
            {
                return true;
            }
        }
    }
    return false;
}

// system.setPreference()
static int
setPreference( lua_State *L )
{
    if ( !L )
    {
        return 0;
    }

    const char kMessage[] =
            "system.setPreference() function is not supported. "
            "Use the system.setPreferences() function instead.";
    CoronaLuaError( L, kMessage );
    lua_pushboolean( L, 0 );
    return 1;
}

// system.setPreferences( categoryName, { key1=value1, key2=value2, ... } )
static int
setPreferences( lua_State *L )
{
    const char* kApiName = "system.setPreferences()";

    // Validate.
    if ( !L )
    {
        return 0;
    }

    // Fetch the preference category name argument.
    const char *categoryName = NULL;
    if ( lua_type( L, 1 ) == LUA_TSTRING )
    {
        categoryName = lua_tostring( L, 1 );
    }
    if ( Rtt_StringIsEmpty( categoryName ) )
    {
        CoronaLuaError( L, "%s - category name argument must be set to a non-empty string", kApiName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Do not continue if given a legacy category name such as "locale" or "ui". These are read-only.
    if ( IsPreferenceCategoryReadOnly( categoryName ) )
    {
        CoronaLuaError( L, "%s - Cannot write to category \"%s\". It is read-only.", kApiName, categoryName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Fetch the last argument's table of preference key-value pairs.
    PreferenceCollection preferenceCollection;
    if ( lua_type( L, 2 ) == LUA_TTABLE )
    {
        for ( lua_pushnil( L ); lua_next( L, 2 ) != 0; lua_pop( L, 1 ) )
        {
            // Fetch the next preference name in the Lua table.
            const char* keyName = NULL;
            if ( lua_type( L, -2 ) == LUA_TSTRING )
            {
                keyName = lua_tostring( L, -2 );
            }
            if ( Rtt_StringIsEmpty( keyName ) )
            {
                continue;
            }

            // Fetch the next preference value from the Lua table.
            ValueResult<PreferenceValue> valueResult = PreferenceValue::From( L, -1 );
            if ( valueResult.HasFailed() )
            {
                CoronaLuaWarning(
                        L, "%s - Cannot write to preference \"%s\". %s",
                        kApiName, keyName, valueResult.GetUtf8Message() );
                continue;
            }

            // Add the preference key-value pair to the collection. To be written to storage later.
            preferenceCollection.Add( keyName, valueResult.GetValue() );
        }
    }
    else
    {
        // Do not continue. The 2nd argument is invalid.
        CoronaLuaError( L, "%s argument 2 must be set to a preference table", kApiName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Do not continue if not given any valid preferences to write to storage.
    if ( preferenceCollection.GetCount() <= 0 )
    {
        CoronaLuaWarning( L, "%s was given an empty preference table", kApiName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Attempt to write the given preference(s) to stoarge.
    OperationResult writeResult = LuaContext::GetPlatform( L ).SetPreferences( categoryName, preferenceCollection );

    // Log an error if the operation failed.
    // Log a warning if the operation succeeded and has a message.
    if ( writeResult.HasFailed() )
    {
        CoronaLuaError( L, "%s - %s", kApiName, writeResult.GetUtf8Message() );
    }
    else if ( Rtt_StringIsEmpty( writeResult.GetUtf8Message() ) == false )
    {
        CoronaLuaWarning( L, "%s - %s", kApiName, writeResult.GetUtf8Message() );
    }

    // Return true to Lua if the operation was successful.
    lua_pushboolean( L, writeResult.HasSucceeded() ? 1 : 0 );
    return 1;
}

// system.deletePreference()
static int
deletePreference( lua_State *L )
{
    if ( !L )
    {
        return 0;
    }

    const char kMessage[] =
            "system.deletePreference() function is not supported. "
            "Use the system.deletePreferences() function instead.";
    CoronaLuaError( L, kMessage );
    lua_pushboolean( L, 0 );
    return 1;
}

// system.deletePreferences( categoryName, { "key1", "key2", ... } )
static int
deletePreferences( lua_State *L )
{
    const char* kApiName = "system.deletePreferences()";

    // Validate.
    if ( !L )
    {
        return 0;
    }

    // Fetch the preference category name argument.
    const char *categoryName = NULL;
    if ( lua_type( L, 1 ) == LUA_TSTRING )
    {
        categoryName = lua_tostring( L, 1 );
    }
    if ( Rtt_StringIsEmpty( categoryName ) )
    {
        CoronaLuaError( L, "%s - category name argument must be set to a non-empty string", kApiName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Do not continue if given a legacy category name such as "locale" or "ui". These are read-only.
    if ( IsPreferenceCategoryReadOnly( categoryName ) )
    {
        CoronaLuaError( L, "%s - Cannot delete from category \"%s\". It is read-only.", kApiName, categoryName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Fetch the last argument's array of preferences key names.
    std::vector<const char*> keyNameCollection;
    if ( lua_type( L, 2 ) == LUA_TTABLE )
    {
        size_t arrayLength = lua_objlen( L, 2 );
        if ( arrayLength > 0 )
        {
            keyNameCollection.reserve( arrayLength );
            for ( lua_pushnil( L ); lua_next( L, 2 ) != 0; lua_pop( L, 1 ) )
            {
                const char* keyName = NULL;
                if ( lua_type( L, -1 ) == LUA_TSTRING )
                {
                    keyName = lua_tostring( L, -1 );
                }
                if ( Rtt_StringIsEmpty( keyName ) )
                {
                    int index = 0;
                    if (lua_type( L, -2 ) == LUA_TNUMBER )
                    {
                        index = (int)lua_tointeger( L, -2 );
                    }
                    CoronaLuaWarning( L, "%s - preference key[%d] must be set to a non-empty string", kApiName, index );
                    continue;
                }
                keyNameCollection.push_back( keyName );
            }
        }
    }
    else
    {
        CoronaLuaError( L, "%s argument 2 must be set to an array of preference key strings", kApiName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Do not continue if not given any preference keys to delete.
    if ( keyNameCollection.empty() )
    {
        CoronaLuaWarning( L, "%s was given an empty preference name array", kApiName );
        lua_pushboolean( L, 0 );
        return 1;
    }

    // Attempt to delete the given preferences from storage.
    OperationResult deleteResult = LuaContext::GetPlatform( L ).DeletePreferences(
            categoryName, &keyNameCollection.front(), (int)keyNameCollection.size() );

    // Log an error if the operation failed.
    // Log a warning if the operation succeeded and has a message.
    if ( deleteResult.HasFailed() )
    {
        CoronaLuaError( L, "%s - %s", kApiName, deleteResult.GetUtf8Message() );
    }
    else if ( Rtt_StringIsEmpty( deleteResult.GetUtf8Message() ) == false )
    {
        CoronaLuaWarning( L, "%s - %s", kApiName, deleteResult.GetUtf8Message() );
    }

    // Return true to Lua if the operation was successful.
    lua_pushboolean( L, deleteResult.HasSucceeded() ? 1 : 0 );
    return 1;
}

static int
gcNotification( lua_State *L )
{
    void *notificationId = Lua::ToUserdata( L, -1 );

    const MPlatform& platform = LuaContext::GetPlatform( L );
    platform.ReleaseNotification( notificationId );

    return 0;
}

// local notification = system.scheduleNotification( secondsFromNow [, options] )
//
// local coordinatedUniversalTime = { day=, month=, year=, hour=, min=, sec= }
// local notification = system.scheduleNotification( coordinatedUniversalTime [, options] )
static int
scheduleNotification( lua_State *L )
{
    CoronaLuaWarning(L, "system.scheduleNotification: This function has been deprecated, use 'plugin.notifications' instead.");

    const MPlatform& platform = LuaContext::GetPlatform( L );

    void *notificationId = platform.CreateAndScheduleNotification( L, 1 );
    if ( notificationId )
    {
        Lua::PushUserdata( L, notificationId, kNotificationMetatable );
    }
    else
    {
        lua_pushnil( L );
    }

    return 1;
}

// system.cancelNotification( [notification] )
static int
cancelNotification( lua_State *L )
{
    CoronaLuaWarning(L, "system.cancelNotification: This function has been deprecated, use 'plugin.notifications' instead.");

    if ( lua_isuserdata( L, 1 ) || lua_isnoneornil( L, 1 ) )
    {
        const MPlatform& platform = LuaContext::GetPlatform( L );

        void *notificationId = lua_isnone( L, 1 ) ? NULL : Lua::CheckUserdata( L, 1, kNotificationMetatable );
        platform.CancelNotification( notificationId );
    }

    return 0;
}

// system.request( actionName [, options] )
static int
request( lua_State *L )
{
    // Do not continue if not given any arguments.
    if (lua_gettop(L) <= 0)
    {
        CoronaLuaError(L, "system.request() expects at least one parameter (action name)");
    }

    // Fetch the action name argument.
    const char *actionName = NULL;
    if (lua_type(L, 1) == LUA_TSTRING)
    {
        actionName = lua_tostring(L, 1);
    }
    else
    {
        CoronaLuaError(L, "system.request() expects a string as parameter #1 (got %s)",
                       lua_typename(L, lua_type(L, 1)));
    }

    // Fetch the index to the options table, if it exists.
    int optionsIndex = 0;
    if (lua_istable( L, 2 ))
    {
        optionsIndex = 2;
    }

    // Attempt to execute the requested operation.
    const MPlatform& platform = LuaContext::GetPlatform( L );
    bool result = platform.RequestSystem( L, actionName, optionsIndex );

    // Return true if the requested operation was accepted.
    lua_pushboolean( L, result ? 1 : 0 );
    return 1;
}

static int
getInputDevices( lua_State *L )
{
    // Fetch all "connected" input devices from the device manager.
    const MPlatform& platform = LuaContext::GetPlatform(L);
    InputDeviceCollection connectedDevices(&platform.GetAllocator());
    platform.GetDevice().GetInputDeviceManager().GetDevices().CopyConnectedDevicesTo(connectedDevices);

    // Return the input devices as a Lua table.
    lua_createtable(L, connectedDevices.GetCount(), 0);
    for (S32 index = 0; index < connectedDevices.GetCount(); index++)
    {
        PlatformInputDevice *devicePointer = connectedDevices.GetByIndex(index);
        if (devicePointer)
        {
            devicePointer->PushTo(L);
            lua_rawseti(L, -2, (int)index + 1);
        }
    }
    return 1;
}

static int
setAccelerometerInterval( lua_State *L )
{
    Rtt_TRACE_SIM( ( "WARNING: Accelerometer events are only available on the device.\n" ) );
    const int kMinFrequency = 10;
    const int kMaxFrequency = 100;
    int frequency = (int) lua_tointeger( L, 1 );
    if ( frequency < kMinFrequency )
    {
        CoronaLuaWarning(L, "system.setAccelerometerInterval() frequency of %d below minimum. Using minimum allowed frequency of %d instead", frequency, kMinFrequency );
        
        frequency = kMinFrequency;
    }

    if ( frequency > kMaxFrequency )
    {
        CoronaLuaWarning(L, "system.setAccelerometerInterval() frequency of %d above maximum. Using maximum allowed frequency of %d instead", frequency, kMaxFrequency );
        
        frequency = kMaxFrequency;
    }

    LuaContext::GetPlatform( L ).GetDevice().SetAccelerometerInterval( frequency );
    return 0;
}

static int
setGyroscopeInterval( lua_State *L )
{
    Rtt_TRACE_SIM( ( "WARNING: Gyroscope events are only available on the device.\n" ) );
    const int kMinFrequency = 10;
    const int kMaxFrequency = 100;
    int frequency = (int) lua_tointeger( L, 1 );
    if ( frequency < kMinFrequency )
    {
        CoronaLuaWarning(L, "system.setGyroscopeInterval() frequency of %d below minimum. Using minimum allowed frequency of %d instead", frequency, kMinFrequency );
        frequency = kMinFrequency;
    }
    
    if ( frequency > kMaxFrequency )
    {
        CoronaLuaWarning(L, "system.setGyroscopeInterval() frequency of %d above maximum. Using maximum allowed frequency of %d instead", frequency, kMaxFrequency );

        frequency = kMaxFrequency;
    }
    
    LuaContext::GetPlatform( L ).GetDevice().SetGyroscopeInterval( frequency );
    return 0;
}

static int
setLocationAccuracy( lua_State *L )
{
    Rtt_TRACE_SIM( ( "WARNING: Location updates are only available on the device.\n" ) );
    Real distance = luaL_toreal( L, 1 );
    if ( distance < Rtt_REAL_0 )
    {
        distance = Rtt_REAL_0;
    }

    LuaContext::GetPlatform( L ).GetDevice().SetLocationAccuracy( distance );
    return 0;
}

static int
setLocationThreshold( lua_State *L )
{
    Rtt_TRACE_SIM( ( "WARNING: Location updates are only available on the device.\n" ) );
    Real distance = luaL_toreal( L, 1 );
    if ( distance < Rtt_REAL_0 )
    {
        distance = Rtt_REAL_0;
    }

    LuaContext::GetPlatform( L ).GetDevice().SetLocationThreshold( distance );
    return 0;
}

static int
setTapDelay( lua_State *L )
{
    Real delay = luaL_toreal( L, 1 );
    if ( delay < Rtt_REAL_0 )
    {
        delay = Rtt_REAL_0;
    }

    LuaContext::GetPlatform( L ).SetTapDelay( delay );
    return 0;
}

static const char kDirs[MPlatform::kNumDirs + 1] = "01234567";

// Need default constructor for const use by C++ spec
LuaLibSystem::LuaLibSystem()
    : LuaProxyVTable()
{
}

const char*
LuaLibSystem::Directories()
{
    return kDirs;
}

/*
static void*
UserdataForDir( MPlatform::Directory dir )
{
    return const_cast< char* >( & kDirs[dir] );
}

MPlatform::Directory
LuaLibSystem::DirForUserdata( void* p )
{
    U32 offset = (const char*)p - kDirs;
    return offset < MPlatform::kNumDirs ? (MPlatform::Directory)offset : MPlatform::kResourceDir;
}
*/

int
LuaLibSystem::PathForTable( lua_State *L, int index, FileType& fileType )
{
    int result = 0;
    fileType = kUnknownFileType;

    if ( lua_type( L, index ) == LUA_TTABLE )
    {
        if ( index < 0 )
        {
            // Get positive indices
            index = lua_gettop( L ) + index + 1;
        }

        int oldTop = lua_gettop( L );

        lua_checkstack( L, 3 );

        lua_getfield( L, index, "filename" );
        if ( lua_isstring( L, -1 ) )
        {
            lua_getfield( L, index, "baseDir" );

            if ( lua_islightuserdata( L, -1 ) )
            {
                result = PathForFile( L, oldTop );
                if ( result > 0 )
                {
                    lua_replace( L, oldTop + 1 );

                    lua_getfield( L, index, "type" );
                    const char *value = lua_tostring( L, -1 );
                    if ( Rtt_StringCompareNoCase( value, "image" ) == 0 )
                    {
                        fileType = kImageFileType;
                    }
                }
            }
        }

        lua_settop( L, oldTop + result );
    }

    return result;
}

int
LuaLibSystem::PathForTable( lua_State *L )
{
    LuaLibSystem::FileType fileType;
    return LuaLibSystem::PathForTable( L, 1, fileType );
}

MPlatform::Directory
LuaLibSystem::ToDirectory( lua_State *L, int index )
{
    return ToDirectory( L, index, MPlatform::kResourceDir );
}

MPlatform::Directory
LuaLibSystem::ToDirectory( lua_State *L, int index, MPlatform::Directory defaultDir )
{
    MPlatform::Directory result = defaultDir;

    if ( lua_islightuserdata( L, index ) )
    {
         result = (MPlatform::Directory )EnumForUserdata( kDirs,
                                                            lua_touserdata( L, index ),
                                                            MPlatform::kNumDirs,
                                                            defaultDir );
    }

    return result;
}

void
LuaLibSystem::PushDirectory(lua_State *L, MPlatform::Directory directory)
{
    lua_pushlightuserdata( L, UserdataForEnum( kDirs, directory ) );
}

bool
LuaLibSystem::IsWritableDirectory( MPlatform::Directory dir )
{
#if defined(Rtt_NXS_ENV) 
	if (dir == MPlatform::kDocumentsDir && is_mounted(sSaveMountPoint) == false)
	{
		return false;
	}

	if (dir == MPlatform::kTmpDir && is_mounted(sTmpMountPoint) == false)
	{
		return false;
	}
#endif

    return ( ( dir == MPlatform::kDocumentsDir ) ||
             ( dir == MPlatform::kTmpDir ) ||
             ( dir == MPlatform::kApplicationSupportDir ) ||
             ( dir == MPlatform::kCachesDir ) );
}

int
LuaLibSystem::PathForFile( lua_State *L, int base )
{
    int result = 0;

    if ( base < 0 )
    {
        // Get positive indices
        base = lua_gettop( L ) + base + 1;
    }

    const char* filename = lua_tostring( L, base + 1 );

    // Either filename is NULL or its length is reasonable
    if ( ! filename || Rtt_VERIFY( strlen( filename ) < 128 ) )
    {
        const MPlatform& platform = LuaContext::GetPlatform( L );

        if ( ( lua_type( L, base + 2 ) != LUA_TLIGHTUSERDATA ) &&
            ( lua_type( L, base + 2 ) != LUA_TNONE ) &&
            ( lua_type( L, base + 2 ) != LUA_TNIL ) )
        {
            CoronaLuaWarning( L, "system.pathForFile: invalid base directory (expected constant got a %s)",  lua_typename( L, lua_type( L, base + 2 )));
        }
        
        MPlatform::Directory baseDir = (MPlatform::Directory )EnumForUserdata(
            kDirs,
            lua_touserdata( L, base + 2 ),
            MPlatform::kNumDirs,
            MPlatform::kResourceDir );
        
        bool testExistence = lua_type( L, base + 3 ) != LUA_TNONE && lua_toboolean( L, base + 3 );
        U32 flags = ( testExistence ? MPlatform::kTestFileExists : MPlatform::kDefaultPathFlags );
        String path( & platform.GetAllocator() );

        platform.PathForFile( filename, baseDir, flags, path );
        lua_pushstring( L, path.GetString() );
        result = 1;
    }

    return result;
}

int
LuaLibSystem::PathForFile( lua_State *L )
{
    return PathForFile( L, 0 );
}

/*
static int
FileExists( lua_State *L )
{
    bool result = false;

    const char* filename = lua_tostring( L, 1 );

    // Either filename is NULL or its length is reasonable
    if ( ! filename || Rtt_VERIFY( strlen( filename ) < 128 ) )
    {
        const MPlatform& platform = LuaContext::GetPlatform( L );

        MPlatform::Directory baseDir = (MPlatform::Directory )EnumForUserdata(
            kDirs,
            lua_touserdata( L, 2 ),
            MPlatform::kNumDirs,
            MPlatform::kResourceDir );
        Rtt_ASSERT( lua_type( L, 2 ) == LUA_TLIGHTUSERDATA || lua_type( L, 2 ) == LUA_TNONE );

        const char* basePath = platform.PathForFile( NULL, baseDir, MPlatform::kDefaultPathFlags );
        char* path = (char*)malloc( strlen(basePath) + strlen(filename) + sizeof( LUA_DIRSEP ) + 1 );
        sprintf( path, "%s" LUA_DIRSEP "%s", basePath, filename );

        FILE* f = fopen( path, "r" );
        if ( f )
        {
            result = ( NULL != path );
            fclose( f );
        }

        free( path );
    }

    lua_pushboolean( L, result );
    return 1;
}
*/

static const char kOrientationName[] = "orientation";
static const char kAccelerometerName[] = "accelerometer";
static const char kGyroscopeName[] = "gyroscope";
static const char kLocationName[] = "location";
static const char kHeadingName[] = "heading";
static const char kMultitouchName[] = "multitouch";
static const char kCollisionName[] = "collision";
static const char kPreCollisionName[] = "preCollision";
static const char kPostCollisionName[] = "postCollision";
static const char kParticleCollisionName[] = "particleCollision";
static const char kKeyName[] = "key";
static const char kInputDeviceStatusName[] = "inputDeviceStatus";
static const char kMouseName[] = "mouse";

static MPlatformDevice::EventType
EventTypeForName( const char *eventName )
{
    MPlatformDevice::EventType result = MPlatformDevice::kUnknownEvent;

    if ( strcmp( kOrientationName, eventName ) == 0 )
    {
        result = MPlatformDevice::kOrientationEvent;
    }
    else if ( strcmp( kAccelerometerName, eventName ) == 0 )
    {
        result = MPlatformDevice::kAccelerometerEvent;
    }
    else if ( strcmp( kGyroscopeName, eventName ) == 0 )
    {
        result = MPlatformDevice::kGyroscopeEvent;
    }
    else if ( strcmp( kLocationName, eventName ) == 0 )
    {
        result = MPlatformDevice::kLocationEvent;
    }
    else if ( strcmp( kHeadingName, eventName ) == 0 )
    {
        result = MPlatformDevice::kHeadingEvent;
    }
    else if ( strcmp( kMultitouchName, eventName ) == 0 )
    {
        result = MPlatformDevice::kMultitouchEvent;
    }
    else if ( strcmp( kCollisionName, eventName ) == 0 )
    {
        result = MPlatformDevice::kCollisionEvent;
    }
    else if ( strcmp( kPreCollisionName, eventName ) == 0 )
    {
        result = MPlatformDevice::kPreCollisionEvent;
    }
    else if ( strcmp( kPostCollisionName, eventName ) == 0 )
    {
        result = MPlatformDevice::kPostCollisionEvent;
    }
    else if ( strcmp( kParticleCollisionName, eventName ) == 0 )
    {
        result = MPlatformDevice::kParticleCollisionEvent;
    }
    else if ( strcmp( kKeyName, eventName ) == 0 )
    {
        result = MPlatformDevice::kKeyEvent;
    }
    else if ( strcmp( kInputDeviceStatusName, eventName ) == 0 )
    {
        result = MPlatformDevice::kInputDeviceStatusEvent;
    }
    else if ( strcmp( kMouseName, eventName ) == 0 )
    {
        result = MPlatformDevice::kMouseEvent;
    }
    
    return result;
}
    
static const char kControllerUserInteraction[] = "controllerUserInteraction";
    
static MPlatformDevice::ActivationType
ActivationTypeForName( const char* name )
{
    MPlatformDevice::ActivationType result = MPlatformDevice::kActivateUnknown;
    
    if ( strcmp( kControllerUserInteraction, name ) == 0 )
    {
        result = MPlatformDevice::kActivateControllerUserInteraction;
    }
    
    return result;
}

static PhysicsWorld::Properties
MaskForEvent( MPlatformDevice::EventType t )
{
    PhysicsWorld::Properties result = 0;
    switch( t )
    {
        case MPlatformDevice::kCollisionEvent:
            result = PhysicsWorld::kCollisionListenerExists;
            break;
        case MPlatformDevice::kPreCollisionEvent:
            result = PhysicsWorld::kPreCollisionListenerExists;
            break;
        case MPlatformDevice::kPostCollisionEvent:
            result = PhysicsWorld::kPostCollisionListenerExists;
            break;
        case MPlatformDevice::kParticleCollisionEvent:
            result = PhysicsWorld::kParticleCollisionListenerExists;
            break;
        default:
            break;
    }
    return result;
}

int
LuaLibSystem::BeginListener( lua_State *L )
{
    const char* eventName = lua_tostring( L, -1 );
    if ( eventName )
    {
        MPlatformDevice::EventType t = EventTypeForName( eventName );
        if ( t > MPlatformDevice::kUnknownEvent )
        {
            Runtime *runtime = LuaContext::GetRuntime( L );
#ifdef Rtt_PHYSICS
            PhysicsWorld::Properties mask = MaskForEvent( t );
            if ( mask > 0 )
            {
                runtime->GetPhysicsWorld().SetProperty( mask, true );
            }
            else
#endif
            {
                runtime->Platform().GetDevice().BeginNotifications( t );
            }
        }
    }

    return 0;
}

int
LuaLibSystem::EndListener( lua_State *L )
{
    const char* eventName = lua_tostring( L, -1 );
    if ( eventName )
    {
        MPlatformDevice::EventType t = EventTypeForName( eventName );
        if ( t > MPlatformDevice::kUnknownEvent )
        {
            Runtime *runtime = LuaContext::GetRuntime( L );
#ifdef Rtt_PHYSICS
            PhysicsWorld::Properties mask = MaskForEvent( t );
            if ( mask )
            {
                runtime->GetPhysicsWorld().SetProperty( mask, false );
            }
            else
#endif
            {
                runtime->Platform().GetDevice().EndNotifications( t );
            }
        }
    }

    return 0;
}

int
LuaLibSystem::HasEventSource( lua_State *L )
{
    bool hasEventSource = false;
    
    const char* eventName = lua_tostring( L, -1 );
    if ( eventName )
    {
        MPlatformDevice::EventType eventType = EventTypeForName( eventName );
        if ( eventType > MPlatformDevice::kUnknownEvent )
        {
            Runtime *runtime = LuaContext::GetRuntime( L );
            hasEventSource = runtime->Platform().GetDevice().HasEventSource(eventType);
        }
    }
    
    lua_pushboolean( L, hasEventSource );
    return 1;
}

int
LuaLibSystem::Activate( lua_State *L )
{
    bool activationRecognized = false;
        
    // system.activate( activationName(string) )
    const char* activationName = lua_tostring( L, -1 );
    if ( activationName ) {
        MPlatformDevice::ActivationType activationType = ActivationTypeForName( activationName );
        activationRecognized = LuaContext::GetRuntime( L )->Platform().GetDevice().Activate( activationType );
    }
    
    // system.activate used to pass to BeginListener. To provide backwards compatability, we
    // run BeginListener if the device did not claim support for the activation.
    if ( ! activationRecognized )
    {
        return BeginListener( L );
    }
    
    return 0;
}
    
int
LuaLibSystem::Deactivate( lua_State *L )
{
    bool activationRecognized = false;
    
    // system.deactivate( key(string) )
    const char* activationName = lua_tostring( L, -1 );
    if ( activationName ) {
        MPlatformDevice::ActivationType activationType = ActivationTypeForName( activationName );
        activationRecognized = LuaContext::GetRuntime( L )->Platform().GetDevice().Deactivate( activationType );
    }
    
    // system.deactivate used to pass to EndListener. To provide backwards compatibility, we
    // run EndListener if the device did not claim support for the deactivation.
    if ( ! activationRecognized )
    {
        return EndListener( L );
    }
    
    return 0;
}

const char*
LuaLibSystem::GetFilename( lua_State *L, int& index, MPlatform::Directory& baseDir )
{
    // Assign defaults
    const char *result = NULL;
    baseDir = MPlatform::kResourceDir;

    if ( lua_isstring( L, index ) )
    {
        result = lua_tostring( L, index++ );
        
        if ( lua_islightuserdata( L, index ) )
        {
            void* p = lua_touserdata( L, index );
            baseDir = (MPlatform::Directory)EnumForUserdata(
                 LuaLibSystem::Directories(),
                 p,
                 MPlatform::kNumDirs,
                 MPlatform::kResourceDir );
            ++index;
        }
    }

    return result;
}

void
LuaLibSystem::Initialize( lua_State *L )
{
    static const luaL_Reg kVTable[] =
    {
        { "__proxyindex", LuaProxy::__proxyindex },
        { "__proxynewindex", LuaProxy::__proxynewindex },
        { "__proxyregister", LuaProxy::__proxyregister },
        { "pathForFile", LuaLibSystem::PathForFile },
        { "pathForTable", LuaLibSystem::PathForTable }, // private
        { "beginListener", LuaLibSystem::BeginListener }, // private; use system.activate() publicly
        { "endListener", LuaLibSystem::EndListener }, // private; use system.activate() publicly
        { "hasEventSource", LuaLibSystem::HasEventSource }, // private
        { "getInfo", getInfo },
        { "getTimer", getTimer },
        { "openURL", openURL },
        { "canOpenURL", canOpenURL },
        { "vibrate", vibrate },
        { "setIdleTimer", setIdleTimer },
        { "getIdleTimer", getIdleTimer },
        { "getPreference", getPreference },
        { "setPreference", setPreference },
        { "setPreferences", setPreferences },
        { "deletePreference", deletePreference },
        { "deletePreferences", deletePreferences },
        { "scheduleNotification", scheduleNotification },
        { "cancelNotification", cancelNotification },
        { "request", request },

        // TODO: Move this into a Lua "device" library
        { "getInputDevices", getInputDevices },
        { "setAccelerometerInterval", setAccelerometerInterval },
        { "setGyroscopeInterval", setGyroscopeInterval },
        { "setLocationAccuracy", setLocationAccuracy },
        { "setLocationThreshold", setLocationThreshold },
        { "setTapDelay", setTapDelay },
        { "activate", LuaLibSystem::Activate }, // public use
        { "deactivate", LuaLibSystem::Deactivate }, // public use

        { NULL, NULL }
    };
    static const LuaLibSystem kProxyVTable;

    luaL_register( L, "system", kVTable );
    {
        LuaLibSystem::PushDirectory( L, MPlatform::kDocumentsDir );
        lua_setfield( L, -2, "DocumentsDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kTmpDir );
        lua_setfield(L, -2, "TemporaryDirectory");
        LuaLibSystem::PushDirectory( L, MPlatform::kResourceDir );
        lua_setfield( L, -2, "ResourceDirectory" );
#ifdef Rtt_AUTHORING_SIMULATOR
        LuaLibSystem::PushDirectory( L, MPlatform::kSystemResourceDir );
        lua_setfield( L, -2, "SystemResourceDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kProjectResourceDir );
        lua_setfield( L, -2, "ProjectResourceDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kSkinResourceDir );
        lua_setfield( L, -2, "SkinResourceDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kUserSkinsDir );
        lua_setfield(L, -2, "UserSkinsDirectory");
#endif
        LuaLibSystem::PushDirectory( L, MPlatform::kCachesDir );
        lua_setfield( L, -2, "CachesDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kSystemCachesDir );
        lua_setfield( L, -2, "SystemCachesDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kPluginsDir );
        lua_setfield( L, -2, "PluginsDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kVirtualTexturesDir );
        lua_setfield( L, -2, "VirtualTexturesDirectory" );
        LuaLibSystem::PushDirectory( L, MPlatform::kApplicationSupportDir );
        lua_setfield( L, -2, "ApplicationSupportDirectory" );
    }

    Lua::InitializeGCMetatable( L, kNotificationMetatable, gcNotification );

    luaL_newmetatable( L, "LuaLibSystem" ); // push mt
    LuaProxyConstant* proxyIndex =
        Rtt_NEW( LuaContext::GetAllocator( L ), LuaProxyConstant( L, kProxyVTable ) );
    proxyIndex->Push( L );                // push proxy
    lua_setfield( L, -2, "__index" );    // mt.__index = proxy
    lua_setmetatable( L, -2 );            // t.mt = mt

    lua_pop( L, 1 );
}

int
LuaLibSystem::ValueForKey( lua_State *L, const MLuaProxyable&, const char key[], bool overrideRestriction /* = false */ ) const
{
    if ( strcmp( "orientation", key ) == 0 )
    {
        DeviceOrientation::Type t = LuaContext::GetPlatform( L ).GetDevice().GetOrientation();
        const char *str = DeviceOrientation::StringForType( t ); Rtt_ASSERT( str );
        lua_pushstring( L, str );
    }
    else if ( strcmp( "launchOrientation", key ) == 0 )
    {
        DeviceOrientation::Type t = LuaContext::GetRuntime( L )->GetDisplay().GetLaunchOrientation();
        const char *str = DeviceOrientation::StringForType( t ); Rtt_ASSERT( str );
        lua_pushstring( L, str );
    }
    else
    {
        lua_pushnil( L );
    }

    return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


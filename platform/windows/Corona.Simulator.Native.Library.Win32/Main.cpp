//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

//
// Summary:
// This is a thin proxy DLL that provides the same Corona public APIs as
// the "CoronaLabs.Corona.Native" library project provides, except this
// library's public exports are "forwarded" to the "Corona Simulator.exe"
// file's public exports. We do this because the Corona native library is
// statically linked into the simulator, but plugins are set up to implicitly
// link against an external Corona DLL. So, this proxy DLL allows plugins
// to correctly link against the simulator's version of the Corona library.

#include "CoronaAssert.h"
#include "CoronaEvent.h"
#include "CoronaLibrary.h"
#include "CoronaLog.h"
#include "CoronaLua.h"
#include "CoronaMacros.h"
#include "CoronaVersion.h"
#include "CoronaGraphics.h"
//#include "CoronaWin32.h"
#include <Windows.h>


/// <summary>Called when the library has been loaded or unloaded.</summary>
/// <param name="moduleHandle">Windows handle assigned to this library.</param>
/// <param name="reasonCode">
///  Unique ID indicating if the library is being loaded/unload or if a thread is being attached/detached.
/// </param>
/// <param name="reserved">This parameter is reserved by Windows.</param>
BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD reasonCode, LPVOID reserved)
{
	switch (reasonCode)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
		default:
			break;
	}
	return TRUE;
}


#pragma region Private Functions/Macros
/// <summary>Fetches a handle to the "Corona Simulator.exe" this library will interface with.</summary>
/// <returns>Returns a handle to the "Corona Simulator.exe" module.</returns>
static HMODULE GetSimulatorModuleHandle()
{
	static HMODULE sModuleHandle = nullptr;
	if (!sModuleHandle)
	{
//TODO: Make this work with the "Corona.Console.exe" too.
//      Perhaps change it to use the main module's file path which will always reference the EXE.
		sModuleHandle = ::LoadLibraryW(L"Corona Simulator.exe");
	}
	return sModuleHandle;
}

/// <summary>
///  <para>
///   Macro used to fetch and store a public Corona API callback from the "Corona Simulator.exe"
///   matching the calling function's name.
///  </para>
///  <para>
///   A typedef named "CoronaCallbackType" matching the API's function signature must be defined before this macro.
///  </para>
/// </summary>
#define CoronaCallbackLoad() \
	static CoronaCallbackType sCallback = nullptr; \
	if (!sCallback) \
	{ \
		sCallback = (CoronaCallbackType)::GetProcAddress(::GetSimulatorModuleHandle(), __FUNCTION__); \
	}

/// <summary>Invokes the "Corona Simulator.exe" module's callback loaded via the CoronaCallbackLoad() macro.</summary>
#define CoronaCallbackInvoke ((CoronaCallbackType)sCallback)

#pragma endregion


#pragma region Public CoronaEvent APIs
CORONA_API const char *CoronaEventNameKey()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventProviderKey()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventPhaseKey()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventTypeKey()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventResponseKey()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventIsErrorKey()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventErrorCodeKey()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventAdsRequestName()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventGameNetworkName()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API const char *CoronaEventPopupName()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

#pragma endregion


#pragma region Public CoronaLibrary APIs
CORONA_API const char *CoronaLibraryClassName()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API int CoronaLibraryInitialize(lua_State *L)
{
	typedef int(*CoronaCallbackType)(lua_State*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L);
}

CORONA_API int CoronaLibraryNew(
	lua_State *L,
	const char *libName, const char *publisherId, int version, int revision,
	const luaL_Reg libFuncs[], void *context)
{
	typedef int(*CoronaCallbackType)(lua_State*, const char*, const char*, int, int, const luaL_Reg[], void*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, libName, publisherId, version, revision, libFuncs, context);
}

CORONA_API int CoronaLibraryNewWithFactory(lua_State *L, lua_CFunction factory, const luaL_Reg libFuncs[], void *context)
{
	typedef int(*CoronaCallbackType)(lua_State*, lua_CFunction, const luaL_Reg[], void*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, factory, libFuncs, context);
}

CORONA_API void CoronaLibrarySetExtension(lua_State *L, int index)
{
	typedef void(*CoronaCallbackType)(lua_State*, int);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, index);
}

CORONA_API void CoronaLibraryProviderDefine(lua_State *L, const char *libName)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char*);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, libName);
}

CORONA_API int CoronaLibraryProviderNew(
	lua_State *L, const char *libName, const char *providerName, const char *publisherId)
{
	typedef int(*CoronaCallbackType)(lua_State*, const char*, const char*, const char*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, libName, providerName, publisherId);
}

CORONA_API void CoronaLibraryPushProperty(lua_State *L, const char *libName, const char *name)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char*, const char*);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, libName, name);
}

CORONA_API int CoronaLibraryCallFunction(lua_State *L, const char *libName, const char *name, const char *sig, ...)
{
	va_list arguments;
	va_start(arguments, sig);
	int result = CoronaLibraryCallFunctionV(L, libName, name, sig, arguments);
	va_end(arguments);
	return result;
}

CORONA_API int CoronaLibraryCallFunctionV(
	lua_State *L, const char *libName, const char *name, const char *sig, va_list arguments)
{
	typedef int(*CoronaCallbackType)(lua_State*, const char*, const char*, const char*, va_list);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, libName, name, sig, arguments);
}

CORONA_API int CoronaLibraryCallMethod(lua_State *L, const char *libName, const char *name, const char *sig, ...)
{
	va_list arguments;
	va_start(arguments, sig);
	int result = CoronaLibraryCallMethodV(L, libName, name, sig, arguments);
	va_end(arguments);
	return result;
}

CORONA_API int CoronaLibraryCallMethodV(
	lua_State *L, const char *libName, const char *name, const char *sig, va_list arguments)
{
	typedef int(*CoronaCallbackType)(lua_State*, const char*, const char*, const char*, va_list);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, libName, name, sig, arguments);
}

#pragma endregion


#pragma region Public CoronaLog APIs
CORONA_API int CoronaLog(const char *format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	int result = CoronaLogV(format, arguments);
	va_end(arguments);
	return result;
}

CORONA_API int CoronaLogV(const char *format, va_list arguments)
{
	typedef int(*CoronaCallbackType)(const char*, va_list);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(format, arguments);
}

#pragma endregion


#pragma region Public CoronaLua APIs
CORONA_API lua_State *CoronaLuaGetCoronaThread(lua_State *coroutine)
{
	typedef lua_State*(*CoronaCallbackType)(lua_State*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(coroutine);
}

CORONA_API lua_State *CoronaLuaNew(int flags)
{
	typedef lua_State*(*CoronaCallbackType)(int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(flags);
}

CORONA_API void CoronaLuaDelete(lua_State *L)
{
	typedef void(*CoronaCallbackType)(lua_State*);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L);
}

CORONA_API int CoronaLuaNormalize(lua_State *L, int index)
{
	typedef int(*CoronaCallbackType)(lua_State*, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, index);
}

CORONA_API void *CoronaLuaGetContext(lua_State *L)
{
	typedef void*(*CoronaCallbackType)(lua_State*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L);
}

CORONA_API void CoronaLuaInitializeContext(lua_State *L, void *context, const char *metatableName)
{
	typedef void(*CoronaCallbackType)(lua_State*, void*, const char*);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, context, metatableName);
}

CORONA_API CoronaLuaRef CoronaLuaNewRef(lua_State *L, int index)
{
	typedef CoronaLuaRef(*CoronaCallbackType)(lua_State*, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, index);
}

CORONA_API void CoronaLuaDeleteRef(lua_State *L, CoronaLuaRef ref)
{
	typedef void(*CoronaCallbackType)(lua_State*, CoronaLuaRef);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, ref);
}

CORONA_API int CoronaLuaEqualRef(lua_State *L, CoronaLuaRef ref, int index)
{
	typedef int(*CoronaCallbackType)(lua_State*, CoronaLuaRef, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, ref, index);
}

CORONA_API void CoronaLuaNewEvent(lua_State *L, const char *eventName)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char*);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, eventName);
}

CORONA_API void CoronaLuaDispatchEvent(lua_State *L, CoronaLuaRef listenerRef, int nresults)
{
	typedef void(*CoronaCallbackType)(lua_State*, CoronaLuaRef, int);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, listenerRef, nresults);
}

CORONA_API void CoronaLuaDispatchRuntimeEvent(lua_State *L, int nresults)
{
	typedef void(*CoronaCallbackType)(lua_State*, int);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, nresults);
}

CORONA_API int CoronaLuaIsListener(lua_State *L, int index, const char *eventName)
{
	typedef int(*CoronaCallbackType)(lua_State*, int, const char*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, index, eventName);
}

CORONA_API void CoronaLuaPushRuntime(lua_State *L)
{
	typedef void(*CoronaCallbackType)(lua_State*);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L);
}

CORONA_API void CoronaLuaRuntimeDispatchEvent(lua_State *L, int index)
{
	typedef void(*CoronaCallbackType)(lua_State*, int);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, index);
}

CORONA_API void CoronaLuaNewGCMetatable(lua_State *L, const char name[], lua_CFunction __gc1)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char[], lua_CFunction);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, name, __gc1);
}

CORONA_API void CoronaLuaNewMetatable(lua_State *L, const char name[], const luaL_Reg vtable[])
{
	typedef void(*CoronaCallbackType)(lua_State*, const char[], const luaL_Reg[]);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, name, vtable);
}

CORONA_API void CoronaLuaInitializeGCMetatable(lua_State *L, const char name[], lua_CFunction __gc1)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char[], lua_CFunction);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, name, __gc1);
}

CORONA_API void CoronaLuaInitializeMetatable(lua_State *L, const char name[], const luaL_Reg vtable[])
{
	typedef void(*CoronaCallbackType)(lua_State*, const char[], const luaL_Reg[]);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, name, vtable);
}

CORONA_API void CoronaLuaPushUserdata(lua_State *L, void *ud, const char metatableName[])
{
	typedef void(*CoronaCallbackType)(lua_State*, void*, const char[]);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, ud, metatableName);
}

CORONA_API void *CoronaLuaToUserdata(lua_State *L, int index)
{
	typedef void*(*CoronaCallbackType)(lua_State*, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, index);
}

CORONA_API void *CoronaLuaCheckUserdata(lua_State *L, int index, const char metatableName[])
{
	typedef void*(*CoronaCallbackType)(lua_State*, int, const char[]);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, index, metatableName);
}

CORONA_API void CoronaLuaRegisterModuleLoader(lua_State *L, const char *name, lua_CFunction loader, int nupvalues)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char*, lua_CFunction, int);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, name, loader, nupvalues);
}

CORONA_API void CoronaLuaRegisterModuleLoaders(lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues)
{
	typedef void(*CoronaCallbackType)(lua_State*, const luaL_Reg[], int);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, moduleLoaders, nupvalues);
}

CORONA_API int CoronaLuaOpenModule(lua_State *L, lua_CFunction loader)
{
	typedef int(*CoronaCallbackType)(lua_State*, lua_CFunction);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, loader);
}

CORONA_API void CoronaLuaInsertPackageLoader(lua_State *L, lua_CFunction loader, int index)
{
	typedef void(*CoronaCallbackType)(lua_State*, lua_CFunction, int);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, loader, index);
}

CORONA_API lua_CFunction CoronaLuaGetErrorHandler()
{
	typedef lua_CFunction(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

CORONA_API void CoronaLuaSetErrorHandler(lua_CFunction newValue)
{
	typedef void(*CoronaCallbackType)(lua_CFunction);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(newValue);
}

CORONA_API int CoronaLuaDoCall(lua_State *L, int narg, int nresults)
{
	typedef int(*CoronaCallbackType)(lua_State*, int, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, narg, nresults);
}

CORONA_API int CoronaLuaDoBuffer(lua_State *L, lua_CFunction loader, lua_CFunction pushargs)
{
	typedef int(*CoronaCallbackType)(lua_State*, lua_CFunction, lua_CFunction);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, loader, pushargs);
}

CORONA_API int CoronaLuaDoFile(lua_State *L, const char* file, int narg, int clear)
{
	typedef int(*CoronaCallbackType)(lua_State*, const char*, int, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, file, narg, clear);
}

CORONA_API int CoronaLuaPushModule(lua_State *L, const char *name)
{
	typedef int(*CoronaCallbackType)(lua_State*, const char*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, name);
}

CORONA_API void CoronaLuaLog(lua_State *L, const char *fmt, ...)
{
	va_list arguments;
	va_start(arguments, fmt);
	CoronaLuaLogV(L, fmt, arguments);
	va_end(arguments);
}

CORONA_API void CoronaLuaLogV(lua_State *L, const char *fmt, va_list arguments)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char*, va_list);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, fmt, arguments);
}

CORONA_API void CoronaLuaLogPrefix(lua_State *L, const char *prefix, const char *fmt, ...)
{
	va_list arguments;
	va_start(arguments, fmt);
	CoronaLuaLogPrefixV(L, prefix, fmt, arguments);
	va_end(arguments);
}

CORONA_API void CoronaLuaLogPrefixV(lua_State *L, const char *prefix, const char *fmt, va_list arguments)
{
	typedef void(*CoronaCallbackType)(lua_State*, const char*, const char*, va_list);
	CoronaCallbackLoad();
	CoronaCallbackInvoke(L, prefix, fmt, arguments);
}

CORONA_API void CoronaLuaWarning(lua_State *L, const char *fmt, ...)
{
	va_list arguments;
	va_start(arguments, fmt);
	CoronaLuaLogPrefixV(L, "WARNING: ", fmt, arguments);
	va_end(arguments);
}

CORONA_API void CoronaLuaError(lua_State *L, const char *fmt, ...)
{
	va_list arguments;
	va_start(arguments, fmt);
	CoronaLuaLogPrefixV(L, "ERROR: ", fmt, arguments);
	va_end(arguments);
}

CORONA_API int CoronaLuaPropertyToJSON(lua_State *L, int idx, const char *key, char *buf, int bufLen, int pos)
{
	typedef int(*CoronaCallbackType)(lua_State*, int, const char*, char*, int, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, idx, key, buf, bufLen, pos);
}

#pragma endregion


#pragma region Public CoronaVersion APIs
CORONA_API const char *CoronaVersionBuildString()
{
	typedef const char*(*CoronaCallbackType)();
	CoronaCallbackLoad();
	return CoronaCallbackInvoke();
}

#pragma endregion



#pragma region Corona Graphics APIs
CORONA_API int CoronaExternalPushTexture(lua_State *L, const struct CoronaExternalTextureCallbacks *callbacks, void* userData)
{
	typedef int(*CoronaCallbackType)(lua_State*, const struct CoronaExternalTextureCallbacks *, void*);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, callbacks, userData);
}

CORONA_API
void* CoronaExternalGetUserData(lua_State *L, int index)
{
	typedef void*(*CoronaCallbackType)(lua_State*, int);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, index);
}

CORONA_API
int CoronaExternalFormatBPP(CoronaExternalBitmapFormat format)
{
	typedef int(*CoronaCallbackType)(CoronaExternalBitmapFormat);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(format);
}
#pragma endregion


#pragma region Corona Memory APIs
CORONA_API
int CoronaMemoryCreateInterface(lua_State *L, const struct CoronaMemoryInterfaceInfo *info)
{
	typedef int(*CoronaCallbackType)(lua_State *, const struct CoronaMemoryInterfaceInfo *);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, info);
}

CORONA_API
void* CoronaMemoryBindLookupSlot(lua_State *L, unsigned short *id)
{
	typedef void*(*CoronaCallbackType)(lua_State*, unsigned short *);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, id);
}

CORONA_API
int CoronaMemoryReleaseLookupSlot(lua_State *L, unsigned short id)
{
	typedef int(*CoronaCallbackType)(lua_State *, unsigned short);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, id);
}

CORONA_API
void* CoronaMemoryPushLookupEncoding(lua_State *L, unsigned short id, unsigned short context)
{
	typedef void*(*CoronaCallbackType)(lua_State *, unsigned short, unsigned short);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, id, context);
}

CORONA_API
int CoronaMemoryAcquireInterface(lua_State *L, int arg, struct CoronaMemoryAcquireState *state)
{
	typedef int(*CoronaCallbackType)(lua_State *, int, struct CoronaMemoryAcquireState *);
	CoronaCallbackLoad();
	return CoronaCallbackInvoke(L, arg, state);
}

#pragma endregion
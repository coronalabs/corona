// ----------------------------------------------------------------------------
// 
// NativeCoronaNetworkLibraryHandler.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#include <collection.h>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
extern "C"
{
#	include "lua.h"
}
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	ref class CoronaRuntimeEnvironment;
} } }
#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

ref class CoronaHttpRequestOperation;

/// <summary>Corona's Lua "network" library binding, which provides the implementation for this library's APIs.</summary>
class NativeCoronaNetworkLibraryHandler
{
	private:
		/// <summary>Creates a new Lua "network" library binding associated with the given Corona environment.</summary>
		/// <param name="environment">
		///  <para>
		///   The Corona runtime environment providing the Lua state to register the library to as well as the
		///   INetworkServices object which provides the implementation.
		///  </para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		NativeCoronaNetworkLibraryHandler(CoronaRuntimeEnvironment^ environment);

	public:
		/// <summary>Aborts all active network operations and destroys owned resources</summary>
		virtual ~NativeCoronaNetworkLibraryHandler();

		/// <summary>Registers a "network" Lua library binding with the given Corona runtime's Lua state.</summary>
		/// <param name="environment">The Corona runtime to register this library binding to.</param>
		/// <returns>
		///  <para>Returns true if successfully registered this library binding with Lua.</para>
		///  <para>
		///   Returns false if given null, if the runtime has been terminated, or if it does not provide an
		///   "INetworkServices" object which provides the network implementation for the Lua library.
		///  </para>
		/// </returns>
		static bool RegisterTo(CoronaRuntimeEnvironment^ environment);

	private:
		/// <summary>Unique name of this library's finalizer metatable registered into the Lua registry.</summary>
		static const char kLuaFinalizerMetatableName[];

		/// <summary>Called when the network.getConnectionStatus() function gets invoked.</summary>
		/// <param name="luaStatePointer">Pointer to the Lua state that invoked the Lua function.</param>
		/// <returns>
		///  <para>Returns the number of return values pushed to the top of the Lua stack.</para>
		///  <para>Returns zero if no return values has been pushed to Lua.</para>
		/// </returns>
		static int OnGetConnectionStatus(lua_State *luaStatePointer);

		/// <summary>Called when the network.request() function gets invoked.</summary>
		/// <param name="luaStatePointer">Pointer to the Lua state that invoked the Lua function.</param>
		/// <returns>
		///  <para>Returns the number of return values pushed to the top of the Lua stack.</para>
		///  <para>Returns zero if no return values has been pushed to Lua.</para>
		/// </returns>
		static int OnRequest(lua_State *luaStatePointer);

		/// <summary>Called when the network.canel() function gets invoked.</summary>
		/// <param name="luaStatePointer">Pointer to the Lua state that invoked the Lua function.</param>
		/// <returns>
		///  <para>Returns the number of return values pushed to the top of the Lua stack.</para>
		///  <para>Returns zero if no return values has been pushed to Lua.</para>
		/// </returns>
		static int OnCancel(lua_State *luaStatePointer);

		/// <summary>
		///  Called when the Lua library is being garbaged collected by Lua, which should only happen when
		///  Corona is being terminated.
		/// </summary>
		/// <param name="luaStatePointer">Pointer to the Lua state that this library belongs to.</param>
		/// <returns>Always returns zero.</returns>
		static int OnFinalizing(lua_State *luaStatePointer);

		/// <summary>Called when one HTTP request operation has ended.</summary>
		/// <param name="operation">The HTTP operation that has just ended.</param>
		/// <param name="contextPointer">Pointer to the NativeCoronaNetworkLibraryHandler object.</param>
		static void OnHttpRequestEnded(CoronaHttpRequestOperation ^operation, void *contextPointer);

		/// <summary>
		///  <para>Corona runtime environment this library binding is associated with.</para>
		///  <para>Provides Corona's main Lua state and the INetworkServices implementation object.</para>
		/// </summary>
		CoronaRuntimeEnvironment^ fEnvironment;

		/// <summary>Integer ID to be assigned to the next HTTP request operation.</summary>
		int fNextRequestId;

		/// <summary>
		///  <para>Dictionary of HTTP request operations that are currently executing.</para>
		///  <para>Uses their network request integer ID as the key for fast lookups.</para>
		/// </summary>
		Platform::Collections::Map<int, CoronaHttpRequestOperation^>^ fHttpRequestOperations;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking

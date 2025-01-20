//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_RuntimeDelegatePlayer.h"
#	include "Rtt_WinRTCallback.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	ref class CoronaRuntimeEnvironment;
} } }

#pragma endregion


namespace Rtt
{

/// <summary>
///  <para>Delegate used to receive events from the Corona Runtime.</para>
///  <para>This is done by assigning an instance of this class to the Runtime.SetDelegate() function.</para>
/// </summary>
class WinRTRuntimeDelegate : public RuntimeDelegatePlayer
{
	public:
		/// <summary>Creates a new delegate used to receive events from the Corona runtime.</summary>
		/// <param name="environment">
		///  <para>Reference to the Corona runtime environment.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		WinRTRuntimeDelegate(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment);

		/// <summary>Destroys this delegates resources.</summary>
		virtual ~WinRTRuntimeDelegate();

		/// <summary>Called just after all core Lua libraries have been loaded into the runtime's Lua state.</summary>
		/// <param name="sender">The Corona runtime that is raising this event.</param>
		virtual void DidInitLuaLibraries(const Runtime& sender) const;

		/// <summary>Determines if the Corona library is licensed and authorized to continue.</summary>
		/// <param name="sender">The Corona runtime that is raising this event.</param>
		/// <returns>Returns true if the Corona library is licensed/authorized. Returns false if not.</returns>
		virtual bool HasDependencies(const Runtime& sender) const;
		
		/// <summary>
		///  <para>Called just before the "main.lua" file has been loaded.</para>
		///  <para>This is the application's opportunity to register custom APIs into Lua.</para>
		/// </summary>
		/// <param name="sender">The Corona runtime that is raising this event.</param>
		virtual void WillLoadMain(const Runtime& sender) const;

		/// <summary>Called after the "main.lua" file has been loaded and executed.</summary>
		/// <param name="sender">The Corona runtime that is raising this event.</param>
		virtual void DidLoadMain(const Runtime& sender) const;

		/// <summary>Called when the "config.lua" file has been loaded into Lua, but before it has been read.</summary>
		/// <param name="sender">The Corona runtime that is raising this event.</param>
		/// <param name="L">The Lua state that the "config.lua" will be loaded into.</param>
		virtual void WillLoadConfig(const Runtime& sender, lua_State *L) const;

		/// <summary>Called after the "config.lua" file has been read.</summary>
		/// <param name="sender">The Corona runtime that is raising this event.</param>
		/// <param name="L">The Lua state that the "config.lua" settings were loaded into.</param>
		virtual void DidLoadConfig(const Runtime& sender, lua_State *L) const;

		/// <summary>
		///  <para>Called when the "config.lua" file's table has been pushed to the top of the Lua stack.</para>
		///  <para>This is the application's opportunity to modify these setting before Corona processes them.</para>
		/// </summary>
		virtual void InitializeConfig(const Runtime& sender, lua_State *L) const;

		/// <summary>Sets the callback to be invoked to raise a "Loaded" event by the C++/CX side of Corona.</summary>
		/// <param name="callback">
		///  The callback to be invoked. Expected to reference a private instance method in the CoronaRuntimeEnvironment class.
		/// </param>
		void SetLoadedCallback(const WinRTMethodCallback &callback);

		/// <summary>Sets the callback to be invoked to raise a "Started" event by the C++/CX side of Corona.</summary>
		/// <param name="callback">
		///  The callback to be invoked. Expected to reference a private instance method in the CoronaRuntimeEnvironment class.
		/// </param>
		void SetStartedCallback(const WinRTMethodCallback &callback);

	private:
		/// <summary>The Corona runtime instance this delegate is assigned to.</summary>
		CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ fEnvironment;

		/// <summary>Callback to be invoked when the C++/CX CoronaRuntime class' "Loaded" event should be raised.</summary>
		mutable WinRTMethodCallback fLoadedCallback;

		/// <summary>Callback to be invoked when the C++/CX CoronaRuntime class' "Started" event should be raised.</summary>
		mutable WinRTMethodCallback fStartedCallback;
};

} // namespace Rtt

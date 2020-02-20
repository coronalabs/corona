//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\EventArgs.h"
#include "Key.h"
#include "ModifierKeyStates.h"


namespace Interop { namespace Input {

/// <summary>
///  Event arguments providing information about the key that was pressed/released
///  and the modifier keys that were held down at the time.
/// </summary>
class KeyEventArgs : public EventArgs
{
	public:
		#pragma region Public Settings Structure
		/// <summary>
		///  <para>Stores the data for a key up/down event. Used to create a KeyEventArgs object.</para>
		///  <para>This data structure was designed to be passed around by value cheaply/efficiently.</para>
		/// </summary>
		struct Data
		{
			/// <summary>
			///  <para>The platform's unique integer ID for the key that was pressed/released.</para>
			///  <para>Expected to be assigned a Win32 virtual key constant such as VK_SPACE, VK_RETURN, etc.</para>
			///  <para>
			///   Can also be assigned via the Corona "Key" class constants' GetNativeCodeValue() method, which
			///   also provides XInput and Corona defined key codes.
			///  </para>
			/// </summary>
			int NativeCode;

			/// <summary>Set true if the key was pressed down. Set false if the key was released.</summary>
			bool IsDown;

			/// <summary>
			///  Flags indicating which Alt, Ctrl, Shift, and Windows/Command modifiers were held down when
			///  this key event occurred.
			/// </summary>
			ModifierKeyStates::Flags ModifierFlags;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new event arguments object with the given data.</summary>
		/// <param name="data">The received key event data.</param>
		KeyEventArgs(const KeyEventArgs::Data& data);

		/// <summary>Destroys this object.</summary>
		virtual ~KeyEventArgs();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the key that was pressed or released.</summary>
		/// <returns>Returns the key that was pressed or released.</returns>
		Key GetKey() const;

		/// <summary>Determines if the key was pressed down.</summary>
		/// <returns>Returns true if the key was pressed down. Returns false if the key was released.</returns>
		bool IsDown() const;

		/// <summary>Determines if the key was released.</summary>
		/// <returns>Returns true if the key was released. Returns false if the key was pressed down.</returns>
		bool IsUp() const;

		/// <summary>Gets the up/down states of all modifier keys when the key event occurred.</summary>
		/// <returns>Returns the up/down state of modifier keys Alt, Ctrl, Shift, and the Windows/Command key.</returns>
		ModifierKeyStates GetModifierKeyStates() const;

		/// <summary>Gets this key event's full data as a structure.</summary>
		/// <returns>Returns a copy of this key event's full data as a structure.</returns>
		KeyEventArgs::Data ToData() const;

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>Creates a new event arguments object with the given key event data.</summary>
		/// <param name="data">The received key data.</param>
		/// <returns>Returns a new event arguments object providing the given key data.</returns>
		static KeyEventArgs From(const KeyEventArgs::Data& data);

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Assignment operator made private to make this object immutable.</summary>
		void operator=(const KeyEventArgs& value) {}

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Stores the key that was pressed/released and the modifier key up/down states.</summary>
		KeyEventArgs::Data fData;

		/// <summary>
		///  <para>Key object matching member variable's "fSettings.NativeCode" field.</para>
		///  <para>
		///   Cached for quick access in the constructor so that we only have to call Key::FromNativeCode() once.
		///  </para>
		/// </summary>
		Key fKey;

		#pragma endregion
};

} }	// namespace Interop::Input

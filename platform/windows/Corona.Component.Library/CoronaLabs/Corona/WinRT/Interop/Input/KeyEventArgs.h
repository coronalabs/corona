// ----------------------------------------------------------------------------
// 
// KeyEventArgs.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

// Forward declarations.
ref class Key;


/// <summary>
///  Provides a key down/up event's information such as the key name, native integer code, which Alt/Control/Shift modifiers
///  were held down at the time, etc.
/// </summary>
public ref class KeyEventArgs sealed
{
	public:
		/// <summary>Creates a new arguments object providing information for a key input event.</summary>
		/// <param name="key">The key input that was received. Cannot be null or else an exception will be thrown.</param>
		/// <param name="isAltDown">Set to true if any of the Alt keys were held down when the key input was received.</param>
		/// <param name="isControlDown">Set to true if any of the Control keys were held down when the key input was received.</param>
		/// <param name="isShiftDown">
		///  Set to true if any of the Shift keys were held down or if the CapsLock was toggled on when
		///  the key input was received.
		/// </param>
		/// <param name="isControlDown">
		///  Set to true if any of the Windows/Command keys were held down when the key input was received.
		/// </param>
		KeyEventArgs(
				CoronaLabs::Corona::WinRT::Interop::Input::Key^ key,
				bool isAltDown, bool isControlDown, bool isShiftDown, bool isCommandDown);

		/// <summary>Gets the key associated with this event.</summary>
		/// <value>The key input that was received providing the native key code and Corona's unique key name for it.</value>
		property CoronaLabs::Corona::WinRT::Interop::Input::Key^ Key { CoronaLabs::Corona::WinRT::Interop::Input::Key^ get(); }

		/// <summary>Determines if an Alt key was held down while the key event occurred.</summary>
		/// <value>Set to true if any of the Alt keys were held down. Set to false if not.</value>
		property bool IsAltDown { bool get(); }

		/// <summary>Determines if a Control key was held down while the key event occurred.</summary>
		/// <value>Set to true if any of the Control keys were held down. Set to false if not.</value>
		property bool IsControlDown { bool get(); }

		/// <summary>Determines if a Shift key was held down while the key event occurred.</summary>
		/// <value>
		///  <para>Set to true if any of the Shift keys were held down or if the CapsLock key was toggled on.</para>
		///  <para>Set to false if not.</para>
		/// </value>
		property bool IsShiftDown { bool get(); }

		/// <summary>Determines if a Windows/Command key was held down while the key event occurred.</summary>
		/// <value>Set to true if any of the Windows/Command keys were held down. Set to false if not.</value>
		property bool IsCommandDown { bool get(); }

		/// <summary>Gets or sets whether or not the key event was consumed by the event handler.</summary>
		/// <value>
		///  <para>
		///   Set to false if this key event was not handled. In which case, the control which received this event
		///   will perform its default action.
		///  </para>
		///  <para>
		///   Set to true if the event handler has handled this event. This overrides the default behavior of the
		///   control that received this event.
		///  </para>
		/// </value>
		property bool Handled { bool get(); void set(bool value); }

	private:
		CoronaLabs::Corona::WinRT::Interop::Input::Key^ fKey;
		bool fIsAltDown;
		bool fIsControlDown;
		bool fIsShiftDown;
		bool fIsCommandDown;
		bool fWasHandled;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input

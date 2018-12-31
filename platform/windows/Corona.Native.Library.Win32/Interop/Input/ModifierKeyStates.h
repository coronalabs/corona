//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>


namespace Interop { namespace Input {

/// <summary>
///  <para>Stores the keyboard's up/down states for modifier keys Alt, Control, Shift, and the Windows/Command key.</para>
///  <para>
///   Also provides an easy means of fetching the current keyboard modifier states via the
///   static FromKeyBoards() function.
///  </para>
/// </summary>
class ModifierKeyStates
{
	public:
		#pragma region Public Constants
		/// <summary>Defines a byte type whose bits flag which modifier keys are being held down.</summary>
		typedef int8_t Flags;

		/// <summary>Bit flag constant indicating that the "Alt" modifier key is held down.</summary>
		static const ModifierKeyStates::Flags kFlagAlt;

		/// <summary>Bit flag constant indicating that the "Control" modifier key is held down.</summary>
		static const ModifierKeyStates::Flags kFlagControl;

		/// <summary>Bit flag constant indicating that the "Shift" modifier key is held down.</summary>
		static const ModifierKeyStates::Flags kFlagShift;

		/// <summary>Bit flag constant indicating that the Windows/Command modifier key is held down.</summary>
		static const ModifierKeyStates::Flags kFlagCommand;

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new object where all modifier key states are initialize to up/false.</summary>
		ModifierKeyStates();

		/// <summary>Creates a new object initialized with the given modifier flags.</summary>
		/// <param name="flags">Flags indicating which modifier keys are being held down.</param>
		ModifierKeyStates(const ModifierKeyStates::Flags& flags);

		/// <summary>Destroys this object.</summary>
		virtual ~ModifierKeyStates();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Determines if modifier key "Alt" is held down.</summary>
		/// <returns>Returns true if the key state is flagged as down. Returns false if flagged as up.</returns>
		bool IsAltDown() const;

		/// <summary>Set the up/down state of the "Alt" modifier key.</summary>
		/// <param name="value">Set true to flag the key state as down. Set false to flag it as up.</param>
		void SetIsAltDown(bool value);

		/// <summary>Determines if modifier key "Control" is held down.</summary>
		/// <returns>Returns true if the key state is flagged as down. Returns false if flagged as up.</returns>
		bool IsControlDown() const;

		/// <summary>Set the up/down state of the "Control" modifier key.</summary>
		/// <param name="value">Set true to flag the key state as down. Set false to flag it as up.</param>
		void SetIsControlDown(bool value);

		/// <summary>Determines if modifier key "Shift" is held down.</summary>
		/// <returns>Returns true if the key state is flagged as down. Returns false if flagged as up.</returns>
		bool IsShiftDown() const;

		/// <summary>Set the up/down state of the "Shift" modifier key.</summary>
		/// <param name="value">Set true to flag the key state as down. Set false to flag it as up.</param>
		void SetIsShiftDown(bool value);

		/// <summary>Determines if the Windows/Command modifier key is held down.</summary>
		/// <returns>Returns true if the key state is flagged as down. Returns false if flagged as up.</returns>
		bool IsCommandDown() const;

		/// <summary>Set the up/down state of the Windows/Command modifier key.</summary>
		/// <param name="value">Set true to flag the key state as down. Set false to flag it as up.</param>
		void SetIsCommandDown(bool value);

		/// <summary>Gets a copy of this object's modifier key flags in byte form.</summary>
		/// <returns>Returns flags indicating which modifier keys are currently held down in byte form.</returns>
		ModifierKeyStates::Flags GetFlags() const;

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>Fetches the keyboard's current up/down modifier key states.</summary>
		/// <returns>Returns the current up/down states of all modifier keys.</returns>
		static ModifierKeyStates FromKeyboard();

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>Stores the state of all modifier keys in bit form.</summary>
		ModifierKeyStates::Flags fFlags;

		#pragma endregion
};

} }	// namespace Interop::Input

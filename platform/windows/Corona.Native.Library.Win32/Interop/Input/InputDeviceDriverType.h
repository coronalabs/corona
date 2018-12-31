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


namespace Interop { namespace Input {

/// <summary>
///  Identifies the driver/library used to access an input device such as XInput, Direct Input, or Raw Input.
/// </summary>
/// <remarks>
///  You cannot create instances of this class. Instead, you access pre-defined InputDeviceDriverType objects
///  via this class' static constants like you would with an enum.
/// </remarks>
class InputDeviceDriverType
{
	private:
		#pragma region Private Constructors
		/// <summary>Creates a new device driver type with the given unique string ID.</summary>
		/// <remarks>
		///  This is a private constructor that is used to create predefined object of this class
		///  made available publicly via static constants.
		/// </remarks>
		/// <param name="stringId">
		///  <para>The device driver's unique string ID such as "xInput", "directInput", or "rawInput".</para>
		///  <para>Used by Lua to determine which driver an input device is being handled by.</para>
		///  <para>Cannot be empty/null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="integerId">The driver's unique integer ID.</param>
		InputDeviceDriverType(const char* stringId, int integerId);

		#pragma endregion

	public:
		#pragma region Destructor
		/// <summary>Destroys this object.</summary>
		virtual ~InputDeviceDriverType();

		#pragma endregion


		#pragma region Public Constants
		/// <summary>
		///  <para>Indicates that the input device is handled by Microsoft DirectInput.</para>
		///  <para>This is typically used by game controllers that are HID or have DirectInput drivers.</para>
		/// </summary>
		static const InputDeviceDriverType kDirectInput;

		/// <summary>
		///  <para>Indicates that the input device is handled by Microsoft's "Raw Input" APIs.</para>
		///  <para>
		///   This is typically used to access HID devices such as mice and keyboards via WM_INPUT Windows messages.
		///  </para>
		/// </summary>
		static const InputDeviceDriverType kRawInput;

		/// <summary>
		///  <para>Indicates that the input device is handled by Microsoft XInput.</para>
		///  <para>This is typically used by Xbox controllers.</para>
		/// </summary>
		static const InputDeviceDriverType kXInput;

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the driver type's unique integer ID.</summary>
		/// <returns>Returns the driver type's unique integer ID.</returns>
		int GetIntegerId() const;

		/// <summary>Gets the driver type's unique string ID such as "xInput", "directInput", or "rawInput".</summary>
		/// <returns>Returns the driver type's unique string ID such as "xInput", "directInput", or "rawInput".</returns>
		const char* GetStringId() const;

		/// <summary>Determines if this device driver type matches the given type.</summary>
		/// <param name="value">The device driver type to be compared with.</param>
		/// <returns>Return true if the device driver types match. Returns false if not.</returns>
		bool Equals(const InputDeviceDriverType& value) const;

		/// <summary>Determines if this device driver type does not match the given type.</summary>
		/// <param name"value">The device driver type to be compared with.</param>
		/// <returns>Return true if the device driver types do not match. Returns false if they do match.</returns>
		bool NotEquals(const InputDeviceDriverType& value) const;

		/// <summary>Determines if this device driver type matches the given type.</summary>
		/// <param name="value">The device driver type to be compared with.</param>
		/// <returns>Return true if the device driver types match. Returns false if not.</returns>
		bool operator==(const InputDeviceDriverType& value) const;

		/// <summary>Determines if this device driver type does not match the given type.</summary>
		/// <param name"value">The device driver type to be compared with.</param>
		/// <returns>Return true if the device driver types do not match. Returns false if they do match.</returns>
		bool operator!=(const InputDeviceDriverType& value) const;

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>Fetches a driver type having the given unique integer ID.</summary>
		/// <param name="integerId">
		///  <para>Unique integer ID of the driver type to be fetched.</para>
		///  <para>
		///   Expected to match the value returned by GetIntegerId() of one of this class' driver type constants.
		///  </para>
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to a matching driver type constant such as kDirectInput, kXInput, etc.</para>
		///  <para>Returns null if the given integer ID is unknown.</para>
		/// </returns>
		static const InputDeviceDriverType* FromIntegerId(int integerId);

		/// <summary>Fetches a driver type having the given unique string ID such as "directInput", "xInput", etc.</summary>
		/// <param name="stringId">
		///  Unique string ID of the driver type to fetch such as "directInput", "xInput", or "rawInput".
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to a matching driver type constant such as kDirectInput, kXInput, etc.</para>
		///  <para>Returns null if given string ID is unkown or if given a null/empty string.</para>
		/// </returns>
		static const InputDeviceDriverType* FromStringId(const char* stringId);

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>The driver type's unique integer ID.</summary>
		const int fIntegerId;

		/// <summary>The driver type's unique string ID.</summary>
		const char* fStringId;

		#pragma endregion
};

} }	// namespace Interop::Input

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

#include "Input\Rtt_InputAxisType.h"
#include "Input\Rtt_InputDeviceType.h"
#include "Key.h"
#include "WinString.h"
#include <string>
#include <vector>


namespace Interop { namespace Input {

class InputAxisSettings;

/// <summary>Stores the configuration for one input device such as a keyboard, gamepad, etc.</summary>
class InputDeviceSettings
{
	public:
		#pragma region Public AxisCollectionAdapter Class
		/// <summary>
		///  <para>Provides access to an InputDeviceSettings object's axis collection.</para>
		///  <para>Instances of this class was designed to be passed by value efficiently.</para>
		/// </summary>
		class AxisCollectionAdapter
		{
			public:
				/// <summary>
				///  Creates a new object providing writeable access to the given input device's axis collection.
				/// </summary>
				/// <param name="settings">Reference to the input device to access the axis collection of.</param>
				AxisCollectionAdapter(InputDeviceSettings& settings);

				/// <summary>Creates a new object providing access to the given input device's axis collection.</summary>
				/// <param name="settings">Reference to the input device to access the axis collection of.</param>
				AxisCollectionAdapter(const InputDeviceSettings& settings);

				/// <summary>Destroys this object.</summary>
				virtual ~AxisCollectionAdapter();

				/// <summary>
				///  Adds a new input axis configuration initialized to its defaults to the end of the
				///  axis collection and returns it.
				/// </summary>
				/// <returns>Returns a pointer to the newly added axis configuration.</returns>
				InputAxisSettings* Add();

				/// <summary>Removes all axes from the collection.</summary>
				void Clear();

				/// <summary>Gets the number of axis configurations in the collection.</summary>
				/// <returns>
				///  <para>Returns the number of axes in the collection.</para>
				///  <para>Returns zero if the device has no axes.</para>
				/// </returns>
				int GetCount() const;

				/// <summary>Fetches an axis configuration from the collection by its zero based index.</summary>
				/// <param name="index">Zero based index to the axis configuration.</param>
				/// <returns>
				///  <para>Returns a pointer to the index axis configuration.</para>
				///  <para>Returns null if given an invalid index.</para>
				/// </returns>
				InputAxisSettings* GetByIndex(int index) const;

				/// <summary>Determines if this collection contains at least one axis of the given type.</summary>
				/// <param name="value">The axis type to search for such as kLeftX, kLeftY, kLeftTrigger, etc.</param>
				/// <returns>
				///  <para>Returns true if this collection contains at least one axis of the given type.</para>
				///  <para>Returns false if the given axis type was not found in the collection.</para>
				/// </returns>
				bool Contains(const Rtt::InputAxisType& value) const;

				/// <summary>
				///  Determines if all of the axis configurations in this collection exactly match
				///  the axis configurations in the given collection.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>
				///   Returns true if the axis configurations in this collection exactly match with the given collection.
				///  </para>
				///  <para>
				///   Returns false if at least one axis configuration does not match or if the number of axes differ
				///   between the two collections.
				///  </para>
				/// </returns>
				bool Equals(const AxisCollectionAdapter& collection) const;

				/// <summary>
				///  Determines if the axis configurations between this collection and the given collection
				///  do not exactly match.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>
				///   Returns true if at least one axis configuration does not match or if the number of axes differ
				///   between the two collections.
				///  </para>
				///  <para>
				///   Returns false if the axis configurations in this collection exactly match with the given collection.
				///  </para>
				/// </returns>
				bool NotEquals(const AxisCollectionAdapter& collection) const;

				/// <summary>
				///  Determines if all of the axis configurations in this collection exactly match
				///  the axis configurations in the given collection.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>
				///   Returns true if the axis configurations in this collection exactly match with the given collection.
				///  </para>
				///  <para>
				///   Returns false if at least one axis configuration does not match or if the number of axes differ
				///   between the two collections.
				///  </para>
				/// </returns>
				bool operator==(const AxisCollectionAdapter& collection) const;

				/// <summary>
				///  Determines if the axis configurations between this collection and the given collection
				///  do not exactly match.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>
				///   Returns true if at least one axis configuration does not match or if the number of axes differ
				///   between the two collections.
				///  </para>
				///  <para>
				///   Returns false if the axis configurations in this collection exactly match with the given collection.
				///  </para>
				/// </returns>
				bool operator!=(const AxisCollectionAdapter& collection) const;

			private:
				/// <summary>
				///  <para>Reference to the input device object this class wraps.</para>
				///  <para>Used to access the actual InputAxisSettings collection object that this class manipulates.</para>
				/// </summary>
				InputDeviceSettings& fSettings;
		};

		#pragma endregion


		#pragma region Public KeyCollectionAdapter Class
		/// <summary>
		///  <para>Provides access to an InputDeviceSettings object's key collection.</para>
		///  <para>Instances of this class was designed to be passed by value efficiently.</para>
		/// </summary>
		class KeyCollectionAdapter
		{
			public:
				/// <summary>
				///  Creates a new object providing writeable access to the given input device's key collection.
				/// </summary>
				/// <param name="settings">Reference to the input device to access the key collection of.</param>
				KeyCollectionAdapter(InputDeviceSettings& settings);

				/// <summary>Creates a new object providing access to the given input device's key collection.</summary>
				/// <param name="settings">Reference to the input device to access the key collection of.</param>
				KeyCollectionAdapter(const InputDeviceSettings& settings);

				/// <summary>Destroys this object.</summary>
				virtual ~KeyCollectionAdapter();

				/// <summary>
				///  <para>Adds the given key to the collection, if it hasn't been added already.</para>
				///  <para>Note that the collection will only store a unique set of keys. Duplicates are not allowed.</para>
				/// </summary>
				/// <param name="value">The key to be added to the collection.</param>
				/// <returns>
				///  <para>Returns true if the key was added.</para>
				///  <para>Returns false if the given key already exists in the collection.</para>
				/// </returns>
				bool Add(const Key& value);

				/// <summary>Removes all keys from the collection.</summary>
				void Clear();

				/// <summary>Gets the number of keys in the collection.</summary>
				/// <returns>
				///  <para>Returns the number of keys in the collection.</para>
				///  <para>Returns zero if the device has no keys.</para>
				/// </returns>
				int GetCount() const;

				/// <summary>Fetches a key from the collection by its zero based index.</summary>
				/// <param name="index">Zero based index to the key.</param>
				/// <returns>
				///  <para>Returns a pointer to the indexed key.</para>
				///  <para>Returns null if given an invalid index.</para>
				/// </returns>
				const Key* GetByIndex(int index) const;

				/// <summary>Determines if the given key exists in the collection.</summary>
				/// <param name="value">The key to search for.</param>
				/// <returns>Returns true if the given key exists in the collection. Returns false if not.</returns>
				bool Contains(const Key& value) const;

				/// <summary>
				///  Determines if all of the keys in this collection match the keys in the given collection.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>Returns true if the keys in this collection exactly match with the given collection.</para>
				///  <para>
				///   Returns false if at least one key does not match or if the number of keys differ
				///   between the two collections.
				///  </para>
				/// </returns>
				bool Equals(const KeyCollectionAdapter& collection) const;

				/// <summary>
				///  Determines if the keys between this collection and the given collection do not exactly match.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>
				///   Returns true if at least one key does not match or if the number of keys differ
				///   between the two collections.
				///  </para>
				///  <para>Returns false if the keys in this collection exactly match with the given collection.</para>
				/// </returns>
				bool NotEquals(const KeyCollectionAdapter& collection) const;

				/// <summary>
				///  Determines if all of the keys in this collection match the keys in the given collection.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>Returns true if the keys in this collection exactly match with the given collection.</para>
				///  <para>
				///   Returns false if at least one key does not match or if the number of keys differ
				///   between the two collections.
				///  </para>
				/// </returns>
				bool operator==(const KeyCollectionAdapter& collection) const;

				/// <summary>
				///  Determines if the keys between this collection and the given collection do not exactly match.
				/// </summary>
				/// <param name="collection">The collection to be compared against.</param>
				/// <returns>
				///  <para>
				///   Returns true if at least one key does not match or if the number of keys differ
				///   between the two collections.
				///  </para>
				///  <para>Returns false if the keys in this collection exactly match with the given collection.</para>
				/// </returns>
				bool operator!=(const KeyCollectionAdapter& collection) const;

			private:
				/// <summary>
				///  <para>Reference to the input device object this class wraps.</para>
				///  <para>Used to access the actual Key collection member variable that this class manipulates.</para>
				/// </summary>
				InputDeviceSettings& fSettings;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new input device configuration initialized to its defaults.</summary>
		InputDeviceSettings();

		/// <summary>Creates a new input device configuration providing a copy of the given configuration.</summary>
		/// <param name="settings">The input device configuration to be copied.</param>
		InputDeviceSettings(const InputDeviceSettings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~InputDeviceSettings();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the type of input device this is such as kKeyboard, kJoystick, etc.</summary>
		/// <returns>Returns the type of input device this is.</returns>
		Rtt::InputDeviceType GetType() const;

		/// <summary>Sets the type of input device this is such as kKeyboard, kJoystick, etc.</summary>
		/// <param name="value">The type of input device this is.</param>
		void SetType(const Rtt::InputDeviceType& value);

		/// <summary>
		///  <para>Gets a unique string ID assigned to the device that can be saved to file.</para>
		///  <para>This string ID will persist after restarting the app or after rebooting the system.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the device's unique string ID.</para>
		///  <para>Returns an empty string if the device does not have a permanent string ID assigned to it.</para>
		/// </returns>
		const char* GetPermanentStringId() const;

		/// <summary>
		///  <para>Sets a unique string ID that is assigned to the device that can be saved to file.</para>
		///  <para>This string ID will persist after restarting the app or after rebooting the system.</para>
		/// </summary>
		/// <param name="value">The unique string ID. Can be null or empty string.</param>
		void SetPermanentStringId(const char* value);

		/// <summary>Gets a UTF-8 encoded name of the input device as assigned by the manufacturer.</summary>
		/// <remarks>
		///  If the device is a gamepad or joystick, then this name is typically used to determined
		///  which axis inputs and key/button inputs are what on the device.  Especially when determining
		///  which axis inputs make up the right thumbstick on a gamepad since there is no standard.
		/// </remarks>
		/// <returns>
		///  <para>Returns the device's UTF-8 encoded name as assigned to it by the manufacturer.</para>
		///  <para>Returns an empty string if the product name could not be obtained.</para>
		/// </returns>
		const char* GetProductNameAsUtf8() const;

		/// <summary>Gets a UTF-16 encoded name of the input device as assigned by the manufacturer.</summary>
		/// <remarks>
		///  If the device is a gamepad or joystick, then this name is typically used to determined
		///  which axis inputs and key/button inputs are what on the device.  Especially when determining
		///  which axis inputs make up the right thumbstick on a gamepad since there is no standard.
		/// </remarks>
		/// <returns>
		///  <para>Returns the device's UTF-16 encoded name as assigned to it by the manufacturer.</para>
		///  <para>Returns an empty string if the product name could not be obtained.</para>
		/// </returns>
		const wchar_t* GetProductNameAsUtf16() const;

		/// <summary>Sets the UTF-8 encoded name of the input device as assigned by the manufacturer.</summary>
		/// <param name="value">The UTF-8 encoded name. Can be null or empty string.</param>
		void SetProductName(const char* value);

		/// <summary>Sets the UTF-16 encoded name of the input device as assigned by the manufacturer.</summary>
		/// <param name="value">The UTF-16 encoded name. Can be null or empty string.</param>
		void SetProductName(const wchar_t* value);

		/// <summary>
		///  <para>Gets a UTF-8 encoded descriptive name assigned to the device by the end-user or by the system.</para>
		///  <para>This is typically the product name if the display/alias name could not be obtained.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the device's display name as a UTF-8 encoded string.</para>
		///  <para>Returns an empty string if the display name could not be obtained.</para>
		/// </returns>
		const char* GetDisplayNameAsUtf8() const;

		/// <summary>
		///  <para>Gets a UTF-16 encoded descriptive name assigned to the device by the end-user or by the system.</para>
		///  <para>This is typically the product name if the display/alias name could not be obtained.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the device's display name as a UTF-16 encoded string.</para>
		///  <para>Returns an empty string if the display name could not be obtained.</para>
		/// </returns>
		const wchar_t* GetDisplayNameAsUtf16() const;

		/// <summary>
		///  <para>Sets the descriptive name assigned to the device by the end-user or by the system.</para>
		///  <para>This is typically the product name if the display/alias name could not be obtained.</para>
		/// </summary>
		/// <param name="value">The UTF-8 encoded name. Can be null or empty string.</param>
		void SetDisplayName(const char* value);

		/// <summary>
		///  <para>Sets the descriptive name assigned to the device by the end-user or by the system.</para>
		///  <para>This is typically the product name if the display/alias name could not be obtained.</para>
		/// </summary>
		/// <param name="value">The UTF-16 encoded name. Can be null or empty string.</param>
		void SetDisplayName(const wchar_t* value);

		/// <summary>
		///  <para>Gets a one based player number assigned to the device by the system.</para>
		///  <para>Player numbers are typically assigned to XInput devices that are currently connected.</para>
		///  <para>DirectInput and RawInput devices do not support player numbers.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a one based player number assigned to the device.</para>
		///  <para>Returns zero if a player number is not assigned to the device.</para>
		/// </returns>
		unsigned int GetPlayerNumber() const;

		/// <summary>
		///  <para>Sets a one based player number to the device, if it has one.</para>
		///  <para>This is typically XInput's user index (which is zero based) plus one.</para>
		///  <para>Should be set to zero if a player number is not assigned to the device by the system.</para>
		/// </summary>
		/// <param name="value">
		///  <para>A one based player number assigned to the device.</para>
		///  <para>Set to zero to indicate that a player number is not assigned to the device. This is the default.</para>
		/// </param>
		void SetPlayerNumber(unsigned int value);

		/// <summary>Determines if the input device supports vibrate/rumble functionality.</summary>
		/// <returns>Returns true if the device support vibration feedback. Returns false if not.</returns>
		bool CanVibrate() const;

		/// <summary>Sets whether or not the input device supports vibrate/rumble functionality.</summary>
		/// <param name="value>Set true to indicate the device supports vibration feedback. Set false if not.</param>
		void SetCanVibrate(bool value);

		/// <summary>Gets modifiable access to this input device's axis configuration collection.</summary>
		/// <returns>Returns an object providing modifiable access to the input device's axis collection.</returns>
		InputDeviceSettings::AxisCollectionAdapter GetAxes();

		/// <summary>Gets read-only access to this input device's axis configuration collection.</summary>
		/// <returns>Returns an object providing read-only access to the input device's axis collection.</returns>
		const InputDeviceSettings::AxisCollectionAdapter GetAxes() const;

		/// <summary>Gets modifiable access to this input device's key collection.</summary>
		/// <returns>Returns an object providing modifiable access to the input device's key collection.</returns>
		InputDeviceSettings::KeyCollectionAdapter GetKeys();

		/// <summary>Gets modifiable access to this input device's key collection.</summary>
		/// <returns>Returns an object providing modifiable access to the input device's key collection.</returns>
		const InputDeviceSettings::KeyCollectionAdapter GetKeys() const;

		/// <summary>Copies the given input device settings to this object's settings.</summary>
		/// <param name="settings">The settings to be copied.</param>
		void CopyFrom(const InputDeviceSettings& settings);

		/// <summary>Copies the given input device settings to this object's settings.</summary>
		/// <param name="settings">The settings to be copied.</param>
		void operator=(const InputDeviceSettings& settings);

		/// <summary>Determines if this configuration matches the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations match. Returns false if not.</returns>
		bool Equals(const InputDeviceSettings& value) const;

		/// <summary>Determines if this configuration does not match the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations do not match. Returns false if they do match.</returns>
		bool NotEquals(const InputDeviceSettings& value) const;

		/// <summary>Determines if this configuration matches the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations match. Returns false if not.</returns>
		bool operator==(const InputDeviceSettings& value) const;

		/// <summary>Determines if this configuration does not match the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations do not match. Returns false if they do match.</returns>
		bool operator!=(const InputDeviceSettings& value) const;

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>The type of input device this is such as kJoystick, kGamepad, kKeyboard, etc.</summary>
		Rtt::InputDeviceType fType;

		/// <summary>The device's unique string ID that will persist after rebooting the system.</summary>
		std::string fPermanentStringId;

		/// <summary>The product name assigned to this device by the manufacturer.</summary>
		WinString fProductName;

		/// <summary>Human readable name assigned to this device by the system.</summary>
		WinString fDisplayName;

		/// <summary>
		///  <para>One based player number assigned to the device by the system, such as XInput.</para>
		///  <para>This is normally the XInput user index plus 1 since it is zero based.</para>
		///  <para>Set to zero if a number was not assigned to the device.</para>
		/// </summary>
		unsigned int fPlayerNumber;

		/// <summary>Set true if the device supports vibrate/rumble functionality.</summary>
		bool fCanVibrate;

		/// <summary>Collection of axis configurations owned by this input device. Order matters.</summary>
		std::vector<InputAxisSettings*> fAxisCollection;

		/// <summary>Stores a collection of unique keys owned by this input device. Order matters.</summary>
		std::vector<Key> fKeyCollection;

		#pragma endregion
};

} }	// namespace Interop::Input

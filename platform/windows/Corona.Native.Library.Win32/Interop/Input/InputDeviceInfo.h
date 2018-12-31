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
#include "InputDeviceSettings.h"
#include <vector>


namespace Interop { namespace Input {

class InputAxisInfo;

/// <summary>
///  <para>Provides read-only information about one input device.</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
class InputDeviceInfo
{
	public:
		#pragma region Public AxisCollectionAdapter Class
		/// <summary>
		///  <para>Provides read-only access to an InputDeviceInfo object's axis collection.</para>
		///  <para>This class was designed to pass instances by value efficiently.</para>
		/// </summary>
		class AxisCollectionAdapter
		{
			public:
				/// <summary>
				///  Creates a new object providing read-only access to the given input device's axis collection.
				/// </summary>
				/// <param name="info">Reference to the input device to access the axis collection of.</param>
				AxisCollectionAdapter(const InputDeviceInfo& info);

				/// <summary>Destroys this object.</summary>
				virtual ~AxisCollectionAdapter();

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
				InputAxisInfo* GetByIndex(int index) const;

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
				///  <para>Used to access the actual InputAxisSettings collection member variable.</para>
				/// </summary>
				const InputDeviceInfo& fInputDevice;
		};

		#pragma endregion


		#pragma region Public KeyCollectionAdapter Class
		/// <summary>
		///  <para>Provides read-only access to an InputDeviceInfo object's key collection.</para>
		///  <para>This class was designed to pass instances by value efficiently.</para>
		/// </summary>
		class KeyCollectionAdapter
		{
			public:
				/// <summary>
				///  Creates a new object providing read-only access to the given input device's key collection.
				/// </summary>
				/// <param name="info">Reference to the input device to access the key collection of.</param>
				KeyCollectionAdapter(const InputDeviceInfo& info);

				/// <summary>Destroys this object.</summary>
				virtual ~KeyCollectionAdapter();

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
				///  <para>Used to access the actual key collection member variable.</para>
				/// </summary>
				const InputDeviceInfo& fInputDevice;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>
		///  <para>Creates an immutable object initialized to InputDeviceSettings' defaults.</para>
		///  <para>This type of object is effectively useless.</para>
		/// </summary>
		InputDeviceInfo();

		/// <summary>Creates an immutable object providing a copy of the given input device information.</summary>
		/// <param name="settings">The input device settings to be copied.</param>
		InputDeviceInfo(const InputDeviceSettings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~InputDeviceInfo();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Copies this object's immutable device information to the given mutable device configuration.</summary>
		/// <param name="settings">Reference to the configuration to copy this object device info to.</param>
		void CopyTo(InputDeviceSettings& settings) const;

		/// <summary>Gets the type of input device this is such as kKeyboard, kJoystick, etc.</summary>
		/// <returns>Returns the type of input device this is.</returns>
		Rtt::InputDeviceType GetType() const;

		/// <summary>
		///  <para>Gets a unique string ID assigned to the device that can be saved to file.</para>
		///  <para>This string ID will persist after restarting the app or after rebooting the system.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the device's unique string ID.</para>
		///  <para>Returns an empty string if the device does not have a permanent string ID assigned to it.</para>
		/// </returns>
		const char* GetPermanentStringId() const;

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
		///  <para>Gets a one based player number assigned to the device by the system.</para>
		///  <para>Player numbers are typically assigned to XInput devices that are currently connected.</para>
		///  <para>DirectInput and RawInput devices do not support player numbers.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a one based player number assigned to the device.</para>
		///  <para>Returns zero if a player number is not assigned to the device.</para>
		/// </returns>
		unsigned int GetPlayerNumber() const;

		/// <summary>Determines if the input device supports vibrate/rumble functionality.</summary>
		/// <returns>Returns true if the device support vibration feedback. Returns false if not.</returns>
		bool CanVibrate() const;

		/// <summary>Gets read-only access to this input device's axis configuration collection.</summary>
		/// <returns>Returns an object providing read-only access to the input device's axis collection.</returns>
		InputDeviceInfo::AxisCollectionAdapter GetAxes() const;

		/// <summary>Gets read-only access to this input device's key collection.</summary>
		/// <returns>Returns an object providing read-only access to the input device's key collection.</returns>
		InputDeviceInfo::KeyCollectionAdapter GetKeys() const;

		/// <summary>Determines if this configuration matches the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations match. Returns false if not.</returns>
		bool Equals(const InputDeviceInfo& value) const;

		/// <summary>Determines if this configuration matches the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations match. Returns false if not.</returns>
		bool Equals(const InputDeviceSettings& value) const;

		/// <summary>Determines if this configuration does not match the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations do not match. Returns false if they do match.</returns>
		bool NotEquals(const InputDeviceInfo& value) const;

		/// <summary>Determines if this configuration does not match the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations do not match. Returns false if they do match.</returns>
		bool NotEquals(const InputDeviceSettings& value) const;

		/// <summary>Determines if this configuration matches the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations match. Returns false if not.</returns>
		bool operator==(const InputDeviceInfo& value) const;

		/// <summary>Determines if this configuration matches the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations match. Returns false if not.</returns>
		bool operator==(const InputDeviceSettings& value) const;

		/// <summary>Determines if this configuration does not match the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations do not match. Returns false if they do match.</returns>
		bool operator!=(const InputDeviceInfo& value) const;

		/// <summary>Determines if this configuration does not match the given configuration.</summary>
		/// <param name="value">Reference to the configuration to compare against.</param>
		/// <returns>Returns true if the configurations do not match. Returns false if they do match.</returns>
		bool operator!=(const InputDeviceSettings& value) const;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Assignment operator made private to make this object immutable.</summary>
		void operator=(const InputDeviceInfo& info) {}

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>A copy of the input device settings wrapped by this immutable object.</summary>
		InputDeviceSettings fSettings;

		/// <summary>Collection of immutable axis configurations owned by this input device.</summary>
		std::vector<InputAxisInfo*> fAxisCollection;

		#pragma endregion
};

} }	// namespace Interop::Input

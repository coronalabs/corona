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

package com.ansca.corona.input;


/** Stores a collection of input devices. */
public class InputDeviceInterfaceCollection implements Iterable<InputDeviceInterface>, Cloneable {
	/** Stores a collection of AxisInfo objects. */
	private java.util.ArrayList<InputDeviceInterface> fCollection;


	/** Creates a new collection for storing input devices. */
	public InputDeviceInterfaceCollection() {
		fCollection = new java.util.ArrayList<InputDeviceInterface>();
	}

	/**
	 * Creates a shallow copy of this collection.
	 * <p>
	 * The item contained in this collection are not cloned.
	 * @return Returns a copy of this collection.
	 */
	@Override
	public InputDeviceInterfaceCollection clone() {
		InputDeviceInterfaceCollection clone = null;
		try {
			clone = (InputDeviceInterfaceCollection)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Adds the given item to the end of the collection.
	 * @param item The object to be added to the collection. Cannot be null.
	 */
	public void add(InputDeviceInterface item) {
		// Validate.
		if (item == null) {
			return;
		}

		// Do not continue if the given item already exists in the collection.
		if (fCollection.indexOf(item) >= 0) {
			return;
		}

		// Add the given item to the collection.
		fCollection.add(item);
	}

	/**
	 * Adds the collection of items to the end of this collection.
	 * <p>
	 * Items will not be added the given collection contains references that already exist in this collection.
	 * @param collection The collection of items to be added.
	 */
	public void addAll(Iterable<InputDeviceInterface> collection) {
		// Validate.
		if (collection == null) {
			return;
		}

		// Add the collection of items to this collection.
		for (InputDeviceInterface item : collection) {
			add(item);
		}
	}

	/**
	 * Gets the zero based index for the given item reference in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns a zero based index to the referenced object in this collection.
	 *         <p>
	 *         Returns -1 if the object reference could not be found or if given null.
	 */
	public int indexOf(InputDeviceInterface item) {
		// Validate.
		if (item == null) {
			return -1;
		}

		// Fetch the index of the given item.
		return fCollection.indexOf(item);
	}

	/**
	 * Gets an item from this collection by its zero based index.
	 * @param index The zero based index to the item in the collection.
	 * @return Returns a reference to the index item in the collection.
	 *         <p>
	 *         Returns null if the given index was out of range.
	 */
	public InputDeviceInterface getByIndex(int index) {
		// Validate.
		if ((index < 0) || (index >= fCollection.size())) {
			return null;
		}

		// Fetch the item by index.
		return fCollection.get(index);
	}

	/**
	 * Fetches an input device by its unique Corona assigned device ID.
	 * @param id The unique ID assigned to the device by Corona.
	 * @return Returns a reference to the item having the given ID.
	 *         <p>
	 *         Returns null if the given ID was not found.
	 */
	public InputDeviceInterface getByCoronaDeviceId(int id) {
		for (InputDeviceInterface item : fCollection) {
			if (item.getCoronaDeviceId() == id) {
				return item;
			}
		}
		return null;
	}

	/**
	 * Fetches an input device by its unique Android device ID and its device type.
	 * @param id The unique ID assigned to the device by Android.
	 * @param type The device type such as KEYBOARD, MOUSE, JOYSTICK, etc.
	 * @return Returns a reference to the item having the given ID and device type.
	 *         <p>
	 *         Returns null if the given ID and device type was not found.
	 */
	public InputDeviceInterface getByAndroidDeviceIdAndType(int id, InputDeviceType type) {
		if (type != null) {
			for (InputDeviceInterface item : fCollection) {
				InputDeviceInfo deviceInfo = item.getDeviceInfo();
				if (deviceInfo.hasAndroidDeviceId() &&
					(deviceInfo.getAndroidDeviceId() == id) &&
					deviceInfo.getInputSources().contains(type))
				{
					return item;
				}
			}
		}
		return null;
	}

	/**
	 * Fetches an input device by its permanent string ID and device type.
	 * <p>
	 * Note that it is possible for more than one input device to have the same permanent string ID
	 * and device type. In this case, this method will return the first device found in the collection.
	 * @param stringId The device's permanent string ID.
	 * @param type The device type such as KEYBOARD, MOUSE, JOYSTICK, etc.
	 * @return Returns a reference to the item having the given ID and device type.
	 *         <p>
	 *         Returns null if the given ID and device type was not found.
	 */
	public InputDeviceInterface getByPermanentStringIdAndType(String stringId, InputDeviceType type) {
		if ((stringId != null) && (stringId.length() > 0) && (type != null)) {
			for (InputDeviceInterface item : fCollection) {
				InputDeviceInfo deviceInfo = item.getDeviceInfo();
				if (deviceInfo.hasPermanentStringId() &&
				    deviceInfo.getPermanentStringId().equals(stringId) &&
				    deviceInfo.getInputSources().contains(type))
				{
					return item;
				}
			}
		}
		return null;
	}

	/**
	 * Fetches an input device by its permanent string ID, device type, and display name.
	 * @param stringId The device's permanent string ID.
	 * @param type The device type such as KEYBOARD, MOUSE, JOYSTICK, etc.
	 * @param displayName The display/alias name assigned to the device.
	 * @return Returns a reference to the item having the given ID, device type, and display name.
	 *         <p>
	 *         Returns null if a device having the given settings was not found.
	 */
	public InputDeviceInterface getByPermanentStringIdAndTypeAndDisplayName(
		String stringId, InputDeviceType type, String displayName)
	{
		// Validate arguments.
		if ((stringId == null) || (stringId.length() <= 0)) {
			return null;
		}
		if (type == null) {
			return null;
		}
		if ((displayName == null) || (displayName.length() <= 0)) {
			return null;
		}

		// Fetch the device having the given values.
		for (InputDeviceInterface item : fCollection) {
			InputDeviceInfo deviceInfo = item.getDeviceInfo();
			if (deviceInfo.hasPermanentStringId() &&
			    deviceInfo.getPermanentStringId().equals(stringId) &&
			    deviceInfo.getInputSources().contains(type) &&
			    deviceInfo.getDisplayName().equals(displayName))
			{
				return item;
			}
		}
		return null;
	}

	/**
	 * Fetches an input device by its device type and display name.
	 * <p>
	 * Note that it is possible for more than one input device to have the same device type and display name.
	 * In this case, this method will return the first device found in the collection.
	 * @param type The device type such as KEYBOARD, MOUSE, JOYSTICK, etc.
	 * @param displayName The display/alias name assigned to the device.
	 * @return Returns a reference to the item having the given device type and display name.
	 *         <p>
	 *         Returns null if a device having the given settings was not found.
	 */
	public InputDeviceInterface getByTypeAndDisplayName(InputDeviceType type, String displayName) {
		// Validate arguments.
		if (type == null) {
			return null;
		}
		if ((displayName == null) || (displayName.length() <= 0)) {
			return null;
		}

		// Fetch the device having the given values.
		for (InputDeviceInterface item : fCollection) {
			InputDeviceInfo deviceInfo = item.getDeviceInfo();
			if (deviceInfo.getInputSources().contains(type) && deviceInfo.getDisplayName().equals(displayName)) {
				return item;
			}
		}
		return null;
	}

	/**
	 * Fetches all input devices having the given connection state.
	 * @param state The connection state such as CONNECTED, DISCONNECTED, etc.
	 *              Cannot be null or else an exception will be thrown.
	 * @return Returns a collection of input devices have the given connection state.
	 *         <p>
	 *         Returns an empty collection if no devices match the given connection state.
	 */
	public InputDeviceInterfaceCollection getBy(ConnectionState state) {
		// Validate.
		if (state == null) {
			throw new NullPointerException();
		}

		// Copy all devices having the given connection state to a new collection and return it.
		InputDeviceInterfaceCollection collection = new InputDeviceInterfaceCollection();
		for (InputDeviceInterface item : fCollection) {
			if (item.getConnectionState() == state) {
				collection.add(item);
			}
		}
		return collection;
	}

	/**
	 * Determines if the given item reference exists in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns true if the given item reference exists in the collection. Returns false if not.
	 */
	public boolean contains(InputDeviceInterface item) {
		if (item == null) {
			return false;
		}
		return fCollection.contains(item);
	}

	/**
	 * Determines if the given Android device ID is assigned to at least one of the
	 * items in this collection.
	 * @param id The unique ID assigned to the device by Android.
	 * @return Returns true if the given ID exists in the collection. Returns false if not.
	 */
	public boolean containsAndroidDeviceId(int id) {
		for (InputDeviceInterface item : fCollection) {
			InputDeviceInfo deviceInfo = item.getDeviceInfo();
			if (deviceInfo.hasAndroidDeviceId() && (deviceInfo.getAndroidDeviceId() == id)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Removes the given item from the collection by reference.
	 * @param item The item to be removed from the collection by reference.
	 * @return Returns true if the given item was successfully removed.
	 *         <p>
	 *         Returns false if not found or if given a null reference.
	 */
	public boolean remove(InputDeviceInterface item) {
		// Validate.
		if (item == null) {
			return false;
		}
		
		// Remove the given item from the collection.
		return fCollection.remove(item);
	}

	/** Removes all items in the collection. */
	public void clear() {
		fCollection.clear();
	}

	/**
	 * Gets the number of items in this collection.
	 * @return Returns the number of items in this collection.
	 *         <p>
	 *         Returns zero if the collection is empty.
	 */
	public int size() {
		return fCollection.size();
	}

	/**
	 * Gets an iterator used to iterate through all elements in this collection.
	 * @return Returns an iterator for the elements in this collection.
	 */
	@Override
	public java.util.Iterator<InputDeviceInterface> iterator() {
		return fCollection.iterator();
	}
}

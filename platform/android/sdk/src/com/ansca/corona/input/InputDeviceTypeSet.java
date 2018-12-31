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


/** Stores a set of unique input device types. This type of collection does not allow duplicates to be stored. */
public class InputDeviceTypeSet implements Iterable<InputDeviceType>, Cloneable {
	/** Stores a collection of InputDeviceType objects. */
	private java.util.LinkedHashSet<InputDeviceType> fCollection;


	/** Creates a new collection for storing input device types. */
	public InputDeviceTypeSet() {
		fCollection = new java.util.LinkedHashSet<InputDeviceType>();
	}

	/**
	 * Creates a shallow copy of this collection.
	 * <p>
	 * The item contained in this collection are not cloned.
	 * @return Returns a copy of this collection.
	 */
	@Override
	public InputDeviceTypeSet clone() {
		InputDeviceTypeSet clone = null;
		try {
			clone = (InputDeviceTypeSet)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Adds the given item to the end of the collection.
	 * @param item The object to be added to the collection. Cannot be null.
	 *             <p>
	 *             If the given object already exists in the collection, then it will not be added.
	 */
	public void add(InputDeviceType item) {
		// Validate.
		if (item == null) {
			return;
		}

		// Add the given item to the collection.
		fCollection.add(item);
	}

	/**
	 * Adds the collection of items to the end of this collection.
	 * <p>
	 * Items in the given collection will not be added if they already exist in this collection.
	 * @param collection The collection of items to be added.
	 */
	public void addAll(Iterable<InputDeviceType> collection) {
		// Validate.
		if (collection == null) {
			return;
		}

		// Add the collection of items to this collection.
		for (InputDeviceType item : collection) {
			add(item);
		}
	}

	/**
	 * Determines if the given item reference exists in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns true if the given item reference exists in the collection. Returns false if not.
	 */
	public boolean contains(InputDeviceType item) {
		if (item == null) {
			return false;
		}
		return fCollection.contains(item);
	}

	/**
	 * Removes the given item from the collection by reference.
	 * @param item The item to be removed from the collection by reference.
	 * @return Returns true if the given item was successfully removed.
	 *         <p>
	 *         Returns false if not found or if given a null reference.
	 */
	public boolean remove(InputDeviceType item) {
		// Validate.
		if (item == null) {
			return false;
		}
		
		// Remove the given item from the collection.
		return fCollection.remove(item);
	}

	/**
	 * Removes all items in the given collection from this collection by reference.
	 * @param collection Collection of items to remove by reference.
	 * @return Returns true if at least one item has been removed from this collection.
	 *         <p>
	 *         Returns false if no items have been removed from this collection,
	 *         if given an empty collection, or if given a null argument.
	 */
	public boolean removeAll(Iterable<InputDeviceType> collection) {
		// Validate.
		if (collection == null) {
			return false;
		}

		// Remove all of the given items from this collection.
		boolean hasRemovedAnItem = false;
		for (InputDeviceType item : collection) {
			hasRemovedAnItem |= remove(item);
		}
		return hasRemovedAnItem;
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
	public java.util.Iterator<InputDeviceType> iterator() {
		return fCollection.iterator();
	}

	/**
	 * Determines if the given set contains the same contents as this set.
	 * @param collection Reference to the set to compare with.
	 * @return Returns true if the given set is of the same size as this set and it contains the
	 *         same items as this set.
	 *         <p>
	 *         Returns false if the given set contains different items than this set
	 *         or if given a null argument.
	 */
	public boolean equals(InputDeviceTypeSet collection) {
		if (collection == null) {
			return false;
		}
		return fCollection.equals(collection.fCollection);
	}

	/**
	 * Determines if the given object is of type InputDeviceTypeSet and it contains the same
	 * items as this set.
	 * @param value Reference to an InputDeviceTypeSet object.
	 * @return Returns true if the given set is of the same size as this set and it contains the
	 *         same items as this set.
	 *         <p>
	 *         Returns false if the given argument was null, if the argument is not of type
	 *         InputDeviceTypeSet, or if the given set does not match the items contained
	 *         in this set.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof InputDeviceTypeSet) == false) {
			return false;
		}
		return equals((InputDeviceTypeSet)value);
	}

	/**
	 * Creates and returns an Android "sources" bit field which has a flag set for every
	 * input device type that is in this collection. Intended to match what is returned
	 * by the Android InputDevice.getSources() method.
	 * @return Returns a bit field with flags matching SOURCE constants contained in the
	 *         Android "InputDevice" class.
	 *         <p>
	 *         Returns 0 if this collection is empty or if this collection contains an
	 *         UNKNOWN input device type.
	 */
	public int toAndroidSourcesBitField() {
		int sources = 0;
		for (InputDeviceType deviceType : fCollection) {
			sources |= deviceType.toAndroidSourceId();
		}
		return sources;
	}
}

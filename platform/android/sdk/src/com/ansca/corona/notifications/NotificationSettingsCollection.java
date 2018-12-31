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

package com.ansca.corona.notifications;


/** Stores a collection of NotificationSettings derived objects. */
public class NotificationSettingsCollection<T extends NotificationSettings> implements Iterable<T> {
	/** Stores a collection of notification settings objects. */
	private java.util.ArrayList<T> fCollection;


	/** Creates a new collection object for storing notification settings. */
	public NotificationSettingsCollection() {
		fCollection = new java.util.ArrayList<T>();
	}

	/**
	 * Adds the given item to the end of the collection.
	 * @param item The object to be added to the collection. Cannot be null.
	 */
	public void add(T item) {
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
	 * Gets the zero based index for the given item reference in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns a zero based index to the referenced object in this collection.
	 *         <p>
	 *         Returns -1 if the object reference could not be found or if given null.
	 */
	public int indexOf(T item) {
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
	public T getByIndex(int index) {
		// Validate.
		if ((index < 0) || (index >= fCollection.size())) {
			return null;
		}

		// Fetch the item by index.
		return fCollection.get(index);
	}

	/**
	 * Gets an item from this collection by its unique notification ID.
	 * @param id Unique integer ID assigned to the item.
	 * @return Returns a reference to the item having the given notification ID.
	 *         If this collection contains multiple items assigned the same ID, then this method
	 *         will return the first matching item in the collection.
	 *         <p>
	 *         Returns null if no items in the collection are assigned the given ID.
	 */
	public T getById(int id) {
		for (T item : fCollection) {
			if ((item != null) && (item.getId() == id)) {
				return item;
			}
		}
		return null;
	}

	/**
	 * Removes the given item from the collection by reference.
	 * @param item The item to be removed from the collection by reference. Cannot be null.
	 * @return Returns true if the given item was successfully removed.
	 *         <p>
	 *         Returns false if not found or if given a null reference.
	 */
	public boolean remove(T item) {
		// Validate.
		if (item == null) {
			return false;
		}
		
		// Remove the given item from the collection.
		return fCollection.remove(item);
	}

	/**
	 * Removes an item in the collection by its unique notification ID.
	 * <p>
	 * Note that if multiple items in this list have the same ID, then they will all be removed.
	 * @param id Unique integer ID assigned to the item.
	 * @return Returns true if the item was successfully removed.
	 *         <p>
	 *         Returns false if the given ID was not found in the collection.
	 */
	public boolean removeById(int id) {
		boolean wasRemoved = false;
		while (remove(getById(id))) {
			wasRemoved = true;
		}
		return wasRemoved;
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
	public java.util.Iterator<T> iterator() {
		return fCollection.iterator();
	}
}

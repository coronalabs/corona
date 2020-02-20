//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/** Stores a collection of touch trackers. */
public class TouchTrackerCollection implements Iterable<TouchTracker>, Cloneable {
	/** Stores a collection of touch tracker objects. */
	private java.util.ArrayList<TouchTracker> fCollection;


	/** Creates a new collection for storing touch trackers. */
	public TouchTrackerCollection() {
		fCollection = new java.util.ArrayList<TouchTracker>();
	}

	/**
	 * Creates a deep copy of this collection.
	 * @return Returns a deep copy of this collection.
	 */
	@Override
	public TouchTrackerCollection clone() {
		TouchTrackerCollection clone = new TouchTrackerCollection();
		for (TouchTracker item : fCollection) {
			clone.fCollection.add(item.clone());
		}
		return clone;
	}

	/**
	 * Adds the given item to the end of the collection.
	 * @param item The object to be added to the collection. Cannot be null.
	 */
	public void add(TouchTracker item) {
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
	public int indexOf(TouchTracker item) {
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
	public TouchTracker getByIndex(int index) {
		// Validate.
		if ((index < 0) || (index >= fCollection.size())) {
			return null;
		}

		// Fetch the item by index.
		return fCollection.get(index);
	}

	/**
	 * Gets an item from this collection by its unique touch ID.
	 * @param id Unique touch ID assigned to the touch tracker. This ID is used by Corona's touch event in Lua.
	 * @return Returns a reference to the item having the given touch ID.
	 *         If this collection contains multiple items assigned the same ID, then this method
	 *         will return the first matching item in the collection.
	 *         <p>
	 *         Returns null if no items in the collection are assigned the given ID.
	 */
	public TouchTracker getByTouchId(int id) {
		for (TouchTracker item : fCollection) {
			if ((item != null) && (item.getTouchId() == id)) {
				return item;
			}
		}
		return null;
	}

	/**
	 * Gets a touch tracker from this collection by its assigned device ID and pointer ID.
	 * @param deviceId The ID of the device the touch tracker's events come from.
	 * @param pointerId The ID of the touch pointer that the tracker's events come from.
	 * @return Returns a reference to the item having the given ID pair.
	 *         If this collection contains multiple items assigned the same ID pair, then this method
	 *         will return the first matching item in the collection.
	 *         <p>
	 *         Returns null if no items in the collection are assigned the given ID pair.
	 */
	public TouchTracker getByDeviceAndPointerId(int deviceId, int pointerId) {
		for (TouchTracker item : fCollection) {
			if ((item != null) && (item.getDeviceId() == deviceId) && (item.getPointerId() == pointerId)) {
				return item;
			}
		}
		return null;
	}

	/**
	 * Determines if the given item reference exists in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns true if the given item reference exists in the collection. Returns false if not.
	 */
	public boolean contains(TouchTracker item) {
		if (item == null) {
			return false;
		}
		return fCollection.contains(item);
	}

	/**
	 * Determines if a touch tracker assigned the given touch ID exists in this collection.
	 * @param id Unique touch ID assigned to the touch tracker. This ID is used by Corona's touch event in Lua.
	 * @return Returns true if an item having the given ID exists. Returns false if not.
	 */
	public boolean containsTouchId(int id) {
		return (getByTouchId(id) != null);
	}

	/**
	 * Determines if a touch tracker assigned the given device ID and pointer ID exists in this collection.
	 * @param deviceId The ID of the device the touch tracker's events come from.
	 * @param pointerId The ID of the touch pointer that the tracker's events come from.
	 * @return Returns true if an item having the given ID pair exists. Returns false if not.
	 */
	public boolean containsDeviceAndPointerId(int deviceId, int pointerId) {
		return (getByDeviceAndPointerId(deviceId, pointerId) != null);
	}

	/**
	 * Removes the given item from the collection by reference.
	 * @param item The item to be removed from the collection by reference. Cannot be null.
	 * @return Returns true if the given item was successfully removed.
	 *         <p>
	 *         Returns false if not found or if given a null reference.
	 */
	public boolean remove(TouchTracker item) {
		// Validate.
		if (item == null) {
			return false;
		}
		
		// Remove the given item from the collection.
		return fCollection.remove(item);
	}

	/**
	 * Removes an item in the collection by its unique touch ID.
	 * <p>
	 * Note that if multiple items in this list have the same touch ID, then they will all be removed.
	 * @param id Unique touch ID assigned to the touch tracker. This ID is used by Corona's touch event in Lua.
	 * @return Returns true if the item was successfully removed.
	 *         <p>
	 *         Returns false if the given ID was not found in the collection.
	 */
	public boolean removeByTouchId(int id) {
		boolean wasRemoved = false;
		while (remove(getByTouchId(id))) {
			wasRemoved = true;
		}
		return wasRemoved;
	}

	/**
	 * Removes all items in the collection assigned the given device ID.
	 * @param id The ID of the device the touch tracker's events come from.
	 * @return Returns true if at least one item in the collection was successfully removed.
	 *         <p>
	 *         Returns false if the given ID was not found in the collection.
	 */
	public boolean removeByDeviceId(int id) {
		boolean wasRemoved = false;
		for (int index = fCollection.size() - 1; index >= 0; index--) {
			TouchTracker item = fCollection.get(index);
			if (item.getDeviceId() == id) {
				fCollection.remove(index);
				wasRemoved = true;
			}
		}
		return wasRemoved;
	}

	/**
	 * Removes an item in the collection by its assigned device ID and pointer ID.
	 * <p>
	 * Note that if multiple items in this list have the same ID pair, then they will all be removed.
	 * @param deviceId The ID of the device the touch tracker's events come from.
	 * @param pointerId The ID of the touch pointer that the tracker's events come from.
	 * @return Returns true if the item was successfully removed.
	 *         <p>
	 *         Returns false if the given ID pair was not found in the collection.
	 */
	public boolean removeByDeviceAndPointerId(int deviceId, int pointerId) {
		boolean wasRemoved = false;
		while (remove(getByDeviceAndPointerId(deviceId, pointerId))) {
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
	public java.util.Iterator<TouchTracker> iterator() {
		return fCollection.iterator();
	}
}

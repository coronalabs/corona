//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/** Stores a collection of axis information objects. */
public class AxisInfoCollection implements Iterable<AxisInfo> {
	/** Stores a collection of AxisInfo objects. */
	private java.util.ArrayList<AxisInfo> fCollection;


	/** Creates a new collection for storing axis information. */
	public AxisInfoCollection() {
		fCollection = new java.util.ArrayList<AxisInfo>();
	}

	/**
	 * Adds the given item to the end of the collection.
	 * @param item The object to be added to the collection. Cannot be null.
	 */
	public void add(AxisInfo item) {
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
	public void addAll(Iterable<AxisInfo> collection) {
		// Validate.
		if (collection == null) {
			return;
		}

		// Add the collection of items to this collection.
		for (AxisInfo item : collection) {
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
	public int indexOf(AxisInfo item) {
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
	public AxisInfo getByIndex(int index) {
		// Validate.
		if ((index < 0) || (index >= fCollection.size())) {
			return null;
		}

		// Fetch the item by index.
		return fCollection.get(index);
	}

	/**
	 * Determines if the given item reference exists in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns true if the given item reference exists in the collection. Returns false if not.
	 */
	public boolean contains(AxisInfo item) {
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
	public boolean remove(AxisInfo item) {
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
	public java.util.Iterator<AxisInfo> iterator() {
		return fCollection.iterator();
	}
}

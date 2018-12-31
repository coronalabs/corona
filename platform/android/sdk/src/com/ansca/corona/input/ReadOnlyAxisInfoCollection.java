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


/** Collection which wraps and provides read-only access to an existing AxisInfoCollection instance. */
public class ReadOnlyAxisInfoCollection implements Iterable<AxisInfo> {
	/** Reference to the mutable axis information collection that this collection wraps. */
	private AxisInfoCollection fCollection;


	/**
	 * Create a read-only collection wrapping the given mutable collection.
	 * @param collection Reference to an existing axis info collection to be wrapped by this collection.
	 *                   <p>
	 *                   Setting this to null will create an empty read-only collection.
	 */
	public ReadOnlyAxisInfoCollection(AxisInfoCollection collection) {
		// If given collection is null, then create an empty collection for this collection to wrap.
		if (collection == null) {
			collection = new AxisInfoCollection();
		}

		// Store a reference to the mutable collection.
		fCollection = collection;
	}

	/**
	 * Gets the zero based index for the given item reference in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns a zero based index to the referenced object in this collection.
	 *         <p>
	 *         Returns -1 if the object reference could not be found or if given null.
	 */
	public int indexOf(AxisInfo item) {
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
		return fCollection.getByIndex(index);
	}

	/**
	 * Determines if the given item reference exists in this collection.
	 * @param item The item to search for by reference.
	 * @return Returns true if the given item reference exists in the collection. Returns false if not.
	 */
	public boolean contains(AxisInfo item) {
		return fCollection.contains(item);
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

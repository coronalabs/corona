//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/**
 * Collection which wraps and provides read-only access to a mutable InputDeviceTypeSet collection.
 * <p>
 * This type of collection does not allow duplicates InputDeviceType objects to be stored.
 */
public class ReadOnlyInputDeviceTypeSet implements Iterable<InputDeviceType> {
	/** Reference to the mutable input device type set. */
	private InputDeviceTypeSet fCollection;


	/**
	 * Creates a read-only collection wrapping the given mutable collection.
	 * @param set Reference to the collection to be wrapped.
	 *            <p>
	 *            Setting this to null will create an empty read-only collection.
	 */
	public ReadOnlyInputDeviceTypeSet(InputDeviceTypeSet set) {
		// If given collection is null, then create an empty collection for this collection to wrap.
		if (set == null) {
			set = new InputDeviceTypeSet();
		}

		// Store a reference to the mutable collection.
		fCollection = set;
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
		return fCollection.toAndroidSourcesBitField();
	}
}

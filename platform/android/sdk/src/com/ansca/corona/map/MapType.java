//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;


/**
 * Indicates the type of map to be shown in a map view such as "standard", "satellite", or "hybrid".
 * Provides string IDs matching map types used by the native side of Corona.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public class MapType {
	/** The unique string ID for this map type. This string is never localized. */
	private String fInvariantName;

	
	/**
	 * Creates a new map type object with the given string ID.
	 * @param invariantName Unique string ID matching a map type from the native side of Corona.
	 */
	private MapType(String invariantName) {
		fInvariantName = invariantName;
	}
	

	/** Displays a street map without satellite imagery. */
	public static final MapType STANDARD = new MapType("standard");

	/** Displays a map with satellite imagery without street information. */
	public static final MapType SATELLITE = new MapType("satellite");
	
	/** Displays a map with satellite imagery with street information overlaid on top of it. */
	public static final MapType HYBRID = new MapType("hybrid");

	
	/**
	 * Gets the unique string ID matching the map types from the native side of Corona.
	 * @return Returns this map type's unqiue string ID.
	 */
	public String toInvariantString() {
		return fInvariantName;
	}
	
	/**
	 * Gets a map type matching the given invariant string ID.
	 * @param value Unique string ID matching a map type from the native side of Corona.
	 * @return Returns a map type object matching the given invariant string.
	 *         <p>
	 *         Returns null if the given string ID is unknown.
	 */
	public static MapType fromInvariantString(String value) {
		// Return a pre-allocated object matching the given integer ID via reflection.
		try {
			for (java.lang.reflect.Field field : MapType.class.getDeclaredFields()) {
				if (field.getType().equals(MapType.class)) {
					MapType mapType = (MapType)field.get(null);
					if (mapType.fInvariantName.equals(value)) {
						return mapType;
					}
				}
			}
		}
		catch (Exception ex) { }
		
		// The given string ID is unknown.
		return null;
	}
}

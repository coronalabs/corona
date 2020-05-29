//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/**
 * Stores system ticks which is a constantly increasing number of milliseconds since
 * system bootup, including sleep time. Ticks are unaffected by clock changes.
 * <p>
 * The stored ticks value will eventually overflow and become negative. This class will
 * correctly handle negative ticks when performing arithmetic and comparison operations.
 * For example, large negative tick values will be considered larger tha large postive values.
 * <p>
 * Instances of this class are immutable.
 */
public class Ticks implements Comparable {
	/** The number of milliseconds that have elapsed since system bootup. */
	private long fMilliseconds;
	
	
	/**
	 * Creates a copy of the given ticks object.
	 * @param value The ticks object to be copied.
	 */
	public Ticks(Ticks value) {
		if (value != null) {
			fMilliseconds = value.fMilliseconds;
		}
	}
	
	/**
	 * Creates a new ticks object with the given integer value.
	 * <p>
	 * This constructor was made private to force calling code to generate Ticks object via its public methods.
	 * @param value Integer value representing system ticks as milliseconds since system bootup.
	 */
	private Ticks(long value) {
		fMilliseconds = value;
	}
	
	/**
	 * Adds the given time span to this ticks object and returns the result.
	 * <p>
	 * This Ticks object is immutable and will not be changed by this operation.
	 * @param value The time span to be added to this object. A null object will be treated as a zero duration.
	 * @return Returns a new Ticks object containing the added result.
	 */
	public Ticks add(TimeSpan value) {
		if (value == null) {
			return this;
		}
		return new Ticks(fMilliseconds + (long)value.getTotalMilliseconds());
	}
	
	/**
	 * Adds the given milliseconds to this ticks object and returns the result.
	 * <p>
	 * This Ticks object is immutable and will not be changed by this operation.
	 * @param value The number of milliseconds to be added. Set to a negative number to subtract.
	 * @return Returns a new Ticks object containing the added result.
	 */
	public Ticks addMilliseconds(long value) {
		if (value == 0) {
			return this;
		}
		return new Ticks(fMilliseconds + value);
	}
	
	/**
	 * Adds the given seconds to this ticks object and returns the result.
	 * <p>
	 * This Ticks object is immutable and will not be changed by this operation.
	 * @param value The number of seconds to be added. Set to a negative number to subtract.
	 * @return Returns a new Ticks object containing the added result.
	 */
	public Ticks addSeconds(long value) {
		if (value == 0) {
			return this;
		}
		return new Ticks(fMilliseconds + (value * 1000));
	}
	
	/**
	 * Subtracts this objects ticks with the given ticks and returns the result as a time span.
	 * @param value The ticks to be subtracted from this object's ticks.
	 */
	public TimeSpan subtract(Ticks value) {
		// Get the milliseconds from the given ticks object.
		long milliseconds = 0;
		if (value != null) {
			milliseconds = value.fMilliseconds;
		}
		
		// You cannot negate the min integer value. Increment it by one before doing so.
		if (milliseconds == Long.MIN_VALUE) {
			milliseconds++;
		}
		
		// Return the result of the subtraction as a time span.
		return TimeSpan.fromMilliseconds(fMilliseconds - milliseconds);
	}
	
	/**
	 * Determines if this ticks object matches the given ticks.
	 * @param value The ticks object to be compare with this instance.
	 * @return Returns true if ticks equal. Returns false if not or if given a null object.
	 */
	public boolean equals(Ticks value) {
		if (value == null) {
			return false;
		}
		return (fMilliseconds == value.fMilliseconds);
	}
	
	/**
	 * Determines if this ticks object matches the given object.
	 * @param value The ticks object to be compare with this instance.
	 * @return Returns true if ticks equal.
	 *         Returns false if they do not equal, if given null, or if given object is not of type Ticks.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof Ticks) == false) {
			return false;
		}
		return equals((Ticks)value);
	}
	
	/**
	 * Compares this ticks object with the given ticks.
	 * @param value The ticks to be compared with this object.
	 * @return Returns a positive number if this object is greater than the given object.
	 *         Returns zero if this object equals the given object.
	 *         Returns a negative number if this object is less than the given object.
	 */
	public int compareTo(Ticks value) {
		// Return "greater" if given a null object.
		if (value == null) {
			return 1;
		}
		
		// Get the ticks object's raw integer value.
		long milliseconds = value.fMilliseconds;
		
		// You cannot negate the min integer value. Increment it by one before doing so.
		if (milliseconds == Long.MIN_VALUE) {
			milliseconds++;
		}
		
		// Compare ticks.
		long result = fMilliseconds - milliseconds;
		if (result < 0) {
			return -1;				// This object is lesser.
		}
		else if (result == 0) {
			return 0;				// Objects are equal.
		}
		return 1;					// This object is greater.
	}
	
	/**
	 * Compares this ticks object with the given object.
	 * @param value The object to be compared with this object.
	 * @return Returns a positive number if this object is greater than the given object.
	 *         Returns zero if this object equals the given object.
	 *         Returns a negative number if this object is less than the given object.
	 */
	@Override
	public int compareTo(Object value) {
		// Return "greater" if given a null or invalid object.
		if ((value instanceof Ticks) == false) {
			return 1;
		}
		
		// Compare objects.
		return compareTo((Ticks)value);
	}
	
	/**
	 * Gets an integer hash code for this object.
	 * @return Returns this object's hash code.
	 */
	@Override
	public int hashCode() {
		return (int)(fMilliseconds ^ (fMilliseconds >>> 32));
	}
	
	/**
	 * Gets the ticks value in string form.
	 * @return Returns a string representing the ticks numeric value.
	 */
	@Override
	public String toString() {
		return Long.toString(fMilliseconds);
	}
	
	/**
	 * Gets the ticks to long integer form representing the number of milliseconds since bootup.
	 * @return Returns the ticks in long integer form.
	 */
	public long toLong() {
		return fMilliseconds;
	}
	
	/**
	 * Creates a new ticks object using the integer value returned by a Ticks.toLong() method.
	 * @param value A ticks value in long integer form returned by a Ticks.toLong() method.
	 * @return Returns a new ticks object set to the given ticks value.
	 */
	public static Ticks fromLong(long value) {
		return new Ticks(value);
	}
	
	/**
	 * Gets the current time in ticks.
	 * @return Returns a new ticks object set to current time.
	 */
	public static Ticks fromCurrentTime() {
		return new Ticks(android.os.SystemClock.elapsedRealtime());
	}
}

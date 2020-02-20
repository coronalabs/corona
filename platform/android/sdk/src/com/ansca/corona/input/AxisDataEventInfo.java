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
 * Provides information about an axis and the data it has received.
 * <p>
 * Instances of this class are immutable.
 */
public class AxisDataEventInfo {
	/** Reference to the device that the axis belongs to. */
	private InputDeviceInfo fDeviceInfo;

	/** Zero based index to the axis that received data. */
	private int fAxisIndex;

	/** The axis data that was received. */
	private AxisDataPoint fDataPoint;


	/**
	 * Creates a new information about about data received for one axis input.
	 * @param deviceInfo Reference to the device that the axis belongs to. Cannot be null.
	 * @param axisIndex Zero based index to the axis that received data.
	 *                  This index must reference the axis stored in argument "deviceInfo".
	 * @param dataPoint The axis data that was received. Cannot be null.
	 */
	public AxisDataEventInfo(InputDeviceInfo deviceInfo, int axisIndex, AxisDataPoint dataPoint) {
		if ((deviceInfo == null) || (dataPoint == null)) {
			throw new NullPointerException();
		}
		fDeviceInfo = deviceInfo;
		fAxisIndex = axisIndex;
		fDataPoint = dataPoint;
	}

	/**
	 * Gets the input device that the axis belongs to.
	 * @return Returns a reference to the axis' device.
	 */
	public InputDeviceInfo getDeviceInfo() {
		return fDeviceInfo;
	}

	/**
	 * Gets a zero based index to the axis that received data.
	 * <p>
	 * This index can be passed into the "getDeviceInfo().getAxes().getByIndex()" method
	 * to fetch the axis' information.
	 * @return Returns the axis' zero based index.
	 */
	public int getAxisIndex() {
		return fAxisIndex;
	}

	/**
	 * Gets the data that was received from the axis.
	 * @return Returns the axis data.
	 */
	public AxisDataPoint getDataPoint() {
		return fDataPoint;
	}
}

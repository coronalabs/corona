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
 * Indicates an input device's current connection state such as CONNECTED, DISCONNECTED, etc.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static fields.
 */
public class ConnectionState {
	/** Unique integer value matching an ID from Corona's InputDeviceConnectState class in C++. */
	private int fCoronaIntegerId;

	/** Unique string matching a string ID from Corona's InputDeviceConnectState class in C++. */
	private String fCoronaStringId;


	/** Indicates that the device is disconnected from the system. */
	public static final ConnectionState DISCONNECTED = new ConnectionState(0, "disconnected");

	/** Indicates that the device is connected and available to the system. */
	public static final ConnectionState CONNECTED = new ConnectionState(1, "connected");

	/** Indicates that the device is connecting to the system. */
	public static final ConnectionState CONNECTING = new ConnectionState(2, "connecting");

	/** Indicates that the device is disconnecting from the system. */
	public static final ConnectionState DISCONNECTING = new ConnectionState(3, "disconnecting");


	/**
	 * Creates a new connection state object.
	 * @param coronaIntegerId Corona's unique integer ID for the connection state
	 * @param coronaStringId Corona's unique string ID for the connection state.
	 */
	private ConnectionState(int coronaIntegerId, String coronaStringId) {
		fCoronaIntegerId = coronaIntegerId;
		fCoronaStringId = coronaStringId;
	}

	/**
	 * Determines if the device is in the "connected" state.
	 * @return Returns true if connected.
	 *         <p>
	 *         Returns false if disconnected, disconnecting, or connecting.
	 */
	public boolean isConnected() {
		return (this == CONNECTED);
	}

	/**
	 * Gets Corona's unique integer ID for this connection state.
	 * @return Returns a unique integer ID used by Corona to identify the connection state.
	 */
	public int toCoronaIntegerId() {
		return fCoronaIntegerId;
	}

	/**
	 * Gets Corona's unique string ID for this connection state such as "connected", "disconnected", etc.
	 * @return Returns a unique string ID used by Corona's input device object in Lua.
	 */
	public String toCoronaStringId() {
		return fCoronaStringId;
	}

	/**
	 * Gets Corona's unique string ID for this connection state such as "connected", "disconnected", etc.
	 * @return Returns a unique string ID used by Corona's input device object in Lua.
	 */
	@Override
	public String toString() {
		return fCoronaStringId;
	}

	/**
	 * Gets an integer hash code for this object.
	 * @return Returns this object's hash code.
	 */
	@Override
	public int hashCode() {
		return fCoronaIntegerId;
	}
}

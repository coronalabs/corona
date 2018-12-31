/*
 * $Id: LuaStackTraceElement.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Represents an execution point in a Lua stack trace.
 */
public class LuaStackTraceElement {
	// -- State
	private String functionName;
	private String sourceName;
	private int lineNumber;

	// -- Construction
	/**
	 * Creates a new instance.
	 * 
	 * @param sourceName
	 *            the source name, or <code>null</code> if unavailable
	 * @param functionName
	 *            the function name, or <code>null</code> if unavailable
	 * @param lineNumber
	 *            the line number, or a negative number if unavailable
	 */
	public LuaStackTraceElement(String functionName, String sourceName,
			int lineNumber) {
		this.functionName = functionName;
		this.sourceName = sourceName;
		this.lineNumber = lineNumber;
	}

	// -- Properties
	/**
	 * Returns the name of the function containing the execution point
	 * represented by this stack trace element. If there is no function name for
	 * the execution point, the method returns <code>null</code>.
	 * 
	 * @return the name of the function containing the execution point
	 *         represented by this stack trace element, or <code>null</code>
	 */
	public String getFunctionName() {
		return functionName;
	}

	/**
	 * Returns the name of the source containing the execution point represented
	 * by this this stack trace element. The source name is passed to the Lua
	 * state when the Lua source code is loaded. If there is no source name for
	 * the execution point, the method returns <code>null</code>.
	 * 
	 * @return the source name, or <code>null</code>
	 * @see LuaState#load(java.io.InputStream, String)
	 * @see LuaState#load(String, String)
	 */
	public String getSourceName() {
		return sourceName;
	}

	/**
	 * Returns the line number in the source containing the execution point
	 * represented by this stack trace element. If there is no line number for
	 * the execution point, the method returns a negative number.
	 * 
	 * @return the line number, or a negative number if there is no line number
	 */
	public int getLineNumber() {
		return lineNumber;
	}

	// Object methods
	@Override
	public int hashCode() {
		int result = functionName != null ? functionName.hashCode() : 0;
		result = result * 65599 + sourceName != null ? sourceName.hashCode()
				: 0;
		result = result * 65599 + lineNumber;
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}
		if (!(obj instanceof LuaStackTraceElement)) {
			return false;
		}
		LuaStackTraceElement other = (LuaStackTraceElement) obj;
		return safeEquals(functionName, other.functionName)
				&& safeEquals(sourceName, other.sourceName)
				&& lineNumber == other.lineNumber;
	}

	@Override
	public String toString() {
		StringBuffer sb = new StringBuffer();
		if (functionName != null) {
			sb.append(functionName);
		} else {
			sb.append("(Unknown Function)");
		}
		sb.append(" (");
		if (sourceName != null) {
			sb.append(sourceName);
			if (lineNumber >= 0) {
				sb.append(':');
				sb.append(lineNumber);
			}
		} else {
			sb.append("External Function");
		}
		sb.append(')');
		return sb.toString();
	}

	// -- Private methods
	/**
	 * Returns whether two objects are equal, handling <code>null</code>.
	 */
	private boolean safeEquals(Object a, Object b) {
		return a == b || a != null && a.equals(b);
	}
}

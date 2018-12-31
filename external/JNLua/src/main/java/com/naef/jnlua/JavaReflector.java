/*
 * $Id: JavaReflector.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Reflects Java objects for access from Lua.
 * 
 * <p>
 * The interface can be implemented to provide a generic Java reflector that is
 * then configured in a Lua state. It can also be implemented by individual Java
 * classes to provide class-specific Java reflection. If an object implements
 * the Java reflector interface, its own Java reflector is queried first for a
 * requested metamethod. Only if the metamethod requested is not supported, the
 * Java reflector configured in the Lua state is queried.
 * </p>
 */
public interface JavaReflector {
	/**
	 * Returns the metamethod implementation of this Java reflector for the
	 * specified metamethod. If this reflector does not support the metamethod,
	 * the method returns <code>null</code>.
	 * 
	 * @param metamethod
	 *            the metamethod
	 * @return the implementation, or <code>null</code> if this Java reflector
	 *         does not support the metamethod
	 */
	public JavaFunction getMetamethod(Metamethod metamethod);

	// -- Nested types
	/**
	 * Lua metamethod.
	 */
	public enum Metamethod {
		/**
		 * <code>__index</code> metamethod.
		 */
		INDEX,

		/**
		 * <code>__newindex</code> metamethod.
		 */
		NEWINDEX,

		/**
		 * <code>__len</code> metamethod.
		 */
		LEN,

		/**
		 * <code>__eq</code> metamethod.
		 */
		EQ,

		/**
		 * <code>__lt</code> metamethod.
		 */
		LT,

		/**
		 * <code>__le</code> metamethod.
		 */
		LE,

		/**
		 * <code>__unm</code> metamethod.
		 */
		UNM,

		/**
		 * <code>__add</code> metamethod.
		 */
		ADD,

		/**
		 * <code>__sub</code> metamethod.
		 */
		SUB,

		/**
		 * <code>__mul</code> metamethod.
		 */
		MUL,

		/**
		 * <code>__div</code> metamethod.
		 */
		DIV,

		/**
		 * <code>__mod</code> metamethod.
		 */
		MOD,

		/**
		 * <code>__pow</code> metamethod.
		 */
		POW,

		/**
		 * <code>__concat</code> metamethod.
		 */
		CONCAT,

		/**
		 * <code>__call</code> metamethod.
		 */
		CALL,

		/**
		 * <code>__tostring</code> metamethod.
		 */
		TOSTRING,

		/**
		 * <code>__javafields</code> metamethod.
		 */
		JAVAFIELDS,

		/**
		 * <code>__javamethods</code> metamethod.
		 */
		JAVAMETHODS,

		/**
		 * <code>__javaproperties</code> metamethod.
		 */
		JAVAPROPERTIES;

		// -- Operations
		/**
		 * Returns the Lua metamethod name.
		 * 
		 * @return the metamethod name
		 */
		public String getMetamethodName() {
			return "__" + toString().toLowerCase();
		}
	};
}

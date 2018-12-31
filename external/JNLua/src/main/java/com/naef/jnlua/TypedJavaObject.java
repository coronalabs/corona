/*
 * $Id: TypedJavaObject.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Represents a Java object with an explicit type.
 * 
 * <p>
 * The interface is implemented by objects needing to specify an explicit type
 * for a wrapped object. This typically occurs in casting situations. Such typed
 * Java object are considered <i>weak</i> since they have no representative
 * value of their own. Weak typed Java objects always convert to wrapped object.
 * </p>
 * 
 * <p>
 * The interface is also implemented by objects wrapping another object and
 * offering transparent conversion to the wrapped object if needed. This
 * situation for example occurs when an object implements the
 * {@link com.naef.jnlua.JavaReflector} interface to provide custom Java
 * reflection for a wrapped object and at the same time wants to ensure
 * transparent conversion to the wrapped object if needed. Such typed Java
 * objects are considered <i>strong</i> since they have a representative value
 * of their own. Strong typed Java objects convert to wrapped object only if
 * this is required to satisfy a type conversion.
 * </p>
 */
public interface TypedJavaObject {
	/**
	 * Returns the object.
	 * 
	 * @return the object
	 */
	public Object getObject();

	/**
	 * Returns the type.
	 * 
	 * @return the type
	 */
	public Class<?> getType();

	/**
	 * Returns whether this is a strong typed Java object.
	 * 
	 * @return <code>true</code> if this typed Java object is strong, and
	 *         <code>false</code> if it is weak
	 */
	public boolean isStrong();
}

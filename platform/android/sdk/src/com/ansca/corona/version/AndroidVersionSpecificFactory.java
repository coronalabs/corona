//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.version;

import java.lang.reflect.Field;


public class AndroidVersionSpecificFactory {
	private static String getAndroidVersion()
	{
		String className = "com.ansca.corona.version.AndroidVersion";
		String version = "";
		try {
			Class c = Class.forName( className );
			Field f = c.getField("apiVersion");
			version = (String) f.get(null);
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NoSuchFieldException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return version;
	}
	
	/**
	 * Create appropriate IAndroidVersionSpecific subclass
	 * 
	 * @return			IAndroidVersionSpecific implementation
	 */
	public static IAndroidVersionSpecific create() {
		String version = getAndroidVersion();
		String className = "com.ansca.corona.version.android" + version + ".AndroidVersionSpecific";
		IAndroidVersionSpecific result = null;
		try {
			Class c = Class.forName( className );
			
			Object created = c.newInstance();

			result = (IAndroidVersionSpecific) created;
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InstantiationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return result;
	}
}

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

package com.ansca.corona;


/** Base class used to store data compatible with Lua. */
public abstract class CoronaData implements Cloneable, java.io.Serializable, android.os.Parcelable {
	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public CoronaData clone() {
		CoronaData clone = null;
		try {
			clone = (CoronaData)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Describe the kinds of special objects contained in this parcelable object's marshalled representation.
	 * @return Returns a bitmask indicating the set of special object types marshalled by this object.
	 */
	@Override
	public int describeContents() {
		return 0;
	}

	/**
	 * Pushes this object's data to the top of the Lua stack.
	 * @param luaState The Lua state to push data to. Cannot be null.
	 * @return Returns true if the push was successful. Returns false if failed.
	 */
	public abstract boolean pushTo(com.naef.jnlua.LuaState luaState);

	/**
	 * Writes this object's data to the given XML writer.
	 * @param xmlWriter The serializer used to output this object's data to XML. Cannot be null.
	 */
	public abstract void writeTo(org.xmlpull.v1.XmlSerializer xmlWriter) throws java.io.IOException;

	/**
	 * Creates a new Corona data object storing a copy of the indexed Lua data.
	 * @param luaState The Lua state to copy data from.
	 * @param luaStackIndex Index to a Lua object in the Lua stack such as a boolean, number, string, or table.
	 * @return Returns a data object containing a copy of the indexed Lua object's data.
	 *         <p>
	 *         Returns null if unable to copy the indexed data in Lua or if given invalid arguments.
	 */
	public static CoronaData from(com.naef.jnlua.LuaState luaState, int luaStackIndex) {
		// Validate.
		if (luaState == null) {
			return null;
		}

		// Create a data object storing a copy of the value(s) indexed in Lua.
		CoronaData data = null;
		com.naef.jnlua.LuaType luaType = luaState.type(luaStackIndex);
		if (luaType == com.naef.jnlua.LuaType.BOOLEAN) {
			data = CoronaData.Boolean.from(luaState.toBoolean(luaStackIndex));
		}
		else if (luaType == com.naef.jnlua.LuaType.NUMBER) {
			data = new CoronaData.Double(luaState.toNumber(luaStackIndex));
		}
		else if (luaType == com.naef.jnlua.LuaType.STRING) {
			data = new CoronaData.String(luaState.toString(luaStackIndex));
		}
		else if (luaType == com.naef.jnlua.LuaType.TABLE) {
			if (luaStackIndex < 0) {
				luaStackIndex = luaState.getTop() + (luaStackIndex + 1);
			}
			CoronaData.Table table = new CoronaData.Table();
			for (luaState.pushNil(); luaState.next(luaStackIndex); luaState.pop(1)) {
				CoronaData key = CoronaData.from(luaState, -2);
				if ((key instanceof CoronaData.Value) == false) {
					continue;
				}
				CoronaData value = CoronaData.from(luaState, -1);
				table.put((CoronaData.Value)key, value);
			}
			data = table;
		}
		return data;
	}

	/**
	 * Creates a new data object containing the data read from the given XML reader.
	 * @param xmlReader Reader used to extract one Corona data object from XML.
	 *                  This method expects the next tag to reference Corona data.
	 * @return Returns a data object containing read from XML.
	 *         <p>
	 *         Returns null if unable to find Corona data.
	 */
	public static CoronaData from(org.xmlpull.v1.XmlPullParser xmlReader)
		throws java.io.IOException, org.xmlpull.v1.XmlPullParserException
	{
		// Validate.
		if (xmlReader == null) {
			return null;
		}

		// Get the next XML tag.
		int xmlEventType = xmlReader.nextTag();
		if (xmlEventType != org.xmlpull.v1.XmlPullParser.START_TAG) {
			return null;
		}
		java.lang.String tagName = xmlReader.getName();
		if ((tagName == null) || (tagName.length() <= 0)) {
			return null;
		}

		// Read Corona data from XML.
		CoronaData data = null;
		if ("boolean".equals(tagName)) {
			boolean value = java.lang.Boolean.parseBoolean(xmlReader.getAttributeValue("", "value"));
			data = CoronaData.Boolean.from(value);
		}
		else if ("double".equals(tagName)) {
			double value = java.lang.Double.parseDouble(xmlReader.getAttributeValue("", "value"));
			data = new CoronaData.Double(value);
		}
		else if ("string".equals(tagName)) {
			data = CoronaData.String.EMPTY;
			xmlEventType = xmlReader.next();
			if (xmlEventType == org.xmlpull.v1.XmlPullParser.TEXT) {
				java.lang.String text = xmlReader.getText();
				if (text != null) {
					data = new CoronaData.String(text);
				}
			}
		}
		else if ("proxy".equals(tagName)) {
			data = new CoronaData.Proxy(CoronaData.from(xmlReader));
		}
		else if ("list".equals(tagName)) {
			CoronaData.List list = new CoronaData.List();
			do {
				data = CoronaData.from(xmlReader);
				list.add(data);
			} while (data != null);
			data = list;
		}
		else if ("table".equals(tagName)) {
			CoronaData.Table table = new CoronaData.Table();
			while (true) {
				xmlEventType = xmlReader.nextTag();
				if ((xmlEventType == org.xmlpull.v1.XmlPullParser.START_TAG) && "entry".equals(xmlReader.getName())) {
					CoronaData.Value key = null;
					CoronaData value = null;
					while (true) {
						xmlEventType = xmlReader.nextTag();
						if (xmlEventType == org.xmlpull.v1.XmlPullParser.START_TAG) {
							if ("key".equals(xmlReader.getName())) {
								data = CoronaData.from(xmlReader);
								if (data instanceof CoronaData.Value) {
									key = (CoronaData.Value)data;
								}
							}
							else if ("value".equals(xmlReader.getName())) {
								value = CoronaData.from(xmlReader);
							}
						}
						else if ((xmlEventType == org.xmlpull.v1.XmlPullParser.END_TAG) && "entry".equals(xmlReader.getName())) {
							break;
						}
					}
					if ((key != null) && (value != null)) {
						table.put(key, value);
					}
				}
				else if ((xmlEventType == org.xmlpull.v1.XmlPullParser.END_TAG) && "table".equals(xmlReader.getName())) {
					break;
				}
			}
			data = table;
		}

		// Advance the XML reader to the end tag.
		while (true) {
			if (xmlReader.getEventType() == org.xmlpull.v1.XmlPullParser.END_TAG) {
				if (tagName.equals(xmlReader.getName())) {
					break;
				}
			}
			xmlReader.nextTag();
		}

		// Return the Corona data read from XML.
		return data;
	}

	/**
	 * Attempts to convert the given Java object to a CoronaData object.
	 * @param value The Java object to be converted.
	 *              <p>
	 *              Supports the following types:
	 *              <ul>
	 *               <li>Boolean
	 *               <li>Number  (ie: Integer, Long, Float, Double, etc.)
	 *               <li>Character
	 *               <li>CharSequence  (ie: String, StringBuffer, CharBuffer, etc.)
	 *               <li>java.io.File
	 *               <li>android.net.Uri
	 *               <li>android.os.Bundle
	 *               <li>java.util.Map
	 *               <li>Iterable  (ie: List, Queue, Stack, Set, etc.)
	 *               <li>Arrays  (Supports both primitive arrays and Object arrays.)
	 *              </ul>
	 *
	 * @return Returns a new Corona data object if able to convert the given Java object.
	 *         <p>
	 *         Returns the given object if it is already a Corona data object.
	 *         <p>
	 *         Returns null if unable to convert the given object.
	 */
	public static CoronaData from(Object value) {
		// Validate.
		if (value == null) {
			return null;
		}

		// If given a Corona data object, then no conversion is necessary. Return it as is.
		if (value instanceof CoronaData) {
			return (CoronaData)value;
		}

		// Convert the given object to its equivalent CoronaData type, if possible.
		CoronaData data = null;
		if (value instanceof java.lang.Boolean) {
			data = CoronaData.Boolean.from(((java.lang.Boolean)value).booleanValue());
		}
		else if (value instanceof Number) {
			data = new CoronaData.Double(((Number)value).doubleValue());
		}
		else if ((value instanceof Character) || (value instanceof CharSequence)) {
			data = new CoronaData.String(value.toString());
		}
		else if (value instanceof java.io.File) {
			data = new CoronaData.String(((java.io.File)value).getAbsolutePath());
		}
		else if (value instanceof android.net.Uri) {
			data = new CoronaData.String(value.toString());
		}
		else if (value instanceof android.os.Bundle) {
			data = CoronaData.Table.from((android.os.Bundle)value);
		}
		else if (value instanceof org.json.JSONArray) {
			try {
				data = CoronaData.List.from((org.json.JSONArray)value);
			}
			catch (Exception ex) {
				try {
					data = new CoronaData.String(((org.json.JSONArray)value).toString());
				}
				catch (Exception innerEx) { }
			}
		}
		else if (value instanceof org.json.JSONObject) {
			try {
				data = CoronaData.Table.from((org.json.JSONObject)value);
			}
			catch (Exception ex) {
				try {
					data = new CoronaData.String(((org.json.JSONObject)value).toString());
				}
				catch (Exception innerEx) { }
			}
		}
		else if (value.getClass().isArray()) {
			CoronaData.List list = new CoronaData.List();
			int arrayLength = java.lang.reflect.Array.getLength(value);
			for (int arrayIndex = 0; arrayIndex < arrayLength; arrayIndex++) {
				list.add(CoronaData.from(java.lang.reflect.Array.get(value, arrayIndex)));
			}
			data = list;
		}
		else if (value instanceof java.util.Map) {
			CoronaData.Table table = new CoronaData.Table();
			java.util.Map map = (java.util.Map)value;
			for (java.util.Map.Entry entry : (java.util.Set<java.util.Map.Entry>)map.entrySet()) {
				CoronaData entryKey = CoronaData.from(entry.getKey());
				if (entryKey instanceof CoronaData.Value) {
					table.put((CoronaData.Value)entryKey, CoronaData.from(entry.getValue()));
				}
			}
			data = table;
		}
		else if (value instanceof Iterable) {
			CoronaData.List list = new CoronaData.List();
			for (Object collectionValue : (Iterable)value) {
				list.add(CoronaData.from(collectionValue));
			}
			data = list;
		}

		// Return the result.
		return data;
	}


	/** Base class for data that only provides a single value such as a boolean, number, or string. */
	public static abstract class Value extends CoronaData {
		/**
		 * Gets the value as an object reference.
		 * <p>
		 * If the value stored by this object is a primitive type, then this method will return a boxed value.
		 * For example, a boolean value will be returned as a Boolean object.
		 * @return Returns the value as an object reference.
		 */
		public abstract Object getValueAsObject();

		/**
		 * Determines if this object matches the given object.
		 * @param value The object to be compare with this instance.
		 * @return Returns true if the given object's value matches this object's value.
		 *         <p>
		 *         Returns false if they do not equal, if given null, or if the given object is of a different type.
		 */
		@Override
		public boolean equals(Object value) {
			if ((value instanceof CoronaData.Value) == false) {
				return false;
			}
			Object objectValue1 = getValueAsObject();
			Object objectValue2 = ((CoronaData.Value)value).getValueAsObject();
			if (objectValue1 == objectValue2) {
				return true;
			}
			if (objectValue1 == null) {
				return false;
			}
			return objectValue1.equals(objectValue2);
		}

		/**
		 * Gets an integer hash code for this object.
		 * @return Returns this object's hash code.
		 */
		@Override
		public int hashCode() {
			Object objectValue = getValueAsObject();
			if (objectValue == null) {
				return 0;
			}
			return objectValue.hashCode();
		}
	}


	/**
	 * Stores a boolean value convertible to a Lua boolean value.
	 * <p>
	 * Instances of this class are immutable.
	 */
	public static class Boolean extends CoronaData.Value {
		/** The stored boolean value. */
		private boolean fValue;

		/** Pre-allocated instance of this class set to true. */
		public static final CoronaData.Boolean TRUE = new CoronaData.Boolean(true);

		/** Pre-allocated instance of this class set to false. */
		public static final CoronaData.Boolean FALSE = new CoronaData.Boolean(false);

		/** Creates Boolean objects from data read from a parcelable. */
		public static final android.os.Parcelable.Creator<CoronaData.Boolean> CREATOR = new ParcelableCreator();

		/**
		 * Creates a boolean value wrapper.
		 * @param value The value to be stored.
		 */
		public Boolean(boolean value) {
			fValue = value;
		}

		/**
		 * Gets the value as a "java.lang.Boolean" object.
		 * @return Returns the value as an object reference.
		 */
		@Override
		public Object getValueAsObject() {
			return java.lang.Boolean.valueOf(fValue);
		}

		/**
		 * Gets the boolean value.
		 * @return Returns the boolean value.
		 */
		public boolean getValue() {
			return fValue;
		}

		/**
		 * Determines if the given value matches this object's value.
		 * @param value The value to be compared with this object's value.
		 * @return Returns true if the values match. Returns false if not.
		 */
		public boolean equals(boolean value) {
			return (fValue == value);
		}

		/**
		 * Pushes this object's data to the top of the Lua stack.
		 * @param luaState The Lua state to push data to. Cannot be null.
		 * @return Returns true if the push was successful. Returns false if failed.
		 */
		@Override
		public boolean pushTo(com.naef.jnlua.LuaState luaState) {
			luaState.pushBoolean(fValue);
			return true;
		}

		/**
		 * Writes this object's data to the given XML writer.
		 * @param xmlWriter The serializer used to output this object's data to XML. Cannot be null.
		 */
		@Override
		public void writeTo(org.xmlpull.v1.XmlSerializer xmlWriter) throws java.io.IOException {
			xmlWriter.startTag("", "boolean");
			xmlWriter.attribute("", "value", java.lang.Boolean.toString(fValue));
			xmlWriter.endTag("", "boolean");
		}

		/**
		 * Writes this object's data to the given parcel.
		 * @param parcel The parcel to write to. Cannot be null.
		 * @param flags Indicates how this object should be written. May be 0 or PARCELABLE_WRITE_RETURN_VALUE.
		 */
		@Override
		public void writeToParcel(android.os.Parcel parcel, int flags) {
			parcel.writeByte((byte)(fValue ? 1 : 0));
		}

		/**
		 * Gets a Boolean object for the given boolean value.
		 * <p>
		 * This method provides a reference to the pre-allocated TRUE and FALSE constants matching the
		 * given boolean value. This is a more memory efficient way of getting a Boolean object because
		 * a new instance does not have to be created.
		 * @return Returns a pre-allocated instance of this class matching the given value.
		 */
		public static CoronaData.Boolean from(boolean value) {
			return (value ? CoronaData.Boolean.TRUE : CoronaData.Boolean.FALSE);
		}

		/** Private class used to create instances of its outer class from data read from an Android parcel. */
		private static class ParcelableCreator implements android.os.Parcelable.Creator<CoronaData.Boolean> {
			/**
			 * Creates an object from data read from the given Android parcel.
			 * @param parcel The parcel to read data from. Cannot be null.
			 * @return Returns a new object containing data read from the given parcel.
			 */
			public CoronaData.Boolean createFromParcel(android.os.Parcel parcel) {
				return CoronaData.Boolean.from(parcel.readByte() != 0);
			}

			/**
			 * Creates an array of objects.
			 * @param size The length of the array to be created. Must be greater than zero.
			 * @return Returns the requested object array.
			 */
			public CoronaData.Boolean[] newArray(int size) {
				return new CoronaData.Boolean[size];
			}
		}
	}

	/**
	 * Stores a floating point value convertible to a Lua number value.
	 * <p>
	 * Instances of this class are immutable.
	 */
	public static class Double extends CoronaData.Value {
		/** The stored value. */
		private double fValue;

		/** Creates Double objects from data read from a parcelable. */
		public static final android.os.Parcelable.Creator<CoronaData.Double> CREATOR = new ParcelableCreator();

		/**
		 * Creates a floating point value wrapper.
		 * @param value The value to be stored.
		 */
		public Double(double value) {
			fValue = value;
		}

		/**
		 * Gets the value as a "java.lang.Double" object.
		 * @return Returns the value as an object reference.
		 */
		@Override
		public Object getValueAsObject() {
			return java.lang.Double.valueOf(fValue);
		}

		/**
		 * Gets the floating point value.
		 * @return Returns the floating point value.
		 */
		public double getValue() {
			return fValue;
		}

		/**
		 * Determines if the given value matches this object's value.
		 * @param value The value to be compared with this object's value.
		 * @return Returns true if the values match. Returns false if not.
		 */
		public boolean equals(double value) {
			return (fValue == value);
		}

		/**
		 * Pushes this object's data to the top of the Lua stack.
		 * @param luaState The Lua state to push data to. Cannot be null.
		 * @return Returns true if the push was successful. Returns false if failed.
		 */
		@Override
		public boolean pushTo(com.naef.jnlua.LuaState luaState) {
			luaState.pushNumber(fValue);
			return true;
		}

		/**
		 * Writes this object's data to the given XML writer.
		 * @param xmlWriter The serializer used to output this object's data to XML. Cannot be null.
		 */
		@Override
		public void writeTo(org.xmlpull.v1.XmlSerializer xmlWriter) throws java.io.IOException {
			xmlWriter.startTag("", "double");
			xmlWriter.attribute("", "value", java.lang.Double.toString(fValue));
			xmlWriter.endTag("", "double");
		}

		/**
		 * Writes this object's data to the given parcel.
		 * @param parcel The parcel to write to. Cannot be null.
		 * @param flags Indicates how this object should be written. May be 0 or PARCELABLE_WRITE_RETURN_VALUE.
		 */
		@Override
		public void writeToParcel(android.os.Parcel parcel, int flags) {
			parcel.writeDouble(fValue);
		}

		/** Private class used to create instances of its outer class from data read from an Android parcel. */
		private static class ParcelableCreator implements android.os.Parcelable.Creator<CoronaData.Double> {
			/**
			 * Creates an object from data read from the given Android parcel.
			 * @param parcel The parcel to read data from. Cannot be null.
			 * @return Returns a new object containing data read from the given parcel.
			 */
			public CoronaData.Double createFromParcel(android.os.Parcel parcel) {
				return new CoronaData.Double(parcel.readDouble());
			}

			/**
			 * Creates an array of objects.
			 * @param size The length of the array to be created. Must be greater than zero.
			 * @return Returns the requested object array.
			 */
			public CoronaData.Double[] newArray(int size) {
				return new CoronaData.Double[size];
			}
		}
	}

	/**
	 * Stores a string convertible to a Lua string.
	 * <p>
	 * Instances of this class are immutable.
	 */
	public static class String extends CoronaData.Value {
		/** The stored string value. */
		private java.lang.String fValue;

		/** Pre-allocated instance of this class set to an empty string. */
		public static final CoronaData.String EMPTY = new CoronaData.String("");

		/** Creates String objects from data read from a parcelable. */
		public static final android.os.Parcelable.Creator<CoronaData.String> CREATOR = new ParcelableCreator();

		/**
		 * Creates a new string wrapper.
		 * @param value The string to be stored.
		 */
		public String(java.lang.String value) {
			if (value == null) {
				value = "";
			}
			fValue = value;
		}

		/**
		 * Gets the string.
		 * @return Returns this object's string.
		 */
		@Override
		public Object getValueAsObject() {
			return fValue;
		}

		/**
		 * Gets the string.
		 * @return Returns this object's string.
		 */
		public java.lang.String getValue() {
			return fValue;
		}

		/**
		 * Pushes this object's data to the top of the Lua stack.
		 * @param luaState The Lua state to push data to. Cannot be null.
		 * @return Returns true if the push was successful. Returns false if failed.
		 */
		@Override
		public boolean pushTo(com.naef.jnlua.LuaState luaState) {
			luaState.pushString(getValue());
			return true;
		}

		/**
		 * Writes this object's data to the given XML writer.
		 * @param xmlWriter The serializer used to output this object's data to XML. Cannot be null.
		 */
		@Override
		public void writeTo(org.xmlpull.v1.XmlSerializer xmlWriter) throws java.io.IOException {
			xmlWriter.startTag("", "string");
			xmlWriter.text(fValue);
			xmlWriter.endTag("", "string");
		}

		/**
		 * Writes this object's data to the given parcel.
		 * @param parcel The parcel to write to. Cannot be null.
		 * @param flags Indicates how this object should be written. May be 0 or PARCELABLE_WRITE_RETURN_VALUE.
		 */
		@Override
		public void writeToParcel(android.os.Parcel parcel, int flags) {
			parcel.writeString(fValue);
		}

		/** Private class used to create instances of its outer class from data read from an Android parcel. */
		private static class ParcelableCreator implements android.os.Parcelable.Creator<CoronaData.String> {
			/**
			 * Creates an object from data read from the given Android parcel.
			 * @param parcel The parcel to read data from. Cannot be null.
			 * @return Returns a new object containing data read from the given parcel.
			 */
			public CoronaData.String createFromParcel(android.os.Parcel parcel) {
				return new CoronaData.String(parcel.readString());
			}

			/**
			 * Creates an array of objects.
			 * @param size The length of the array to be created. Must be greater than zero.
			 * @return Returns the requested object array.
			 */
			public CoronaData.String[] newArray(int size) {
				return new CoronaData.String[size];
			}
		}
	}

	/**
	 * Container for one data object.
	 * <p>
	 * This class is used to work-around an Android bug where passing a "CoronaData.List" or
	 * "CoronaData.Table" object to an Android Intent object's putSerializable() method causes those
	 * objects to be wrongly serialized as an "ArrayList" or "HashMap" object instead. This is due to
	 * interfaces that those classes implement. This issue can be worked-around by wrapping those object
	 * with a proxy object before passing them into the intent.
	 */
	public static class Proxy extends CoronaData {
		/** The data this proxy object references. */
		private CoronaData fData;

		/** Creates Proxy objects from data read from a parcelable. */
		public static final android.os.Parcelable.Creator<CoronaData.Proxy> CREATOR = new ParcelableCreator();

		/** Creates a new proxy object that does not reference any data. */
		public Proxy() {
			this(null);
		}

		/**
		 * Creates a new proxy object referencing the given data.
		 * @param data The data to be referenced by this proxy. Can be null.
		 */
		public Proxy(CoronaData data) {
			fData = data;
		}

		/**
		 * Gets the data that this proxy references.
		 * @return Returns the data object this proxy references.
		 *         <p>
		 *         Returns null if this proxy does not reference any data.
		 */
		public CoronaData getData() {
			return fData;
		}

		/**
		 * Sets the data to be referenced by this proxy.
		 * @param data The data to be referenced by this proxy. Can be null.
		 */
		public void setData(CoronaData data) {
			fData = data;
		}

		/**
		 * Creates a new copy of this object.
		 * <p>
		 * This is a shallow copy. This method does not clone the data that this proxy references.
		 * @return Returns a shallow copy of this object.
		 */
		@Override
		public CoronaData.Proxy clone() {
			return (CoronaData.Proxy)super.clone();
		}

		/**
		 * Pushes this object's data to the top of the Lua stack.
		 * @param luaState The Lua state to push data to. Cannot be null.
		 * @return Returns true if the push was successful. Returns false if failed.
		 */
		public boolean pushTo(com.naef.jnlua.LuaState luaState) {
			if (fData == null) {
				return false;
			}
			return fData.pushTo(luaState);
		}

		/**
		 * Writes this object's data to the given XML writer.
		 * @param xmlWriter The serializer used to output this object's data to XML. Cannot be null.
		 */
		public void writeTo(org.xmlpull.v1.XmlSerializer xmlWriter) throws java.io.IOException {
			xmlWriter.startTag("", "proxy");
			if (fData != null) {
				fData.writeTo(xmlWriter);
			}
			xmlWriter.endTag("", "proxy");
		}

		/**
		 * Writes this object's data to the given parcel.
		 * @param parcel The parcel to write to. Cannot be null.
		 * @param flags Indicates how this object should be written. May be 0 or PARCELABLE_WRITE_RETURN_VALUE.
		 */
		@Override
		public void writeToParcel(android.os.Parcel parcel, int flags) {
			parcel.writeByte((byte)((fData != null) ? 1 : 0));
			if (fData != null) {
				parcel.writeParcelable(fData, flags);
			}
		}

		/** Private class used to create instances of its outer class from data read from an Android parcel. */
		private static class ParcelableCreator implements android.os.Parcelable.Creator<CoronaData.Proxy> {
			/**
			 * Creates an object from data read from the given Android parcel.
			 * @param parcel The parcel to read data from. Cannot be null.
			 * @return Returns a new object containing data read from the given parcel.
			 */
			public CoronaData.Proxy createFromParcel(android.os.Parcel parcel) {
				CoronaData.Proxy proxy = new CoronaData.Proxy();
				if (parcel.readByte() != 0) {
					proxy.setData((CoronaData)parcel.readParcelable(getClass().getClassLoader()));
				}
				return proxy;
			}

			/**
			 * Creates an array of objects.
			 * @param size The length of the array to be created. Must be greater than zero.
			 * @return Returns the requested object array.
			 */
			public CoronaData.Proxy[] newArray(int size) {
				return new CoronaData.Proxy[size];
			}
		}
	}

	/** Stores a list of data convertible to a Lua array. */
	public static class List extends CoronaData implements Iterable<CoronaData> {
		/** Stores a collection of data. */
		private java.util.ArrayList<CoronaData> fCollection;

		/** Creates List objects from data read from a parcelable. */
		public static final android.os.Parcelable.Creator<CoronaData.List> CREATOR = new ParcelableCreator();

		/** Creates a new empty list. */
		public List() {
			fCollection = new java.util.ArrayList<CoronaData>();
		}

		/**
		 * Creates a new copy of this object.
		 * @return Returns a copy of this object.
		 */
		@Override
		public CoronaData.List clone() {
			CoronaData.List clone = (CoronaData.List)super.clone();
			clone.fCollection = new java.util.ArrayList<CoronaData>();
			for (CoronaData data : fCollection) {
				clone.add(data);
			}
			return clone;
		}

		/**
		 * Adds the given item to the end of the collection.
		 * @param item The object to be added to the collection. Cannot be null.
		 */
		public void add(CoronaData item) {
			// Validate.
			if (item == null) {
				return;
			}

			// Add the given item to the collection.
			fCollection.add(item);
		}

		/**
		 * Removes the given item from the collection by reference.
		 * @param item The item to be removed from the collection by reference. Cannot be null.
		 * @return Returns true if the given item was successfully removed.
		 *         <p>
		 *         Returns false if not found or if given a null reference.
		 */
		public boolean remove(CoronaData item) {
			// Validate.
			if (item == null) {
				return false;
			}
			
			// Remove the given item from the collection.
			return fCollection.remove(item);
		}

		/** Removes all items in the collection. */
		public void clear() {
			fCollection.clear();
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
		public java.util.Iterator<CoronaData> iterator() {
			return fCollection.iterator();
		}

		/**
		 * Pushes this object's data to the top of the Lua stack.
		 * @param luaState The Lua state to push data to. Cannot be null.
		 * @return Returns true if the push was successful. Returns false if failed.
		 */
		@Override
		public boolean pushTo(com.naef.jnlua.LuaState luaState) {
			// If the collection is empty, then create an empty Lua table and stop here.
			int count = fCollection.size();
			if (count <= 0) {
				luaState.newTable();
				return true;
			}

			// Push the collection's value to Lua.
			luaState.newTable(count, 0);
			int luaTableStackIndex = luaState.getTop();
			for (int index = 0; index < count; index++) {
				CoronaData data = fCollection.get(index);
				if (data != null) {
					boolean wasPushed = data.pushTo(luaState);
					if (wasPushed) {
						luaState.rawSet(luaTableStackIndex, index + 1);
					}
				}
			}
			return true;
		}

		/**
		 * Writes this object's data to the given XML writer.
		 * @param xmlWriter The serializer used to output this object's data to XML. Cannot be null.
		 */
		@Override
		public void writeTo(org.xmlpull.v1.XmlSerializer xmlWriter) throws java.io.IOException {
			xmlWriter.startTag("", "list");
			for (CoronaData data : fCollection) {
				if (data != null) {
					data.writeTo(xmlWriter);
				}
			}
			xmlWriter.endTag("", "list");
		}

		/**
		 * Writes this object's data to the given parcel.
		 * @param parcel The parcel to write to. Cannot be null.
		 * @param flags Indicates how this object should be written. May be 0 or PARCELABLE_WRITE_RETURN_VALUE.
		 */
		@Override
		public void writeToParcel(android.os.Parcel parcel, int flags) {
			parcel.writeInt(fCollection.size());
			for (CoronaData data : fCollection) {
				if (data != null) {
					parcel.writeParcelable(data, flags);
				}
			}
		}

		/**
		 * Creates a new list of Corona data from the given JSON.
		 * @param jsonArray A JSON array to copy data from.
		 * @return Returns a new list containing a copy of the data in the given JSON array.
		 *         <p>
		 *         Returns null if the given JSON array was null.
		 */
		public static CoronaData.List from(org.json.JSONArray jsonArray)
			throws org.json.JSONException
		{
			// Validate.
			if (jsonArray == null) {
				return null;
			}

			// Copy the given JSON array's data to a new list and return it.
			CoronaData.List list = new CoronaData.List();
			for (int index = 0; index < jsonArray.length(); index++) {
				list.add(CoronaData.from(jsonArray.get(index)));
			}
			return list;
		}

		/** Private class used to create instances of its outer class from data read from an Android parcel. */
		private static class ParcelableCreator implements android.os.Parcelable.Creator<CoronaData.List> {
			/**
			 * Creates an object from data read from the given Android parcel.
			 * @param parcel The parcel to read data from. Cannot be null.
			 * @return Returns a new object containing data read from the given parcel.
			 */
			public CoronaData.List createFromParcel(android.os.Parcel parcel) {
				CoronaData.List list = new CoronaData.List();
				ClassLoader classLoader = getClass().getClassLoader();
				int count = parcel.readInt();
				for (int index = 0; index < count; index++) {
					list.add((CoronaData)parcel.readParcelable(classLoader));
				}
				return list;
			}

			/**
			 * Creates an array of objects.
			 * @param size The length of the array to be created. Must be greater than zero.
			 * @return Returns the requested object array.
			 */
			public CoronaData.List[] newArray(int size) {
				return new CoronaData.List[size];
			}
		}
	}

	/** Stores a table of key/value pairs that is convertible to a Lua table. */
	public static class Table extends CoronaData implements java.util.Map<CoronaData.Value, CoronaData> {
		/** Stores a table of data. */
		private java.util.HashMap<CoronaData.Value, CoronaData> fHashMap;

		/** Creates Table objects from data read from a parcelable. */
		public static final android.os.Parcelable.Creator<CoronaData.Table> CREATOR = new ParcelableCreator();

		/** Creates an empty table for storing CoronaData derived objects. */
		public Table() {
			fHashMap = new java.util.HashMap<CoronaData.Value, CoronaData>();
		}

		/**
		 * Creates a new copy of this object.
		 * @return Returns a copy of this object.
		 */
		@Override
		public CoronaData.Table clone() {
			CoronaData.Table clone = (CoronaData.Table)super.clone();
			clone.fHashMap = new java.util.HashMap<CoronaData.Value, CoronaData>();
			for (java.util.Map.Entry<CoronaData.Value, CoronaData> entry : entrySet()) {
				if (entry.getKey() != null) {
					CoronaData.Value key = (CoronaData.Value)entry.getKey().clone();
					CoronaData value = entry.getValue();
					if (value != null) {
						value = value.clone();
					}
					clone.put(key, value);
				}
			}
			return clone;
		}

		/** Removes all items in the table. */
		@Override
		public void clear() {
			fHashMap.clear();
		}

		/**
		 * Determines if the given key exists in the table.
		 * @param key The key to search for.
		 * @return Returns true if the key was found. Returns false if not found or if the key was null.
		 */
		@Override
		public boolean containsKey(Object key) {
			if (key == null) {
				return false;
			}
			return fHashMap.containsKey(key);
		}

		/**
		 * Determines if the given value exists in the table.
		 * @param value The value to search for.
		 * @return Returns true if the value was found. Returns false if not.
		 */
		@Override
		public boolean containsValue(Object value) {
			return fHashMap.containsValue(value);
		}

		/**
		 * Gets a collection of all of the key/value pairs in the table.
		 * @return Returns table's key/value pairs.
		 *         <p>
		 *         Returns an empty collection if the table is empty.
		 */
		@Override
		public java.util.Set<java.util.Map.Entry<CoronaData.Value, CoronaData>> entrySet() {
			return fHashMap.entrySet();
		}

		/**
		 * Determines if the given table contains the same entries as this table.
		 * @param object The object to be compared with this object.
		 * @return Returns true if the given object contains the same entries as this table.
		 *         <p>
		 *         Returns false if they do not match, the object is not of type Table, or if given null.
		 */
		@Override
		public boolean equals(Object object) {
			if ((object instanceof CoronaData.Table) == false) {
				return false;
			}
			return fHashMap.equals(((CoronaData.Table)object).fHashMap);
		}

		/**
		 * Fetches a value from the table by its key.
		 * @param key The key to use to fetch the value.
		 * @return Returns a reference to the key's value.
		 *         <p>
		 *         Returns null if the key was not found or if the key was null.
		 */
		@Override
		public CoronaData get(Object key) {
			if (key == null) {
				return null;
			}
			return fHashMap.get(key);
		}

		/**
		 * Gets an integer hash code for this object.
		 * @return Returns this object's hash code.
		 */
		@Override
		public int hashCode() {
			return fHashMap.hashCode();
		}

		/**
		 * Determines if this table is empty.
		 * @return Returns true if this table is empty. Returns false if not.
		 */
		@Override
		public boolean isEmpty() {
			return fHashMap.isEmpty();
		}

		/**
		 * Gets all of the keys contained in this table.
		 * @return Returns a collection of keys. Returns an empty collection if this table is empty.
		 */
		@Override
		public java.util.Set<CoronaData.Value> keySet() {
			return fHashMap.keySet();
		}

		/**
		 * Adds the given value to the table under the given key.
		 * Replaces the previous value if the given key already exists in the table.
		 * @param key The unique key to store the value under. Cannot be null.
		 * @param value The value to store in the table. Can be null.
		 * @return Returns a reference to the previous value if the key already exists in the table.
		 *         <p>
		 *         Returns null if inserting a new entry into the table or if given a null key.
		 */
		@Override
		public CoronaData put(CoronaData.Value key, CoronaData value) {
			if (key == null) {
				return null;
			}
			return fHashMap.put(key, value);
		}

		/**
		 * Copies the given map entries to this table.
		 * @param map The map to copy from. Cannot be null.
		 */
		@Override
		public void putAll(java.util.Map<? extends CoronaData.Value, ? extends CoronaData> map) {
			if (map != null) {
				fHashMap.putAll(map);
			}
		}

		/**
		 * Removes an entry from this table by its unique key.
		 * @param key The unique key of the entry to be removed.
		 * @return Returns a reference to the value that was removed from the table.
		 *         <p>
		 *         Returns null if the given key was not found or if the key was null.
		 */
		@Override
		public CoronaData remove(Object key) {
			if (key == null) {
				return null;
			}
			return fHashMap.remove(key);
		}

		/**
		 * Gets the number of entries in this table.
		 * @return Returns the number of entries in the table. Returns zero if the table is empty.
		 */
		@Override
		public int size() {
			return fHashMap.size();
		}

		/**
		 * Gets all of the value contained in this table.
		 * @return Returns a collection of all of the values contained in the table.
		 *         <p>
		 *         Returns an empty collection if the table is empty.
		 */
		@Override
		public java.util.Collection<CoronaData> values() {
			return fHashMap.values();
		}

		/**
		 * Pushes this object's data to the top of the Lua stack.
		 * @param luaState The Lua state to push data to. Cannot be null.
		 * @return Returns true if the push was successful. Returns false if failed.
		 */
		@Override
		public boolean pushTo(com.naef.jnlua.LuaState luaState) {
			// If this table is empty, then create an empty Lua table and stop here.
			int count = fHashMap.size();
			if (count <= 0) {
				luaState.newTable();
				return true;
			}

			// Push this table's entries into Lua.
			luaState.newTable(0, count);
			int luaTableStackIndex = luaState.getTop();
			for (java.util.Map.Entry<CoronaData.Value, CoronaData> entry : entrySet()) {
				if ((entry.getKey() != null) && (entry.getValue() != null)) {
					entry.getKey().pushTo(luaState);
					entry.getValue().pushTo(luaState);
					luaState.rawSet(luaTableStackIndex);
				}
			}
			return true;
		}

		/**
		 * Writes this object's data to the given XML writer.
		 * @param xmlWriter The serializer used to output this object's data to XML. Cannot be null.
		 */
		@Override
		public void writeTo(org.xmlpull.v1.XmlSerializer xmlWriter) throws java.io.IOException {
			xmlWriter.startTag("", "table");
			for (java.util.Map.Entry<CoronaData.Value, CoronaData> entry : entrySet()) {
				if ((entry.getKey() != null) && (entry.getValue() != null)) {
					xmlWriter.startTag("", "entry");
					{
						xmlWriter.startTag("", "key");
						{
							entry.getKey().writeTo(xmlWriter);
						}
						xmlWriter.endTag("", "key");
						xmlWriter.startTag("", "value");
						{
							entry.getValue().writeTo(xmlWriter);
						}
						xmlWriter.endTag("", "value");
					}
					xmlWriter.endTag("", "entry");
				}
			}
			xmlWriter.endTag("", "table");
		}

		/**
		 * Writes this object's data to the given parcel.
		 * @param parcel The parcel to write to. Cannot be null.
		 * @param flags Indicates how this object should be written. May be 0 or PARCELABLE_WRITE_RETURN_VALUE.
		 */
		@Override
		public void writeToParcel(android.os.Parcel parcel, int flags) {
			// Write the number of entries this table has.
			parcel.writeInt(size());

			// Write all entries.
			for (java.util.Map.Entry<CoronaData.Value, CoronaData> entry : entrySet()) {
				// Validate.
				if ((entry == null) || (entry.getKey() == null)) {
					continue;
				}

				// Write the key.
				parcel.writeParcelable(entry.getKey(), flags);

				// Write a flag indicating if the entry has a value.
				boolean hasValue = (entry.getValue() != null);
				parcel.writeByte((byte)(hasValue ? 1 : 0));

				// Write the entry's value if not null.
				if (hasValue) {
					parcel.writeParcelable(entry.getValue(), flags);
				}
			}
		}

		/**
		 * Creates a new table of Corona data from the given bundle.
		 * @param bundle The bundle to copy data from.
		 * @return Returns a new table containing a copy of the data in the given bundle. Note that any
		 *         bundle data that is not convertible to a Corona data type (as documented by the
		 *         CoronaData.from(Object) method) will not be copied into the new table.
		 *         <p>
		 *         Returns null if the given bundle was null.
		 */
		public static CoronaData.Table from(android.os.Bundle bundle) {
			// Validate.
			if (bundle == null) {
				return null;
			}

			// Copy the given bundle's data to a new table and return it.
			CoronaData.Table table = new CoronaData.Table();
			for (java.lang.String key : bundle.keySet()) {
				table.put(new CoronaData.String(key), CoronaData.from(bundle.get(key)));
			}
			return table;
		}

		/**
		 * Creates a new table of Corona data from the given JSON.
		 * @param jsonObject A JSON table to copy data from.
		 * @return Returns a new table containing a copy of the data in the given JSON object.
		 *         <p>
		 *         Returns null if the given JSON object was null.
		 */
		public static CoronaData.Table from(org.json.JSONObject jsonObject)
			throws org.json.JSONException
		{
			// Validate.
			if (jsonObject == null) {
				return null;
			}

			// Copy the given JSON object's data to a new table and return it.
			CoronaData.Table table = new CoronaData.Table();
			java.util.Iterator<java.lang.String> iterator = jsonObject.keys();
			if (iterator != null) {
				while (iterator.hasNext()) {
					java.lang.String key = iterator.next();
					table.put(new CoronaData.String(key), CoronaData.from(jsonObject.get(key)));
				}
			}
			return table;
		}

		/** Private class used to create instances of its outer class from data read from an Android parcel. */
		private static class ParcelableCreator implements android.os.Parcelable.Creator<CoronaData.Table> {
			/**
			 * Creates an object from data read from the given Android parcel.
			 * @param parcel The parcel to read data from. Cannot be null.
			 * @return Returns a new object containing data read from the given parcel.
			 */
			public CoronaData.Table createFromParcel(android.os.Parcel parcel) {
				CoronaData.Table table = new CoronaData.Table();
				ClassLoader classLoader = getClass().getClassLoader();
				int count = parcel.readInt();
				for (int index = 0; index < count; index++) {
					CoronaData.Value key = (CoronaData.Value)parcel.readParcelable(classLoader);
					CoronaData value = null;
					if (parcel.readByte() != 0) {
						value = (CoronaData)parcel.readParcelable(classLoader);
					}
					table.put(key, value);
				}
				return table;
			}

			/**
			 * Creates an array of objects.
			 * @param size The length of the array to be created. Must be greater than zero.
			 * @return Returns the requested object array.
			 */
			public CoronaData.Table[] newArray(int size) {
				return new CoronaData.Table[size];
			}
		}
	}
}

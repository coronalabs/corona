//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

public interface NativePropertyResponder {
	java.util.List<Object> getNativePropertyResponder();
	java.lang.Runnable getCustomPropertyAction(String key, boolean booleanValue, String stringValue, int integerValue, double doubleValue);
}

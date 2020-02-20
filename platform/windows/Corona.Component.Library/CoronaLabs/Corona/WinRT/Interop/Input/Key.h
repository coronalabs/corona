//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#define CORONA_KEY_PROPERTY_PROTOTYPE(propertyName) static property Key^ ##propertyName { Key^ get(); } \
	private: static const Key^ k##propertyName; public:

/// <summary>Provides information about a key such as its native integer key code and its unique Corona key name.</summary>
/// <remarks>
///  <para>
///   This class provides static instances of itself for all keys supported by Corona, such as the "Back" key which
///   represents the physical button on Windows Phone used to navigate backwards from the current page.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
public ref class Key sealed
{
	private:
		/// <summary>Creates a new key with the given information.</summary>
		/// <remarks>
		///  This constructor is made private to force the caller to use this class' static functions and properties.
		/// </remarks>
		/// <param name="coronaKeyName">
		///  <para>
		///   Corona's unique name for this key. This is expected to come from a string constant in Corona's
		///   "Rtt::KeyName" class.
		///  </para>
		///  <para>If the native key code is unknown, then this string should be set to Rtt::KeyName::kUnknown.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="nativeKeyCode">The platform's unique integer ID for this key.</param>
		Key(const char *coronaKeyName, int nativeKeyCode);

	public:
		/// <summary>Gets the platform's unique integer ID for this key.</summary>
		/// <remarks>Corona's "key" event in Lua provides this value via the "event.nativeKeyCode" property.</remarks>
		/// <value>
		///  The platform's unique integer ID for this key. This matches Windows' "virtual key constants"
		///  such as VK_BROWSER_BACK for the back key.
		/// </value>
		property int NativeKeyCode { int get(); }

		/// <summary>Gets Corona's unique name for this key.</summary>
		/// <remarks>Corona's "key" event in Lua provides this name via the "event.keyName" property.</remarks>
		/// <value>
		///  <para>Corona's unique name for this key.</para>
		///  <para>Set to "unknown" if Corona does not recognize the native key code for this key.</para>
		/// </value>
		property Platform::String^ CoronaName { Platform::String^ get(); }

		/// <summary>Gets Corona's unique name for this key.</summary>
		/// <remarks>Corona's "key" event in Lua provides this name via the "event.keyName" property.</remarks>
		/// <returns>
		///  <para>Returns Corona's unique name for this key.</para>
		///  <para>Returns "unknown" if Corona does not recognize the native key code for this key.</para>
		/// </returns>
		Platform::String^ ToString();

		/// <summary>The navigate "back" key. On Windows Phone, this is the physical back key on the device..</summary>
		CORONA_KEY_PROPERTY_PROTOTYPE(Back);

	internal:
		/// <summary>Gets a native C/C++ UTF-8 string pointer for this object's key name.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <value>C/C++ string pointer matching a constant in Corona's Rtt:KeyName class.</value>
		property const char* CoronaNameAsStringPointer { const char* get(); }

	private:
		/// <summary>Gets a private mutable collection used to store all keys provided by this class.</summary>
		/// <remarks>
		///  This is used by this class' constructor to automatically store all pre-defined key objects
		///  to be used by this class' static From() methods to do the equivalent of a Java/.NET reflection lookup
		///  for pre-existing objects.
		/// </remarks>
		static property Platform::Collections::Vector<Key^>^ MutableCollection
		{
			Platform::Collections::Vector<Key^>^ get();
		}

		Platform::String^ fCoronaName;
		const char *fCoronaNameAsStringPointer;
		int fNativeKeyCode;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input

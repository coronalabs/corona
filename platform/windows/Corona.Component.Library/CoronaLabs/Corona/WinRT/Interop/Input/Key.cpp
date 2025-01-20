//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Key.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#pragma warning(disable: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)
#	include "Rtt_KeyName.h"
#pragma warning(default: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#define CORONA_KEY_PROPERTY_IMPLEMENTATION(propertyName, coronaKeyName, nativeKeyCode) \
	const Key^ Key::k##propertyName = ref new Key(coronaKeyName, nativeKeyCode); \
	Key^ Key::##propertyName::get() { return const_cast<Key^>(Key::k##propertyName); }


#pragma region Pre-allocated Key Objects
	CORONA_KEY_PROPERTY_IMPLEMENTATION(Back, Rtt::KeyName::kBack, VK_BROWSER_BACK);

#pragma endregion


#pragma region Consructors/Destructors
Key::Key(const char *coronaKeyName, int nativeKeyCode)
:	fCoronaNameAsStringPointer(coronaKeyName),
	fCoronaName(nullptr),
	fNativeKeyCode(nativeKeyCode)
{
	// Add this instance to the collection.
	// Used to fetch a pre-existing instance via this class' static From() functions.
	Key::MutableCollection->Append(this);
}

#pragma endregion


#pragma region Public Functions/Properties
int Key::NativeKeyCode::get()
{
	return fNativeKeyCode;
}

Platform::String^ Key::CoronaName::get()
{
	return this->ToString();
}

Platform::String^ Key::ToString()
{
	if (!fCoronaName)
	{
		fCoronaName = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(fCoronaNameAsStringPointer);
	}
	return fCoronaName;
}

#pragma endregion


#pragma region Internal Functions/Properties
const char* Key::CoronaNameAsStringPointer::get()
{
	return fCoronaNameAsStringPointer;
}

#pragma endregion


#pragma region Private Functions/Properties
Platform::Collections::Vector<Key^>^ Key::MutableCollection::get()
{
	static Platform::Collections::Vector<Key^> sCollection;
	return %sCollection;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input

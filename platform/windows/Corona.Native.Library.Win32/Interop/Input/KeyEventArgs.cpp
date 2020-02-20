//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KeyEventArgs.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
KeyEventArgs::KeyEventArgs(const KeyEventArgs::Data& data)
:	EventArgs(),
	fData(data),
	fKey(Key::FromNativeCode(data.NativeCode))
{
}

KeyEventArgs::~KeyEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
Key KeyEventArgs::GetKey() const
{
	return fKey;
}

bool KeyEventArgs::IsDown() const
{
	return fData.IsDown;
}

bool KeyEventArgs::IsUp() const
{
	return !fData.IsDown;
}

ModifierKeyStates KeyEventArgs::GetModifierKeyStates() const
{
	return ModifierKeyStates(fData.ModifierFlags);
}

KeyEventArgs::Data KeyEventArgs::ToData() const
{
	return fData;
}

#pragma endregion


#pragma region Public Static Functions
KeyEventArgs KeyEventArgs::From(const KeyEventArgs::Data& data)
{
	return KeyEventArgs(data);
}

#pragma endregion

} }	// namespace Interop::Input

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

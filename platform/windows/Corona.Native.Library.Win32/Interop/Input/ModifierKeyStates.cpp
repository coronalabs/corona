//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ModifierKeyStates.h"
#include <Windows.h>


namespace Interop { namespace Input {

#pragma region Public Constants
const ModifierKeyStates::Flags ModifierKeyStates::kFlagAlt = 1;
const ModifierKeyStates::Flags ModifierKeyStates::kFlagControl = 2;
const ModifierKeyStates::Flags ModifierKeyStates::kFlagShift = 4;
const ModifierKeyStates::Flags ModifierKeyStates::kFlagCommand = 8;

#pragma endregion


#pragma region Constructors/Destructors
ModifierKeyStates::ModifierKeyStates()
:	fFlags(0)
{
}

ModifierKeyStates::ModifierKeyStates(const ModifierKeyStates::Flags& flags)
:	fFlags(flags)
{
}

ModifierKeyStates::~ModifierKeyStates()
{
}

#pragma endregion


#pragma region Public Methods
bool ModifierKeyStates::IsAltDown() const
{
	return ((fFlags & kFlagAlt) != 0);
}

void ModifierKeyStates::SetIsAltDown(bool value)
{
	if (value)
	{
		fFlags |= kFlagAlt;
	}
	else
	{
		fFlags &= ~kFlagAlt;
	}
}

bool ModifierKeyStates::IsControlDown() const
{
	return ((fFlags & kFlagControl) != 0);
}

void ModifierKeyStates::SetIsControlDown(bool value)
{
	if (value)
	{
		fFlags |= kFlagControl;
	}
	else
	{
		fFlags &= ~kFlagControl;
	}
}

bool ModifierKeyStates::IsShiftDown() const
{
	return ((fFlags & kFlagShift) != 0);
}

void ModifierKeyStates::SetIsShiftDown(bool value)
{
	if (value)
	{
		fFlags |= kFlagShift;
	}
	else
	{
		fFlags &= ~kFlagShift;
	}
}

bool ModifierKeyStates::IsCommandDown() const
{
	return ((fFlags & kFlagCommand) != 0);
}

void ModifierKeyStates::SetIsCommandDown(bool value)
{
	if (value)
	{
		fFlags |= kFlagCommand;
	}
	else
	{
		fFlags &= ~kFlagCommand;
	}
}

ModifierKeyStates::Flags ModifierKeyStates::GetFlags() const
{
	return fFlags;
}

#pragma endregion


#pragma region Public Static Functions
ModifierKeyStates ModifierKeyStates::FromKeyboard()
{
	ModifierKeyStates keyStates;
	if ((::GetKeyState(VK_LMENU) & 0x80) || (::GetKeyState(VK_RMENU) & 0x80))
	{
		keyStates.SetIsAltDown(true);
	}
	if ((::GetKeyState(VK_LCONTROL) & 0x80) || (::GetKeyState(VK_RCONTROL) & 0x80))
	{
		keyStates.SetIsControlDown(true);
	}
	if ((::GetKeyState(VK_LSHIFT) & 0x80) || (::GetKeyState(VK_RSHIFT) & 0x80) || (::GetKeyState(VK_CAPITAL) & 1))
	{
		keyStates.SetIsShiftDown(true);
	}
	if ((::GetKeyState(VK_LWIN) & 0x80) || (::GetKeyState(VK_RWIN) & 0x80))
	{
		keyStates.SetIsCommandDown(true);
	}
	return keyStates;
}

#pragma endregion

} }	// namespace Interop::Input

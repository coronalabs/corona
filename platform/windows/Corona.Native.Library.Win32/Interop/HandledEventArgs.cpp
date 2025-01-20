//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HandledEventArgs.h"


namespace Interop {

#pragma region Constructors/Destructors
HandledEventArgs::HandledEventArgs()
:	EventArgs(),
	fWasHandled(false)
{
}

HandledEventArgs::~HandledEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
bool HandledEventArgs::WasHandled() const
{
	return fWasHandled;
}

void HandledEventArgs::SetHandled()
{
	fWasHandled = true;
}

#pragma endregion

}	// namespace Interop

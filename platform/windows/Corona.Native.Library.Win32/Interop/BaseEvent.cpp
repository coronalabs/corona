//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseEvent.h"


namespace Interop {

#pragma region BaseEvent Constructors/Destructors
BaseEvent::BaseEvent()
{
}

BaseEvent::BaseEvent(const BaseEvent &event)
{
}

BaseEvent::~BaseEvent()
{
}

#pragma endregion

#pragma region BaseEvent::Handler Constructors/Destructors
BaseEvent::Handler::Handler()
{
}

BaseEvent::Handler::Handler(const BaseEvent::Handler &event)
{
}

BaseEvent::Handler::~Handler()
{
}

#pragma endregion

}	// namespace Interop

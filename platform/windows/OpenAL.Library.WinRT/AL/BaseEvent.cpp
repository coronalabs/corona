// ----------------------------------------------------------------------------
// 
// BaseEvent.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "BaseEvent.h"


namespace AL {

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

}	// namespace AL

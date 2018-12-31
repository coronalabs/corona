// ----------------------------------------------------------------------------
// 
// MessageEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "MessageEventArgs.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
MessageEventArgs::MessageEventArgs(Platform::String^ message)
:	fMessage(message)
{
}

#pragma endregion


#pragma region Public Properties
Platform::String^ MessageEventArgs::Message::get()
{
	return fMessage;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT

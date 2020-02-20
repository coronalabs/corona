//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

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

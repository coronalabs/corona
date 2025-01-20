//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HandleMessageEventArgs.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
HandleMessageEventArgs::HandleMessageEventArgs(const MessageSettings &settings)
:	HandledEventArgs(),
	fMessageSettings(settings),
	fReturnResult(0)
{
}

HandleMessageEventArgs::~HandleMessageEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
HWND HandleMessageEventArgs::GetWindowHandle() const
{
	return fMessageSettings.WindowHandle;
}

UINT HandleMessageEventArgs::GetMessageId() const
{
	return fMessageSettings.MessageId;
}

WPARAM HandleMessageEventArgs::GetWParam() const
{
	return fMessageSettings.WParam;
}

LPARAM HandleMessageEventArgs::GetLParam() const
{
	return fMessageSettings.LParam;
}

LRESULT HandleMessageEventArgs::GetReturnResult() const
{
	return fReturnResult;
}

void HandleMessageEventArgs::SetReturnResult(LRESULT value)
{
	fReturnResult = value;
}

#pragma endregion

} }	// namespace Interop::UI

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_Callback.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

/// <summary>Internal delegate intended to be used by the WinRTMethodCallback class.</summary>
delegate void WinRTMethodCallbackHandler();


/// <summary>Callback used to invoke a component class' method.</summary>
/// <remarks>Implements the MCallback interface so that it can be used by Corona's core classes.</remarks>
class WinRTMethodCallback : public BaseCallback
{
	public:
		WinRTMethodCallback()
		{
			fHandler = nullptr;
		}

		WinRTMethodCallback(WinRTMethodCallbackHandler^ handler)
		{
			fHandler = handler;
		}

		virtual void Invoke()
		{
			if (fHandler)
			{
				fHandler();
			}
		}

	private:
		WinRTMethodCallbackHandler^ fHandler;
};

}

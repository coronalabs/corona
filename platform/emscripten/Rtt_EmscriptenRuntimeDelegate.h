//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_RuntimeDelegate.h"
#include <string>

namespace Rtt
{
	/// Delegate used to receive events from the Corona Runtime.
	/// This is done by assigning an instance of this class to the Runtime.SetDelegate() function.
	class EmscriptenRuntimeDelegate : public RuntimeDelegate
	{
	public:
		EmscriptenRuntimeDelegate();
		virtual ~EmscriptenRuntimeDelegate();

		virtual void WillLoadMain(const Runtime& sender) const;
		virtual void DidLoadConfig( const Runtime& sender, lua_State *L ) const;

		mutable int fContentWidth;
		mutable int fContentHeight;
		mutable std::string fScaleMode;


	};
} // namespace Rtt


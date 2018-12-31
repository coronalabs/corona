// ----------------------------------------------------------------------------
// 
// Rtt_WinRTDisplayObject.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTDisplayObject.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_Lua.h"
#	include "Rtt_Runtime.h"
#	include "Rtt_RenderingStream.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTDisplayObject::WinRTDisplayObject(const Rect& bounds)
	: PlatformDisplayObject()
{
	//	// Convert the given x/y coordinates from a top-left reference point to a center reference point.
	//	Translate(bounds.xMin + Rtt_RealDiv2(bounds.Width()), bounds.yMin + Rtt_RealDiv2(bounds.Height()));
	//	fSelfBounds.MoveCenterToOrigin();
}

WinRTDisplayObject::~WinRTDisplayObject()
{
}

#pragma endregion


#pragma region Public Member Functions
void WinRTDisplayObject::InitializeView(void *view)
{
	// What is this?
}

void WinRTDisplayObject::SetFocus()
{
}

void WinRTDisplayObject::DidMoveOffscreen()
{
}

void WinRTDisplayObject::WillMoveOnscreen()
{
}

bool WinRTDisplayObject::CanCull() const
{
	// Disable culling for all native UI objects.
	// Note: This is needed so that the Build() function will get called when a native object
	//       is being moved partially or completely offscreen.
	return false;
}

void WinRTDisplayObject::Prepare(const Display& display)
{
	Super::Prepare(display);

	if (ShouldPrepare())
	{
/*
		// First, update this object's cached scale factors in case the app window has been resized.
		Rtt::Runtime *runtimePointer = NativeToJavaBridge::GetInstance()->GetRuntime();
		Preinitialize(runtimePointer->GetDisplay());

		// Update the native object's screen bounds.
		Rect bounds;
		GetScreenBounds(bounds);
		NativeToJavaBridge::GetInstance()->DisplayObjectUpdateScreenBounds(
		fId, bounds.xMin, bounds.yMin, bounds.Width(), bounds.Height());
*/
	}
}

void WinRTDisplayObject::Draw(Renderer& renderer) const
{
}

void WinRTDisplayObject::GetSelfBounds(Rect& rect) const
{
}

bool WinRTDisplayObject::HasBackground() const
{
	return true;
}

void WinRTDisplayObject::SetBackgroundVisible(bool isVisible)
{
}

int WinRTDisplayObject::ValueForKey(lua_State *L, const char key[]) const
{
	Rtt_ASSERT(key);

	int result = 1;

	if (strcmp("isVisible", key) == 0)
	{
	}
	else if (strcmp("alpha", key) == 0)
	{
	}
	else if (strcmp("hasBackground", key) == 0)
	{
		lua_pushboolean(L, HasBackground() ? 1 : 0);
	}
	else
	{
		result = 0;
	}

	return result;
}

bool WinRTDisplayObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	Rtt_ASSERT(key);

	bool result = true;

	if (strcmp("isVisible", key) == 0)
	{
	}
	else if (strcmp("alpha", key) == 0)
	{
	}
	else if (strcmp("hasBackground", key) == 0)
	{
	}
	else
	{
		result = false;
	}

	return result;
}

#pragma endregion

} // namespace Rtt

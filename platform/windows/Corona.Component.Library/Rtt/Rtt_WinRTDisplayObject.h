// ----------------------------------------------------------------------------
// 
// Rtt_WinRTDisplayObject.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_PlatformDisplayObject.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTDisplayObject : public PlatformDisplayObject
{
	public:
		WinRTDisplayObject(const Rect& bounds);
		virtual ~WinRTDisplayObject();

		void InitializeView(void * view);
		void SetFocus();
		bool HasBackground() const;
		void SetBackgroundVisible(bool isVisible);
		virtual void DidMoveOffscreen();
		virtual void WillMoveOnscreen();
		virtual bool CanCull() const;
		virtual void Prepare(const Display& display);
		virtual void Draw(Renderer& renderer) const;
		virtual void GetSelfBounds(Rect& rect) const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
};

} // namespace Rtt

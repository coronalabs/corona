// ----------------------------------------------------------------------------
// 
// Rtt_WinRTTextFieldObject.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
//		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "Rtt_WinRTDisplayObject.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTTextBoxObject : public WinRTDisplayObject
{
	public:
		typedef WinRTTextBoxObject Self;
		typedef WinRTDisplayObject Super;

		WinRTTextBoxObject(const Rect& bounds, bool isSingleLine);
		virtual ~WinRTTextBoxObject();

		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);

	protected:
		static int SetTextColor(lua_State *L);
		static int SetReturnKey(lua_State *L);

	private:
		/// Set TRUE if this is a single line text field. Set FALSE for a multiline text box.
		/// This value is not expected to change after initialization.
		bool fIsSingleLine;
};

} // namespace Rtt

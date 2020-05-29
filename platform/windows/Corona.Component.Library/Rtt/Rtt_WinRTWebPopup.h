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
#	include "Core/Rtt_Build.h"
#	include "Rtt_PlatformWebPopup.h"
#	include "Rtt_MPlatform.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTWebPopup : public PlatformWebPopup
{
	public:
		typedef WinRTWebPopup Self;
		typedef PlatformWebPopup Super;

		WinRTWebPopup();
		virtual ~WinRTWebPopup();

		virtual void Show(const MPlatform& platform, const char *url);
		virtual bool Close();
		virtual void Reset();
		virtual void SetPostParams(const MKeyValueIterable& params);
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
};

} // namespace Rtt

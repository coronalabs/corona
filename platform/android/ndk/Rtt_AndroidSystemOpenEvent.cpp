//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"
#include "NativeToJavaBridge.h"
#include "Rtt_AndroidSystemOpenEvent.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"


namespace Rtt
{

AndroidSystemOpenEvent::AndroidSystemOpenEvent(NativeToJavaBridge *ntjb)
:	SystemOpenEvent(NULL), fNativeToJavaBridge(ntjb)
{
}

AndroidSystemOpenEvent::~AndroidSystemOpenEvent()
{
}

int AndroidSystemOpenEvent::Push(lua_State *L) const
{
	if (Rtt_VERIFY(SystemOpenEvent::Push(L)))
	{
		fNativeToJavaBridge->PushApplicationOpenArgumentsToLuaTable(L);
	}
	return 1;
}

} // namespace Rtt

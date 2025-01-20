//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MLuaProxyable_H__
#define _Rtt_MLuaProxyable_H__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class LuaProxy;
//class LuaProxyVTable;

// ----------------------------------------------------------------------------

class MLuaProxyable
{
	public:
		virtual void InitProxy( lua_State *L ) = 0;
		virtual LuaProxy* GetProxy() const = 0;
		virtual void ReleaseProxy() = 0;
//		virtual const LuaProxyVTable& GetMetatable() = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MLuaProxyable_H__

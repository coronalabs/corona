//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaProxy_H__
#define _Rtt_LuaProxy_H__

// ----------------------------------------------------------------------------

struct lua_State;
struct luaL_Reg;

namespace Rtt
{

class LuaProxyVTable;
class MLuaProxyable;

// ----------------------------------------------------------------------------

class LuaProxyConstant
{
	public:
		typedef LuaProxyConstant Self;

	public:
		static Self* GetProxy( lua_State *L, int index );
		static const MLuaProxyable& NullProxyableObject();

		static int __index( lua_State *L );
		static int __gcMeta( lua_State *L );

	public:
		static void Initialize( lua_State *L );

	public:
		LuaProxyConstant( lua_State *L, const LuaProxyVTable& proxyMt );
		virtual ~LuaProxyConstant();

	public:
		Rtt_FORCE_INLINE const LuaProxyVTable& Delegate() const { return fDelegate; }

		void Push( lua_State *L ) const;

	private:
		const LuaProxyVTable& fDelegate;

	private:
		static const char kProxyKey[];
		static const luaL_Reg kMetatable[];
};

// ----------------------------------------------------------------------------

// USAGE:
// LuaProxy was designed to support exporting C++ object hierarchies to Lua
// Our main use case is DisplayObjects. Ultimately, the lua code sees a normal
// lua table that represents the proxy (which yields access to the DisplayObject). 
// One of the fields in the table is _proxy, which is a LuaProxy instance. 
// 
// There is some trickiness here b/c of memory mgmt:
// 
// On the lua side, as long as there are references to this lua table, 
// we cannot dispose of the C++ object (the _proxy userdata) that it references.
// 
// On the C++ side, the contents of this table must be persistent. Consider
// the situation where, on the lua side, other properties are added to the
// lua table, but the reference to that table was local, so it gets GC'd.
// Later in a different scope, the lua code attempts to access a table that 
// represents the same DisplayObject. The table we get better have those
// other properties persist! Our solution is to have LuaProxy luaL_ref() 
// the table *whenever* the DisplayObject is on the display list.
class LuaProxy
{
	public:
		typedef LuaProxy Self;

	public:
		static bool IsProxy(lua_State *L, int index);
		static LuaProxy* GetProxy( lua_State *L, int index );
		static Self* GetProxyMeta( lua_State *L, int index );
		static MLuaProxyable* GetProxyableObject( lua_State *L, int index );

		// static int luaopen_proxy( lua_State *L );
		static int __proxyindex( lua_State *L );
		static int __proxynewindex( lua_State *L );
		static int __proxyregister( lua_State *L );
//		static int __proxylen( lua_State *L );
		static int __gcMeta( lua_State *L );

	public:
		static void Initialize( lua_State *L );

	public:
		LuaProxy( lua_State *L, MLuaProxyable& object, const LuaProxyVTable& proxyMt, const char* className );
		virtual ~LuaProxy();

	protected:
		void CreateTable( lua_State *L, const char* className );

	public:
		// Restores table (ref'd by fTableRef) back to a plain table (i.e. resets 
		// metatable, resets reserved properties, etc).  Also does equivalent of
		// Release()
		void RestoreTable( lua_State *L );

	protected:
		// Releases reference to the MLuaProxyable object. Used when Lua calls
		// LuaProxy's __gc metamethod.
		void Release();

	public:
		void AcquireTableRef( lua_State *L );
		void ReleaseTableRef( lua_State *L );

	public:
		Rtt_INLINE void Invalidate() 
		{ 
			fObject = NULL; 
		}
		Rtt_INLINE MLuaProxyable* Object() const { return fObject; }
		Rtt_INLINE int TableRef() const { return fTableRef; }
		Rtt_INLINE const LuaProxyVTable& Delegate() const { return fDelegate; }

		// Push associated table on stack. Nothing is pushed when returning false.
		bool PushTable( lua_State *L ) const;

	public:
		const LuaProxyVTable* GetExtensionsDelegate() const { return fExtensionsDelegate; }
		void SetExtensionsDelegate( const LuaProxyVTable *newValue ) { fExtensionsDelegate = newValue; }

	private:
		MLuaProxyable* fObject;
		const LuaProxyVTable& fDelegate;
		const LuaProxyVTable *fExtensionsDelegate;

		int fTableRef; // id for associated Lua table that wraps this proxy

	private:
		static const char kProxyKey[];
		static const luaL_Reg kMetatable[];
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Matrix_H__

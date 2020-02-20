//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaLuaLibrary_H__
#define _CoronaLuaLibrary_H__

#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Corona
{

class LuaLibraryMetadata;

// ----------------------------------------------------------------------------

// NOTE: For ***INTERNAL*** use only.
// C++ ABI issues disallow making this publicly available
//
// Convenience class for creating internal Lua libraries in plugins.
class LuaLibrary
{
	public:
		typedef LuaLibrary Self;

	public:
		static void InitializeMetatable( lua_State *L );

	public:
		template < class T >
		static int OpenWrapper( lua_State *L );

	public:
		LuaLibrary();
		virtual ~LuaLibrary();

	public:
		// Called prior to OpenWrapper()
		virtual bool Initialize( lua_State *L, void *platformContext );

	public:
		virtual const char *GetName() const;
		virtual const char *GetPublisherId() const;
		virtual int GetVersion() const;
		virtual int GetRevision() const;

	public:
		virtual int ValueForKey( lua_State *L );

	protected:
		virtual lua_CFunction GetFactory() const;
		virtual const luaL_Reg *GetFunctions() const;
		virtual int Open( lua_State *L ) const;
		int OpenWrapper( lua_State *L );

	protected:
		static int Finalizer( lua_State *L );
		static Self *GetLibrary( lua_State *L );
		static int GetProperty( lua_State *L );

	private:
		mutable LuaLibraryMetadata *fMetadata;
};

// ----------------------------------------------------------------------------

template < class T >
int LuaLibrary::OpenWrapper( lua_State *L )
{
	T *library = new T;
	return library->OpenWrapper( L );
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

#endif // _CoronaLuaLibrary_H__

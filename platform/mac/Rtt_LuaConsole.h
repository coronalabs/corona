//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_LuaConsole_H__
#define __Rtt_LuaConsole_H__

#include "Rtt_MCallback.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class LuaConsole : public MCallback
{
	public:
		typedef LuaConsole Self;

	public:
		static void Initialize();
		static void Finalize();

	public:
		LuaConsole( lua_State *L, const Runtime& runtime );
		~LuaConsole();

	public:
		// MCallback
		virtual void operator()();

	protected:
		char* CheckBuffer( size_t numBytes );
		bool ReadLine( bool firstline );
		int LoadLine();
		void dotty();
		static int dotty_glue( lua_State *L );

	private:
		lua_State* fL;
		const Runtime& fRuntime;
		char* fBuffer;
		size_t fBufferSize;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_LuaConsole_H__

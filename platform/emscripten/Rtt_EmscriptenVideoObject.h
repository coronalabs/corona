//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_EmscriptenDisplayObject.h"
#include "Corona/CoronaLua.h"
#include <string.h>

namespace Rtt
{
	class EmscriptenVideoObject : public EmscriptenDisplayObject
	{
	public:
		typedef EmscriptenVideoObject Self;
		typedef EmscriptenDisplayObject Super;

		struct listener : public ref_counted
		{
			listener(const ResourceHandle<lua_State> & handle, const char* eventName, int listenerIndex);
			virtual ~listener();
			void dispatch(int elementID, int eventID);

			CoronaLuaRef fLuaReference;
			std::string fEventName;
			ResourceHandle<lua_State> fLuaState;
		};

		EmscriptenVideoObject(const Rect& bounds);
		virtual ~EmscriptenVideoObject();

		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		void dispatch(int eventID);
		void load(const char* source);

		bool fAutoPlay;

	protected:
		static int LuaLoad(lua_State *L);
		static int Play(lua_State *L);
		static int Pause(lua_State *L);
		static int Seek(lua_State *L);
		static int addEventListener(lua_State *L);

		smart_ptr<listener> fListener;
	};

} // namespace Rtt


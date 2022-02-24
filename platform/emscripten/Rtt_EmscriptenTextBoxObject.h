//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Corona/CoronaLua.h"
#include "Rtt_EmscriptenDisplayObject.h"
#include "Display/Rtt_TextObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EmscriptenTextBoxObject : public EmscriptenDisplayObject
{
	public:
		typedef EmscriptenTextBoxObject Self;
		typedef EmscriptenDisplayObject Super;

		struct listener : public ref_counted
		{
			listener(const ResourceHandle<lua_State> & handle, const char* eventName, int listenerIndex);
			virtual ~listener();
			void dispatch(int elementID, int eventID, LuaProxy* proxy);

			CoronaLuaRef fLuaReference;
			std::string fEventName;
			ResourceHandle<lua_State> fLuaState;
		};

		EmscriptenTextBoxObject(const Rect& bounds, bool isSingleLine);
		virtual ~EmscriptenTextBoxObject();

		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		static int addEventListener(lua_State *L);

		void dispatch(int eventID);

	protected:
		static int SetTextColor(lua_State *L);
		static int SetReturnKey(lua_State *L);

	private:
		/// Set TRUE if this is a single line text field. Set FALSE for a multiline text box.
		/// This value is not expected to change after initialization.
		bool fIsSingleLine;
		smart_ptr<listener> fListener;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

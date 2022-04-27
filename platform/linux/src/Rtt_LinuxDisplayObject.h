//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Geometry.h"
#include "CoronaLua.h"
#include "Rtt_PlatformDisplayObject.h"
#include "Rtt_LinuxContainer.h"
#include "SDL.h"

#pragma region Forward Declarations

namespace Rtt
{
	class Display;
	class Renderer;
};

extern "C"
{
	struct lua_State;
}

#pragma endregion

namespace Rtt
{
	class LinuxDisplayObject : public PlatformDisplayObject
	{
	public:
		LinuxDisplayObject(const Rect &bounds);
		virtual ~LinuxDisplayObject();

		void SetBackgroundVisible(bool isVisible);
		virtual bool CanCull() const;
		virtual void Draw(Renderer& renderer) const override {}
		virtual void Draw() {}		// for ImGui renderer
		virtual void GetSelfBounds(Rect &rect) const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		virtual bool Initialize() override { return true; };
		virtual void Prepare(const Display &display) override;
		void showControls(bool val);
		void setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual bool ProcessEvent(const SDL_Event& evt) { return false; }

	protected:

		void addEventListener(lua_State* L);
		CoronaLuaRef fLuaReference;
		Rect fBounds;
	};
}; // namespace Rtt

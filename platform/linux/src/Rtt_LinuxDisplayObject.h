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
#include "Rtt_PlatformDisplayObject.h"
#include "Rtt_LinuxContainer.h"
#include <wx/window.h>

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
		LinuxDisplayObject(const Rect &bounds, const char *elementType);
		virtual ~LinuxDisplayObject();
		void SetFocus();
		bool HasBackground() const;
		void SetBackgroundVisible(bool isVisible);
		virtual void DidMoveOffscreen(StageObject *oldStage);
		virtual void WillMoveOnscreen(StageObject *newStage);
		virtual bool CanCull() const;
		virtual void Draw(Renderer &renderer) const;
		virtual void GetSelfBounds(Rect &rect) const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		virtual bool Initialize();
		virtual void Prepare(const Display &display);
		lua_State *GetLuaState() const { return fLuaState; }
		int GetListenerRef() const { return fListenerRef; }
		void showControls(bool val);
		void setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	protected:
		wxWindow *fWindow;

	private:
		Rect fSelfBounds;
		int fListenerRef;
		lua_State *fLuaState;
	};
}; // namespace Rtt

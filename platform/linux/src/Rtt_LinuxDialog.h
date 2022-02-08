//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_LinuxContainer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

namespace Rtt
{
	struct Im : public ref_counted
	{
		virtual void Draw() = 0;
	};

	struct ImAbout : public Im
	{
		void Draw() override;
	};

	struct ImMenu : public Im
	{
		ImMenu(const std::string& appName)
		{
			isMainMenu = appName == "Solar2D Simulator";
		}
		void Draw() override;

	private:
		bool isMainMenu;
	};

}

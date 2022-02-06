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
// nuklear
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
//#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#include <SDL2/SDL.h>

namespace Rtt
{
	struct DlgWindow : public ref_counted
	{
		DlgWindow(nk_context* nkctx, SDL_Window* sdlwin)
			: ctx(nkctx) 
			, win(sdlwin)
		{}

		virtual void advance(const char* appName) = 0;

	protected:
		nk_context* ctx;
		SDL_Window* win;
	};

	struct DlgAbout : public DlgWindow
	{
		DlgAbout(nk_context* nkctx, SDL_Window* sdlwin) 
			: DlgWindow(nkctx, sdlwin) 
		{}

		void advance(const char* appName) override;
	};

	struct DlgFile : public DlgWindow
	{
		DlgFile(nk_context* nkctx, SDL_Window* sdlwin, const std::string& fProjectPath);
		virtual ~DlgFile();
		void advance(const char* appName) override;

	};

	struct DlgMenu : public DlgWindow
	{
		DlgMenu(nk_context* nkctx, SDL_Window* sdlwin, const std::string& fProjectPath);
		virtual ~DlgMenu();
		void advance(const char* appName) override;
		inline bool IsSubMenuVisible() const { return fSubMenuVisible;	};

	private:

		bool fSubMenuVisible;

	};

}




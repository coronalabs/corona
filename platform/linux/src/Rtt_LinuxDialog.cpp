//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxDialog.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxApp.h"
#include "Rtt_Version.h"
#include "Rtt_FileSystem.h"
#include <dirent.h>
#include <unistd.h>

using namespace std;

namespace Rtt
{

	void DlgAbout::advance(const char* appName)
	{
		int w, h;
		SDL_GetWindowSize(win, &w, &h);

		int about_width = 450;
		int about_height = 225;
		int x = (w - about_width) / 2;
		int y = (h - about_height) / 2;

		struct nk_style& style = ctx->style;
		style.button.border_color = nk_rgba(0, 0, 0, 0);
		style.button.text_background = nk_rgb(255, 255, 255);
		style.button.text_normal = style.button.text_hover = style.button.text_active = nk_rgb(0, 0, 0);
		style.button.normal = nk_style_item_color(nk_rgb(222, 222, 222));
		style.button.hover = nk_style_item_color(nk_rgb(177, 177, 177));
		style.button.active = nk_style_item_color(nk_rgb(111, 111, 111));

		style.menu_button.border_color = nk_rgba(0, 0, 0, 0);
		style.menu_button.text_background = nk_rgb(255, 255, 255);
		style.menu_button.text_normal = style.menu_button.text_hover = style.menu_button.text_active = nk_rgb(0, 0, 0);
		style.menu_button.normal = nk_style_item_color(nk_rgb(255, 255, 255));
		style.menu_button.hover = nk_style_item_color(nk_rgb(222, 222, 222));

		style.window.background = nk_rgb(255, 255, 255);
		style.window.fixed_background = nk_style_item_color(nk_rgb(255, 255, 255));
		style.window.header.normal = style.window.header.active = style.window.header.hover = nk_style_item_color(nk_rgb(0, 0, 200));

		if (nk_begin(ctx, "About", nk_rect(x, y, about_width, about_height), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Solar2D Simulator", NK_TEXT_CENTERED);

			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, Rtt_STRING_BUILD, NK_TEXT_CENTERED);

			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, Rtt_STRING_COPYRIGHT, NK_TEXT_CENTERED);

			nk_layout_row_dynamic(ctx, 30, 1);
			const char* url = "https://solar2d.com";
			if (nk_button_label(ctx, url))
			{
				OpenURL(url);
			}

			nk_layout_row_dynamic(ctx, 30, 1);
			if (nk_button_label(ctx, "OK"))
			{
				SDL_Event e = {};
				e.type = sdl::OnCloseDialog;
				SDL_PushEvent(&e);
			}

			string iconPath = GetStartupPath(NULL);
			iconPath.append("/Resources/solar2d.png");
			if (Rtt_FileExists(iconPath.c_str()))
			{
				//wxIcon icon = wxIcon(iconPath.c_str(), wxBITMAP_TYPE_PNG, 60, 60);
				//info.SetIcon(icon);
			}

		}
		nk_end(ctx);
	}

	//
	// file_browser
	//
	DlgFile::DlgFile(nk_context* nkctx, SDL_Window* sdlwin, const string& fProjectPath)
		: DlgWindow(nkctx, sdlwin)
	{
	}

	DlgFile::~DlgFile()
	{
	}

	void DlgFile::advance(const char* appName)
	{
	}

	//
	//	main menu
	//
	DlgMenu::DlgMenu(nk_context* nkctx, SDL_Window* sdlwin, const string& fProjectPath)
		: DlgWindow(nkctx, sdlwin)
		, fSubMenuVisible(false)
	{
	}

	DlgMenu::~DlgMenu()
	{
	}

	void DlgMenu::advance(const char* appName)
	{
		int w, h;
		SDL_GetWindowSize(win, &w, &h);

		struct nk_style& style = ctx->style;
		style.button.border_color = nk_rgba(0, 0, 0, 0);
		style.button.text_background = nk_rgb(255, 255, 255);
		style.button.text_normal = style.button.text_hover = style.button.text_active = nk_rgb(0, 0, 0);
		style.button.normal = nk_style_item_color(nk_rgb(222, 222, 222));
		style.button.hover = nk_style_item_color(nk_rgb(177, 177, 177));
		style.button.active = nk_style_item_color(nk_rgb(111, 111, 111));

		style.menu_button.border_color = nk_rgba(0, 0, 0, 0);
		style.menu_button.text_background = nk_rgb(255, 255, 255);
		style.menu_button.text_normal = style.menu_button.text_hover = style.menu_button.text_active = nk_rgb(0, 0, 0);
		style.menu_button.normal = nk_style_item_color(nk_rgb(255, 255, 255));
		style.menu_button.hover = nk_style_item_color(nk_rgb(222, 222, 222));
		style.menu_button.active = nk_style_item_color(nk_rgb(111, 111, 111));

		style.contextual_button.border_color = nk_rgba(0, 0, 0, 0);
		style.contextual_button.text_background = nk_rgb(255, 255, 255);
		style.contextual_button.text_normal = style.menu_button.text_hover = style.menu_button.text_active = nk_rgb(0, 0, 0);
		style.contextual_button.normal = nk_style_item_color(nk_rgb(255, 255, 255));
		style.contextual_button.hover = nk_style_item_color(nk_rgb(222, 222, 222));
		style.contextual_button.active = nk_style_item_color(nk_rgb(111, 111, 111));

		style.window.background = nk_rgb(255, 255, 255);
		style.window.fixed_background = nk_style_item_color(nk_rgb(255, 255, 255));
		style.window.border_color = nk_rgba(0, 0, 0, 0);
		style.window.popup_border_color = nk_rgba(0, 0, 0, 0);
		style.window.combo_border_color = nk_rgba(0, 0, 0, 0);
		style.window.contextual_border_color = nk_rgba(0, 0, 0, 0);
		style.window.menu_border_color = nk_rgba(0, 0, 0, 0);
		style.window.group_border_color = nk_rgba(0, 0, 0, 0);
		style.window.tooltip_border_color = nk_rgba(0, 0, 0, 0);

		int menu_width = 250;
		fSubMenuVisible = false;
		if (strcmp(appName, "Solar2D Simulator") == 0)		// hack
		{
			// main menu
			if (nk_begin(ctx, "main", nk_rect(0, 0, w, 34), 0))
			{
				nk_menubar_begin(ctx);
				nk_layout_row_begin(ctx, NK_STATIC, 20, 2);
				nk_layout_row_push(ctx, 45);
				if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(menu_width, 200)))
				{
					fSubMenuVisible = true;
					nk_layout_row_dynamic(ctx, 30, 1);

					if (nk_menu_item_label(ctx, "New Project / Ctrl-N", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnNewProject;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Open Project / Ctrl-O", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnOpenFileDialog;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Relaunch Last Prokect", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnRelaunchLastProject;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Preferences...", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnOpenPreferences;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = SDL_QUIT;
						SDL_PushEvent(&e);
					}
					nk_menu_end(ctx);
				}
				nk_layout_row_push(ctx, 45);
				if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(menu_width, 200)))
				{
					fSubMenuVisible = true;
					nk_layout_row_dynamic(ctx, 30, 1);

					if (nk_menu_item_label(ctx, "Online Documentation...", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnOpenDocumentation;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Sample projects...", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnOpenSampleProjects;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "About Simulator...", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnAbout;
						SDL_PushEvent(&e);
					}
					nk_menu_end(ctx);
				}
				nk_layout_row_end(ctx);
				nk_menubar_end(ctx);
			}
			nk_end(ctx);
		}
		else
		{
			// project's menu
			if (nk_begin(ctx, "project", nk_rect(0, 0, w, 28), 0))
			{
				nk_menubar_begin(ctx);
				nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
				nk_layout_row_push(ctx, 45);
				if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(menu_width, 200)))
				{
					fSubMenuVisible = true;
					nk_layout_row_dynamic(ctx, 30, 1);

					if (nk_menu_item_label(ctx, "Build", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnBuild;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Open in Editor", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnOpenInEditor;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Show Project Files", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnShowProjectFiles;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Show Project Sandbox", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnShowProjectSandbox;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Clear Project Sandbox", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnClearProjectSandbox;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Relaunch / Ctrl+R", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnRelaunch;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Close Project", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnClose;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = SDL_QUIT;
						SDL_PushEvent(&e);
					}
					nk_menu_end(ctx);
				}
				nk_layout_row_push(ctx, 45);
				if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(menu_width, 200)))
				{
					fSubMenuVisible = true;
					nk_layout_row_dynamic(ctx, 30, 1);

					if (nk_menu_item_label(ctx, "Online Documentation...", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnOpenDocumentation;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "Sample projects...", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnOpenSampleProjects;
						SDL_PushEvent(&e);
					}

					if (nk_menu_item_label(ctx, "About Simulator...", NK_TEXT_LEFT))
					{
						SDL_Event e = {};
						e.type = sdl::OnAbout;
						SDL_PushEvent(&e);
					}
					nk_menu_end(ctx);
				}
				nk_layout_row_end(ctx);
				nk_menubar_end(ctx);
			}
			nk_end(ctx);
		}
	}

}

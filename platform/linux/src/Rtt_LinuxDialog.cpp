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

		style.window.background = nk_rgb(255, 255, 255);
		style.window.fixed_background = nk_style_item_color(nk_rgb(255, 255, 255));

		int menu_width = 250;
		if (strcmp(appName, "Solar2D Simulator") == 0)		// hack
		{
			// main menu
			if (nk_begin(ctx, "main", nk_rect(0, 0, w, 28), 0))
			{
				nk_menubar_begin(ctx);
				nk_layout_row_begin(ctx, NK_STATIC, 20, 2);
				nk_layout_row_push(ctx, 45);
				if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(menu_width, 200)))
				{
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
			if (nk_begin(ctx, "project", nk_rect(0, 0, 300, 100), 0))
			{
				nk_menubar_begin(ctx);
				nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
				nk_layout_row_push(ctx, 45);
				if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(menu_width, 200)))
				{
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

		/* {
		// project's menu
		{
			fMenuProject = new wxMenuBar();

			// file Menu
			wxMenu* fileMenu = new wxMenu();
			fileMenu->Append(ID_MENU_NEW_PROJECT, _T("&New Project	\tCtrl-N"));
			fileMenu->Append(ID_MENU_OPEN_PROJECT, _T("&Open Project	\tCtrl-O"));
			fileMenu->AppendSeparator();

			wxMenu* buildMenu = new wxMenu();
			buildMenu->Append(ID_MENU_BUILD_ANDROID, _T("Android	\tCtrl-B"));
			wxMenuItem* buildForWeb = buildMenu->Append(ID_MENU_BUILD_WEB, _T("HTML5	\tCtrl-Shift-Alt-B"));
			wxMenu* buildForLinuxMenu = new wxMenu();
			buildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("x64	\tCtrl-Alt-B"));
			wxMenuItem* buildForARM = buildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("ARM	\tCtrl-Alt-A"));
			buildMenu->AppendSubMenu(buildForLinuxMenu, _T("&Linux"));
			fileMenu->AppendSubMenu(buildMenu, _T("&Build"));
			buildForARM->Enable(false);

			fileMenu->Append(ID_MENU_OPEN_IN_EDITOR, _T("&Open In Editor	\tCtrl-Shift-O"));
			fileMenu->Append(ID_MENU_SHOW_PROJECT_FILES, _T("&Show Project Files"));
			fileMenu->Append(ID_MENU_SHOW_PROJECT_SANDBOX, _T("&Show Project Sandbox"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_CLEAR_PROJECT_SANDBOX, _T("&Clear Project Sandbox"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_RELAUNCH_PROJECT, _T("Relaunch	\tCtrl-R"));
			fileMenu->Append(ID_MENU_CLOSE_PROJECT, _T("Close Project	\tCtrl-W"));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_PREFERENCES, _T("&Preferences..."));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_EXIT, _T("&Exit"));
			fMenuProject->Append(fileMenu, _T("&File"));

			// hardware menu
			fHardwareMenu = new wxMenu();
			wxMenuItem* rotateLeft = fHardwareMenu->Append(wxID_HELP_CONTENTS, _T("&Rotate Left"));
			wxMenuItem* rotateRight = fHardwareMenu->Append(wxID_HELP_INDEX, _T("&Rotate Right"));
			//fHardwareMenu->Append(wxID_ABOUT, _T("&Shake"));
			fHardwareMenu->AppendSeparator();
			wxMenuItem* back = fHardwareMenu->Append(ID_MENU_BACK_BUTTON, _T("&Back"));
			fHardwareMenu->AppendSeparator();
			fHardwareMenu->Append(ID_MENU_SUSPEND, _T("&Suspend	\tCtrl-Down"));
			fMenuProject->Append(fHardwareMenu, _T("&Hardware"));
			rotateLeft->Enable(false);
			rotateRight->Enable(false);

			// view menu
			fViewMenu = new wxMenu();
			fZoomIn = fViewMenu->Append(ID_MENU_ZOOM_IN, _T("&Zoom In \tCtrl-KP_ADD"));
			fZoomOut = fViewMenu->Append(ID_MENU_ZOOM_OUT, _T("&Zoom Out \tCtrl-KP_Subtract"));
			fViewMenu->AppendSeparator();
			fMenuProject->Append(fViewMenu, _T("&View"));

			// about menu
			wxMenu* helpMenu = new wxMenu();
			helpMenu->Append(ID_MENU_OPEN_DOCUMENTATION, _T("&Online Documentation..."));
			helpMenu->Append(ID_MENU_OPEN_SAMPLE_CODE, _T("&Sample projects..."));
			//			helpMenu->Append(ID_MENU_HELP_BUILD_ANDROID, _T("&Building For Android"));
			helpMenu->Append(wxID_ABOUT, _T("&About Simulator..."));
			fMenuProject->Append(helpMenu, _T("&Help"));
		}*/
	}



}

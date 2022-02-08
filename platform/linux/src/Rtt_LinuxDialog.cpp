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

	void PushEvent(int evt)
	{
		SDL_Event e = {};
		e.type = evt;
		SDL_PushEvent(&e);
	}

	void ImAbout::Draw()
	{
	}

	void ImMenu::Draw()
	{
		if (isMainMenu)
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New Project/Ctrl+N", "CTRL+N"))
					{
						PushEvent(sdl::OnNewProject);
					}
					if (ImGui::MenuItem("Open Project/Ctrl+N", "CTRL+O")) 
					{
						PushEvent(sdl::OnOpenFileDialog);
					}
					if (ImGui::MenuItem("Relaunch Last Prokect", NULL)) 
					{
						PushEvent(sdl::OnRelaunchLastProject);
					}
					if (ImGui::MenuItem("Preferences...", NULL)) 
					{
						PushEvent(sdl::OnOpenPreferences);
					}
					if (ImGui::MenuItem("Exit", NULL)) 
					{
						PushEvent(SDL_QUIT);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Help"))
				{
					if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
					if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
					ImGui::Separator();
					if (ImGui::MenuItem("Cut", "CTRL+X")) {}
					if (ImGui::MenuItem("Copy", "CTRL+C")) {}
					if (ImGui::MenuItem("Paste", "CTRL+V")) {}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}

		}
		else
		{

		}

/*		if (strcmp(appName, "Solar2D Simulator") == 0)		// hack
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
		}*/
	}

}

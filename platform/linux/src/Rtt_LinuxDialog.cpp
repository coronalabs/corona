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

	//
	// About dialog
	//

	void ImAbout::Draw()
	{
	}

	//
	// File dialog
	//

	ImFile::ImFile(const std::string& startFolder)
	{
		// (optional) set browser properties
		fileDialog.SetTitle("title");
		fileDialog.SetTypeFilters({ ".lua" });
		fileDialog.Open();
	}

	ImFile::~ImFile()
	{

	}

	void ImFile::Draw()
	{
		fileDialog.Display();
		if (fileDialog.HasSelected())
		{
			std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
			fileDialog.ClearSelected();

			SDL_Event e = {};
			e.type = sdl::OnFileBrowserSelected;
			e.user.data1 = strdup(fileDialog.GetSelected().string().c_str());
			SDL_PushEvent(&e);
		}

	}

	//
	// Menu
	//

	void ImMenu::Draw()
	{
		if (isMainMenu)
		{
			// main menu
			if (ImGui::BeginMainMenuBar())
			{
				// main menu
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New Project...", "Ctrl+N"))
					{
						PushEvent(sdl::OnNewProject);
					}
					if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
					{
						PushEvent(sdl::OnOpenProject);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Relaunch Last Project", "Ctrl+R"))
					{
						PushEvent(sdl::OnRelaunchLastProject);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Preferences...", NULL))
					{
						PushEvent(sdl::OnOpenPreferences);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Exit", NULL))
					{
						PushEvent(SDL_QUIT);
					}
					ImGui::EndMenu();
				}

				// main menu
				if (ImGui::BeginMenu("Help"))
				{
					if (ImGui::MenuItem("Online Documentation...", NULL))
					{
						PushEvent(sdl::OnOpenDocumentation);
					}
					if (ImGui::MenuItem("Sample projects...", NULL))
					{
						PushEvent(sdl::OnOpenSampleProjects);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("About Simulator...", NULL))
					{
						PushEvent(sdl::OnAbout);
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}
		else
		{
			// project menu
			if (ImGui::BeginMainMenuBar())
			{
				// project menu
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New Project...", "Ctrl+N"))
					{
						PushEvent(sdl::OnNewProject);
					}
					if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
					{
						PushEvent(sdl::OnOpenProject);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Build", NULL))
					{
					}
					if (ImGui::MenuItem("Open in Editor", "Ctrl+Shift+O"))
					{
						PushEvent(sdl::OnOpenInEditor);
					}
					if (ImGui::MenuItem("Show Project Files", NULL))
					{
						PushEvent(sdl::OnShowProjectFiles);
					}
					if (ImGui::MenuItem("Show Project Sandbox", NULL))
					{
						PushEvent(sdl::OnShowProjectSandbox);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Clear Project Sandbox", NULL))
					{
						PushEvent(sdl::OnClearProjectSandbox);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Relaunch", "Ctrl+R"))
					{
						PushEvent(sdl::OnRelaunch);
					}
					if (ImGui::MenuItem("Close Project", "Ctrl+W"))
					{
						PushEvent(sdl::OnCloseProject);
					}
					if (ImGui::MenuItem("Preferences...", NULL))
					{
						PushEvent(sdl::OnOpenPreferences);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Exit", NULL))
					{
						PushEvent(SDL_QUIT);
					}
					ImGui::EndMenu();
				}

				// project menu
				if (ImGui::BeginMenu("Hardware"))
				{
					if (ImGui::MenuItem("Rotate Left", "Ctrl+Left"))
					{
					}
					if (ImGui::MenuItem("Rotate Right", "Ctrl+Right"))
					{
					}
					if (ImGui::MenuItem("Shake", "Ctrl+Up"))
					{
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Back", "Alt+Left"))
					{
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Suspend", "Ctrl+Down"))
					{
					}
					ImGui::EndMenu();
				}

				// project menu
				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Zoom In", "Ctrl++"))
					{
					}
					if (ImGui::MenuItem("Zoom Out", "Ctrl+-"))
					{
					}
					ImGui::Separator();
					if (ImGui::MenuItem("View As...", NULL))
					{
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Welcome screen", NULL))
					{
					}
					if (ImGui::MenuItem("Console", NULL))
					{
					}
					ImGui::EndMenu();
				}

				// project menu
				if (ImGui::BeginMenu("Help"))
				{
					if (ImGui::MenuItem("Online Documentation...", NULL))
					{
						PushEvent(sdl::OnOpenDocumentation);
					}
					if (ImGui::MenuItem("Sample projects...", NULL))
					{
						PushEvent(sdl::OnOpenSampleProjects);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("About Simulator...", NULL))
					{
						PushEvent(sdl::OnAbout);
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}
	}
}


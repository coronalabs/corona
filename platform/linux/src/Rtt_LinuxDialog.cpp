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

#define STB_IMAGE_IMPLEMENTATION
#include "imgui/stb_image.h"

using namespace std;

namespace Rtt
{

	void PushEvent(int evt)
	{
		SDL_Event e = {};
		e.type = evt;
		SDL_PushEvent(&e);
	}

	// Simple helper function to load an image into a OpenGL texture with common settings
	bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
	{
		// Load from file
		int image_width = 0;
		int image_height = 0;
		unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
		if (image_data == NULL)
			return false;

		// Create a OpenGL texture identifier
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

		// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		stbi_image_free(image_data);

		*out_texture = image_texture;
		*out_width = image_width;
		*out_height = image_height;

		return true;
	}

	//
	// About dialog
	//

	ImAbout::ImAbout()
		: tex_id(0)
		, width(0)
		, height(0)
	{
		string iconPath = GetStartupPath(NULL);
		iconPath.append("/Resources/solar2d.png");
		LoadTextureFromFile(iconPath.c_str(), &tex_id, &width, &height);
	}

	ImAbout::~ImAbout()
	{
		if (tex_id > 0)
		{
			glDeleteTextures(1, &tex_id);
		}
	}

	void ImAbout::Draw()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();

			if (tex_id > 0)
			{
				ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
				ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
				ImVec4 border_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
				ImGui::SetCursorPosX((window_size.x - width) * 0.5f);
				ImGui::Image((void*)(intptr_t)tex_id, ImVec2(width / 2, height / 2), uv_min, uv_max, tint_col, border_col);
			}

			string s = "Solar2D Simulator";
			ImGui::SetCursorPosX((window_size.x - ImGui::CalcTextSize(s.c_str()).x) * 0.5f);
			ImGui::TextUnformatted(s.c_str());

			s = Rtt_STRING_BUILD;
			ImGui::SetCursorPosX((window_size.x - ImGui::CalcTextSize(s.c_str()).x) * 0.5f);
			ImGui::Text(s.c_str());

			s = Rtt_STRING_COPYRIGHT;
			ImGui::SetCursorPosX((window_size.x - ImGui::CalcTextSize(s.c_str()).x) * 0.5f);
			ImGui::Text(s.c_str());

			s = "https://solar2d.com";
			ImGui::SetCursorPosX((window_size.x - ImGui::CalcTextSize(s.c_str()).x) * 0.5f);
			if (ImGui::Button(s.c_str()))
			{
				OpenURL(s);
			}

			s = "OK";
			ImGui::Dummy(ImVec2(20, 20));
			ImGui::SetItemDefaultFocus();
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}

			ImGui::Dummy(ImVec2(20, 20));
			ImGui::EndPopup();
		}
		ImGui::OpenPopup("About");
	}

	//
	// File dialog
	//

	ImFile::ImFile(const std::string& startFolder)
	{
		// (optional) set browser properties
		fileDialog.SetTitle("Open");
		fileDialog.SetTypeFilters({ ".lua" });
		std::filesystem::path pwd;
		fileDialog.SetPwd(startFolder);
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
			SDL_Event e = {};
			e.type = sdl::OnFileBrowserSelected;
			e.user.data1 = strdup(fileDialog.GetSelected().string().c_str());
			SDL_PushEvent(&e);

			fileDialog.ClearSelected();
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

	//
	//
	//
	ImNewProject::ImNewProject()
		: fProjectName(""),
		fTemplateName(""),
		fScreenWidth(320),
		fScreenHeight(480),
		fOrientationIndex(""),
		fProjectPath(""),
		fProjectSavePath(""),
		fResourcePath("")
	{
		// open project in the simulator
/*			string projectPath(newProjectDlg->GetProjectFolder().c_str());
			projectPath.append("/").append(newProjectDlg->GetProjectName().c_str());
			projectPath.append("/main.lua");

			//	wxCommandEvent eventOpen(eventOpenProject);
			//	eventOpen.SetString(projectPath.c_str());
			//	wxPostEvent(solarApp, eventOpen);

				// open the project folder in the file browser
			string command("xdg-open \"");
			command.append(newProjectDlg->GetProjectFolder().c_str());
			command.append("/").append(newProjectDlg->GetProjectName().c_str());
			command.append("\"");
			wxExecute(command.c_str());*/
	}

	void ImNewProject::Draw()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			string s;
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Dummy(ImVec2(10, 10));

			s = "   Application Name :";
			float label_width = ImGui::CalcTextSize(s.c_str()).x;
			static char sApplicationName[100] = "";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ApplicationName", sApplicationName, sizeof(sApplicationName));

			s = "   Project Folder: ";
			static char sProjectFolder[100] = "";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ProjectFolder", sProjectFolder, sizeof(sProjectFolder));
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				OpenURL(s);
			}

			// ok + cancel
			s = "OK";
			ImGui::Dummy(ImVec2(20, 20));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::SetItemDefaultFocus();

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::EndPopup();
		}
		ImGui::OpenPopup("New Project");
	}
}


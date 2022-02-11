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
#include "Rtt_LuaContext.h"
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "imgui/stb_image.h"

using namespace std;


extern "C"
{
	int luaopen_lfs(lua_State* L);
}

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

	DlgAbout::DlgAbout()
		: tex_id(0)
		, width(0)
		, height(0)
	{
		string iconPath = GetStartupPath(NULL);
		iconPath.append("/Resources/solar2d.png");
		LoadTextureFromFile(iconPath.c_str(), &tex_id, &width, &height);
	}

	DlgAbout::~DlgAbout()
	{
		if (tex_id > 0)
		{
			glDeleteTextures(1, &tex_id);
		}
	}

	void DlgAbout::Draw()
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

			ImGui::EndPopup();
		}
		ImGui::OpenPopup("About");
	}

	//
	// File dialog
	//

	DlgFile::DlgFile(const std::string& startFolder)
	{
		// (optional) set browser properties
		fileDialog.SetTitle("Open");
		fileDialog.SetTypeFilters({ ".lua" });
		fileDialog.SetPwd(startFolder);
		fileDialog.Open();
	}

	DlgFile::~DlgFile()
	{
	}

	void DlgFile::Draw()
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

	void DlgMenu::Draw()
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
	// DlgNewProject
	//

	DlgNewProject::DlgNewProject()
		: fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir)
		, fTemplateName("blank")
		, fScreenWidth(320)
		, fScreenHeight(480)
		, fOrientation("Upright")
	{
		*fApplicationNameInput = 0;
		*fProjectDirInput = 0;

		struct passwd* pw = getpwuid(getuid());
		const char* homedir = pw->pw_dir;
		fProjectDir = std::string(homedir);
		fProjectDir += LUA_DIRSEP;
		fProjectDir += "Documents";
		fProjectDir += LUA_DIRSEP;
		fProjectDir += "Solar2D Projects";
		if (!Rtt_IsDirectory(fProjectDir.c_str()))
		{
			int rc = Rtt_MakeDirectory(fProjectDir.c_str());
			if (!rc)
			{
				Rtt_LogException("Failed to create %s\n", fProjectDir.c_str());
			}
		}

		fileDialog.SetTitle("Browse For Folder");
	}

	void DlgNewProject::Draw()
	{

		fileDialog.Display();
		if (fileDialog.HasSelected())
		{
			fProjectDir = fileDialog.GetSelected().string();
			strncpy(fProjectDirInput, fProjectDir.c_str(), sizeof(fProjectDirInput) - 1);
			fileDialog.ClearSelected();
		}


		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::Begin("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			string s;
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Dummy(ImVec2(10, 10));

			ImGui::PushItemWidth(350);		// input field width

			s = "   Application Name :";
			float label_width = ImGui::CalcTextSize(s.c_str()).x;
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ApplicationName", fApplicationNameInput, sizeof(fApplicationNameInput), ImGuiInputTextFlags_CharsNoBlank);

			s = "   Project Folder: ";
			strncpy(fProjectDirInput, fProjectDir.c_str(), sizeof(fProjectDirInput) - 1);
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ProjectFolder", fProjectDirInput, sizeof(fProjectDirInput), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				fileDialog.SetPwd(fProjectDir);
				fileDialog.Open();
			}

			ImGui::PopItemWidth();

			// ok + cancel
			s = "OK";
			ImGui::Dummy(ImVec2(20, 20));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				// sanity check
				if (*fApplicationNameInput != 0)
				{
					if (CreateProject())
					{
						PushEvent(sdl::onClosePopupModal);
					}
					else
					{
						Rtt_LogException("Failed to create Application %s\n", fApplicationNameInput);
					}
				}
				else
				{
					Rtt_LogException("Empty Application name\n");
				}
			}
			ImGui::SetItemDefaultFocus();

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::End();
		} //ImGui::Begin
	}

	bool DlgNewProject::CreateProject()
	{
		string fResourcePath = string(GetStartupPath(NULL));
		fResourcePath.append("/Resources");

		string fNewProjectLuaScript(fResourcePath);
		fNewProjectLuaScript.append("/homescreen/newproject.lua");

		string fTemplatesDir(fResourcePath);
		fTemplatesDir.append("/homescreen/templates");

		string projectPath = fProjectDir;
		projectPath.append(LUA_DIRSEP);
		projectPath.append(fApplicationNameInput);

		// check if project folder already exists and that the height and width are numbers
		if (Rtt_IsDirectory(projectPath.c_str()))
		{
			//wxMessageBox(wxT("Project of that name already exists."), wxT("Duplicate Project Name"), wxICON_INFORMATION);
			Rtt_LogException("Project of that name already exists\n");
			return false;
		}

		if (!Rtt_MakeDirectory(projectPath.c_str()))
		{
			Rtt_LogException("Failed to create %s\n", projectPath.c_str());
			return false;
		}

		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		Rtt::LuaContext::RegisterModuleLoader(L, "lfs", luaopen_lfs);

		const char* script = fNewProjectLuaScript.c_str();
		int status = luaL_loadfile(L, script);
		if (0 != status)
		{
			Rtt_LogException("Failed to load %s\n", script);
			return false;
		}

		lua_createtable(L, 0, 6);
		{
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "isSimulator");

			lua_pushstring(L, fTemplateName.c_str());
			lua_setfield(L, -2, "template");

			lua_pushinteger(L, fScreenWidth);
			lua_setfield(L, -2, "width");

			lua_pushinteger(L, fScreenHeight);
			lua_setfield(L, -2, "height");

			lua_pushstring(L, fOrientation.c_str()); // Index ? "portrait" : "landscapeRight");
			lua_setfield(L, -2, "orientation");

			lua_pushstring(L, projectPath.c_str());
			lua_setfield(L, -2, "savePath");

			lua_pushstring(L, fTemplatesDir.c_str());
			lua_setfield(L, -2, "templateBaseDir");
		}

		status = Rtt::LuaContext::DoCall(L, 1, 0);
		lua_close(L);
		if (0 == status)
		{
			// show the project folder
			OpenURL(projectPath);

			// open project in the simulator
			projectPath.append("/main.lua");

			SDL_Event e = {};
			e.type = sdl::OnOpenProject;
			e.user.data1 = strdup(projectPath.c_str());
			SDL_PushEvent(&e);
		}
		else
		{
			Rtt_LogException("Failed to create %s project\n", fApplicationNameInput);
			return false;
		}
		return true;
	}

}	// Rtt


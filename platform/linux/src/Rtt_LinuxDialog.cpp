//////////////////////////////////////////////////////////////////////////////
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

	void DrawActivity()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::Begin("##DrawActivity", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs))
		{
			ImGui::Text("%c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
			ImGui::End();
		}
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

	DlgFile::DlgFile(const string& startFolder)
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

		if (!fileDialog.IsOpened())
		{
			PushEvent(sdl::onClosePopupModal);
		}
	}

	//
	// Menu
	//

	void DlgMenu::Draw()
	{
		ImGuiIO& io = ImGui::GetIO();
		bool ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) | ImGui::IsKeyDown(ImGuiKey_RightCtrl);
		bool shift = ImGui::IsKeyDown(ImGuiKey_LeftShift) | ImGui::IsKeyDown(ImGuiKey_RightShift);
		bool alt = ImGui::IsKeyDown(ImGuiKey_LeftAlt) | ImGui::IsKeyDown(ImGuiKey_RightAlt);

		if (isMainMenu)
		{
			// hot keys
			if (ctrl && !shift && !alt)
			{
				if (ImGui::IsKeyPressed(ImGuiKey_N, false))
				{
					PushEvent(sdl::OnNewProject);
				}
				else if (ImGui::IsKeyPressed(ImGuiKey_O, false))
				{
					PushEvent(sdl::OnOpenProject);
				}
				else if (ImGui::IsKeyPressed(ImGuiKey_R, false))
				{
					PushEvent(sdl::OnRelaunchLastProject);
				}
				else if (ImGui::IsKeyPressed(ImGuiKey_Q, false))
				{
					PushEvent(SDL_QUIT);
				}
			}

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
					if (ImGui::MenuItem("Exit", "Ctrl+Q"))
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

			// shortcuts
			if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_N, false))
			{
				PushEvent(sdl::OnNewProject);
			}
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_O, false))
			{
				PushEvent(sdl::OnOpenProject);
			}
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_R, false))
			{
				PushEvent(sdl::OnRelaunch);
			}
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_W, false))
			{
				PushEvent(sdl::OnCloseProject);
			}
			else if (ctrl && shift && !alt && ImGui::IsKeyPressed(ImGuiKey_O, false))
			{
				PushEvent(sdl::OnOpenInEditor);
			}
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Q, false))
			{
				PushEvent(SDL_QUIT);
			}
			// Suspend
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_DownArrow, false))
			{
				if (app->IsSuspended())
					app->Resume();
				else
					app->Pause();
			}
			// Back
			else if (alt && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
			{
				PushEvent(sdl::OnCloseProject);
			}
			// Build Android
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_B, false))
			{
				PushEvent(sdl::OnBuildAndroid);
			}
			// Build Linux
			else if (ctrl && shift && alt && ImGui::IsKeyPressed(ImGuiKey_B, false))
			{
				PushEvent(sdl::OnBuildLinux);
			}
			// Build HTML5
			else if (ctrl && !shift && alt && ImGui::IsKeyPressed(ImGuiKey_B, false))
			{
				PushEvent(sdl::OnBuildHTML5);
			}


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

					if (ImGui::BeginMenu("Build"))
					{
						if (ImGui::MenuItem("Android...", "Ctrl+B"))
						{
							PushEvent(sdl::OnBuildAndroid);
						}
						if (ImGui::MenuItem("HTML5...", "Ctrl+Alt+B"))
						{
							PushEvent(sdl::OnBuildHTML5);
						}
						if (ImGui::MenuItem("Linux...", "Ctrl+Shift+Alt+B"))
						{
							PushEvent(sdl::OnBuildLinux);
						}
						ImGui::EndMenu();
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
					if (ImGui::MenuItem("Exit", "Ctrl+Q"))
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
						PushEvent(sdl::OnRotateLeft);
					}
					if (ImGui::MenuItem("Rotate Right", "Ctrl+Right"))
					{
						PushEvent(sdl::OnRotateRight);
					}
					if (ImGui::MenuItem("Shake", "Ctrl+Up"))
					{
						PushEvent(sdl::OnShake);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Back", "Alt+Left"))
					{
						PushEvent(sdl::OnCloseProject);
					}

					ImGui::Separator();
					if (ImGui::MenuItem(app->IsSuspended() ? "Resume" : "Suspend", "Ctrl+Down"))
					{
						if (app->IsSuspended())
							app->Resume();
						else
							app->Pause();
					}

					ImGui::EndMenu();
				}

				// project menu
				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Zoom In", "Ctrl++"))
					{
						PushEvent(sdl::OnZoomIn);
					}
					if (ImGui::MenuItem("Zoom Out", "Ctrl+-"))
					{
						PushEvent(sdl::OnZoomOut);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("View As...", NULL))
					{
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Welcome screen", NULL))
					{
						PushEvent(sdl::OnCloseProject);
					}
					if (ImGui::MenuItem("Console", NULL))
					{
						PushEvent(sdl::OnConsole);
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
		, fTemplateIndex(0)
		, fSizeIndex(0)
		, fOrientationIndex(0)
	{
		*fApplicationNameInput = 0;
		*fProjectDirInput = 0;
		strcpy(fWidthInput, "320");
		strcpy(fHeightInput, "480");

		struct passwd* pw = getpwuid(getuid());
		const char* homedir = pw->pw_dir;
		fProjectDir = string(homedir);
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

			// templates
			ImGui::Dummy(ImVec2(10, 10));
			float yGroup1 = ImGui::GetCursorPosY();
			ImGui::Text("Project Template");
			ImGui::RadioButton("Blank", &fTemplateIndex, 0);
			ImGui::Text("   Creates a project folder with an empty \"main.lua\"");
			ImGui::RadioButton("Tab Bar Application", &fTemplateIndex, 1);
			ImGui::Text("   Multiscreen application using a Tab Bar for");
			ImGui::RadioButton("Physics Based Game", &fTemplateIndex, 2);
			ImGui::Text("   Application using the physics and composer");
			ImGui::RadioButton("eBook", &fTemplateIndex, 3);
			ImGui::Text("   Multi-page interface using the composer");

			float x = 400;
			ImGui::SetCursorPosY(yGroup1);

			// project template combo
			ImGui::SetCursorPosX(x);
			ImGui::Text("Upright Screen Size");
			const char* templates[] = { "Phone Preset", "Tablet Preset", "Custom" };
			ImGui::SetCursorPosX(x);
			if (ImGui::Combo("##UprightScreenSize", &fSizeIndex, templates, IM_ARRAYSIZE(templates)))
			{
				if (fSizeIndex == 0)
				{
					strcpy(fWidthInput, "320");
					strcpy(fHeightInput, "480");
				}
				else if (fSizeIndex == 1)
				{
					strcpy(fWidthInput, "768");
					strcpy(fHeightInput, "1024");
				}
			}

			ImGui::BeginDisabled(fSizeIndex < 2);		// disable if not custom
			ImGui::PushItemWidth(50);		// input field width

			s = "Width:";
			float label2 = x + ImGui::CalcTextSize(s.c_str()).x;
			ImGui::SetCursorPosX(x);
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label2 + 20);
			//			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##templateWidth", fWidthInput, sizeof(fWidthInput), ImGuiInputTextFlags_CharsDecimal);

			s = "Height:";
			ImGui::SetCursorPosX(x);
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label2 + 20);
			//			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##templateHeight", fHeightInput, sizeof(fHeightInput), ImGuiInputTextFlags_CharsDecimal);

			ImGui::PopItemWidth();
			ImGui::EndDisabled();

			// default orientation
			ImGui::Dummy(ImVec2(20, 20));
			ImGui::SetCursorPosX(x);
			ImGui::Text("Default orientation");
			ImGui::SetCursorPosX(x);
			ImGui::RadioButton("Upright", &fOrientationIndex, 0);	ImGui::SameLine();
			ImGui::RadioButton("Sideways", &fOrientationIndex, 1);

			// ok + cancel
			s = "OK";
			ImGui::Dummy(ImVec2(70, 40));
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

			Rtt_ASSERT(fTemplateIndex >= 0 && fTemplateIndex < 4);
			array<string, 4> templates = { "blank","app","game","ebook" };
			lua_pushstring(L, templates[fTemplateIndex].c_str());
			lua_setfield(L, -2, "template");

			int w = atoi(fWidthInput);
			lua_pushinteger(L, w);
			lua_setfield(L, -2, "width");

			int h = atoi(fHeightInput);
			lua_pushinteger(L, h);
			lua_setfield(L, -2, "height");

			lua_pushstring(L, fOrientationIndex == 0 ? "portrait" : "landscapeRight");
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

	//
	//
	//

	DlgPreferences::DlgPreferences()
		: fRelaunchIndex(1)
		, fShowWelcome(false)
		, fShowErrors(true)
		, fOpenlastProject(false)
		, fStyleIndex(0)
	{
		if (app->ConfigGet())
		{
			map<string, string>& cfg = *app->ConfigGet();
			fRelaunchIndex = cfg["relaunchOnFileChange"] == "Always" ? 0 : (cfg["relaunchOnFileChange"] == "Ask" ? 2 : 1);
			fShowWelcome = atoi(cfg["ShowWelcome"].c_str());
			fShowErrors = atoi(cfg["showRuntimeErrors"].c_str());
			fOpenlastProject = atoi(cfg["openLastProject"].c_str());
			fStyleIndex = cfg["ColorScheme"] == "Light" ? 0 : (cfg["ColorScheme"] == "Dark" ? 2 : 1);
		}
	}

	void DlgPreferences::Draw()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		const char* title = "Solar2D Simulator Preferences";
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Checkbox("Don't show the Welcome window", &fShowWelcome);
			ImGui::Checkbox("Show Runtime Errorrs", &fShowErrors);
			ImGui::Checkbox("Automatically open last project", &fOpenlastProject);

			string s = "Relaunch Simulator when project is modified";
			ImGui::Dummy(ImVec2(100, 10));
			ImGui::Text(s.c_str());
			ImGui::RadioButton("Always", &fRelaunchIndex, 0);
			ImGui::RadioButton("Never", &fRelaunchIndex, 1);
			ImGui::RadioButton("Ask every time", &fRelaunchIndex, 2);

			s = "GUI Color Scheme";
			ImGui::Dummy(ImVec2(100, 10));
			ImGui::Text(s.c_str());
			if (ImGui::RadioButton("Light", &fStyleIndex, 0))
			{
				ImGui::StyleColorsLight();
			}
			if (ImGui::RadioButton("Classic", &fStyleIndex, 1))
			{
				ImGui::StyleColorsClassic();
			}
			if (ImGui::RadioButton("Dark", &fStyleIndex, 2))
			{
				ImGui::StyleColorsDark();
			}

			// ok + cancel
			s = "OK";
			ImGui::Dummy(ImVec2(100, 30));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				if (app->ConfigGet())
				{
					map<string, string>& cfg = *app->ConfigGet();
					cfg["relaunchOnFileChange"] = fRelaunchIndex == 0 ? "Always" : (fRelaunchIndex == 2 ? "Ask" : "Never");
					cfg["ShowWelcome"] = to_string(fShowWelcome);
					cfg["showRuntimeErrors"] = to_string(fShowErrors);
					cfg["openLastProject"] = to_string(fOpenlastProject);
					cfg["ColorScheme"] = fStyleIndex == 0 ? "Light" : (fStyleIndex == 2 ? "Dark" : "Standard");
					app->ConfigSave();
				}
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
		ImGui::OpenPopup(title);
	}

	//
	// DlgAskRelaunch
	//

	void DlgAskRelaunch::Draw()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		const char* title = "Solar2D Simulator";
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Dummy(ImVec2(100, 10));
			ImGui::Text("   Solar2D project has been modified");
			ImGui::Text("Whould you like to relaunch the project?");

			ImGui::Dummy(ImVec2(150, 30));
			ImGui::Checkbox("Remember my preference", &fSaveMyPreference);

			// ok + cancel
			string s = "Relaunch";
			ImGui::Dummy(ImVec2(100, 30));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				SaveMyPreference("Always");
				PushEvent(sdl::onClosePopupModal);
				PushEvent(sdl::OnRelaunchLastProject);
			}
			ImGui::SetItemDefaultFocus();

			s = "Ignore";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				SaveMyPreference("Never");
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::EndPopup();
		}
		ImGui::OpenPopup(title);
	}

	void DlgAskRelaunch::SaveMyPreference(const char* val)
	{
		if (fSaveMyPreference && app->ConfigGet())
		{
			map<string, string>& cfg = *app->ConfigGet();
			cfg["relaunchOnFileChange"] = val;
		}
	}

}	// Rtt

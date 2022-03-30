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
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_Version.h"
#include "Rtt_FileSystem.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_BitmapUtils.h"
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>

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
	bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* w, int* h)
	{
		// Load from file
		FILE * f = fopen(filename, "rb");
		if (f)
		{
			unsigned char* img = bitmapUtil::loadPNG(f, *w, *h);
			fclose(f);

			if (img)
			{
				// Create a OpenGL texture identifier
				GLuint image_texture;
				glGenTextures(1, &image_texture);
				glBindTexture(GL_TEXTURE_2D, image_texture);

				// Setup filtering parameters for display
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *w, *h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

				*out_texture = image_texture;

				free(img);
				return true;
			}
		}
		return false;
	}

	void DrawActivity()
	{
		Window::MoveToCenter();

		ImGui::Begin("##DrawActivity", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		const char* label = "##spinner";
		const float indicator_radius = 14;
		const ImVec4 main_color(0, 0, 1, 1);
		const ImVec4 backdrop_color(0, 0, 1, 0.7f);
		const int circle_count = 8;
		const float speed = 8;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);

		const ImVec2 pos = window->DC.CursorPos;
		const float circle_radius = indicator_radius / 10.0f;
		const ImRect bb(pos, ImVec2(pos.x + indicator_radius * 2.0f, pos.y + indicator_radius * 2.0f));
		ImGui::ItemSize(bb, 0);
		if (ImGui::ItemAdd(bb, id))
		{
			const float t = g.Time;
			const auto degree_offset = 2.0f * IM_PI / circle_count;
			for (int i = 0; i < circle_count; ++i)
			{
				const auto x = indicator_radius * std::sin(degree_offset * i);
				const auto y = indicator_radius * std::cos(degree_offset * i);
				const auto growth = std::max(0.0f, std::sin(t * speed - i * degree_offset));
				ImVec4 color;
				color.x = main_color.x * growth + backdrop_color.x * (1.0f - growth);
				color.y = main_color.y * growth + backdrop_color.y * (1.0f - growth);
				color.z = main_color.z * growth + backdrop_color.z * (1.0f - growth);
				color.w = 1.0f;
				window->DrawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x, pos.y + indicator_radius - y), circle_radius + growth * circle_radius, ImGui::GetColorU32(color));
			}
		}
		ImGui::End();
	}

	//
	// Dlg base class
	//

	Window::Window(const string& title, int w, int h)
	{
		// save state
		window = SDL_GL_GetCurrentWindow();
		glcontext = SDL_GL_GetCurrentContext();
		imctx = ImGui::GetCurrentContext();

		fWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
		fGLcontext = SDL_GL_CreateContext(fWindow);

		fImCtx = ImGui::CreateContext();
		ImGui::SetCurrentContext(fImCtx);

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls

		SetStyle();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL2_InitForOpenGL(fWindow, fGLcontext);
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);

		// restore state
		SDL_GL_MakeCurrent(window, glcontext);
		ImGui::SetCurrentContext(imctx);
	}

	Window::~Window()
	{
		ImGui::DestroyContext(fImCtx);
		SDL_DestroyWindow(fWindow);
	}

	// Setup Dear ImGui style
	void Window::SetStyle()
	{
		Config& cfg = app->GetConfig();
		if (cfg["ColorScheme"].to_string() == "Light")
			PushEvent(sdl::OnStyleColorsLight);
		else if (cfg["ColorScheme"].to_string() == "Dark")
			PushEvent(sdl::OnStyleColorsDark);
		else
			PushEvent(sdl::OnStyleColorsClassic);
	}

	void Window::GetWindowSize(int* w, int* h)
	{
		SDL_GetWindowSize(fWindow, w, h);
	}

	void Window::MoveToCenter()
	{
		// center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	}

	void Window::FocusHere()
	{
		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		//ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	}

	void Window::begin()
	{
		// save state
		window = SDL_GL_GetCurrentWindow();
		glcontext = SDL_GL_GetCurrentContext();
		imctx = ImGui::GetCurrentContext();

		SDL_GL_MakeCurrent(fWindow, fGLcontext);
		ImGui::SetCurrentContext(fImCtx);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		MoveToCenter();

		// set background color
		const ImGuiStyle& style = ImGui::GetStyle();
		const ImVec4& bg = style.Colors[ImGuiCol_WindowBg];
		glClearColor(bg.x, bg.y, bg.z, bg.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Window::end()
	{
		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(fWindow);

		// restore state
		SDL_GL_MakeCurrent(window, glcontext);
		ImGui::SetCurrentContext(imctx);
	}

	void Window::ProcessEvent(const SDL_Event& evt)
	{
		imctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(fImCtx);

		if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_CLOSE && evt.window.windowID == SDL_GetWindowID(fWindow))
		{
			PushEvent(sdl::onCloseDialog);
			goto done;
		}

		// filter events
		switch (evt.type)
		{
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case 	SDL_MOUSEWHEEL:
			if (evt.window.windowID != SDL_GetWindowID(fWindow))
			{
				goto done;
			}
			break;
		case sdl::OnStyleColorsLight:
			ImGui::StyleColorsLight();
			goto done;
		case sdl::OnStyleColorsClassic:
			ImGui::StyleColorsClassic();
			goto done;
		case sdl::OnStyleColorsDark:
			ImGui::StyleColorsDark();
			goto done;
		default:
			break;
		}

		ImGui_ImplSDL2_ProcessEvent(&evt);

	done:
		ImGui::SetCurrentContext(imctx);
	}

	//
	// About dialog
	//

	DlgAbout::DlgAbout(const std::string& title, int w, int h)
		: Window(title, w, h)
		, tex_id(0)
		, width(0)
		, height(0)
	{
		begin();
		string iconPath = GetStartupPath(NULL);
		iconPath.append("/Resources/solar2d.png");
		LoadTextureFromFile(iconPath.c_str(), &tex_id, &width, &height);
		end();
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
		begin();
		if (ImGui::Begin("##DlgAbout", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
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
			ImGui::TextUnformatted(s.c_str());

			s = Rtt_STRING_COPYRIGHT;
			ImGui::SetCursorPosX((window_size.x - ImGui::CalcTextSize(s.c_str()).x) * 0.5f);
			ImGui::TextUnformatted(s.c_str());

			s = "https://solar2d.com";
			ImGui::SetCursorPosX((window_size.x - ImGui::CalcTextSize(s.c_str()).x) * 0.5f);
			if (ImGui::Button(s.c_str()))
			{
				OpenURL(s);
			}

			s = "OK";
			ImGui::Dummy(ImVec2(20, 20));
			ImGui::SetCursorPosX((window_size.x - BUTTON_WIDTH) * 0.5f);
			bool isOkPressed = ImGui::Button(s.c_str(), ImVec2(BUTTON_WIDTH, 0));
			FocusHere();

			//  (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			if (isOkPressed || ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				PushEvent(sdl::onCloseDialog);
			}

			ImGui::End();
		}
		end();
	}

	//
	// File dialog
	//

	DlgOpen::DlgOpen(const std::string& title, int w, int h, const string& startFolder)
		: Window(title, w, h)
		, fileDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_NoTitleBar)
	{
		fileDialog.SetWindowSize(w, h);
		fileDialog.SetTitle("##DlgOpen");
		fileDialog.SetTypeFilters({ ".lua" });
		fileDialog.SetPwd(startFolder.empty() ? GetHomePath() : startFolder);
		fileDialog.Open();
	}

	DlgOpen::~DlgOpen()
	{
	}

	void DlgOpen::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgOpen", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
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
				PushEvent(sdl::onCloseDialog);
			}
			ImGui::End();
		}
		end();
	}

	//
	// Menu
	//

	DlgMenu::DlgMenu(const std::string& appName)
	{
		fIsMainMenu = appName == "homescreen";
	}

	void DlgMenu::Draw()
	{
		ImGuiIO& io = ImGui::GetIO();
		bool ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) | ImGui::IsKeyDown(ImGuiKey_RightCtrl);
		bool shift = ImGui::IsKeyDown(ImGuiKey_LeftShift) | ImGui::IsKeyDown(ImGuiKey_RightShift);
		bool alt = ImGui::IsKeyDown(ImGuiKey_LeftAlt) | ImGui::IsKeyDown(ImGuiKey_RightAlt);

		if (fIsMainMenu)
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
				fMenuSize = ImGui::GetWindowSize();
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
			// ZoomIn
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Equal, false))
			{
				PushEvent(sdl::OnZoomIn);
			}
			// ZoomOut
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Minus, false))
			{
				PushEvent(sdl::OnZoomOut);
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
					if (ImGui::MenuItem("Zoom In", "Ctrl+Plus"))
					{
						PushEvent(sdl::OnZoomIn);
					}
					if (ImGui::MenuItem("Zoom Out", "Ctrl+Minus"))
					{
						PushEvent(sdl::OnZoomOut);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Welcome screen", NULL))
					{
						PushEvent(sdl::OnCloseProject);
					}
					if (ImGui::MenuItem("Console", NULL))
					{
						PushEvent(sdl::OnSetFocusConsole);
					}
					ImGui::Separator();

					if (ImGui::MenuItem("View As..."))
					{
						PushEvent(sdl::OnViewAs);
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
				fMenuSize = ImGui::GetWindowSize();
				ImGui::EndMainMenuBar();
			}
		}
	}

	//
	// DlgNewProject
	//

	DlgNewProject::DlgNewProject(const std::string& title, int w, int h)
		: Window(title, w, h)
		, fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir)
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
		fileDialog.SetWindowSize(w, h);
	}

	void DlgNewProject::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgNewProject", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			fileDialog.Display();
			if (fileDialog.HasSelected())
			{
				fProjectDir = fileDialog.GetSelected().string();
				strncpy(fProjectDirInput, fProjectDir.c_str(), sizeof(fProjectDirInput) - 1);
				fileDialog.ClearSelected();
			}

			string s;
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Dummy(ImVec2(10, 10));

			ImGui::PushItemWidth(350);		// input field width

			s = "   Application Name :";
			float label_width = ImGui::CalcTextSize(s.c_str()).x;
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ApplicationName", fApplicationNameInput, sizeof(fApplicationNameInput), ImGuiInputTextFlags_CharsNoBlank);

			s = "   Project Folder: ";
			strncpy(fProjectDirInput, fProjectDir.c_str(), sizeof(fProjectDirInput) - 1);
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ProjectFolder", fProjectDirInput, sizeof(fProjectDirInput));
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
			ImGui::TextUnformatted("Project Template");
			ImGui::RadioButton("Blank", &fTemplateIndex, 0);
			ImGui::TextUnformatted("   Creates a project folder with an empty \"main.lua\"");
			ImGui::RadioButton("Tab Bar Application", &fTemplateIndex, 1);
			ImGui::TextUnformatted("   Multiscreen application using a Tab Bar for");
			ImGui::RadioButton("Physics Based Game", &fTemplateIndex, 2);
			ImGui::TextUnformatted("   Application using the physics and composer");
			ImGui::RadioButton("eBook", &fTemplateIndex, 3);
			ImGui::TextUnformatted("   Multi-page interface using the composer");

			float x = 400;
			ImGui::SetCursorPosY(yGroup1);

			// project template combo
			ImGui::SetCursorPosX(x);
			ImGui::TextUnformatted("Upright Screen Size");
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
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label2 + 20);
			//			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##templateWidth", fWidthInput, sizeof(fWidthInput), ImGuiInputTextFlags_CharsDecimal);

			s = "Height:";
			ImGui::SetCursorPosX(x);
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label2 + 20);
			//			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##templateHeight", fHeightInput, sizeof(fHeightInput), ImGuiInputTextFlags_CharsDecimal);

			ImGui::PopItemWidth();
			ImGui::EndDisabled();

			// default orientation
			ImGui::Dummy(ImVec2(20, 20));
			ImGui::SetCursorPosX(x);
			ImGui::TextUnformatted("Default orientation");
			ImGui::SetCursorPosX(x);
			ImGui::RadioButton("Upright", &fOrientationIndex, 0);	ImGui::SameLine();
			ImGui::RadioButton("Sideways", &fOrientationIndex, 1);

			// ok + cancel
			s = "OK";
			ImGui::Dummy(ImVec2(70, 40));
			ImGui::SetCursorPosX((window_size.x - BUTTON_WIDTH) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(BUTTON_WIDTH, 0)) || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter)))
			{
				// sanity check
				if (*fApplicationNameInput != 0)
				{
					if (CreateProject())
					{
						PushEvent(sdl::onCloseDialog);
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
			if (ImGui::Button(s.c_str(), ImVec2(BUTTON_WIDTH, 0)) || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)))
			{
				PushEvent(sdl::onCloseDialog);
			}

			ImGui::End();
		}
		end();
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

	DlgPreferences::DlgPreferences(const std::string& title, int w, int h)
		: Window(title, w, h)
		, fRelaunchIndex(1)
		, fShowWelcome(false)
		, fShowErrors(true)
		, fOpenlastProject(false)
		, fStyleIndex(0)
	{
		Config& cfg = app->GetConfig();
		fRelaunchIndex = cfg["relaunchOnFileChange"].to_string() == "Always" ? 0 : (cfg["relaunchOnFileChange"].to_string() == "Ask" ? 2 : 1);
		fShowWelcome = cfg["ShowWelcome"].to_bool();
		fShowErrors = cfg["showRuntimeErrors"].to_bool();
		fOpenlastProject = cfg["openLastProject"].to_bool();
		fStyleIndex = cfg["ColorScheme"].to_string() == "Light" ? 0 : (cfg["ColorScheme"].to_string() == "Dark" ? 2 : 1);
	}

	DlgPreferences::~DlgPreferences()
	{
	}

	void DlgPreferences::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgAbout", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Checkbox("Don't show the Welcome window", &fShowWelcome);
			ImGui::Checkbox("Show Runtime Errorrs", &fShowErrors);
			ImGui::Checkbox("Automatically open last project", &fOpenlastProject);

			string s = "Relaunch Simulator when project is modified";
			ImGui::Dummy(ImVec2(100, 10));
			ImGui::TextUnformatted(s.c_str());
			ImGui::RadioButton("Always", &fRelaunchIndex, 0);
			ImGui::RadioButton("Never", &fRelaunchIndex, 1);
			ImGui::RadioButton("Ask every time", &fRelaunchIndex, 2);

			s = "GUI Color Scheme";
			ImGui::Dummy(ImVec2(100, 10));
			ImGui::TextUnformatted(s.c_str());
			if (ImGui::RadioButton("Light", &fStyleIndex, 0))
			{
				PushEvent(sdl::OnStyleColorsLight);
			}
			if (ImGui::RadioButton("Classic", &fStyleIndex, 1))
			{
				PushEvent(sdl::OnStyleColorsClassic);
			}
			if (ImGui::RadioButton("Dark", &fStyleIndex, 2))
			{
				PushEvent(sdl::OnStyleColorsDark);
			}

			// ok + cancel
			ImGui::Dummy(ImVec2(100, 30));
			ImGui::SetCursorPosX((window_size.x - BUTTON_WIDTH) * 0.5f);
			bool IsOkPressed = ImGui::Button("OK", ImVec2(BUTTON_WIDTH, 0));
			if (IsOkPressed || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter)))
			{
				Config& cfg = app->GetConfig();
				cfg["relaunchOnFileChange"] = fRelaunchIndex == 0 ? "Always" : (fRelaunchIndex == 2 ? "Ask" : "Never");
				cfg["ShowWelcome"] = fShowWelcome;
				cfg["showRuntimeErrors"] = fShowErrors;
				cfg["openLastProject"] = fOpenlastProject;
				cfg["ColorScheme"] = fStyleIndex == 0 ? "Light" : (fStyleIndex == 2 ? "Dark" : "Standard");
				cfg.Save();

				PushEvent(sdl::OnPreferencesChanged);
				PushEvent(sdl::onCloseDialog);
			}

			ImGui::SameLine();
			bool IsCancelPressed = ImGui::Button("Cancel", ImVec2(BUTTON_WIDTH, 0));
			if (IsCancelPressed || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)))
			{
				// reset old values
				SetStyle();
				PushEvent(sdl::onCloseDialog);
			}
			FocusHere();

			ImGui::End();
		}
		end();
	}

	//
	// DlgAskRelaunch
	//

	void DlgAskRelaunch::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgAskRelaunch", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Dummy(ImVec2(100, 10));
			ImGui::TextUnformatted("   Solar2D project has been modified");
			ImGui::TextUnformatted("Whould you like to relaunch the project?");

			ImGui::Dummy(ImVec2(150, 30));
			ImGui::Checkbox("Remember my preference", &fSaveMyPreference);

			// ok + cancel
			string s = "Relaunch";
			ImGui::Dummy(ImVec2(100, 30));
			ImGui::SetCursorPosX((window_size.x - BUTTON_WIDTH) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(BUTTON_WIDTH, 0)) || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter)))
			{
				SaveMyPreference("Always");
				PushEvent(sdl::onCloseDialog);
				PushEvent(sdl::OnRelaunchLastProject);
			}
			ImGui::SetItemDefaultFocus();

			s = "Ignore";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(BUTTON_WIDTH, 0)) || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)))
			{
				SaveMyPreference("Never");
				PushEvent(sdl::onCloseDialog);
			}
			ImGui::End();
		}
		end();
	}

	void DlgAskRelaunch::SaveMyPreference(const char* val)
	{
		if (fSaveMyPreference)
		{
			Config& cfg = app->GetConfig();
			cfg["relaunchOnFileChange"] = val;
		}
	}

	//
	// DlgViewAs
	//

	DlgViewAs::DlgViewAs(const std::string& title, int w, int h, Skins* skins)
		: Window(title, w, h)
		, fViewIndex(0)
		, fSkins(skins)
		, fItems(NULL)
		, fItemsLen(0)
		, fItemCurrent(0)
	{
	}

	DlgViewAs::~DlgViewAs()
	{
		Clear();
	}

	void DlgViewAs::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgViewAs", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
		{
			int w, h;
			GetWindowSize(&w, &h);
			ImGui::SetWindowSize(ImVec2(w, h));
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::SetCursorPosX(10);
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("##DlgViewAsTabBar", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Generic Android"))
				{
					DrawView("genericAndroid");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Named Android"))
				{
					DrawView("namedAndroid");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Generic iOS"))
				{
					DrawView("genericIOS");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Named iOS"))
				{
					DrawView("namedIOS");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("TV"))
				{
					DrawView("tv");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Desktop"))
				{
					DrawView("desktop");
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			ImGui::Dummy(ImVec2(10, 10));
			ImGui::SetCursorPosX((window_size.x - BUTTON_WIDTH) * 0.5f);
			if (ImGui::Button("Close", ImVec2(BUTTON_WIDTH, 0)) || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)))
			{
				PushEvent(sdl::onCloseDialog);
			}

			ImGui::End();
		}
		end();
	}

	void DlgViewAs::Clear()
	{
		if (fItems)
		{
			for (int i = 0; i < fItemsLen; i++)
			{
				free(fItems[i]);
			}
			delete[] fItems;
		}
	}

	void DlgViewAs::DrawView(const string& tabName)
	{
		const std::map<std::string, SkinProperties>* skins = fSkins->GetSkins(tabName);
		if (skins)
		{
			if (fTabCurrent != tabName)
			{
				Clear();
				fItems = new char* [skins->size()];

				int i = 0;
				for (const auto& it : *skins)
				{
					fItems[i++] = strdup(it.first.c_str());
				}
				fTabCurrent = tabName;
				fItemsLen = skins->size();
			}

			if (ImGui::ListBox("", &fItemCurrent, fItems, fItemsLen, 20))
			{
				const auto& it = skins->find(fItems[fItemCurrent]);
				if (it != skins->end())
				{
					SDL_Event e = {};
					e.type = sdl::OnChangeView;
					e.user.data1 = (void*)&it->second;
					SDL_PushEvent(&e);
				}
			}

		}
	}

	//
	// DlgAlert
	//

	DlgAlert::DlgAlert(const char* title, const char* msg, const char** buttonLabels, int numButtons, LuaResource* resource)
		: Window(title, 400, 300)
		, fMsg(msg)
		, fCallback(resource)
	{
		for (int i = 0; i < numButtons; i++)
		{
			fButtons.push_back(buttonLabels[i]);
		}
	}

	DlgAlert::~DlgAlert()
	{
		if (fCallback)
			delete fCallback;
	}

	void DlgAlert::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgAlert", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			int w, h;
			GetWindowSize(&w, &h);
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + w - 40);
			ImGui::SetCursorPosX(20);
			ImGui::SetCursorPosY(20);
			ImGui::TextWrapped("%s", fMsg.c_str());
			ImGui::PopTextWrapPos();

			ImGui::Dummy(ImVec2(20, 20));
			ImGui::SetCursorPosX((window_size.x - (BUTTON_WIDTH + 10) * fButtons.size()) * 0.5f);
			for (int i = fButtons.size() - 1; i >= 0; i--)
			{
				if (ImGui::Button(fButtons[i].c_str(), ImVec2(BUTTON_WIDTH, 0)))
				{
					onClick(i);
				}
				ImGui::SameLine();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				onClick(-1);
			}
			ImGui::End();
		}
		end();
	}

	void DlgAlert::onClick(int nButton)
	{
		if (fCallback)
		{
			// Invoke the Lua listener.
			LuaLibNative::AlertComplete(*fCallback, nButton, nButton < 0);

			// Delete the Lua resource.
			delete fCallback;
			fCallback = NULL;		// lock re-entry
			PushEvent(sdl::onCloseDialog);
		}
	}

	//
	// DlgRuntimeError
	//

	DlgRuntimeError::DlgRuntimeError(const char* title, int w, int h, const char* errorType, const char* message, const char* stacktrace)
		: Window(title, w, h)
		, fErrorType(errorType)
		, fMessage(message)
		, fStackTrace(stacktrace)
	{
	}

	DlgRuntimeError::~DlgRuntimeError()
	{
	}

	void DlgRuntimeError::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgRuntimeError", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			int w, h;
			GetWindowSize(&w, &h);
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + w - 40);
			ImGui::SetCursorPosX(20);
			ImGui::SetCursorPosY(20);
			ImGui::TextWrapped("%s", fMessage.c_str());
			ImGui::PopTextWrapPos();

			ImGui::Dummy(ImVec2(70, 30));
			ImGui::TextUnformatted("Do you want to relaunch the project?");

			// ok + cancel
			string s = "Yes";
			ImGui::Dummy(ImVec2(70, 30));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				PushEvent(sdl::onCloseDialog);
				PushEvent(sdl::OnRelaunch);
			}
			FocusHere();

			s = "No";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				PushEvent(sdl::onCloseDialog);
			}

			ImGui::End();
		}
		end();
	}

}	// Rtt


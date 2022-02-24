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
#include "imgui/imfilebrowser.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <map>

namespace Rtt
{
	void DrawActivity();

	//
	// base class
	//

	struct Window : public ref_counted
	{
		Window(const std::string& title, int w, int h);
		virtual ~Window();

		virtual void Draw() = 0;
		void ProcessEvent(const SDL_Event& evt);
		SDL_Window* GetWindow() const { return fWindow; }
		void GetWindowSize(int* w, int* h);

	protected:

		void begin();
		void end();

	private:

		SDL_Window* fWindow;
		SDL_GLContext fGLcontext;
		ImGuiContext* fImCtx;

		// for state saving 
		SDL_Window* window;
		SDL_GLContext glcontext;
		ImGuiContext* imctx;
	};

	struct DlgAbout : public Window
	{
		DlgAbout(const std::string& title, int w, int h);
		virtual ~DlgAbout();
		void Draw() override;

		GLuint tex_id;
		int width;
		int height;
	};

	struct DlgOpen : public Window
	{
		DlgOpen(const std::string& title, int w, int h, const std::string& startFolder);
		virtual ~DlgOpen();

		void Draw() override;

		ImGui::FileBrowser fileDialog;
	};

	struct DlgMenu : public ref_counted
	{
		DlgMenu(const std::string& appName);

		void Draw();

	private:

		bool isMainMenu;
	};

	struct DlgNewProject : public Window
	{
		DlgNewProject(const std::string& title, int w, int h);
		void Draw() override;

	private:

		bool CreateProject();
		ImGui::FileBrowser fileDialog;
		std::string fProjectDir;

		char fApplicationNameInput[32];
		char fProjectDirInput[1024];
		int fTemplateIndex;
		int fSizeIndex;
		int fOrientationIndex;
		char fWidthInput[10];
		char fHeightInput[10];
	};

	struct DlgPreferences : public Window
	{
		DlgPreferences(const std::string& title, int w, int h);
		void Draw() override;

	private:

		int fRelaunchIndex;
		bool fShowWelcome;
		bool fShowErrors;
		bool fOpenlastProject;
		int fStyleIndex;
	};

	struct DlgAskRelaunch : public Window
	{
		DlgAskRelaunch(const std::string& title, int w, int h)
			: Window(title, w, h)
			, fSaveMyPreference(false)
		{
		}

		void Draw() override;

	private:

		void SaveMyPreference(const char* val);
		bool fSaveMyPreference;
	};

	struct Skins;
	struct DlgViewAs : public Window
	{
		DlgViewAs(const std::string& title, int w, int h, Skins* skins);
		virtual ~DlgViewAs();

		void Draw() override;

	private:

		void DrawView(const std::string& name);
		void Clear();

		int fViewIndex;
		Skins* fSkins;
		char** fItems;
		int fItemsLen;
		int fItemCurrent;
		std::string fTabCurrent;
	};

}

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

namespace Rtt
{
	struct Dlg : public ref_counted
	{
		virtual void Draw() = 0;
	};

	struct DlgAbout : public Dlg
	{
		DlgAbout();
		virtual ~DlgAbout();
		void Draw() override;

		GLuint tex_id;
		int width;
		int height;
	};

	struct DlgFile : public Dlg
	{
		DlgFile(const std::string& startFolder);
		virtual ~DlgFile();

		void Draw() override;

		ImGui::FileBrowser fileDialog;
	};

	struct DlgMenu : public Dlg
	{
		DlgMenu(const std::string& appName)
		{
			isMainMenu = appName == "homescreen";
		}
		void Draw() override;

	private:
		bool isMainMenu;
	};

	struct DlgNewProject : public Dlg
	{
		DlgNewProject();
		void Draw() override;

	private:

		bool CreateProject();
		ImGui::FileBrowser fileDialog;

		char fApplicationNameInput[32];
		char fProjectDirInput[1024];
		std::string fProjectDir;
		std::string fTemplateName;
		int fScreenWidth;
		int fScreenHeight;
		std::string fOrientation;
	};

}

//
//
//

#include "Rtt_LinuxConsoleApp.h"

namespace Rtt
{
	DlgConsole::DlgConsole(std::string* logData)
		: fLogData(logData)
	{
		fWindow = SDL_CreateWindow("Solar2D Simulator Console", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
		fGLcontext = SDL_GL_CreateContext(fWindow);

		ImGuiContext* imctx = ImGui::GetCurrentContext();
		fImCtx = ImGui::CreateContext();
		ImGui::SetCurrentContext(fImCtx);

		// Setup Dear ImGui style
		ImGui::StyleColorsLight(); // StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL2_InitForOpenGL(fWindow, fGLcontext);
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);

		ImGui::SetCurrentContext(imctx);
	}

	DlgConsole::~DlgConsole()
	{
		ImGui::DestroyContext(fImCtx);
		SDL_DestroyWindow(fWindow);
	}

	void DlgConsole::ProcessEvent(SDL_Event* evt)
	{
		ImGuiContext* imctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(fImCtx);

		ImGui_ImplSDL2_ProcessEvent(evt);

		ImGui::SetCurrentContext(imctx);
	}

	void DlgConsole::Draw()
	{
		SDL_Window* window = SDL_GL_GetCurrentWindow();
		SDL_GLContext glcontext = SDL_GL_GetCurrentContext();
		ImGuiContext* imctx = ImGui::GetCurrentContext();

		SDL_GL_MakeCurrent(fWindow, fGLcontext);
		ImGui::SetCurrentContext(fImCtx);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// set size
		const ImVec2& windowSize = ImGui::GetMainViewport()->WorkSize;
		ImGui::SetNextWindowSize(windowSize);

		// move to center
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::Begin("##LogWindow", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs))
		{
			ImVec2 tbSize = windowSize;
			tbSize.x -= 15;	// hack
			tbSize.y -= 15;
			int len = fLogData->size();
			if (len > 0)
			{
				ImGui::InputTextMultiline("##LogData", (char*)fLogData->c_str(), len, tbSize, ImGuiInputTextFlags_ReadOnly);
			}
			ImGui::End();
		}

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(fWindow);

		SDL_GL_MakeCurrent(window, glcontext);
		ImGui::SetCurrentContext(imctx);
	}
}
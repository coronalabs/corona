//
// Console window
//

#pragma once

#include "Rtt_LinuxDialog.h"

namespace Rtt
{
	struct DlgConsole : public Dlg
	{
		DlgConsole(std::string* logData);
		virtual ~DlgConsole();

		void ProcessEvent(SDL_Event* evt);
		void Draw() override;

	private:

		SDL_Window* fWindow;       
		SDL_GLContext fGLcontext;
		ImGuiContext* fImCtx;

		std::string* fLogData;	

	};
}

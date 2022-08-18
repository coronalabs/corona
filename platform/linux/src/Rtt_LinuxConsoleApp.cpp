//
//
//

#include "Rtt_LinuxConsoleApp.h"

using namespace std;

namespace Rtt
{
	ConsoleWindow::ConsoleWindow(const string& title, int w, int h, string* logData)
		: Window(title, w, h, SDL_WINDOW_RESIZABLE)
		, fLogData(logData)
	{
	}

	ConsoleWindow::~ConsoleWindow()
	{
	}

	void ConsoleWindow::Draw()
	{
		begin();

		// set size
		const ImVec2& windowSize = ImGui::GetMainViewport()->WorkSize;
		ImGui::SetNextWindowSize(windowSize);

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
		end();
	}
}
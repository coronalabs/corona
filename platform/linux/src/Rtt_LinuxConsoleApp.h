//
// Console window
//

#pragma once

#include "Rtt_LinuxDialog.h"

namespace Rtt
{
	struct ConsoleWindow : public Window
	{
		ConsoleWindow(const std::string& title, int w, int h, std::string* logData);
		virtual ~ConsoleWindow();

		void Draw() override;

	private:
		std::string* fLogData;	
	};
}

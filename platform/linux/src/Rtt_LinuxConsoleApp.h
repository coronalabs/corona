//
// Console window
//

#pragma once

#include "Rtt_LinuxDialog.h"

namespace Rtt
{
	struct DlgConsole : public Dlg
	{
		DlgConsole(const std::string& title, int w, int h, std::string* logData);
		virtual ~DlgConsole();

		void Draw() override;

	private:
		std::string* fLogData;	
	};
}

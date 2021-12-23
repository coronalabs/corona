#ifndef LINUX_CONSOLE_APP_H
#define LINUX_CONSOLE_APP_H

#include <wx/app.h>
#include "Rtt_LinuxConsole.h"

class Rtt_LinuxConsoleApp: public wxApp
{
public:
	enum MessageType {Normal, Warning, Error};
	Rtt_LinuxConsole *Solar2DConsole;
	bool OnInit();
	void ClearLog();
	void UpdateLog(wxString message);
	void UpdateLog(wxString message, int messageType);
};

DECLARE_APP(Rtt_LinuxConsoleApp);

#endif //LINUXCONSOLEAPP_H

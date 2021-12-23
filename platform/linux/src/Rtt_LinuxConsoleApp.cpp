#include "Rtt_LinuxConsoleApp.h"

bool Rtt_LinuxConsoleApp::OnInit()
{
	wxInitAllImageHandlers();
	Solar2DConsole = new Rtt_LinuxConsole(NULL, wxID_ANY, wxEmptyString);
	SetTopWindow(Solar2DConsole);
	Solar2DConsole->Show();

	return true;
}

void Rtt_LinuxConsoleApp::ClearLog()
{
	Solar2DConsole->ClearLog();
}

void Rtt_LinuxConsoleApp::UpdateLog(wxString message)
{
	Solar2DConsole->UpdateLog(message);
}

void Rtt_LinuxConsoleApp::UpdateLog(wxString message, int messageType)
{
	switch(messageType)
	{
		case MessageType::Warning:
			Solar2DConsole->UpdateLogWarning(message);
			break;

		case MessageType::Error:
			Solar2DConsole->UpdateLogError(message);
			break;

		default:
			Solar2DConsole->UpdateLog(message);
			break;
	}
}

IMPLEMENT_APP(Rtt_LinuxConsoleApp);

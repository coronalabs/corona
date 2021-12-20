#include "Rtt_LinuxIPCServerConnection.h"

bool Rtt_LinuxIPCServerConnection::OnPoke(const wxString &topic, const wxString &item, const void *data, size_t size, wxIPCFormat format)
{
	int logType = 0;
	string input(static_cast<const char *>(data), size);
	wxString msg;
	msg.append(input.c_str());

	if (item.IsSameAs("warning"))
	{
		logType = 1;
	}
	else if (item.IsSameAs("error"))
	{
		logType = 2;
	}
	else if (item.IsSameAs("clear"))
	{
		wxGetApp().ClearLog();
		return true;
	}
	else if (item.IsSameAs("quit"))
	{
		Disconnect();
		wxExit();
		return true;
	}

	wxGetApp().UpdateLog(msg.c_str(), logType);
	return wxConnection::OnPoke(topic, item, data, size, format);
}

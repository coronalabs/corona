#ifndef LINUX_IPC_SERVER_CONNECTION_H
#define LINUX_IPC_SERVER_CONNECTION_H

#include <wx/wx.h>
#include <wx/ipc.h>
#include <string.h>
#include "Rtt_LinuxConsoleApp.h"

class Rtt_LinuxIPCServerConnection : public wxConnection
{
public:
	virtual bool Disconnect() wxOVERRIDE { return wxConnection::Disconnect(); }
	virtual bool OnPoke(const wxString &topic, const wxString &item, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
};

#endif //LINUX_IPC_SERVER_CONNECTION_H

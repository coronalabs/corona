#ifndef Rtt_LINUX_IPC_CLIENT_CONNECTION_H
#define Rtt_LINUX_IPC_CLIENT_CONNECTION_H

#include <wx/wx.h>
#include <wx/ipc.h>
#include <string.h>

class Rtt_LinuxIPCClientConnection: public wxConnection
{
public:
	virtual bool DoPoke(const wxString &item, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual bool OnDisconnect() wxOVERRIDE;
};

#endif //Rtt_LINUX_IPC_CLIENT_CONNECTION_H

#ifndef LINUX_IPC_SERVER_H
#define LINUX_IPC_SERVER_H

#include <wx/ipc.h>

// the default service name
#define IPC_SERVICE "/tmp/solar2d_logconsole"
// the hostname
#define IPC_HOST "localhost"
// the IPC topic
#define IPC_TOPIC "IPC Solar2D"
// the name of the item we're being advised about
#define IPC_ADVISE_NAME "Item"

class Rtt_LinuxConsole;

class Rtt_LinuxIPCServer : public wxServer
{
public:
	Rtt_LinuxIPCServer();
	virtual ~Rtt_LinuxIPCServer();
	void Disconnect();
	bool IsConnected() { return mConnection != NULL; }
	virtual wxConnectionBase *OnAcceptConnection(const wxString &topic) wxOVERRIDE;
protected:
	wxConnection *mConnection;
};

#endif //LINUX_IPC_SERVER_H

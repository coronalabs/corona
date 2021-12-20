#include <wx/wx.h>
#include "Rtt_LinuxIPCServerConnection.h"
#include "Rtt_LinuxIPCServer.h"

Rtt_LinuxIPCServer::Rtt_LinuxIPCServer()
{
	mConnection = NULL;
}

Rtt_LinuxIPCServer::~Rtt_LinuxIPCServer()
{
	Disconnect();
}

wxConnectionBase *Rtt_LinuxIPCServer::OnAcceptConnection(const wxString &topic)
{
	if (topic == IPC_TOPIC)
	{
		mConnection = new Rtt_LinuxIPCServerConnection();
	}
	else // unknown topic
	{
		return NULL;
	}

	return mConnection;
}

void Rtt_LinuxIPCServer::Disconnect()
{
	if (mConnection)
	{
		mConnection->Disconnect();
		wxDELETE(mConnection);
	}
}

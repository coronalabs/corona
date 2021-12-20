#include "Rtt_LinuxIPCClient.h"

Rtt_LinuxIPCClient::Rtt_LinuxIPCClient()
{
	mConnection = NULL;
}

Rtt_LinuxIPCClient::~Rtt_LinuxIPCClient()
{
	Disconnect();
}

bool Rtt_LinuxIPCClient::Connect(const wxString &sHost, const wxString &sService, const wxString &sTopic)
{
	// suppress the log messages from MakeConnection()
	wxLogNull nolog;

	mConnection = (Rtt_LinuxIPCClientConnection*)MakeConnection(sHost, sService, sTopic);
	return mConnection != NULL;
}

wxConnectionBase *Rtt_LinuxIPCClient::OnMakeConnection()
{
	return new Rtt_LinuxIPCClientConnection();
}

void Rtt_LinuxIPCClient::Disconnect()
{
	if (mConnection)
	{
		//mConnection->Disconnect();
		wxDELETE(mConnection);
	}
}

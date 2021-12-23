#include "Rtt_LinuxIPCClientConnection.h"

bool Rtt_LinuxIPCClientConnection::OnDisconnect()
{
	return true;
}

bool Rtt_LinuxIPCClientConnection::DoPoke(const wxString &item, const void *data, size_t size, wxIPCFormat format)
{
	return wxConnection::DoPoke(item, data, size, format);
}

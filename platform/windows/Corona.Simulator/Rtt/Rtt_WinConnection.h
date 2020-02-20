//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinConnection_H__
#define _Rtt_WinConnection_H__

#include <WinInet.h>
#include "TimXmlRpc.h"

#include "Rtt_PlatformConnection.h"

class XmlRpcValue;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class WinConnection : public PlatformConnection
{
	public:
		typedef PlatformConnection Super;

	public:
		WinConnection( const MPlatformServices& platform, const char* url );
		virtual ~WinConnection();

	public:
		virtual PlatformDictionaryWrapper* Call( const char* method, const KeyValuePair* pairs, int numPairs );
		virtual const char* Error() const;
		virtual const char* Url() const;
		virtual bool Download(const char *urlStr, const char *filename);
		virtual void CloseConnection();

    protected:
        XmlRpcValue *CreateDictionary( const KeyValuePair *pairs, int numPairs );
		void ReportError(const char *mesg);

	private:
		XmlRpcClient *m_connection;
		char * m_sUrl;
		char * m_sError;
		HINTERNET m_hInternetFile;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinConnection_H__

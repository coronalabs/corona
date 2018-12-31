//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

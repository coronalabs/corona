//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidConnection_H__
#define _Rtt_AndroidConnection_H__

#include "Rtt_PlatformConnection.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidConnection : public PlatformConnection
{
	public:
		typedef PlatformConnection Super;

	public:
		AndroidConnection( const MPlatformServices& platform, const char* url );
		virtual ~AndroidConnection();

	public:
		virtual PlatformDictionaryWrapper* Call( const char* method, const KeyValuePair* pairs, int numPairs );
		virtual const char* Error() const;
		virtual const char* Url() const;

	private:
		const char * fUrl;
		const char * fError;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidConnection_H__

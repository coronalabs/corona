//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidConnection.h"

#include "Rtt_MPlatformServices.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidConnection::AndroidConnection( const MPlatformServices& platform, const char* url )
:	Super( platform ),
	fUrl( NULL ),
	fError( NULL )
{
//"http://web-a1.ath.cx/ansca-drupal-head/services/xmlrpc"

//	NSString* str = [[NSString alloc] initWithUTF8String:url];
//	fUrl = [[NSURL alloc] initWithString:str];
//	[str release];
}

AndroidConnection::~AndroidConnection()
{
//	[fError release];
//	[fUrl release];
}

PlatformDictionaryWrapper*
AndroidConnection::Call( const char* m, const KeyValuePair* pairs, int numPairs )
{
	PlatformDictionaryWrapper* result = NULL;

	return result;
}

const char*
AndroidConnection::Error() const
{
	return fError;
}

const char*
AndroidConnection::Url() const
{
	return fUrl;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


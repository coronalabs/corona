//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformConnection_H__
#define _Rtt_PlatformConnection_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

class MPlatformServices;
class PlatformDictionaryWrapper;

// ----------------------------------------------------------------------------

// TODO: This is a total kludge.  Redesign...
enum 
{
	kNullValueType = 0,
	kStringValueType = -1,
	kIntegerValueType = -2
};

struct KeyValuePair
{
	const char* key;
	const void* value;
	int type; // if > 0, then value is a (KeyValuePair*) and type is the number of pairs
};

class PlatformConnection
{
	public:
		PlatformConnection( const MPlatformServices& platform );
		virtual ~PlatformConnection();

	public:
		virtual PlatformDictionaryWrapper* Call( const char* method, const KeyValuePair* pairs, int numPairs ) = 0;
		virtual const char* Error() const = 0;
		virtual const char* Url() const = 0;
		virtual bool Download(const char *urlStr, const char *filename) = 0;
		virtual void CloseConnection() = 0;

	protected:
		const MPlatformServices& fPlatform;
        int fDebugWebServices;
//		char* fUrl;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformConnection_H__

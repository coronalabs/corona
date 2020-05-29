//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _H_AntHost_
#define _H_AntHost_

#if USE_JNI

#include <string.h>

class jHashMapParam;

class AntHost
{
	static jHashMapParam *	myHashMapParam;

public:
	AntHost()
	{
	}
	
	~AntHost();

	void SetProperty( const char * key, const char * value );
	int AntCall( const char * buildFilePath, const char * target, int logLevel, Rtt::String * result );
};

#endif // USE_JNI

#endif


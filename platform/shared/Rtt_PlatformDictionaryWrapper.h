//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformDictionaryWrapper_H__
#define _Rtt_PlatformDictionaryWrapper_H__

#include "Rtt_PlatformObjectWrapper.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

//class PlatformStringWrapper;

// ----------------------------------------------------------------------------

class PlatformDictionaryWrapper : public PlatformObjectWrapper
{
	public:
		virtual ~PlatformDictionaryWrapper();

	public:
//		virtual void SetValueForKey( const PlatformStringWrapper* key, const PlatformObjectWrapper* value ) = 0;
		virtual const char* ValueForKey( const char* key ) const = 0;
		virtual const char* ValueForKeyPath( const char* keyPath ) const = 0;

	private:
		
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformDictionaryWrapper_H__

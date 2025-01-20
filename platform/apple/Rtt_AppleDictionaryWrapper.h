//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleDictionaryWrapper_H__
#define _Rtt_AppleDictionaryWrapper_H__

#include "Rtt_PlatformDictionaryWrapper.h"

#include <stdio.h>

// ----------------------------------------------------------------------------

@class NSDictionary;
@class NSString;

namespace Rtt
{

// ----------------------------------------------------------------------------

class AppleDictionaryWrapper : public PlatformDictionaryWrapper
{
	public:
		AppleDictionaryWrapper( NSDictionary* dictionary );
		virtual ~AppleDictionaryWrapper();

	protected:
		void Initialize();

	public:
//		virtual void SetValueForKey( const PlatformStringWrapper* key, const PlatformObjectWrapper* value );
		virtual const char* ValueForKey( const char* key ) const;
		virtual const char* ValueForKeyPath( const char* keyPath ) const;

		NSString* ValueForKey( NSString* key ) const;

		virtual void* PlatformObject() const;

	private:
//		MSimulatorPlatform& fPlatform;
		NSDictionary* fObject;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleDictionaryWrapper_H__

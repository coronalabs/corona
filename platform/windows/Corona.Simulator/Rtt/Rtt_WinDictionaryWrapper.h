//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinDictionaryWrapper_H__
#define _Rtt_WinDictionaryWrapper_H__

#include "Rtt_PlatformDictionaryWrapper.h"

class XmlRpcValue; // TimXmlRpc.h

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class WinDictionaryWrapper : public PlatformDictionaryWrapper
{
	public:
		WinDictionaryWrapper( XmlRpcValue *pDict );
		virtual ~WinDictionaryWrapper();

	public:
//		virtual void SetValueForKey( const PlatformStringWrapper* key, const PlatformObjectWrapper* value ) = 0;
		virtual const char* ValueForKey( const char* key ) const;
		virtual const char* ValueForKeyPath( const char* keyPath ) const;

		virtual void* PlatformObject() const;
	private:
		XmlRpcValue *m_pDictionary;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinDictionaryWrapper_H__

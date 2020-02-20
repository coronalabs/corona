//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_WinDictionaryWrapper.h"

#include "TimXmlRpc.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

WinDictionaryWrapper::WinDictionaryWrapper( XmlRpcValue *pDict ) :
	m_pDictionary( pDict )
{
}

WinDictionaryWrapper::~WinDictionaryWrapper()
{
//  Delete ptr allocated by XmlRpc execute()
	delete m_pDictionary;
}

const char*
WinDictionaryWrapper::ValueForKey( const char* key ) const
{
    const char *str = NULL;

    if( m_pDictionary )
	{
         if( m_pDictionary->getType() == XmlRpcValue::TypeStruct)
             str = (const char *)(*m_pDictionary)[key];
		 // Complete hack to parse result of build
		 else if(m_pDictionary->getType() == XmlRpcValue::TypeString)
			 str = (const char *)(*m_pDictionary);
	}

	return str;
}

const char*
WinDictionaryWrapper::ValueForKeyPath( const char* keyPath ) const
{
    // TODO: What is this supposed to do?
    return (*m_pDictionary)[keyPath];
}

void*
WinDictionaryWrapper::PlatformObject() const
{
	return m_pDictionary;
}
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


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


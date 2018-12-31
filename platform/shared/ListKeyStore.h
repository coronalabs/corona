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

#ifndef _H_ListKeyStore_
#define _H_ListKeyStore_

#include <string>

const char *ReplaceString(const char *c_subject, const char * c_search, const char * c_replace);

void ReplaceString(std::string& subject, const std::string& search,
				   const std::string& replace);


class ListKeyStore
{
	char **			myAliases;
	int				myCount;

public:
	ListKeyStore();
	
	~ListKeyStore();
	
	void InitAliasList( int count ) 
	{
		myCount = count;
		myAliases = new char *[ count ];

		for ( int i = 0; i < count; i++ ) {
			myAliases[i] = NULL;
		}
	}
	
	int GetSize() const
	{
		return myCount;
	}

	void SetAlias( int index, const char * alias )
	{
		myAliases[index] = strdup( alias );
	}

	const char * GetAlias( int index )
	{
		return myAliases[index];
	}
	
	enum KeyStoreStatus {
		KeyStoreStatusOk = 0,
		KeyStoreStatusBad1 = 1, // TODO: huh? lol, huh?, indeed
		KeyStoreStatusBad2 = 2,
		KeyStoreStatusBad3 = 3,
		KeyStoreStatusBad4 = 4,
		KeyStoreStatusBad5 = 5
	};
	bool IsValidKeyStore( const char * keyStore, const char * password );
	bool GetAliasList( const char * keyStore, const char * password );
	bool AreKeyStoreAndAliasPasswordsValid( const char *keyStore, const char *keyPW, const char *alias, const char *aliasPW, const char *resourcesDir);

	std::string EscapeArgument(std::string arg);
};


#endif

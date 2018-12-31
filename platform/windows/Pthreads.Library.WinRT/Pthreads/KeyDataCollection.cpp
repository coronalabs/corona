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

#include "KeyDataCollection.h"


namespace Pthreads {

#pragma region Constructors/Destructors
KeyDataCollection::KeyDataCollection()
{
}

KeyDataCollection::~KeyDataCollection()
{
}

#pragma endregion


#pragma region Public Functions
void KeyDataCollection::Clear()
{
	fCollection.clear();
}

const void* KeyDataCollection::GetData(pthread_key_t key)
{
	const void* data = nullptr;

	auto iterator = fCollection.find(key);
	if (iterator != fCollection.end())
	{
		data = (*iterator).second;
	}
	return data;
}

void KeyDataCollection::SetData(pthread_key_t key, const void* data)
{
	// Do not continue if given null data.
	if (nullptr == data)
	{
		return;
	}

	// Add or replace the value in the table under the given key.
	auto iterator = fCollection.find(key);
	if (iterator != fCollection.end())
	{
		(*iterator).second = data;
	}
	else
	{
		fCollection.insert(CollectionPairType(key, data));
	}
}

#pragma endregion

}	// namespace Pthreads

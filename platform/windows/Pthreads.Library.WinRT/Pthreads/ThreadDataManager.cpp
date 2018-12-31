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

#include "ThreadDataManager.h"
#include "ThreadData.h"


namespace Pthreads {

#pragma region Constructors/Destructors
ThreadDataManager::ThreadDataManager()
{
}

ThreadDataManager::~ThreadDataManager()
{
	ThreadData *dataPointer;

	// Delete all data stored in the collection.
	for (auto&& pair : fCollection)
	{
		dataPointer = pair.second;
		if (dataPointer)
		{
			delete dataPointer;
		}
	}
}

#pragma endregion


#pragma region Public Functions
ThreadData* ThreadDataManager::GetBy(std::thread::id &id)
{
	ThreadData* dataPointer = nullptr;

	// Fetch the given thread's data, if it exists.
	auto iterator = fCollection.find(id);
	if (iterator != fCollection.end())
	{
		// Data was successfully found in the collection. Fetch its data.
		dataPointer = (*iterator).second;
	}
	else
	{
		// Data not found in the collection. Create new data and add it to the collection.
		dataPointer = new ThreadData();
		fCollection.insert(CollectionPairType(id, dataPointer));
	}

	// Return the requested thread's data.
	return dataPointer;
}

ThreadData* ThreadDataManager::GetByIndex(int index)
{
	// Validate argument.
	if ((index < 0) || (index >= fCollection.size()))
	{
		return nullptr;
	}

	// Fetch the indexed data.
	for (auto&& nextPair : fCollection)
	{
		if (0 == index)
		{
			return nextPair.second;
		}
		index--;
	}
	return nullptr;
}

int ThreadDataManager::GetCount()
{
	return fCollection.size();
}

#pragma endregion

}	// namespace Pthreads

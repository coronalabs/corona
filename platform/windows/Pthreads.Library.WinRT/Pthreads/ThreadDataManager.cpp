//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

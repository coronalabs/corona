//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <hash_map>
#include <thread>


namespace Pthreads {

class ThreadData;

class ThreadDataManager
{
	public:
		ThreadDataManager();
		virtual ~ThreadDataManager();

		ThreadData* GetBy(std::thread::id &id);
		ThreadData* GetByIndex(int index);
		int GetCount();

	private:
		class CollectionKeyHashComparer
		{
			public:
				enum { bucket_size = 1 };

				size_t operator()(const std::thread::id& id)
				{
					return id.hash();
				}

				bool operator()(const std::thread::id& x, const std::thread::id& y)
				{
					return (x.hash() == y.hash());
				}
		};

		typedef std::hash_map<std::thread::id, ThreadData*, CollectionKeyHashComparer> CollectionType;
		typedef std::pair<std::thread::id, ThreadData*> CollectionPairType;

		CollectionType fCollection;
};

}	// namespace Pthreads

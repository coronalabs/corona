//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "pthread.h"
#include <hash_map>


namespace Pthreads {

class KeyDataCollection
{
	public:
		KeyDataCollection();
		virtual ~KeyDataCollection();

		void Clear();
		const void* GetData(pthread_key_t key);
		void SetData(pthread_key_t key, const void* data);

	private:
		typedef std::hash_map<pthread_key_t, const void*> CollectionType;
		typedef std::pair<pthread_key_t, const void*> CollectionPairType;

		CollectionType fCollection;
};

}	// namespace Pthreads

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
#include "KeyDataCollection.h"
#include <hash_map>
#include <memory>
#include <thread>


namespace Pthreads {

class ThreadData
{
	public:
		ThreadData();
		virtual ~ThreadData();

		std::shared_ptr<std::thread> GetStandardThreadPointer();
		void SetStandardThreadPointer(std::thread *threadPointer);
		KeyDataCollection& GetKeyDataCollection();
		void* GetExitData();
		void SetExitData(void* data);

	private:
		std::shared_ptr<std::thread> fStandardThreadPointer;
		KeyDataCollection fKeyDataCollection;
		void* fExitData;
};

}	// namespace Pthreads

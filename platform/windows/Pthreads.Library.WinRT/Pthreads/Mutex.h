//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "BaseMutex.h"
#include <mutex>


namespace Pthreads {

/// <summary>Mutex used to synchronize access between threads.</summary>
class Mutex : public BaseMutex
{
	public:
		/// <summary>Creates a new mutex.</summary>
		Mutex();

		/// <summary>Destroys resources taken by this mutex.</summary>
		virtual ~Mutex();

		/// <summary>
		///  Locks the mutex for the current thread. Will block the current thread if this mutex is currently locked
		///  by another thread and will not resume until its been unlocked.
		/// </summary>
		virtual void Lock();

		/// <summary>Unlocks the mutex, making it available to other threads.</summary>
		virtual void Unlock();

	private:
		std::mutex fMutex;
};

}	// namespace Pthreads

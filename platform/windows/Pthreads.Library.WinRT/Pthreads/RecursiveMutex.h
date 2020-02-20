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

/// <summary>Recursive mutex used to synchronize access between threads.</summary>
/// <remarks>
///  Supports having the lock() function being called multiple times on the same thread without having to unlock it.
///  A recursive mutex counts the number of times it has been locked and it must be unlocked the same number of times
///  in order to release it and make it available to other threads.
/// </remarks>
class RecursiveMutex : public BaseMutex
{
	public:
		/// <summary>Creates a new recursive mutex.</summary>
		RecursiveMutex();

		/// <summary>Destroys resources taken by this mutex.</summary>
		virtual ~RecursiveMutex();

		/// <summary>
		///  Locks the mutex for the current thread. Will block the current thread if this mutex is currently locked
		///  by another thread and will not resume until its been unlocked.
		/// </summary>
		virtual void Lock();

		/// <summary>Unlocks the mutex, making it available to other threads.</summary>
		virtual void Unlock();

	private:
		std::recursive_mutex fMutex;
};

}	// namespace Pthreads

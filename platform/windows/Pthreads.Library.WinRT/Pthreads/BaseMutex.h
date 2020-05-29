//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


namespace Pthreads {

/// <summary>Mutex used to synchronize access between threads.</summary>
/// <remarks>This is an abstract class from which all mutex classes are expected to derive from.</remarks>
class BaseMutex
{
	public:
		/// <summary>Creates a new mutex.</summary>
		BaseMutex();

		/// <summary>Destroys resources taken by this mutex.</summary>
		virtual ~BaseMutex();

		/// <summary>
		///  Locks the mutex for the current thread. Will block the current thread if this mutex is currently locked
		///  by another thread and will not resume until its been unlocked.
		/// </summary>
		virtual void Lock() = 0;

		/// <summary>Unlocks the mutex, making it available to other threads.</summary>
		virtual void Unlock() = 0;
};

}	// namespace Pthreads

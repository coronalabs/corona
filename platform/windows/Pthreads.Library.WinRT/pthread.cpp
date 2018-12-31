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

#include "pthread.h"
#include "Pthreads\BaseMutex.h"
#include "Pthreads\KeyDataCollection.h"
#include "Pthreads\Mutex.h"
#include "Pthreads\RecursiveMutex.h"
#include "Pthreads\ThreadData.h"
#include "Pthreads\ThreadDataManager.h"
#include <windows.h>
#include <chrono>
#include <errno.h>
#include <mutex>
#include <system_error>
#include <thread>
#include <hash_map>
#include <set>


#pragma region Internal Static Variables
/// <summary>Destructor callback provided to the pthread_key_create() function. Used to delete a key's data.</summary>
typedef void(*DestructorCallback)(void*);

/// <summary>Hash table of pthread keys used to store their associated destructors for their data.</summary>
typedef std::hash_map<pthread_key_t, DestructorCallback> KeyDestructorMap;

/// <summary>The pair type used by a KeyDestructorMap object.</summary>
typedef std::pair<pthread_key_t, DestructorCallback> KeyDestructorMapPair;

#pragma endregion


#pragma region Internal Static Variables
/// <summary>Mutex used to synchronize access to this library's data.</summary>
static std::recursive_mutex sMutex;

/// <summary>Stores a collection of data/information about all threads owned by this application.</summary>
static Pthreads::ThreadDataManager sThreadDataManager;

/// <summary>
///  <para>Set of all pthread keys that are currently in use by this library.</para>
///  <para>Keys are added and removed from this set via the pthread_key_create() and pthread_key_delete() functions.</para>
///  <para>
///   The key pair's value stores the destructor callback used to delete the key data. This callback is assignd
///   via the pthread_key_creat() function.
///  </para>
/// </summary>
static KeyDestructorMap sReservedKeys;

#pragma endregion


#pragma region Internal Functions
static Pthreads::ThreadData* GetThreadDataFor(std::thread::id &id)
{
	// Fetch the requested thread's data.
	Pthreads::ThreadData *dataPointer = nullptr;
	sMutex.lock();
	{
		dataPointer = sThreadDataManager.GetBy(id);
	}
	sMutex.unlock();
	return dataPointer;
}

static void DeleteAllDataFrom(Pthreads::KeyDataCollection &collection)
{
	DestructorCallback destructorCallback;
	void* dataPointer;

	sMutex.lock();
	{
		// First, delete all data via the key's destructor callback.
		for (auto&& item : sReservedKeys)
		{
			destructorCallback = item.second;
			if (destructorCallback)
			{
				dataPointer = (void*)(collection.GetData(item.first));
				if (dataPointer)
				{
					(*destructorCallback)(dataPointer);
				}
			}
			collection.GetData(item.first);
		}

		// Remove all key/data from the collection.
		collection.Clear();
	}
	sMutex.unlock();
}

static void OnExecuteThread(Pthreads::ThreadData **requestedThreadDataPointer, void *(*start_routine)(void*), void *arg)
{
	// Fetch the data for this thread.
	auto threadDataPointer = GetThreadDataFor(std::this_thread::get_id());

	// Provide the above data object to the thread that spawned this thread, which will be blocked until it is assigned.
	// Note: The "requestedThreadDataPointer" will become a wild pointer after it has been assigned.
	if (requestedThreadDataPointer)
	{
		*requestedThreadDataPointer = threadDataPointer;
	}

	// Block this thread until we've acquired the std::thread object that spawned this thread.
	// This ensures that the "start_routine" function below is ran once pthread_create() finishes initializing everything.
	std::shared_ptr<std::thread> standardThreadPointer(nullptr);
	if (threadDataPointer)
	{
		while (true)
		{
			sMutex.lock();
			{
				standardThreadPointer = threadDataPointer->GetStandardThreadPointer();
			}
			sMutex.unlock();

			if (standardThreadPointer.get() != nullptr)
			{
				break;
			}
			pthread_yield();
		}
	}

	// Execute the given function.
	if (start_routine)
	{
		(*start_routine)(arg);
	}

	// Delete the resources used by this thread now that it has finished its work.
	if (threadDataPointer)
	{
		sMutex.lock();
		{
			// Delete the std::thread object that was used to spawn this thread.
			if (standardThreadPointer)
			{
				threadDataPointer->SetStandardThreadPointer(nullptr);
			}

			// Clear this thread's key/data collection now that it is done.
			DeleteAllDataFrom(threadDataPointer->GetKeyDataCollection());
		}
		sMutex.unlock();
	}
}

#pragma endregion


#pragma region Public Functions
int pthread_attr_init(pthread_attr_t *attr)
{
	// Validate.
	if (nullptr == attr)
	{
		return EINVAL;
	}

//TODO: Add support for the "attr" argument.

	*attr = nullptr;
	return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
	// Validate.
	if (nullptr == attr)
	{
		return EINVAL;
	}

//TODO: Add support for the "attr" argument.

	*attr = nullptr;
	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
//TODO: Add support for the "attr" argument.
	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
//TODO: Add support for the "attr" argument.
	return 0;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg)
{
	// Validate arguments.
	if ((nullptr == thread) || (nullptr == start_routine))
	{
		return EINVAL;
	}

//TODO: Add support for the "attr" argument.

	// Attempt to create a new thread which invokes the given function pointer.
	std::thread *standardThreadPointer = nullptr;
	Pthreads::ThreadData *threadDataPointer = nullptr;
	*thread = nullptr;
	try
	{
		standardThreadPointer = new std::thread(&OnExecuteThread, &threadDataPointer, start_routine, arg);
	}
	catch (std::system_error &error)
	{
		return error.code().value();
	}
	catch (...)
	{
		return EAGAIN;
	}

	// Wait for the above thread to assign "threadDataPointer" a data object.
	while (nullptr == threadDataPointer)
	{
		std::this_thread::yield();
	}
	*thread = (pthread_t)threadDataPointer;

	// Initialize the above thread's data.
	// Note: The above thread will block until this data has been provided.
	if (threadDataPointer)
	{
		sMutex.lock();
		{
			// Reset the last exit value.
			threadDataPointer->SetExitData(nullptr);

			// Clear the thread's key/data collection.
			// This is in case there was data left over from the last thread that was using its handle.
			DeleteAllDataFrom(threadDataPointer->GetKeyDataCollection());

			// Store the std::thread object with the thread's data, to be delete when the thread finishes its work.
			threadDataPointer->SetStandardThreadPointer(standardThreadPointer);
		}
		sMutex.unlock();
	}

	// Return a success result.
	return 0;
}

void pthread_exit(void *value_ptr)
{
	// Store the given exit data.
	sMutex.lock();
	{
		auto threadDataPointer = sThreadDataManager.GetBy(std::this_thread::get_id());
		if (threadDataPointer)
		{
			threadDataPointer->SetExitData(value_ptr);
		}
	}
	sMutex.unlock();

//TODO: Find a way to terminate the thread here.
}

int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param)
{
	//NOTE: You cannot set thread priority on the C/C++ side on Windows Phone.
	//      Thread priority can only be set on the .NET side, assuming it actually works.
	return 0;
}

void* pthread_getspecific(pthread_key_t key)
{
	void* value = nullptr;

	// Fetch the requested data.
	sMutex.lock();
	{
		auto threadDataPointer = sThreadDataManager.GetBy(std::this_thread::get_id());
		if (threadDataPointer)
		{
			value = (void*)(threadDataPointer->GetKeyDataCollection().GetData(key));
		}
	}
	sMutex.unlock();
	return value;
}

int pthread_join(pthread_t thread, void **value_ptr)
{
	// Validate arguments.
	if (nullptr == thread)
	{
		return EINVAL;
	}

//TODO: Add support for fetching pthread_exit() value and storing it to "value_ptr" argument.

	// Attempt to fetch the std::thread object.
	auto threadDataPointer = (Pthreads::ThreadData*)thread;
	std::shared_ptr<std::thread> standardThreadPointer(nullptr);
	if (threadDataPointer)
	{
		sMutex.lock();
		{
			standardThreadPointer = threadDataPointer->GetStandardThreadPointer();
		}
		sMutex.unlock();
	}

	// Do not continue if the given thread is not joinable.
	// This can happen if it was not created by this library.
	if (!threadDataPointer || !standardThreadPointer.get())
	{
		return EINVAL;
	}

	// Block until the thread finishes executing.
	int errorCode = 0;
	try
	{
		if (standardThreadPointer->joinable())
		{
			standardThreadPointer->join();
			if (value_ptr)
			{
				*value_ptr = threadDataPointer->GetExitData();
			}
		}
	}
	catch (std::system_error &error)
	{
		errorCode = error.code().value();
	}
	catch (...)
	{
		errorCode = EAGAIN;
	}

	// Return the result.
	return errorCode;
}

int pthread_key_create(pthread_key_t *key, void(*destructor)(void*))
{
	static pthread_key_t sNextKey = 1;

	// Validate.
	if (nullptr == key)
	{
		return EINVAL;
	}

	// Create the key and add it the reserved list.
	sMutex.lock();
	{
		// Obtain a unique key.
		for (; (0 == sNextKey) || (sReservedKeys.find(sNextKey) != sReservedKeys.end()); sNextKey++);
		*key = sNextKey;
		sNextKey++;

		// Add the key to the reserved list.
		sReservedKeys.insert(KeyDestructorMapPair(*key, destructor));
	}
	sMutex.unlock();
	return 0;
}

int pthread_key_delete(pthread_key_t key)
{
	sMutex.lock();
	{
		auto iterator = sReservedKeys.find(key);
		if (iterator != sReservedKeys.end())
		{
			// Null out the key's data from all threads.
			// Note: The official pthreads documentation states that the destructor callback is not used
			//       to delete data by this function. The caller is expected to do this in this case.
			Pthreads::ThreadData *threadDataPointer;
			for (int index = sThreadDataManager.GetCount() - 1; index >= 0; index--)
			{
				threadDataPointer = sThreadDataManager.GetByIndex(index);
				if (threadDataPointer)
				{
					threadDataPointer->GetKeyDataCollection().SetData(key, nullptr);
				}
			}

			// Remove the key from the reserved list, making it available in the future.
			sReservedKeys.erase(iterator);
		}
	}
	sMutex.unlock();
	return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	// Validate.
	if (nullptr == mutex)
	{
		return EINVAL;
	}

	// Fetch the mutex type from the given attribute, if provided.
	int type = PTHREAD_MUTEX_DEFAULT;
	if (attr && (PTHREAD_MUTEX_RECURSIVE == *attr))
	{
		type = PTHREAD_MUTEX_RECURSIVE;
	}

	// Create the mutex.
	Pthreads::BaseMutex *fMutexPointer = nullptr;
	try
	{
		if (PTHREAD_MUTEX_RECURSIVE == type)
		{
			fMutexPointer = new Pthreads::RecursiveMutex();
		}
		else
		{
			fMutexPointer = new Pthreads::Mutex();
		}
	}
	catch (...)
	{
		return EAGAIN;
	}

	// Return the mutex.
	*mutex = (pthread_mutex_t)fMutexPointer;
	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	// Validate.
	if ((nullptr == mutex) || (nullptr == *mutex))
	{
		return EINVAL;
	}

	// Destroy the given mutex.
	delete *((Pthreads::BaseMutex**)mutex);

	// Null out the given mutex and return a success result code.
	*mutex = nullptr;
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	// Validate.
	if ((nullptr == mutex) || (nullptr == *mutex))
	{
		return EINVAL;
	}

//TODO: Add error checking support if the attribute was set.

	// Lock the mutex to the current thread.
	int errorCode = 0;
	try
	{
		(*((Pthreads::BaseMutex**)mutex))->Lock();
	}
	catch (std::system_error &error)
	{
		errorCode = error.code().value();
	}
	catch (...)
	{
		errorCode = EAGAIN;
	}
	return errorCode;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	// Validate.
	if ((nullptr == mutex) || (nullptr == *mutex))
	{
		return EINVAL;
	}

//TODO: Add error checking support if the attribute was set.

	// Unlock the mutex, making it available to other threads.
	int errorCode = 0;
	try
	{
		(*((Pthreads::BaseMutex**)mutex))->Unlock();
	}
	catch (std::system_error &error)
	{
		errorCode = error.code().value();
	}
	catch (...)
	{
		errorCode = EAGAIN;
	}
	return errorCode;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	// Validate.
	if (nullptr == attr)
	{
		return EINVAL;
	}

	// For now, only store the mutex type to the attribute.
	// In the future, we may want this to be struct of attribute settings.
	*attr = PTHREAD_MUTEX_DEFAULT;
	return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	// Validate.
	if (nullptr == attr)
	{
		return EINVAL;
	}

	// Reset the given attribute to its default.
	*attr = PTHREAD_MUTEX_DEFAULT;
	return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	// Validate.
	if (nullptr == attr)
	{
		return EINVAL;
	}

	// Store the given mutex type.
	*attr = type;
	return 0;
}

int pthread_once(pthread_once_t *once_control, void(*init_routine)(void))
{
	// Validate arguments.
	if ((nullptr == once_control) || (nullptr == init_routine))
	{
		return EINVAL;
	}

	// Invoke the given callback if it not done already.
	sMutex.lock();
	{
		if (0 == *once_control)
		{
			(*init_routine)();
			*once_control = 1;
		}
	}
	sMutex.unlock();
	return 0;
}

pthread_t pthread_self(void)
{
	return (pthread_t)GetThreadDataFor(std::this_thread::get_id());
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
	//NOTE: You cannot set thread priority on the C/C++ side on Windows Phone.
	//      Thread priority can only be set on the .NET side, assuming it actually works.
	return 0;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
	int errorCode = 0;

	sMutex.lock();
	{
		auto iterator = sReservedKeys.find(key);
		if (iterator != sReservedKeys.end())
		{
			auto threadDataPointer = sThreadDataManager.GetBy(std::this_thread::get_id());
			if (threadDataPointer)
			{
				threadDataPointer->GetKeyDataCollection().SetData(key, value);
			}
		}
		else
		{
			errorCode = EINVAL;
		}
	}
	sMutex.unlock();
	return errorCode;
}

void pthread_sleep(int milliseconds)
{
	if (milliseconds > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}
}

void pthread_yield(void)
{
	std::this_thread::yield();
}

#pragma endregion

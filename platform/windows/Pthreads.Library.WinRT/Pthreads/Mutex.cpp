//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Mutex.h"


namespace Pthreads {

#pragma region Constructors/Destructors
Mutex::Mutex()
{
}

Mutex::~Mutex()
{
}

#pragma endregion


#pragma region Public Functions
void Mutex::Lock()
{
	fMutex.lock();
}

void Mutex::Unlock()
{
	fMutex.unlock();
}

#pragma endregion

}	// namespace Pthreads

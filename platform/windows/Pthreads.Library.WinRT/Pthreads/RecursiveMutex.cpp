//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "RecursiveMutex.h"


namespace Pthreads {

#pragma region Constructors/Destructors
RecursiveMutex::RecursiveMutex()
{
}

RecursiveMutex::~RecursiveMutex()
{
}

#pragma endregion


#pragma region Public Functions
void RecursiveMutex::Lock()
{
	fMutex.lock();
}

void RecursiveMutex::Unlock()
{
	fMutex.unlock();
}

#pragma endregion

}	// namespace Pthreads

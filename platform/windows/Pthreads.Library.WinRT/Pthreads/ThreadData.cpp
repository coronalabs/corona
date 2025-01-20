//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "ThreadData.h"


namespace Pthreads {

#pragma region Constructors/Destructors
ThreadData::ThreadData()
:	fStandardThreadPointer(nullptr),
	fExitData(nullptr)
{
}

ThreadData::~ThreadData()
{
}

#pragma endregion


#pragma region Public Functions
std::shared_ptr<std::thread> ThreadData::GetStandardThreadPointer()
{
	return fStandardThreadPointer;
}

void ThreadData::SetStandardThreadPointer(std::thread *threadPointer)
{
	fStandardThreadPointer = std::shared_ptr<std::thread>(threadPointer);
}

KeyDataCollection& ThreadData::GetKeyDataCollection()
{
	return fKeyDataCollection;
}

void* ThreadData::GetExitData()
{
	return fExitData;
}

void ThreadData::SetExitData(void* data)
{
	fExitData = data;
}

#pragma endregion

}	// namespace Pthreads

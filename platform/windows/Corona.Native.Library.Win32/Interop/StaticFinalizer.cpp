//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StaticFinalizer.h"


namespace Interop {

#pragma region Constructors/Destructors
StaticFinalizer::StaticFinalizer()
:	StaticFinalizer(nullptr)
{
}

StaticFinalizer::StaticFinalizer(const StaticFinalizer::Callback callback)
:	fWasFinalized(false),
	fCallback(callback)
{
}

StaticFinalizer::~StaticFinalizer()
{
	OnFinalize();
}

#pragma endregion


#pragma region Public Methods
bool StaticFinalizer::HasDependencies() const
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
	return (fDirectDependencies.size() > 0);
}

void StaticFinalizer::AddDependency(const StaticFinalizer& finalizer)
{
	// Do not continue if this finalizer has already been invoked.
	if (fWasFinalized)
	{
		return;
	}

	// Do not continue if given a reference to this instance.
	if (&finalizer == this)
	{
		return;
	}

	// Add the finalizer to the dependency collection.
	bool wasAdded = false;
	StaticFinalizer& mutableFinalizer = const_cast<StaticFinalizer&>(finalizer);
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
		auto iter = fDirectDependencies.insert(&mutableFinalizer);
		wasAdded = iter.second;
	}

	// Add this finalizer to the given finalizer's indirect dependency collection.
	if (wasAdded)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(mutableFinalizer.fMutex);
		if (!mutableFinalizer.fWasFinalized)
		{
			mutableFinalizer.fIndirectDependencies.insert(this);
		}
	}
}

void StaticFinalizer::RemoveDependency(const StaticFinalizer& finalizer)
{
	// Do not continue if this finalizer has already been invoked.
	if (fWasFinalized)
	{
		return;
	}

	// Do not continue if given a reference to this instance.
	if (&finalizer == this)
	{
		return;
	}

	// Remove the given finalizer from the dependency collection.
	StaticFinalizer& mutableFinalizer = const_cast<StaticFinalizer&>(finalizer);
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
		fDirectDependencies.erase(&mutableFinalizer);
	}

	// Remove this finalizer from the given finalizer's indirect dependency collection.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(mutableFinalizer.fMutex);
		if (!mutableFinalizer.fWasFinalized)
		{
			mutableFinalizer.fIndirectDependencies.erase(this);
		}
	}
}

#pragma endregion


#pragma region Private Methods
void StaticFinalizer::OnFinalize()
{
	// Do not continue if this finalizer's callback has already been invoked.
	if (fWasFinalized)
	{
		return;
	}

	// Flag that this object was finalized.
	// Must be done before invoking other finalizers in case there are any recursive dependencies.
	fWasFinalized = true;

	// First invoke the finalizers that depend on this finalizer.
	for (auto&& nextFinalizer : fIndirectDependencies)
	{
		if (nextFinalizer)
		{
			nextFinalizer->OnFinalize();
		}
	}

	// Invoke this finalizer's callback.
	if (fCallback)
	{
		(*fCallback)();
	}

	// Finally, invoke the finalizers that this finalizer depends on.
	for (auto&& nextFinalizer : fDirectDependencies)
	{
		if (nextFinalizer)
		{
			nextFinalizer->OnFinalize();
		}
	}
}

#pragma endregion

}	// namespace Interop

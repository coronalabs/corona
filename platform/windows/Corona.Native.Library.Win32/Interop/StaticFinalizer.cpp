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

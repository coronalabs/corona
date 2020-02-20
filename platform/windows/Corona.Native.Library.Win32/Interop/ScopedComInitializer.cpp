//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScopedComInitializer.h"
#include "Core\Rtt_Assert.h"


namespace Interop {

#pragma region Constructors/Destructors
ScopedComInitializer::ScopedComInitializer()
:	ScopedComInitializer(ApartmentType::kMultithreaded)
{
}

ScopedComInitializer::ScopedComInitializer(ScopedComInitializer::ApartmentType type)
:	fIsInitialized(false),
	fThreadId(std::this_thread::get_id())
{
	// Initialize COM on this thread.
	// Note: If this function fails, then COM was already initialized on this thread, but with a different apartment type.
	DWORD nativeTypeId = (ApartmentType::kSingleThreaded == type) ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED;
	HRESULT result = ::CoInitializeEx(nullptr, nativeTypeId);
	if (SUCCEEDED(result))
	{
		fIsInitialized = true;
	}
}

ScopedComInitializer::~ScopedComInitializer()
{
	// Do not continue if COM was not successfully initialized.
	if (!fIsInitialized)
	{
		return;
	}

	// Do not continue if this destructor is not being called by the same thread as the constructor.
	// This is important because we must balance a CoUninitialize() call on the same thread as CoInitializeEx().
	if (std::this_thread::get_id() != fThreadId)
	{
		Rtt_ASSERT(0);
		return;
	}

	// Unitialize COM on this thread.
	::CoUninitialize();
}

#pragma endregion


#pragma region Public Methods
bool ScopedComInitializer::IsInitialized() const
{
	return fIsInitialized;
}

#pragma endregion

}	// namespace Interop

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScopedOleInitializer.h"
#include "Core\Rtt_Assert.h"


namespace Interop {

#pragma region Constructors/Destructors
ScopedOleInitializer::ScopedOleInitializer()
:	fIsInitialized(false),
	fThreadId(std::this_thread::get_id())
{
	// Initialize OLE on this thread.
	// Note: If this function fails, then COM was initialized on this thread with the multithreaded apartment type.
	HRESULT result = ::OleInitialize(nullptr);
	if (SUCCEEDED(result))
	{
		fIsInitialized = true;
	}
}

ScopedOleInitializer::~ScopedOleInitializer()
{
	// Do not continue if OLE was not successfully initialized.
	if (!fIsInitialized)
	{
		return;
	}

	// Do not continue if this destructor is not being called by the same thread as the constructor.
	// This is important because we must balance a OleUninitialize() call on the same thread as OleInitialize().
	if (std::this_thread::get_id() != fThreadId)
	{
		Rtt_ASSERT(0);
		return;
	}

	// Unitialize COM on this thread.
	::OleUninitialize();
}

#pragma endregion


#pragma region Public Methods
bool ScopedOleInitializer::IsInitialized() const
{
	return fIsInitialized;
}

#pragma endregion

}	// namespace Interop

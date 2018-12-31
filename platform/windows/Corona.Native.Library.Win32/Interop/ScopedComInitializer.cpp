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

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

#pragma once

#include "Core\Rtt_Macros.h"
#include <thread>


namespace Interop {

/// <summary>
///  <para>Object whose constructor initializes COM for the current thread and destructor uninitializes COM.</para>
///  <para>Ensures that the Win32 CoInitialize() and CoUninitialize() calls are properly balanced.</para>
///  <para>Intended to be used as a member variable in a class or as a local variable within a function/method.</para>
///  <para>Note that you must intialize COM a thread that intends to create/use a COM interface.</para>
/// </summary>
class ScopedComInitializer
{
	Rtt_CLASS_NO_COPIES(ScopedComInitializer)

	public:
		/// <summary>Inidicates the type of threading model the COM apartment should be created with.</summary>
		enum class ApartmentType
		{
			/// <summary>
			///  <para>Indicates that the COM apartment model should be single threaded.</para>
			///  <para>Uses the Win32 COINIT_APARTMENTTHREADED flag for COM initialization.</para>
			/// </summary>
			kSingleThreaded,

			/// <summary>
			///  <para>Indicates that the COM apartment model should be multithreaded.</para>
			///  <para>Uses the Win32 COINIT_MULTITHREADED flag for COM initialization.</para>
			/// </summary>
			kMultithreaded
		};

		/// <summary>
		///  <para>Initializes Microsoft COM on the thread this object was created on.</para>
		///  <para>Defaults to the COINIT_MULTITHREADED apartment model.</para>
		/// </summary>
		ScopedComInitializer();

		/// <summary>Initializes Microsoft COM on the thread this object was created on.</summary>
		/// <param name="type">
		///  The COM apartment model that should be created such as kSingleThreaded or kMultithreaded.
		/// </param>
		ScopedComInitializer(ScopedComInitializer::ApartmentType type);

		/// <summary>
		///  <para>Unitializes COM when this object gets destroyed.</para>
		///  <para>The destructor is expected to be called on the same thread the constructor was called on.</para>
		/// </summary>
		virtual ~ScopedComInitializer();

		/// <summary>Determines if this object's constructor successfully initialized COM on the calling thread.</summary>
		/// <returns>
		///  <para>Returns true if COM was successfully initialized, meaning that it's safe to use COM interfaces.</para>
		///  <para>
		///   Returns false if COM initialization failed. This can happen if it was initalized externally before
		///   with the COINIT_APARTMENTTHREADED flag or if attempted to initialize COM within a DllMain() function.
		///  </para>
		/// </returns>
		bool IsInitialized() const;

	private:
		/// <summary>Set true if the constructor successfully initialize COM.</summary>
		bool fIsInitialized;

		/// <summary>The thread that COM was initialized on.</summary>
		std::thread::id fThreadId;
};

}	// namespace Interop

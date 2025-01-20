//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include <thread>


namespace Interop {

/// <summary>
///  <para>Object whose constructor initializes OLE on the current thread and destructor uninitializes OLE.</para>
///  <para>Ensures that the Win32 OleInitialize() and OleUninitialize() calls are properly balanced.</para>
///  <para>Intended to be used as a member variable in a class or as a local variable within a function/method.</para>
///  <para>Note that you must intialize OLE on a thread that intends to create/use an OLE interface.</para>
///  <para>Also note that initializing OLE will also initialize COM using the single threaded (STA) model.</para>
/// </summary>
class ScopedOleInitializer
{
	Rtt_CLASS_NO_COPIES(ScopedOleInitializer)

	public:
		/// <summary>
		///  Initializes Microsoft OLE (and COM) on the thread this object was created on
		///   using the single threaded (STA) model.
		/// </summary>
		ScopedOleInitializer();

		/// <summary>
		///  <para>Unitializes OLE when this object gets destroyed.</para>
		///  <para>The destructor is expected to be called on the same thread the constructor was called on.</para>
		/// </summary>
		virtual ~ScopedOleInitializer();

		/// <summary>Determines if this object's constructor successfully initialized OLE on the calling thread.</summary>
		/// <returns>
		///  <para>Returns true if OLE was successfully initialized, meaning that it's safe to use OLE interfaces.</para>
		///  <para>
		///   Returns false if OLE initialization failed. This can happen if it COM was initalized externally before
		///   using the COINIT_APARTMENTTHREADED flag or if attempted to initialize OLE within a DllMain() function.
		///  </para>
		/// </returns>
		bool IsInitialized() const;

	private:
		/// <summary>Set true if the constructor successfully initialize OLE.</summary>
		bool fIsInitialized;

		/// <summary>The thread that OLE was initialized on.</summary>
		std::thread::id fThreadId;
};

}	// namespace Interop

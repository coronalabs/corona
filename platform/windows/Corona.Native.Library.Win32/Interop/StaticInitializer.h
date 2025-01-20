//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Assert.h"
#include "Core\Rtt_Macros.h"


namespace Interop {

/// <summary>
///  <para>Initializer used to invoke a callback at construction time.</para>
///  <para>
///   Intended to be created as a static object used to intialize all other static variables in the same class/module.
///  </para>
/// </summary>
class StaticInitializer
{
	Rtt_CLASS_NO_COPIES(StaticInitializer)

	public:
		/// <summary>Creates an initializer that will invoke the given callback upon construction.</summary>
		/// <param name="callback">The callback to be invoked upon construction. Cannot be null.</param>
		StaticInitializer(void(*callback)())
		{
			if (Rtt_VERIFY(callback))
			{
				(*callback)();
			}
		}
};

}	// namespace Interop

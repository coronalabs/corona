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

#include "Rtt_PlatformExitCallback.h"

#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}

#pragma endregion


namespace Rtt
{

/// <summary>
///  <para>Callback to be invoked by the Lua os.exit() function.</para>
///  <para>If running in the Corona Simulator, this callback will terminate the runtime instead of exiting the app.</para>
///  <para>If not running in the Corona Simulator, then this callback will forcefully quit the application process.</para>
/// </summary>
class WinExitCallback : public PlatformExitCallback
{
	public:
		/// <summary>Creates a new callback that will terminate the given Corona runtime when invoked.</summary>
		/// <param name="environment">Reference to the Corona runtime environment that owns this object.</param>
		WinExitCallback(Interop::RuntimeEnvironment& environment);

		/// <summary>Destroyes this object.</summary>
		virtual ~WinExitCallback();

		/// <summary>Terminates the Corona runtime with an exit code of zero.</summary>
		virtual void operator()();

		/// <summary>Terminates the Corona runtime with the given exit code.</summary>
		/// <param name="code">Exit code to be returned by this application process. Currently ignored.</param>
		virtual void operator()(int code);

	private:
		/// <summary>Reference to the Corona runtime environment that owns this object.</summary>
		Interop::RuntimeEnvironment& fEnvironment;
};

}	// namespace Rtt

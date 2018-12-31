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

#ifndef _Rtt_PlatformInputDeviceManager_H__
#define _Rtt_PlatformInputDeviceManager_H__

#include "Rtt_InputDeviceCollection.h"
#include "Rtt_ReadOnlyInputDeviceCollection.h"


// Forward declarations.
struct Rtt_Allocator;
namespace Rtt
{
	class InputDeviceType;
	class PlatformInputDevice;
}


namespace Rtt
{

/// Stores a collection of all input devices that Corona supports and assigns them
/// unique identifiers and descriptors for easy lookup within native code and in Lua.
/// <br>
/// A derived version of this class is intended to be made if a platform has its own
/// derived version of a PlatformInputDevice object. In this case, the derived version
/// of this manager class should override the CreateUsing() and Destroy() functions
/// to create and destroy these platform specific input device objects.
class PlatformInputDeviceManager
{
	public:
		PlatformInputDeviceManager(Rtt_Allocator *allocatorPointer);
		virtual ~PlatformInputDeviceManager();

		PlatformInputDevice* Add(InputDeviceType type);
		Rtt_Allocator* GetAllocator() const;
		const ReadOnlyInputDeviceCollection& GetDevices() const;

	protected:
		virtual PlatformInputDevice* CreateUsing(const InputDeviceDescriptor &descriptor);
		virtual void Destroy(PlatformInputDevice* devicePointer);

	private:
		// Copy constructor and assignment operator made private to prevent object from being overwritten.
		void operator=(const PlatformInputDeviceManager &manager) { };

		/// Stores all input devices that have been discovered.
		/// Will contain disconnected devices that were once connected.
		InputDeviceCollection fDeviceCollection;

		/// Read-only collection which wraps the mutable "fDeviceCollection" member variable.
		/// This is the collection that get exposed outside of this class.
		ReadOnlyInputDeviceCollection fReadOnlyDeviceCollection;
};

} // namespace Rtt

#endif // _Rtt_PlatformInputDeviceManager_H__

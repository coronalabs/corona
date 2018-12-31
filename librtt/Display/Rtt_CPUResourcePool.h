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

#ifndef _Rtt_CPUResourcePool_H__
#define _Rtt_CPUResourcePool_H__

#include "Renderer/Rtt_MCPUResourceObserver.h"
#include <map>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class CPUResource;

class CPUResourcePool : public MCPUResourceObserver
{
	Rtt_CLASS_NO_COPIES( CPUResourcePool )

	public:
		typedef CPUResourcePool Self;

	public:
		CPUResourcePool();
		virtual ~CPUResourcePool();
		
	public:
		virtual void ReleaseGPUResources();

	public:
		//Should only be called by the cpu resources to register/unregister
		virtual void AttachResource(CPUResource *resource);
		virtual void DetachResource(CPUResource *resource);
		
	private:
		//Not a fan of this practice, let's come up with a better implementation (linked list?)
		std::map<const CPUResource*,CPUResource*> fCPUResources;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CPUResourcePool_H__

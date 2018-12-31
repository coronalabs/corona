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

#ifndef _Rtt_PhysicsContact_H__
#define _Rtt_PhysicsContact_H__

#ifdef Rtt_PHYSICS

#include "Core/Rtt_ResourceHandle.h"

// ----------------------------------------------------------------------------

class b2Contact;

struct lua_State;

namespace Rtt
{

class UserdataWrapper;
	
// ----------------------------------------------------------------------------

class PhysicsContact
{
	public:
		typedef PhysicsContact Self;

	public:
		static const char kMetatableName[];

	public:
		static UserdataWrapper *CreateWrapper( const ResourceHandle< lua_State >& luaStateHandle, b2Contact *contact );

	public:
		static b2Contact* GetContact( lua_State *L, int index );

	public:
		static void Initialize( lua_State *L );
				
	// Metatable methods
	public:
		static int ValueForKey( lua_State *L );
		static int SetValueForKey( lua_State *L );
		static int Finalizer( lua_State *L );
};

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS	

#endif // _Rtt_PhysicsContact_H__

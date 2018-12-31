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

#ifndef _Rtt_PhysicsJoint_H__
#define _Rtt_PhysicsJoint_H__

#ifdef Rtt_PHYSICS	

//#include "Box2D/Box2D.h"
#include "Rtt_LuaProxyVTable.h"

// ----------------------------------------------------------------------------

class b2Joint;
struct b2Vec2;

struct lua_State;

namespace Rtt
{

//class Runtime;
	
// ----------------------------------------------------------------------------

class PhysicsJoint
{
	public:
		typedef PhysicsJoint Self;

	public:
		static const char kMetatableName[];

	public:
		static b2Joint* GetJoint( lua_State *L, int index );

	public:
		static void Initialize( lua_State *L );
	
	public:
		static int getAnchorA( lua_State *L );
		static int getAnchorB( lua_State *L );
		static bool HasLocalAnchor( b2Joint& joint );
		static b2Vec2 GetLocalAnchorA( b2Joint& joint );
		static b2Vec2 GetLocalAnchorB( b2Joint& joint );
		static int getLocalAnchor( lua_State *L );
		static int getLocalAxis( lua_State *L );
		static int getReactionForce( lua_State *L );
		static int setRotationLimits( lua_State *L );
		static int getRotationLimits( lua_State *L );
		static int setLimits( lua_State *L );
		static int getLimits( lua_State *L );
		static int getGroundAnchorA( lua_State *L );
		static int getGroundAnchorB( lua_State *L );
		static int removeSelf( lua_State *L );
		static int getTarget( lua_State *L );
		static int setTarget( lua_State *L );
				
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

#endif // _Rtt_PhysicsJoint_H__

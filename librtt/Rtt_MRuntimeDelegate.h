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

#ifndef __Rtt_MRuntimeDelegate__
#define __Rtt_MRuntimeDelegate__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class MRuntimeDelegate
{
	public:
		virtual void DidInitLuaLibraries( const Runtime& sender ) const = 0;
		virtual bool HasDependencies( const Runtime& sender ) const = 0;

    public:
        virtual void WillLoadMain( const Runtime& sender ) const = 0;
		virtual void DidLoadMain( const Runtime& sender ) const = 0;

	public:
		virtual void WillLoadConfig( const Runtime& sender, lua_State *L ) const = 0;
		virtual void InitializeConfig( const Runtime& sender, lua_State *L ) const = 0;
		virtual void DidLoadConfig( const Runtime& sender, lua_State *L ) const = 0;

	public:
		virtual void WillSuspend( const Runtime& sender ) const = 0;
		virtual void DidSuspend( const Runtime& sender ) const = 0;
		virtual void WillResume( const Runtime& sender ) const = 0;
		virtual void DidResume( const Runtime& sender ) const = 0;
		virtual void WillDestroy( const Runtime& sender ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_MRuntimeDelegate__

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

#ifndef _Rtt_LuaTableIterator_H__
#define _Rtt_LuaTableIterator_H__

#include "Rtt_MKeyValueIterable.h"

#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class LuaTableIterator : public MKeyValueIterable
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		LuaTableIterator( lua_State *L, int index );
		~LuaTableIterator();

	public:
		virtual bool Next() const;
		virtual const char* Key() const;
		virtual const char* Value() const;

	private:
		const char* GetString( int index ) const;
		void PopPreviousValue() const;

	private:
		lua_State *fL;
		int fIndex;
		int fTop;
		mutable bool fPreviousValueOnStack;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaTableIterator_H__

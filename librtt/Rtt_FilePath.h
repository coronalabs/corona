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

#ifndef __Rtt_FilePath__
#define __Rtt_FilePath__

#include <string>
#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class UserdataWrapper;

// ----------------------------------------------------------------------------

// TODO: Rename to BitmapName
class FilePath
{
	public:
		typedef FilePath Self;

	public:
		static const char kMetatableName[];

	public:
		static int CreateAndPush(
			lua_State *L,
			Rtt_Allocator *allocator,
			const char *filename,
			MPlatform::Directory baseDir );

	// Metatable methods
	protected:
		static int Finalizer( lua_State *L );

	public:
		static void Initialize( lua_State *L );

	protected:
		FilePath( const char *filename, MPlatform::Directory baseDir );

	public:
		const char *GetFilename() const { return fFilename.c_str(); }
		MPlatform::Directory GetBaseDir() const { return (MPlatform::Directory)fBaseDir; }

	private:
		std::string fFilename;
		int fBaseDir;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_FilePath__

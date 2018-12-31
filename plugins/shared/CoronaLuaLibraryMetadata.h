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

#ifndef _CoronaLuaLibraryMetadata_H__
#define _CoronaLuaLibraryMetadata_H__

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

class LuaLibraryMetadata
{
	public:
		typedef LuaLibraryMetadata Self;

	public:
		LuaLibraryMetadata( const char *name, const char *publisherId, int version, int revision );
		~LuaLibraryMetadata();

	public:
		const char *GetName() const { return fName; }
		const char *GetPublisherId() const { return fPublisherId; }
		int GetVersion() const { return fVersion; }
		int GetRevision() const { return fRevision; }

	private:
		char *fName;
		char *fPublisherId;
		int fVersion;
		int fRevision;
};

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

#endif // _CoronaLuaLibraryMetadata_H__

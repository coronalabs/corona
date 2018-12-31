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

#ifndef _Rtt_PlatformConnection_H__
#define _Rtt_PlatformConnection_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

class MPlatformServices;
class PlatformDictionaryWrapper;

// ----------------------------------------------------------------------------

// TODO: This is a total kludge.  Redesign...
enum 
{
	kNullValueType = 0,
	kStringValueType = -1,
	kIntegerValueType = -2
};

struct KeyValuePair
{
	const char* key;
	const void* value;
	int type; // if > 0, then value is a (KeyValuePair*) and type is the number of pairs
};

class PlatformConnection
{
	public:
		PlatformConnection( const MPlatformServices& platform );
		virtual ~PlatformConnection();

	public:
		virtual PlatformDictionaryWrapper* Call( const char* method, const KeyValuePair* pairs, int numPairs ) = 0;
		virtual const char* Error() const = 0;
		virtual const char* Url() const = 0;
		virtual bool Download(const char *urlStr, const char *filename) = 0;
		virtual void CloseConnection() = 0;

	protected:
		const MPlatformServices& fPlatform;
        int fDebugWebServices;
//		char* fUrl;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformConnection_H__

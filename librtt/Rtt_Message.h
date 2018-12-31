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

#ifndef _Rtt_Message_H__
#define _Rtt_Message_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

struct MessageData
{
	U8 digest[16];
	U32 timestamp;
};

bool operator==( const MessageData& lhs, const MessageData& rhs );

class Message
{
	public:
		// Length in 4-byte words
		enum
		{
			kApplicationLen = 16,
			kSubscriptionLen = 64
		};

	public:
		Message( const char* filename );
		~Message();

	public:
		bool operator==( const Message& rhs ) const;
		bool operator==( const MessageData& rhs ) const;

	public:
		const MessageData& Data() const { return fData; }

	private:
		MessageData fData;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Message_H__

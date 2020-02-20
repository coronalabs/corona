//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

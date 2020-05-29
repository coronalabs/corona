//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_WeakCount_H__
#define __Rtt_WeakCount_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

class SharedCount;
class UseCount;

// ----------------------------------------------------------------------------

class WeakCount
{
	public:
		typedef WeakCount Self;

	public:
		WeakCount();
		WeakCount( const WeakCount& rhs );
		WeakCount( const SharedCount& rhs );
		~WeakCount();

	protected:
		WeakCount& Assign( UseCount *newCount );

	public:
		WeakCount& operator=( const WeakCount& rhs );
		WeakCount& operator=( const SharedCount& rhs );

	public:
		bool IsValid() const;
//		bool IsNull() const { return NULL == fCount; }
//		bool NotNull() const { return ! IsNull(); }

		void Log() const;

	public:
		friend inline bool operator==( const WeakCount &lhs, const WeakCount &rhs)
		{
			return lhs.fCount == rhs.fCount;
		}

	private:
		UseCount *fCount;

	friend class SharedCount;
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_WeakCount_H__


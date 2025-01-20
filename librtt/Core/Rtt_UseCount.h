//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_UseCount_H__
#define __Rtt_UseCount_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class UseCount
{
	public:
		UseCount();
		virtual ~UseCount();

	protected:
		virtual void FinalizeStrong() = 0;
		void FinalizeWeak();

	public:
		void RetainStrong() { ++fStrongCount; }
		void ReleaseStrong();

		void RetainWeak() { ++fWeakCount; }
		void ReleaseWeak();

	public:
		bool IsValid() { return fStrongCount > 0; }

		int GetStrongCount() const { return fStrongCount; }
		int GetWeakCount() const { return fWeakCount; }

		void Log() const;

	private:
		int fStrongCount;
		int fWeakCount;
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_UseCount_H__


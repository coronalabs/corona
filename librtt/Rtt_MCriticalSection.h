//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_MCriticalSection__
#define __Rtt_MCriticalSection__

#include "Core/Rtt_Macros.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MCriticalSection
{
	public:
		virtual void Begin() const = 0;
        virtual void End() const = 0;
};

// Use this guard to automatically scope Begin/End calls.
class CriticalSectionGuard
{
	Rtt_CLASS_NO_COPIES( CriticalSectionGuard )
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		CriticalSectionGuard( const MCriticalSection *criticalSection )
		:	fCriticalSection( criticalSection )
		{
			if ( criticalSection )
			{
				criticalSection->Begin();
			}
		}

		~CriticalSectionGuard()
		{
			if ( fCriticalSection )
			{
				fCriticalSection->End();
			}
		}

	private:
		const MCriticalSection *fCriticalSection;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_MCriticalSection__

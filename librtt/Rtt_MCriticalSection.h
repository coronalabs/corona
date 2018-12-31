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

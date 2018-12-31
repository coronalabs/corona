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

#include "Core/Rtt_Build.h"

#include "Rtt_Scheduler.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Task::~Task()
{
}

template class Rtt::Array<Rtt::Task *>;
	
// ----------------------------------------------------------------------------

Scheduler::Scheduler( Runtime& owner )
:	fOwner( owner ),
	fProcessing( false ),
	fTasks( owner.GetAllocator() )
{
}

Scheduler::~Scheduler()
{
}

#if 0
void
Scheduler::Prepend( Task* e )
{
	Rtt_ASSERT_MSG( ! fTasks, "TODO: Make scheduler support more than one event" );
	fTasks = e;
}
#endif
	
void
Scheduler::Append( Task* e )
{
	//Rtt_ASSERT( fProcessing == false );		//**tjn removed
	
	fTasks.Append( e );
}

void
Scheduler::Run()
{
	fProcessing = true;
	
	int i = 0;
	while (i < fTasks.Length())
	{
		Task * t = fTasks[i];
		if (t->getKeepAlive())
		{
			(*t)(*this);
			i++;
		}
		else
		{
			// "pop event"
			fTasks.Remove(i, 1, false);
			(*t)(*this);
			Rtt_DELETE(t);
		}
	}

	fProcessing = false;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


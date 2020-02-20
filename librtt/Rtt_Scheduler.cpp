//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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


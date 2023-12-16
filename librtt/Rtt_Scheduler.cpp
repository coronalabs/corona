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

#include <mutex>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Task::~Task()
{
}

template class Rtt::Array<Rtt::Task *>;
	
static std::mutex sListMutex;
static std::mutex sTaskMutex;

// ----------------------------------------------------------------------------

Scheduler::Scheduler( Runtime& owner )
:	fOwner( owner ),
	fFirstPending( NULL ),
	fProcessing( false ),
	fTasks( owner.GetAllocator() )
{
}

Scheduler::~Scheduler()
{
	SyncPendingList();
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

	std::lock_guard<std::mutex> lock( sListMutex );

	e->setNext( fFirstPending );

	fFirstPending = e;
}

void
Scheduler::Delete(Task* e)
{
	SyncPendingList();

	std::lock_guard<std::mutex> lock( sTaskMutex );

	int i = 0;
	while (i < fTasks.Length())
	{
		Task* t = fTasks[i];
		if (t == e)
		{
			// "pop event"
			fTasks.Remove(i, 1, false);
			Rtt_DELETE(t);
		}
		else
			i++;
	}
}

void
Scheduler::Run()
{
	fProcessing = true;
	
	SyncPendingList();

	std::lock_guard<std::mutex> lock( sTaskMutex );

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

Task*
Scheduler::ExtractPendingList()
{
	std::lock_guard<std::mutex> lock( sListMutex );

	Task* first = fFirstPending;

	fFirstPending = NULL;

	return first;
}

void
Scheduler::SyncPendingList()
{
	Task* first = ExtractPendingList();

	if ( NULL != first )
	{
		std::lock_guard<std::mutex> lock( sTaskMutex );

		for ( Task* cur = first; cur; cur = cur->getNext() )
		{
			fTasks.Append( cur );
		}
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


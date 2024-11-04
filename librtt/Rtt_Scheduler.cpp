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
	fFirstPending( NULL ),
	fProcessing( false ),
	fTasks( owner.GetAllocator() )
{
}

Scheduler::~Scheduler()
{
	SyncPendingList(); // cf. note (also assumes other notifying threads have been shut down)
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

	SetHead( e->getNextRef(), e );
}

void
Scheduler::Delete(Task* e)
{
	// N.B. this is still not thread-safe
	// However, Delete() never actually seems to be called anywhere, so that point, and what follows, is a bit academic
		// On a Windows testing, `Delete()` can be removed and everything compiles fine
		// A search for `->Delete()` and `.Delete()` in the top-level directory, i.e. including other platforms, turns up no results
	// Anyhow, assuming we did want a thread-safe version, it might go like so:
		// Keep a second singly-linked list of `DeletedTask()`
			// uses same atomic CAS
			// just a non-owning box over `Task`, with link to next
		// if non-empty, extract in `Run()` after sync (expected list head == NULL)
			// for each
				// search in tasks
					// remove if found
				// advance to next task and delete box (or do list afterward in one go, cf. note below about destructor)
			// fine if missing, e.g. if a `Run()` already removed it
			// for same reason multiple `Delete()`s safe
		// Clean up deleted list in destructor
		// Caveat, due to the timing:
			// the deleted task might fire, if a `Run()` is already in progress (task must be robust against this possibility)
			// conversely, a `Run()` in progress might not see an incoming delete
			// will either eventually resolve, or be handled by the destructor

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
	
	SyncPendingList(); // cf. note

	// TODO? cf. commentary in `Delete()`

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

void
Scheduler::SetHead( Task::NextType& oldValue, Task* newValue )
{
	// see https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange#Example (Dec 7. 2023)

	oldValue = fFirstPending.load( std::memory_order_relaxed );
 
    while ( !fFirstPending.compare_exchange_weak( oldValue, newValue,
                                        std::memory_order_release,
                                        std::memory_order_relaxed ) )
		; // empty
}

Task*
Scheduler::ExtractPendingList()
{
	Task* first = NULL;
	
	SetHead( first, NULL );

	return first;
}

void
Scheduler::SyncPendingList()
{
	// N.B. assumed to be in main thread
	Task* first = ExtractPendingList();

	S32 currentLength = fTasks.Length();

	for ( Task* cur = first; cur; cur = cur->getNext() )
	{
		fTasks.Append( cur );
	}

	S32 newLength = fTasks.Length();

	// now reverse the new part, since pending list was a stack
	Task** tasks = fTasks.WriteAccess();

	for ( S32 i = currentLength, j = newLength - 1; i < j; i++, j-- )
	{
		Task* temp = tasks[j];

		tasks[j] = tasks[i];
		tasks[i] = temp;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Scheduler_H__
#define _Rtt_Scheduler_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Array.h"

#include <atomic>

namespace Rtt
{

class Runtime;
class Scheduler;

// ----------------------------------------------------------------------------

class Task
{
	public:
		typedef Task* NextType;

		Task() : fKeepAlive(false), fNext(NULL) {}
		Task(bool keepAlive) : fKeepAlive(keepAlive) {}
		virtual ~Task();

		// TODO: return status code???  Or it can re-schedule it internally...
		virtual void operator()( Scheduler& sender ) = 0;

		bool getKeepAlive() const { return fKeepAlive; }
		void setKeepAlive(bool val) { fKeepAlive = val; }

		NextType& getNextRef() { return fNext; }
		Task* getNext() const { return fNext; }

	private:
		bool fKeepAlive;
		Task* fNext;
};

class Scheduler
{
	public:
		typedef Runtime Owner;

	public:
		Scheduler( Owner& owner );
		~Scheduler();

	public:
//		void Prepend( Task* e );
		void Append( Task* e );
		void Delete(Task* e);

	public:
		void Run();

	public:
		Owner& GetOwner() { return fOwner; }

	private:
		void SetHead( Task::NextType& oldValue, Task* newValue );
		Task* ExtractPendingList();
		void SyncPendingList();

	private:
		Owner& fOwner;
		std::atomic< Task* > fFirstPending;
		
		PtrArray< Task > fTasks;
		bool fProcessing;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Scheduler_H__

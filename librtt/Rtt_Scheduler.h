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

namespace Rtt
{

class Runtime;
class Scheduler;

// ----------------------------------------------------------------------------

class Task
{
	public:
		Task() : fKeepAlive(false) {}
		Task(bool keepAlive) : fKeepAlive(keepAlive) {}
		virtual ~Task();

		// TODO: return status code???  Or it can re-schedule it internally...
		virtual void operator()( Scheduler& sender ) = 0;

		bool getKeepAlive() const { return fKeepAlive; }
		void setKeepAlive(bool val) { fKeepAlive = val; }

	private:
		bool fKeepAlive;
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

	public:
		void Run();

	public:
		Owner& GetOwner() { return fOwner; }

	private:
		Owner& fOwner;
		
		PtrArray< Task > fTasks;
		bool fProcessing;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Scheduler_H__

// ----------------------------------------------------------------------------
// 
// Rtt_WinRTTimer.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTTimer.h"
#include "CoronaLabs\Corona\WinRT\Interop\ITimer.h"
#include "CoronaLabs\Corona\WinRT\Interop\ITimerServices.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTTimer::WinRTTimer(MCallback& callback, CoronaLabs::Corona::WinRT::Interop::ITimerServices^ timerServices)
:	PlatformTimer(callback),
	fTimer(nullptr),
	fEventHandler(nullptr)
{
	if (timerServices)
	{
		fTimer = timerServices->CreateTimer();
	}
	fEventHandler = ref new WinRTTimer::EventHandler(this);
}

WinRTTimer::~WinRTTimer()
{
	if (fTimer)
	{
		fTimer->Stop();
	}
}

#pragma endregion


#pragma region Public Member Functions
void WinRTTimer::Start()
{
	if (fTimer)
	{
		fEventHandler->Subscribe();
		fTimer->Start();
	}
}

void WinRTTimer::Stop()
{
	if (fTimer)
	{
		fEventHandler->Unsubscribe();
		fTimer->Stop();
	}
}

void WinRTTimer::SetInterval(U32 milliseconds)
{
	// Do not continue if the timer provider failed to give us a new timer.
	if (!fTimer)
	{
		return;
	}

	// Do not allow the given interval to be less than 1 millisecond.
	if (milliseconds < 1)
	{
		milliseconds = 1;
	}

	// Convert to 100-nanosecond intervals.
	int64 nanoseconds = (int64)milliseconds * 10000LL;

	// Change the timer's interval.
	Windows::Foundation::TimeSpan interval;
	interval.Duration = nanoseconds;
	fTimer->Interval = interval;
}

bool WinRTTimer::IsRunning() const
{
	if (!fTimer)
	{
		return false;
	}
	return fTimer->IsRunning;
}

#pragma endregion


#pragma region Event Handlers
WinRTTimer::EventHandler::EventHandler(WinRTTimer* timerPointer)
:	fTimerPointer(timerPointer)
{
}

void WinRTTimer::EventHandler::Subscribe()
{
	if (fTimerPointer && fTimerPointer->fTimer && (0 == fEventToken.Value))
	{
		fEventToken = fTimerPointer->fTimer->Elapsed += ref new Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::ITimer^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &WinRTTimer::EventHandler::OnTimerElapsed);
	}
}

void WinRTTimer::EventHandler::Unsubscribe()
{
	if (fEventToken.Value)
	{
		fTimerPointer->fTimer->Elapsed -= fEventToken;
		fEventToken.Value = 0;
	}
}

void WinRTTimer::EventHandler::OnTimerElapsed(
	CoronaLabs::Corona::WinRT::Interop::ITimer^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	(*fTimerPointer)();
}

#pragma endregion

}

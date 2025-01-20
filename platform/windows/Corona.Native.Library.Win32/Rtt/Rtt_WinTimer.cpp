//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinTimer.h"
#include <windows.h>


namespace Rtt
{

std::unordered_map<UINT_PTR, Rtt::WinTimer *> WinTimer::sTimerMap;
UINT_PTR WinTimer::sMostRecentTimerID;

#pragma region Constructors/Destructors
WinTimer::WinTimer(MCallback& callback, HWND windowHandle)
:	PlatformTimer(callback)
{
	fWindowHandle = windowHandle;
	fTimerPointer = NULL;
	fIntervalInMilliseconds = 10;
	fNextIntervalTimeInTicks = 0;
}

WinTimer::~WinTimer()
{
	Stop();
}

#pragma endregion


#pragma region Public Methods
void WinTimer::Start()
{
	// Do not continue if the timer is already running.
	if (IsRunning())
	{
		return;
	}

	// Start the timer, but with an interval faster than the configured interval.
	// We do this because Windows timers can invoke later than expected.
	// To compensate, we'll schedule when to invoke the timer's callback using "fIntervalEndTimeInTicks".
	fNextIntervalTimeInTicks = (S32)::GetTickCount() + (S32)fIntervalInMilliseconds;
	fTimerID = ++sMostRecentTimerID; // ID should be non-0, so pre-increment for first time
	fTimerPointer = ::SetTimer(fWindowHandle, fTimerID, 10, WinTimer::OnTimerElapsed);

	if (IsRunning())
	{
		sTimerMap[fTimerID] = this;
	}
}

void WinTimer::Stop()
{
	// Do not continue if the timer has already been stopped.
	if (IsRunning() == false)
	{
		return;
	}

	// Stop the timer.
	::KillTimer(fWindowHandle, fTimerID);

	sTimerMap.erase(fTimerID);

	fTimerPointer = NULL;
	fTimerID = 0;
}

void WinTimer::SetInterval(U32 milliseconds)
{
	fIntervalInMilliseconds = milliseconds;
}

bool WinTimer::IsRunning() const
{
	return (fTimerPointer != NULL);
}

void WinTimer::Evaluate()
{
	// Do not continue if the timer is not running.
	if (IsRunning() == false)
	{
		return;
	}

	// Do not continue if we haven't reached the scheduled time yet.
	if (CompareTicks((S32)::GetTickCount(), fNextIntervalTimeInTicks) < 0)
	{
		return;
	}

	// Schedule the next interval time.
	for (; CompareTicks((S32)::GetTickCount(), fNextIntervalTimeInTicks) > 0; fNextIntervalTimeInTicks += fIntervalInMilliseconds);

	// Invoke this timer's callback.
	this->operator()();
}

#pragma endregion


#pragma region Private Methods/Functions
VOID CALLBACK WinTimer::OnTimerElapsed(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	auto timer = sTimerMap.find(idEvent);
	if (sTimerMap.end() != timer)
	{
		timer->second->Evaluate();
	}
}

S32 WinTimer::CompareTicks(S32 x, S32 y)
{
	// Overflow will occur when flipping sign bit of largest negative number.
	// Give it a one millisecond boost before flipping the sign.
	if (0x80000000 == y)
	{
		y++;
	}

	// Compare the given tick values via subtraction. Overlow for this subtraction operation is okay.
	S32 deltaTime = x - y;
	if (deltaTime < 0)
	{
		return -1;
	}
	else if (0 == deltaTime)
	{
		return 0;
	}
	return 1;
}

#pragma endregion

}	// namespace Rtt

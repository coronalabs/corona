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

#include "stdafx.h"
#include "Rtt_WinTimer.h"
#include <windows.h>


namespace Rtt
{

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
	fTimerPointer = ::SetTimer(fWindowHandle, (UINT_PTR)this, 10, WinTimer::OnTimerElapsed);
}

void WinTimer::Stop()
{
	// Do not continue if the timer has already been stopped.
	if (IsRunning() == false)
	{
		return;
	}

	// Stop the timer.
	::KillTimer(fWindowHandle, fTimerPointer);
	fTimerPointer = NULL;
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
	WinTimer *timer = (WinTimer*)idEvent;
	timer->Evaluate();
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

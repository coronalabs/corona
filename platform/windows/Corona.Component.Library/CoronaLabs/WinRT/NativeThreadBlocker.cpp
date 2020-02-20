//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "NativeThreadBlocker.h"
#include <chrono>
#include <thread>


namespace CoronaLabs { namespace WinRT {

#pragma region Constructors/Destructors
NativeThreadBlocker::NativeThreadBlocker()
{
	fAtomicIsEnabled = true;
	fAtomicBlockOperationCount = 0;
}

#pragma endregion


#pragma region Public Methods
void NativeThreadBlocker::Block()
{
	// Block the current thread infinitely until another thread unblocks it.
	fAtomicBlockOperationCount++;
	while (fAtomicIsEnabled.load())
	{
		std::this_thread::yield();
	}
	fAtomicBlockOperationCount--;
}

NativeThreadBlocker::Result NativeThreadBlocker::BlockUntil(
	const std::chrono::milliseconds &duration)
{
	return BlockUntilDurationInMilliseconds(duration.count());
}

NativeThreadBlocker::Result NativeThreadBlocker::BlockUntil(
	const std::chrono::seconds &duration)
{
	long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return BlockUntilDurationInMilliseconds(milliseconds);
}

NativeThreadBlocker::Result NativeThreadBlocker::BlockUntilDurationInMilliseconds(long long milliseconds)
{
	Result result = Result::kUnblockedManually;

	// Schedule when this block operation should timeout.
	auto timeoutInTicks = ::GetTickCount64() + milliseconds;

	// Block this thread until another thread unblocks it or until timed-out.
	fAtomicBlockOperationCount++;
	while (fAtomicIsEnabled.load())
	{
		// Yield to give the other threads a chance to unblock this object.
		std::this_thread::yield();

		// Unblock if we have timed-out.
		if (CompareTicks(::GetTickCount64(), timeoutInTicks) >= 0)
		{
			result = Result::kTimedOut;
			break;
		}
	}
	fAtomicBlockOperationCount--;
	return result;
}

void NativeThreadBlocker::Enable()
{
	fAtomicIsEnabled = true;
}

void NativeThreadBlocker::Disable()
{
	fAtomicIsEnabled = false;
}

bool NativeThreadBlocker::IsEnabled() const
{
	return fAtomicIsEnabled.load();
}

bool NativeThreadBlocker::IsDisabled() const
{
	return !IsEnabled();
}

bool NativeThreadBlocker::IsBlocking() const
{
	return (fAtomicBlockOperationCount.load() > 0);
}

bool NativeThreadBlocker::IsNotBlocking() const
{
	return !IsBlocking();
}

#pragma endregion


#pragma region Private Methods
int NativeThreadBlocker::CompareTicks(long long x, long long y)
{
	// Overflow will occur when flipping sign bit of largest negative number.
	// Give it a one millisecond boost before flipping the sign.
	if (LLONG_MIN == y)
	{
		y++;
	}

	// Compare the given tick values via subtraction. Overlow for this subtraction operation is okay.
	auto deltaTime = x - y;
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


#pragma region BlockResult Class
const NativeThreadBlocker::Result NativeThreadBlocker::Result::kUnblockedManually(true, false);

const NativeThreadBlocker::Result NativeThreadBlocker::Result::kTimedOut(false, true);

NativeThreadBlocker::Result::Result(bool wasUnblockedManually, bool hasTimedOut)
:	fWasUnblockedManually(wasUnblockedManually),
	fHasTimedOut(hasTimedOut)
{
}

bool NativeThreadBlocker::Result::WasUnblockedManually() const
{
	return fWasUnblockedManually;
}

bool NativeThreadBlocker::Result::HasTimedOut() const
{
	return fHasTimedOut;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT

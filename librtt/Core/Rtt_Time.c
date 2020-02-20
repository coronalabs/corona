//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Time.h"
// ----------------------------------------------------------------------------

#if defined( Rtt_APPLE_ENV )

	#include <mach/mach_time.h>
	// #include <CoreServices/CoreServices.h>

	Rtt_EXPORT Rtt_AbsoluteTime
	Rtt_GetAbsoluteTime(void)
	{
		return mach_absolute_time();
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime )
	{
		static int sInitInfo = 1;
		static double sCoefficient;
		static mach_timebase_info_data_t sTimebaseInfo = { 0, 0 };
		if ( sInitInfo )
		{
			sInitInfo = 0;
			mach_timebase_info( & sTimebaseInfo );
			
			const double kNumer = (double)sTimebaseInfo.numer;
			const double kDenom = (double)sTimebaseInfo.denom;
			const double kNSPerMS = 1000000;
			sCoefficient = kNumer / kDenom / kNSPerMS;
		}
		
		// Rtt_STATIC_ASSERT( sizeof( AbsoluteTime ) == sizeof( Rtt_AbsoluteTime ) );
		// Can't use AbsoluteToDuration b/c only 2^31 milliseconds available
		// Nanoseconds nanoseconds = AbsoluteToNanoseconds( *(AbsoluteTime*)&absoluteTime );
		
		// Convert to milliseconds
		U64 milliseconds = sCoefficient * absoluteTime;
		return milliseconds;
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime )
	{
		static int sInitInfo = 1;
		static double sCoefficient;
		static mach_timebase_info_data_t sTimebaseInfo = { 0, 0 };
		if ( sInitInfo )
		{
			sInitInfo = 0;
			mach_timebase_info( & sTimebaseInfo );
			
			const double kNumer = (double)sTimebaseInfo.numer;
			const double kDenom = (double)sTimebaseInfo.denom;
			const double kNSPerMicroS = 1000;
			sCoefficient = kNumer / kDenom / kNSPerMicroS;
		}

		// Convert to microseconds
		U64 result = sCoefficient * absoluteTime;
		return result;
	}

#elif defined( Rtt_ANDROID_ENV )

	#include <time.h>

	Rtt_EXPORT Rtt_AbsoluteTime
	Rtt_GetAbsoluteTime()
	{
		struct timespec currentTime;
		Rtt_AbsoluteTime currentTimeInMicroseconds;
		
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		currentTimeInMicroseconds = (Rtt_AbsoluteTime)currentTime.tv_sec * (Rtt_AbsoluteTime)1000000;
		currentTimeInMicroseconds += (Rtt_AbsoluteTime)currentTime.tv_nsec / (Rtt_AbsoluteTime)1000;
		return currentTimeInMicroseconds;
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime )
	{
		return absoluteTime / 1000;
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime )
	{
		return absoluteTime;
	}

#elif defined( Rtt_SYMBIAN_ENV )
	Rtt_STATIC_ASSERT( false );
#elif defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )  || defined( Rtt_NINTENDO_ENV )

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	Rtt_EXPORT Rtt_AbsoluteTime
	Rtt_GetAbsoluteTime()
	{
		Rtt_AbsoluteTime timeInMicroseconds;
		LARGE_INTEGER performanceCounterValue;

		// Initialize the following on the first call to this function.
		static BOOL sWasInitialized = FALSE;
		typedef ULONGLONG(WINAPI *GetTickCount64Callback)(void);
		static GetTickCount64Callback sGetTickCount64Callback;
		static uint64_t sPerformanceCounterFrequency;
		if (!sWasInitialized)
		{
			// Fetch a callback to the GetTickCount64() function.
			// This is used as a fallback mechanism in case the Win32 performance counter is not available.
#if defined( Rtt_WIN_DESKTOP_ENV )
			HMODULE moduleHandle = LoadLibraryW(L"kernel32");
			if (moduleHandle)
			{
				sGetTickCount64Callback = (GetTickCount64Callback)GetProcAddress(moduleHandle, "GetTickCount64");
			}
#else
			sGetTickCount64Callback = &GetTickCount64;
#endif

			// Fetch the Win32 performance counter's frequency, if available.
			{
				LARGE_INTEGER frequency;
				if (QueryPerformanceFrequency(&frequency))
				{
					sPerformanceCounterFrequency = (uint64_t)frequency.QuadPart;
				}
			}

			// Flag as initialized.
			sWasInitialized = TRUE;
		}

		// Fetch currrent time in microseconds, favoring Microsoft's high precision timer if available.
		if (sPerformanceCounterFrequency && QueryPerformanceCounter(&performanceCounterValue))
		{
			// Hardware supports a high precision timer.
			// Note: This value always increments up and is unaffected by system clock changes.

			// Convert performance counts to tenths of a millisecond.
			// This makes our timer accuracy/resolution 100 microseconds. (Remaining microsecond digits are stripped off.)
			// Note: We do this because converting performance counts to full microseconds here will cause 64-bit
			//       integer overflow after 3-4 months since system bootup. But if we use tens-of-milliseconds,
			//       we won't overflow until just over 20 years.
			uint64_t tensOfMilliseconds =
					((uint64_t)performanceCounterValue.QuadPart * 10000ULL) / sPerformanceCounterFrequency;

			// Convert the above tens-of-millseconds to microseconds.
			// The last 2 microsecond digits will always be zero.
			timeInMicroseconds = (Rtt_AbsoluteTime)(tensOfMilliseconds * 100ULL);
		}
		else
		{
			// Hardware does not support a high precision timer. Fallback to the next best thing.
			if (sGetTickCount64Callback)
			{
				// Use system ticks, which has an accuracy/resolution around 10-16 milliseconds.
				// Note: This value always increments up and is unaffected by system clock changes.
				timeInMicroseconds = (Rtt_AbsoluteTime)sGetTickCount64Callback() * 1000ULL;
			}
			else
			{
				// Use the system clock's current UTC time. (This is the API Corona used on Win32 in the past.)
				// This value has an accuracy of around 10-16 milliseconds, just like system ticks.
				// Note: Unfortunately, this value is affected by system clock changes except for daylight savings time.
				//       This means the value can jump forwards or backwards when a clock change happens.
				//       By default, Windows updates the system clock once a week.
				//       By default, VMWare updates a Windows virtual machine once a minute. (This is the worst.)
				SYSTEMTIME systemTime;
				FILETIME fileTime;
				ULARGE_INTEGER intConverter;
				GetSystemTime(&systemTime);
				SystemTimeToFileTime(&systemTime, &fileTime);
				intConverter.LowPart = fileTime.dwLowDateTime;
				intConverter.HighPart = fileTime.dwHighDateTime;
				timeInMicroseconds = (intConverter.QuadPart / 10ULL);
			}
		}
		return timeInMicroseconds;
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime )
	{
		return absoluteTime / 1000;
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime )
	{
		return absoluteTime;
	}

#else
	#include <sys/time.h>

	Rtt_EXPORT Rtt_AbsoluteTime
	Rtt_GetAbsoluteTime()
	{
		struct timeval tv;
		gettimeofday( &tv, NULL );

		// Convert to microseconds
		return (tv.tv_sec) * 1000000 + (tv.tv_usec);
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime )
	{
		return absoluteTime / 1000;
	}

	Rtt_EXPORT U64
	Rtt_AbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime )
	{
		return absoluteTime;
	}
#endif

// ----------------------------------------------------------------------------

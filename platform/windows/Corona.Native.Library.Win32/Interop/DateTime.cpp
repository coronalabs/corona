//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DateTime.h"


namespace Interop {

#pragma region Static Members
/// <summary>
///  <para>Fetches the system clock's current local time via the Win32 GetSystemTimePreciseAsFileTime() function.</para>
///  <para>The fetched value will be accurate to 1 millisecond, if available on the current OS version.</para>
/// </summary>
/// <param name="systemTimePointer">Pointer to a Win32 "SYSTEMTIME" struct to copy current date/time to.</param>
/// <returns>
///  <para>Returns true if a precise timestamp was successfully retrieved and copied to the given argument.</para>
///  <para>
///   Returns false if a higher precision timestamp was not copied to the given argument. This can happen if the
///   argument is null or if running on Windows 7 or older OS version.
///  </para>
/// </returns>
static bool CopyPreciseCurrentLocalSystemTimeTo(LPSYSTEMTIME systemTimePointer)
{
	typedef void(WINAPI *GetSystemTimePreciseAsFileTimeCallback)(LPFILETIME);
	static GetSystemTimePreciseAsFileTimeCallback sGetSystemTimePreciseAsFileTimeCallback;

	// Fetch a callback to the needed Win32 API, if not done already.
	// Note: This API is only available on Windows 8 and newer OS versions.
	static bool sWasInitialized;
	if (!sWasInitialized)
	{
		auto moduleHandle = ::LoadLibraryW(L"Kernel32");
		if (moduleHandle)
		{
			sGetSystemTimePreciseAsFileTimeCallback = (GetSystemTimePreciseAsFileTimeCallback)::GetProcAddress(
					moduleHandle, "GetSystemTimePreciseAsFileTime");
		}
		sWasInitialized = true;
	}

	// Fetch the current precise date time to the given argument.
	bool wasCopied = false;
	if (systemTimePointer && sGetSystemTimePreciseAsFileTimeCallback)
	{
		FILETIME utcFileTime{};
		SYSTEMTIME utcSystemTime{};
		sGetSystemTimePreciseAsFileTimeCallback(&utcFileTime);
		if (::FileTimeToSystemTime(&utcFileTime, &utcSystemTime))
		{
			if (::SystemTimeToTzSpecificLocalTime(nullptr, &utcSystemTime, systemTimePointer))
			{
				wasCopied = true;
			}
		}
	}
	return wasCopied;
}

#pragma endregion


#pragma region Constructors/Destructors
DateTime::DateTime()
{
	*this = DateTime::FromCurrentLocal();
}

DateTime::DateTime(const SYSTEMTIME& systemTime, bool isLocal)
:	fSystemTime(systemTime),
	fIsLocal(isLocal)
{
}

DateTime::~DateTime()
{
}

#pragma endregion


#pragma region Public Methods
bool DateTime::IsLocal() const
{
	return fIsLocal;
}

bool DateTime::IsUtc() const
{
	return !fIsLocal;
}

SYSTEMTIME DateTime::ToSystemTime() const
{
	return fSystemTime;
}

FILETIME DateTime::ToFileTime() const
{
	FILETIME fileTime{};
	SYSTEMTIME systemTime = fSystemTime;
	if (fIsLocal)
	{
		systemTime = this->ToUtc().ToSystemTime();
	}
	::SystemTimeToFileTime(&systemTime, &fileTime);
	if (fIsLocal)
	{
		FILETIME localFileTime{};
		::LocalFileTimeToFileTime(&fileTime, &localFileTime);
		fileTime = localFileTime;
	}
	return fileTime;
}

DateTime DateTime::ToLocal() const
{
	// If the date/time is already in local form, then return a copy of this object.
	if (fIsLocal)
	{
		return *this;
	}

	// Convert the date/time from UTC to local and return the result as a new DateTime object.
	SYSTEMTIME localSystemTime{};
	::SystemTimeToTzSpecificLocalTime(nullptr, &fSystemTime, &localSystemTime);
	return DateTime(localSystemTime, true);
}

DateTime DateTime::ToUtc() const
{
	// If the date/time is already in UTC form, then return a copy of this object.
	if (!fIsLocal)
	{
		return *this;
	}

	// Convert the date/time from local to UTC and return the result as a new DateTime object.
	SYSTEMTIME utcSystemTime{};
	::TzSpecificLocalTimeToSystemTime(nullptr, &fSystemTime, &utcSystemTime);
	return DateTime(utcSystemTime, false);
}

#pragma endregion


#pragma region Static Functions
DateTime DateTime::FromCurrentLocal()
{
	// First, attempt to fetch current time that is accurate to 1 millisecond.
	// Note: This will fail on Windows 7 and older OS versions.
	SYSTEMTIME systemTime{};
	bool wasSuccessful = CopyPreciseCurrentLocalSystemTimeTo(&systemTime);

	// If the above was not successful, then fetch a less precise current date/time value.
	if (!wasSuccessful)
	{
		::GetLocalTime(&systemTime);
	}

	// Return a new DateTime object initialized with the above timestamp.
	return DateTime(systemTime, true);
}

DateTime DateTime::FromCurrentUtc()
{
	// Attempt to fetch a current time that is accurate to 1 millisecond.
	SYSTEMTIME systemTime{};
	if (CopyPreciseCurrentLocalSystemTimeTo(&systemTime))
	{
		// Fetch was successful. Convert from local to UTC.
		SYSTEMTIME utcSystemTime{};
		::TzSpecificLocalTimeToSystemTime(nullptr, &systemTime, &utcSystemTime);
		systemTime = utcSystemTime;
	}
	else
	{
		// The above fetch was not successful. Fetch a less accurate system UTC time.
		::GetSystemTime(&systemTime);
	}

	// Return a new DateTime object initialized with the above timestamp.
	return DateTime(systemTime, false);
}

#pragma endregion

}	// namespace Interop

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_VersionTimestamp.h"

#include "Core/Rtt_Version.h"
#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

#if defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )
static time_t Win_mktimeUTC( struct tm *pTime );
#endif

Rtt_EXPORT time_t
Rtt_VersionTimestamp(void)
{
	struct tm t;
	t.tm_sec = 0;
	t.tm_min = 0;
	t.tm_hour = 0;
	t.tm_mday = Rtt_BUILD_DAY;
	t.tm_mon = Rtt_BUILD_MONTH - 1; // months are 0-based (jan = 0)
	t.tm_year = Rtt_BUILD_YEAR - 1900;
	t.tm_isdst = 0;
#if !defined( Rtt_WIN_ENV ) && !defined( Rtt_POWERVR_ENV ) && !defined( Rtt_NINTENDO_ENV )
	t.tm_gmtoff = 0;
	t.tm_zone = NULL;
#endif

#if defined( Rtt_ANDROID_ENV )
	// TODO: borked -- needs to be fixed for android
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return mktime( & t );
#elif defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )
	return Win_mktimeUTC(&t);
#elif defined( Rtt_NINTENDO_ENV )
	return mktime(&t);
#else
	return timegm( & t );
#endif
	
}


#if defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )
// Convert time from mktime to UTC on Windows
// Taken from last post on thread (with correction reversed!) at
// http://social.msdn.microsoft.com/Forums/en/vcgeneral/thread/e28359bc-4995-48b6-ba2b-8ab4d6eef395
// This is a hack - definitely test when DST ends!
static time_t Win_mktimeUTC( struct tm *pTime )
{
  int dstHours;
  long timezoneGeneralOffset;

  pTime->tm_isdst = -1;   // Just tell me if it's DST

  mktime(pTime);      // find out if it's DST or not

  _get_timezone(&timezoneGeneralOffset);   // get nominal timezone offset in seconds
 
  pTime->tm_sec -= timezoneGeneralOffset; 

  // if mktime told us it was DST, then subtract the DST hours.
  if (pTime->tm_isdst > 0)
  {
    _get_daylight(&dstHours);      // Get the general offset here
    pTime->tm_hour += dstHours;   // and subtract it
  }

  pTime->tm_isdst = -1;   // do not adjust for DST - documented as 0 but -1 is what works

  return mktime(pTime);

  // tmStruct now has the original date and time converted to UTC
}
#endif

// ----------------------------------------------------------------------------


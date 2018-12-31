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


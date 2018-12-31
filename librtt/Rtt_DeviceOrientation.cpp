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

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"

#include "Rtt_DeviceOrientation.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kUprightString[] = "portrait";
static const char kSidewaysLeftString[] = "landscapeLeft";
static const char kUpsideDownString[] = "portraitUpsideDown";
static const char kSidewaysRightString[] = "landscapeRight";
static const char kFaceUpString[] = "faceUp";
static const char kFaceDownString[] = "faceDown";
static const char kUnknownString[] = "unknown";
static const char kSidewaysString[] = "landscape";

const char*
DeviceOrientation::StringForType( Type type )
{
	const char* result = NULL;

	switch( type )
	{
		case kUpright:
			result = kUprightString;
			break;
		case kSidewaysLeft:
			result = kSidewaysLeftString;
			break;
		case kUpsideDown:
			result = kUpsideDownString;
			break;
		case kSidewaysRight:
			result = kSidewaysRightString;
			break;
		case kFaceUp:
			result = kFaceUpString;
			break;
		case kFaceDown:
			result = kFaceDownString;
			break;
		default:
			result = kUnknownString;
			break;
	}

	return result;
}

DeviceOrientation::Type
DeviceOrientation::TypeForString( const char *s )
{
	DeviceOrientation::Type result = kUnknown;

	if ( 0 == Rtt_StringCompareNoCase( s, kUprightString ) )
	{
		result = kUpright;
	}
	else if ( 0 == Rtt_StringCompareNoCase( s, kSidewaysLeftString ) )
	{
		result = kSidewaysLeft;
	}
	else if ( 0 == Rtt_StringCompareNoCase( s, kUpsideDownString ) )
	{
		result = kUpsideDown;
	}
	else if ( 0 == Rtt_StringCompareNoCase( s, kSidewaysString ) )
	{
		result = kSidewaysRight;
	}
	else if ( 0 == Rtt_StringCompareNoCase( s, kSidewaysRightString ) )
	{
		result = kSidewaysRight;
	}
	else if ( 0 == Rtt_StringCompareNoCase( s, kFaceUpString ) )
	{
		result = kFaceUp;
	}
	else if ( 0 == Rtt_StringCompareNoCase( s, kFaceDownString ) )
	{
		result = kFaceDown;
	}

	return result;
}

S32
DeviceOrientation::CalculateRotation( Type start, Type end )
{
	S32 result = 0;

	// Only if both orientations are in the same plane
	if ( ( start < kFaceUp && end < kFaceUp  )
		 && ( start > kUnknown && end > kUnknown ) )
	{
		S8 delta = (S8)end - (S8)start;
		bool isNegative = delta < 0;
		U8 magnitude = ( isNegative ? -delta : delta );
		switch( magnitude )
		{
			case 1:
			case 2:
				result = magnitude * 90;
				if ( isNegative ) { result = -result; }
				break;
			case 3:
				result = ( isNegative ? 90 : -90 );
				break;
		}
	}

	return result;
}

bool
DeviceOrientation::IsAngleUpsideDown( S32 angle )
{
	return ( Abs( angle ) - 180 ) == 0;
}

bool
DeviceOrientation::IsAngleUpright( S32 angle )
{
	return ( Abs( angle ) == 0 ) || ( Abs( angle ) - 360 == 0 );
}


DeviceOrientation::Type
DeviceOrientation::OrientationForAngle( S32 angle )
{
	Type result = kUpright;
	if ( 90 == angle || -270 == angle )
	{
		result = kSidewaysRight;
	}
	else if ( -90 == angle || 270 == angle )
	{
		result = kSidewaysLeft;
	}
	else if ( Abs( angle ) == 180 )
	{
		result = kUpsideDown;
	}
	return result;
}

S32
DeviceOrientation::AngleForOrientation( DeviceOrientation::Type orientation)
{
	S32 angle;
	switch ( orientation )
	{
		case kSidewaysRight:
			angle = 90;
			break;
		case kSidewaysLeft:
			angle = 270;
			break;
		case kUpsideDown:
			angle = 180;
			break;
			
		default:
			angle = 0;
			break;
	}
	return angle;
}
	
DeviceOrientation::Type
DeviceOrientation::GetRelativeOrientation( Type start, Type end )
{
	return OrientationForAngle( CalculateRotation( start, end ) );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


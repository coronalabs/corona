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

#ifndef _Rtt_DeviceOrientation_H__
#define _Rtt_DeviceOrientation_H__

#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class DeviceOrientation
{
	public:
		// DO NOT CHANGE THE ORDER OF THESE CONSTANTS
		typedef enum Type
		{
			kUnknown = 0,
			kUpright,			// bottom of device is at the bottom
			kSidewaysRight,		// bottom of device is to the right
			kUpsideDown,		// bottom of device is at the top
			kSidewaysLeft,		// bottom of device is to the left
			kFaceUp,
			kFaceDown,

			kNumTypes
		}
		Type;

	public:
		static const char* StringForType( Type type );
		static Type TypeForString( const char *s );

		static S32 CalculateRotation( Type start, Type end );

		static bool IsInterfaceOrientation( Type t ) { return t >= kUpright && t <= kSidewaysLeft; }
		static bool IsAndroidInterfaceOrientation( Type t ) { return t >= kUpright && t <= kSidewaysRight; }
		static bool IsUpright( Type t ) { return kUpright == t || kUpsideDown == t; }
		static bool IsSideways( Type t ) { return kSidewaysLeft == t || kSidewaysRight == t; }

		static bool IsAngleUpsideDown( S32 angle );

		static bool IsAngleUpright( S32 angle );
		static Type OrientationForAngle( S32 angle );
		static S32 AngleForOrientation( Type orientation );
		static Type GetRelativeOrientation( Type start, Type end );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DeviceOrientation_H__

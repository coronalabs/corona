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

#ifndef _Rtt_InputAxisType_H__
#define _Rtt_InputAxisType_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_AutoPtr.h"


namespace Rtt
{

/// Indicates the type of axis that input events will come from such as x-axis, y-axis, etc.
/// Provides unique IDs that are used by Corona events and by input descriptors.
class InputAxisType
{
	private:
		InputAxisType(S32 integerId, const char *stringId);

	public:
		InputAxisType();
		virtual ~InputAxisType();

		static const InputAxisType kUnknown;
		static const InputAxisType kX;
		static const InputAxisType kY;
		static const InputAxisType kZ;
		static const InputAxisType kRotationX;
		static const InputAxisType kRotationY;
		static const InputAxisType kRotationZ;
		static const InputAxisType kLeftX;
		static const InputAxisType kLeftY;
		static const InputAxisType kRightX;
		static const InputAxisType kRightY;
		static const InputAxisType kHatX;
		static const InputAxisType kHatY;
		static const InputAxisType kHatSwitch;
		static const InputAxisType kLeftTrigger;
		static const InputAxisType kRightTrigger;
		static const InputAxisType kGas;
		static const InputAxisType kBrake;
		static const InputAxisType kWheel;
		static const InputAxisType kRudder;
		static const InputAxisType kThrottle;
		static const InputAxisType kVerticalScroll;
		static const InputAxisType kHorizontalScroll;
		static const InputAxisType kOrientation;
		static const InputAxisType kHoverDistance;
		static const InputAxisType kHoverMajor;
		static const InputAxisType kHoverMinor;
		static const InputAxisType kTouchSize;
		static const InputAxisType kTouchMajor;
		static const InputAxisType kTouchMinor;
		static const InputAxisType kPressure;
		static const InputAxisType kTilt;
		static const InputAxisType kGeneric1;
		static const InputAxisType kGeneric2;
		static const InputAxisType kGeneric3;
		static const InputAxisType kGeneric4;
		static const InputAxisType kGeneric5;
		static const InputAxisType kGeneric6;
		static const InputAxisType kGeneric7;
		static const InputAxisType kGeneric8;
		static const InputAxisType kGeneric9;
		static const InputAxisType kGeneric10;
		static const InputAxisType kGeneric11;
		static const InputAxisType kGeneric12;
		static const InputAxisType kGeneric13;
		static const InputAxisType kGeneric14;
		static const InputAxisType kGeneric15;
		static const InputAxisType kGeneric16;
		static const InputAxisType kWhammyBar;

		S32 GetIntegerId() const;
		const char* GetStringId() const;
		bool Equals(const InputAxisType &value) const;
		bool NotEquals(const InputAxisType &value) const;
		bool operator==(const InputAxisType &value) const;
		bool operator!=(const InputAxisType &value) const;

		/// Object type that is returned by the InputAxisType::From() static functions.
		/// Indicates if conversion was successful, and if so, provides the axis type.
		class ConversionResult
		{
			public:
				ConversionResult();
				ConversionResult(Rtt_Allocator *allocatorPointer, const InputAxisType &type);
				bool HasSucceeded() const;
				bool HasFailed() const;
				const InputAxisType* GetAxisType() const;
			private:
				AutoPtr<InputAxisType> fAxisTypeReference;
		};

		static InputAxisType::ConversionResult FromIntegerId(Rtt_Allocator *allocatorPointer, S32 id);
		static InputAxisType::ConversionResult FromStringId(Rtt_Allocator *allocatorPointer, const char *stringId);

	private:
		/// Stores the axis type's unique integer ID.
		S32 fIntegerId;

		/// The unique string ID assigned to this axis type such as "x", "y", "leftTrigger", etc.
		/// <br>
		/// The string ID is typically used by a Lua event table.
		const char *fStringId;
};

} // namespace Rtt

#endif // _Rtt_InputAxisType_H__

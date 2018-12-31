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
#include "HorizontalAlignment.h"
#include <exception>
#include <list>
#include <string.h>


namespace Interop { namespace Graphics {

#pragma region Static Member Variables
typedef std::list<const HorizontalAlignment*> HorizontalAlignmentCollection;
static HorizontalAlignmentCollection sHorizontalAlignmentCollection;

const HorizontalAlignment HorizontalAlignment::kLeft("left", Gdiplus::StringAlignmentNear, ES_LEFT, DT_LEFT);
const HorizontalAlignment HorizontalAlignment::kCenter("center", Gdiplus::StringAlignmentCenter, ES_CENTER, DT_CENTER);
const HorizontalAlignment HorizontalAlignment::kRight("right", Gdiplus::StringAlignmentFar, ES_RIGHT, DT_RIGHT);

#pragma endregion


#pragma region Constructors/Destructors
HorizontalAlignment::HorizontalAlignment()
{
	throw std::exception();
}

HorizontalAlignment::HorizontalAlignment(
	const char* coronaStringId, Gdiplus::StringAlignment gdiPlusAlignmentId,
	LONG gdiEditControlStyle, UINT gdiDrawTextFormatId)
:	fCoronaStringId(coronaStringId),
	fGdiPlusAlignmentId(gdiPlusAlignmentId),
	fGdiEditControlStyle(gdiEditControlStyle),
	fGdiDrawTextFormatId(gdiDrawTextFormatId)
{
	// Add this alignment type to the global collection.
	sHorizontalAlignmentCollection.push_back(this);
}

HorizontalAlignment::~HorizontalAlignment()
{
}

#pragma endregion


#pragma region Public Methods
const char* HorizontalAlignment::GetCoronaStringId() const
{
	return fCoronaStringId;
}

Gdiplus::StringAlignment HorizontalAlignment::GetGdiPlusStringAlignmentId() const
{
	return fGdiPlusAlignmentId;
}

LONG HorizontalAlignment::GetGdiEditControlStyle() const
{
	return fGdiEditControlStyle;
}

UINT HorizontalAlignment::GetGdiDrawTextFormatId() const
{
	return fGdiDrawTextFormatId;
}

bool HorizontalAlignment::Equals(const HorizontalAlignment& value) const
{
	return (this == &value);
}

bool HorizontalAlignment::NotEquals(const HorizontalAlignment& value) const
{
	return (this != &value);
}

bool HorizontalAlignment::operator==(const HorizontalAlignment& value) const
{
	return this->Equals(value);
}

bool HorizontalAlignment::operator!=(const HorizontalAlignment& value) const
{
	return this->NotEquals(value);
}

#pragma endregion


#pragma region Public Static Functions
const HorizontalAlignment* HorizontalAlignment::FromCoronaStringId(const char* coronaStringId)
{
	if (coronaStringId && (coronaStringId[0] != '\0'))
	{
		for (auto&& alignmentPointer : sHorizontalAlignmentCollection)
		{
			if (strcmp(alignmentPointer->fCoronaStringId, coronaStringId) == 0)
			{
				return alignmentPointer;
			}
		}
	}
	return nullptr;
}

const HorizontalAlignment* HorizontalAlignment::FromGdiEditControl(HWND windowHandle)
{
	if (windowHandle && ::IsWindow(windowHandle))
	{
		auto styles = ::GetWindowLongPtrW(windowHandle, GWL_STYLE);
		return FromGdiEditControlStyles(styles);
	}
	return nullptr;
}

const HorizontalAlignment* HorizontalAlignment::FromGdiEditControlStyles(LONG value)
{
	auto maskedStyle = value & (ES_LEFT | ES_CENTER | ES_RIGHT);
	for (auto&& alignmentPointer : sHorizontalAlignmentCollection)
	{
		if (maskedStyle == alignmentPointer->fGdiEditControlStyle)
		{
			return alignmentPointer;
		}
	}
	return nullptr;
}

const HorizontalAlignment* HorizontalAlignment::From(Gdiplus::StringAlignment value)
{
	for (auto&& alignmentPointer : sHorizontalAlignmentCollection)
	{
		if (value == alignmentPointer->fGdiPlusAlignmentId)
		{
			return alignmentPointer;
		}
	}
	return nullptr;
}

#pragma endregion

} }	// namespace Interop::Graphics

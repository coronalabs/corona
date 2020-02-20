//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

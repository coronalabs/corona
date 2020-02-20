//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MFCVisualManagerWindowsCustom.h"


IMPLEMENT_DYNCREATE(MFCVisualManagerWindowsCustom, CMFCVisualManagerWindows)


#pragma region Constructors/Destructors
MFCVisualManagerWindowsCustom::MFCVisualManagerWindowsCustom(BOOL isTemporary)
:	CMFCVisualManagerWindows(isTemporary)
{
}

MFCVisualManagerWindowsCustom::~MFCVisualManagerWindowsCustom()
{
}

#pragma endregion


#pragma region Public Methods
void MFCVisualManagerWindowsCustom::OnDrawEditBorder(
	CDC* deviceContextPointer, CRect bounds, BOOL isDisabled,
	BOOL isHighlighted, CMFCToolBarEditBoxButton* buttonPointer)
{
	CMFCVisualManagerOfficeXP::OnDrawEditBorder(deviceContextPointer, bounds, isDisabled, isHighlighted, buttonPointer);
}

#pragma endregion

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MFCToolBarLabel.h"


IMPLEMENT_SERIAL(MFCToolBarLabel, CMFCToolBarButton, 1)


#pragma region Constructors/Destructors
MFCToolBarLabel::MFCToolBarLabel()
{
}

MFCToolBarLabel::MFCToolBarLabel(UINT id, LPCTSTR text)
:	MFCToolBarLabel(id, -1, text)
{
}

MFCToolBarLabel::MFCToolBarLabel(UINT id, int imageIndex, LPCTSTR text)
:	CMFCToolBarButton(id, imageIndex, text)
{
	m_bText = text ? TRUE : FALSE;
	m_bImage = (imageIndex >= 0) ? TRUE : FALSE;
}

MFCToolBarLabel::~MFCToolBarLabel()
{
}

#pragma endregion


#pragma region Public Methods
void MFCToolBarLabel::OnDraw(
	CDC* deviceContextPointer, const CRect& bounds, CMFCToolBarImages* imagesPointer,
	BOOL isHorizontal, BOOL isCustomizeModeEnabled, BOOL isHighlighted,
	BOOL isDrawBorderEnabled, BOOL isGrayDisabledButtonsEnabled)
{
	// Temporarily strip-off the disabled flag.
	UINT oldStyles = m_nStyle;
	m_nStyle &= ~(TBBS_DISABLED | TBBS_CHECKED | TBBS_PRESSED);

	// Have the base class draw the text and image.
	CMFCToolBarButton::OnDraw(
			deviceContextPointer, bounds, imagesPointer, isHorizontal,
			FALSE, FALSE, FALSE, FALSE);

	// Restore the previous style flags.
	m_nStyle = oldStyles;
}

#pragma endregion

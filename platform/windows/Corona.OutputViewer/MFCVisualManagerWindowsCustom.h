//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


/// <summary>
///  <para>
///   Customized version of the MFC "CMFCVisualManagerWindows" visual theme which display a Windows XP like UI
///   using the system's current color scheme.
///  </para>
///  <para>Modified to display a border around text boxes, such as the find/search field in this app's toolbar.</para>
/// </summary>
class MFCVisualManagerWindowsCustom : public CMFCVisualManagerWindows
{
	DECLARE_DYNCREATE(MFCVisualManagerWindowsCustom)

	public:
		MFCVisualManagerWindowsCustom(BOOL isTemporary = FALSE);
		virtual ~MFCVisualManagerWindowsCustom();

		virtual void OnDrawEditBorder(
					CDC* deviceContextPointer, CRect bounds, BOOL isDisabled,
					BOOL isHighlighted, CMFCToolBarEditBoxButton* buttonPointer) override;
};

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
///  <para>An unclickable label that can be added to an CMFCToolbar.</para>
///  <para>This label never shows a mouse highlight or a grey disabled color.</para>
/// </summary>
class MFCToolBarLabel : public CMFCToolBarButton
{
	DECLARE_SERIAL(MFCToolBarLabel)

	public:
		/// <summary>Creates a new toolbar label without text.</summary>
		MFCToolBarLabel();

		/// <summary>Creates a new toolbar label with the given ID and text.</summary>
		/// <param name="id">
		///  <para>Unique "command" ID to be assigned to the toolbar item.</para>
		///  <para>Set to zero to not assign it a unique ID.</para>
		/// </param>
		/// <param name="text">The text to be displayed in the toolbar label control. Can be null or empty.</param>
		MFCToolBarLabel(UINT id, LPCTSTR text = nullptr);

		/// <summary>Creates a new toolbar label with the given ID and text.</summary>
		/// <param name="id">
		///  <para>Unique "command" ID to be assigned to the toolbar item.</para>
		///  <para>Set to zero to not assign it a unique ID.</para>
		/// </param>
		/// <param name="imageIndex">
		///  <para>Zero based index to the image in the toolbar's image list to be displayed next to the text label.</para>
		///  <para>Set to -1 to not display an image.</para>
		/// </param>
		/// <param name="text">The text to be displayed in the toolbar label control. Can be null or empty.</param>
		MFCToolBarLabel(UINT id, int imageIndex, LPCTSTR text = nullptr);

		/// <summary>Destroys this object.</summary>
		virtual ~MFCToolBarLabel();

		/// <summary>Called when the MFC toolbar requests this label control to draw itself.</summary>
		/// <param name="deviceContextPointer">The device context to render to.</param>
		/// <param name="bounds">The label control's bounds within the toolbar.</param>
		/// <param name="imagesPointer">Pointer to the toolbar's image collection.</param>
		/// <param name="isHorizontal">Set true if the toolbar is horizontal. Set false if vertical.</param>
		/// <param name="isCustomizeModeEnabled">Set true if enabled for custom rendering.</param>
		/// <param name="isHighlighted">Set true if a mouse-over highlight should be rendered.</param>
		/// <param name="isDrawBorderEnabled">
		///  Set true to render a border around the toolbar item. Set false to not show a border.
		/// </param>
		/// <param name="isGrayDisabledButtonsEnabled">
		///  Set true to render the item as disabled. Set false to render it as enabled.
		/// </param>
		virtual void OnDraw(
				CDC* deviceContextPointer, const CRect& bounds, CMFCToolBarImages* imagesPointer,
				BOOL isHorizontal = TRUE, BOOL isCustomizeModeEnabled = FALSE, BOOL isHighlighted = FALSE,
				BOOL isDrawBorderEnabled = TRUE, BOOL isGrayDisabledButtonsEnabled = TRUE) override;
};

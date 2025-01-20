//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Interop\Graphics\Font.h"
#include "Interop\Event.h"
#include "Control.h"
#include "UITextChangedEventArgs.h"
#include <string>
#include <Windows.h>


#pragma region Forward Declarations
namespace Interop { namespace Graphics {
	class HorizontalAlignment;
} }

#pragma endregion


namespace Interop { namespace UI {

/// <summary>Creates a GDI "EDIT" control (aka: text box) used for text input.</summary>
class TextBox : public Control
{
	Rtt_CLASS_NO_COPIES(TextBox)

	public:
		#pragma region Public Event Types
		/// <summary>
		///  <para>Defines the "TextChanged" event type which is raised when text has been changed by the user.</para>
		///  <para>This event will not be raised if text was changed programmatically via the SetText() method.</para>
		/// </summary>
		typedef Event<TextBox&, const UITextChangedEventArgs&> TextChangedEvent;

		/// <summary>
		///  <para>Defines the "PressedEnterKey" event type which is raised when the enter key was pressed.</para>
		///  <para>Only applies to single line text boxes. Multiline text boxes will not provide this event.</para>
		///  <para>
		///   Call the event arguments' SetHandle() to prevent the enter key from being processed by the text box.
		///  </para>
		/// </summary>
		typedef Event<TextBox&, HandledEventArgs&> PressedEnterKeyEvent;

		#pragma endregion


		#pragma region CreationSettings Structure
		/// <summary>Settings needed to create a TextBox class instances.</summary>
		struct CreationSettings
		{
			/// <summary>Handle to the window or control that will be the parent for the new text box control.</summary>
			HWND ParentWindowHandle;

			/// <summary>The parent window/control's client coordinates to display the text box at upon creation.</summary>
			RECT Bounds;

			/// <summary>
			///  <para>Set true to create a single line text box.</para>
			///  <para>Set false to create a multiline text box.</para>
			/// </summary>
			bool IsSingleLine;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new text box.</summary>
		/// <param name="settings">Provides the settings needed to initialize the text box control.</param>
		TextBox(const CreationSettings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~TextBox();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets an object used to add or remove an event handler for the "TextChanged" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		TextChangedEvent::HandlerManager& GetTextChangedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "PressedEnterKey" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		PressedEnterKeyEvent::HandlerManager& GetPressedEnterKeyEventHandlers();

		/// <summary>Determines if this is a single line text box.</summary>
		/// <returns>Returns true if this is a single line text box. Returns false if multiline.</returns>
		bool IsSingleLine() const;

		/// <summary>Determines if this is a multiline text box.</summary>
		/// <returns>Returns true if this is a multiline text box. Returns false if single line.</returns>
		bool IsMultiline() const;

		/// <summary>Determines if this text box is set up to hide or show the entered in characters.</summary>
		/// <returns>
		///  <para>Returns true if secure and the entered characters will not be revealed. (aka: Password field.)</para>
		///  <para>Returns false if not secure and all entered characters are visible.</para>
		/// </returns>
		bool IsSecure() const;

		/// <summary>Enables or disables whether or not entered in characters will be revealed onscreen.</summary>
		/// <param name="value">
		///  <para>Set true to not show entered in characters. (aka: Password mode.)</para>
		///  <para>Set false to show all entered characters.</para>
		/// </param>
		void SetSecure(bool value);

		/// <summary>Determines if the text box is read-only.</summary>
		/// <returns>
		///  <para>Returns true if the text box is read-only, meaning that the user cannot enter in characters.</para>
		///  <para>Returns false if not read-only and the user can enter in characters.</para>
		/// </returns>
		bool IsReadOnly() const;

		/// <summary>
		///  <para>Enables or disables read-only mode.</para>
		///  <para>Note that you can still change the text programmatically while in read-only mode.</para>
		/// </summary>
		/// <param name="value">
		///  <para>Set true to enable read-only mode which prevents the user from entering in characters.</para>
		///  <para>Set false to allow the user to enter in characters in to the text box.</para>
		/// </param>
		void SetReadOnly(bool value);

		/// <summary>Determines if this text box is set up to accept numbers only.</summary>
		/// <returns>
		///  <para>
		///   Returns true if the user can only type in numbers (0-9) into the text box.
		///   This does not include decimal point or sign characters.
		///  </para>
		///  <para>Returns false if all characters are accepted, which is the default.</para>
		/// </returns>
		bool IsNumericOnly() const;

		/// <summary>
		///  <para>Enables or disables the text box to accept only numbers.</para>
		///  <para>
		///   When in numeric-only mode, it will only accept numbers 0-9. It will not accept the decimal point
		///   or sign characters.
		///  </para>
		/// </summary>
		/// <param name="value">
		///  <para>Set true to restrict characters to numbers only.</para>
		///  <para>Set false to accept all characters.</para>
		/// </param>
		void SetNumericOnly(bool value);

		/// <summary>Determines if this text box is set up to accept decimal numbers only.</summary>
		/// <returns>
		///  <para>
		///   Returns true if the user can only type in numbers (0-9) and "." into the text box.
		///  </para>
		///  <para>Returns false if all characters are accepted, which is the default.</para>
		/// </returns>
		bool IsDecimalNumericOnly() const;

		/// <summary>
		///  <para>Enables or disables the text box to accept only decimal numbers.</para>
		///  <para>
		///   When in decimal numeric-only mode, it will only accept numbers 0-9 and ".".
		///  </para>
		/// </summary>
		/// <param name="value">
		///  <para>Set true to restrict characters to numbers and "." only.</para>
		///  <para>Set false to accept all characters.</para>
		/// </param>
		void SetDecimalNumericOnly(bool value);

		/// <summary>Determines if this text box is set up to reject emoji characters.</summary>
		/// <returns>
		///  <para>
		///   Returns true if the user can not type emoji into the text box.
		///  </para>
		///  <para>Returns false if all characters are accepted, which is the default.</para>
		/// </returns>
		bool IsNoEmoji() const;

		/// <summary>
		///  <para>Enables or disables the text box to reject emoji characters</para>
		///  <para>
		///   When in no emoji mode, it will reject emoji characters.
		///  </para>
		/// </summary>
		/// <param name="value">
		///  <para>Set true to reject emoji characters.</para>
		///  <para>Set false to accept all characters.</para>
		/// </param>
		void SetNoEmoji(bool value);

		/// <summary>Replaces the text in the control with the given string.</summary>
		/// <param name="text">
		///  <para>The text to copy to the control.</para>
		///  <para>Can be null, which sets the control's text to an empty string.</para>
		/// </param>
		void SetText(const wchar_t* text);

		/// <summary>Fetches text box control's current text and copies it to the given string.</summary>
		/// <param name="text">The string to copy the control's text to.</param>
		/// <returns>
		///  <para>Returns true if successfully copied the control's text to the given string parameter.</para>
		///  <para>
		///   Returns false if failed to copy the text. This can happen if the native Win32 text box control
		///   has been destroyed externally.
		///  </para>
		/// </returns>
		bool CopyTextTo(std::wstring& text) const;

		/// <summary>
		///  Sets the grey placeholder text hint to be displayed when no text is currently in the control.
		/// </summary>
		/// <param name="text">
		///  <para>The grey placeholder hint to be displayed.</para>
		///  <para>Set to null or empty string to not show any placeholder text.</para>
		/// </param>
		void SetPlaceholderText(const wchar_t* text);

		/// <summary>
		///  Fetches the text box control's current placeholder hint that is displayed when no text is in the control.
		/// </summary>
		/// <param name="text">The string object to coy the control's placeholder text to.</param>
		/// <returns>
		///  <para>
		///   Returns true if successfully copied the control's placeholder text to the given string parameter.
		///   The string will be empty if the control is not set up to display placeholder text.
		///  </para>
		///  <para>
		///   Returns false if failed to copy the string. This can happen if the native Win32 text box control
		///   has been destroyed externally.
		///  </para>
		/// </returns>
		bool CopyPlaceholderTextTo(std::wstring& text) const;

		/// <summary>
		///  Determines if the text box is set up to display a grey placeholder hint when no text is in the control.
		/// </summary>
		/// <returns>
		///  <para>Returns true if a grey placeholder hint will be displayed when no text is in the control.</para>
		///  <para>Returns false if placeholder text will not be displayed when empty.</para>
		/// </returns>
		bool HasPlaceholderText() const;

		/// <summary>Fetches the text box's horizontal string alignment such as kLeft, KCenter, or kRight.</summary>
		/// <returns>Returns the text box's horizontal string alignment such as kLeft, KCenter, or kRight.</returns>
		const Interop::Graphics::HorizontalAlignment* GetAlignment() const;

		/// <summary>Sets the text box's horizontal string alignment.</summary>
		/// <param name="value">The horizontal string alignment to be used such as kLeft, kCenter, or kRight.</param>
		void SetAlignment(const Interop::Graphics::HorizontalAlignment& value);

		/// <summary>Changes the text box's cursor position and text selection.</summary>
		/// <param name="startCharacterIndex">Zero based index to the first character to highlight/select.</param>
		/// <param name="endCharacterIndex">
		///  <para>Zero based character to the last character in the text box to highlight/select.</para>
		///  <para>
		///   Set to the same index as argument "startCharacterIndex" to not highlight text and just move the cursor.
		///  </para>
		///  <para>Set to -1 to set the selection to the end of the string in the text box.</para>
		/// </param>
		void SetSelection(int startCharacterIndex, int endCharacterIndex);

		/// <summary>Gets the text box's current selection start and end character indices.</summary>
		/// <param name="outStartCharacterIndex">Pointer to an integer that will receive the start index of the selection.</param>
		/// <param name="outEndCharacterIndex">Pointer to an integer that will receive the end index of the selection.</param>
		/// <returns>
		///  <para>Returns true if the indices were successfully fetched.</para>
		///  <para>Returns false if failed to fetch the indices, in which case the output parameters won't be modified.</para>
		/// </returns>
		bool GetSelection(int *outStartCharacterIndex, int *outEndCharacterIndex) const;

		/// <summary>Sets the control's text color to the system default.</summary>
		void SetTextColorToDefault();

		/// <summary>Sets the control's text color to the given value.</summary>
		/// <param name="value">The color to be used by the control's text. (The alpha channel is ignored.)</param>
		void SetTextColor(COLORREF value);

		/// <summary>Gets the control's current text color.</summary>
		/// <returns>
		///  <para>Return the color used by the control's text.</para>
		///  <para>
		///   By default, this will the system's default text color unless you change it via the SetTextColor() method.
		///  </para>
		/// </returns>
		COLORREF GetTextColor() const;

		/// <summary>Sets the font to be used by the control's main text and placeholder hint text.</summary>
		/// <param name="font">The font to be used. Will be ignored if flagged invalid.</param>
		void SetFont(const Interop::Graphics::Font& font);

		/// <summary>Gets the font used by this control's main text and placeholder hint text.</summary>
		/// <returns>
		///  <para>Returns the font currently used by the control.</para>
		///  <para>Will return the system's default font unless you change it via the SetFont() method.</para>
		/// </returns>
		Interop::Graphics::Font GetFont() const;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Called when a Windows message has been dispatched to this control.</summary>
		/// <param name="sender">Reference to this control.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(UIComponent& sender, HandleMessageEventArgs& arguments);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Manages the "TextChanged" event.</summary>
		TextChangedEvent fTextChangedEvent;

		/// <summary>Manages the "PressedEnterKey" event.</summary>
		PressedEnterKeyEvent fPressedEnterKeyEvent;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		UIComponent::ReceivedMessageEvent::MethodHandler<TextBox> fReceivedMessageEventHandler;

		/// <summary>Font currently being used by this control</summary>
		Interop::Graphics::Font fFont;

		/// <summary>
		///  <para>Set true to use member variable "fCustomTextColor" for the text color.</para>
		///  <para>Set false to use the system's default text color.</para>
		/// </summary>
		bool fIsUsingCustomTextColor;

		/// <summary>
		///  <para>Text color to use to draw the control's text.</para>
		///  <para>Only applicable if member variable "fIsUsingCustomTextColor" is true.</para>
		/// </summary>
		COLORREF fCustomTextColor;

		/// <summary>
		///  <para>Stores placeholder text when the text box is in multiline mode.</para>
		///  <para>We need this because Win32 multiline text boxes do not support placeholder text.</para>
		/// </summary>
		std::wstring fMultilinePlaceholderText;

		/// <summary>
		///  Stores the last text entered. Used to detect a text change by the user in order to raise a "TextChange" event.
		/// </summary>
		std::wstring fLastUpdatedText;

		/// <summary>
		///  <para>Set to true if text field is number and decimal only</para>
		/// </summary>
		bool fIsDecimalNumericOnly;

		/// <summary>
		///  <para>Set to true if text field rejects emoji characters</para>
		/// </summary>
		bool fNoEmoji;

#pragma endregion
};

} }	// namespace Interop::UI

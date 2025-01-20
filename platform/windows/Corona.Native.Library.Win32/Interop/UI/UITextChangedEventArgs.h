//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\EventArgs.h"
#include "WinString.h"


namespace Interop { namespace UI {

/// <summary>Event arguments provide the text changes made in a UI control such as a text box.</summary>
class UITextChangedEventArgs : public Interop::EventArgs
{
	public:
		/// <summary>Settings needed to create a new UITextChangedEventArgs instance.</summary>
		struct Settings
		{
			/// <summary>The new text that the UI control is now using.</summary>
			const wchar_t* NewText;

			/// <summary>The previous text in the UI control before it was changed.</summary>
			const wchar_t* PreviousText;

			/// <summary>
			///  <para>The character start selection index before the text was changed.</para>
			///  <para>If no text was selected/highlighted, then this will be the cursor position.</para>
			/// </summary>
			int PreviousStartSelectionIndex;

			/// <summary>The number of characters added to the UI control.</summary>
			int AddedCharacterCount;

			/// <summary>The number of characters deleted from the UI control.</summary>
			int DeletedCharacterCount;
		};


		/// <summary>Creates a new event arguments object with the given settings.</summary>
		/// <param name="settings">Provides the new text, previous text, characters added, characters removed, etc.</param>
		UITextChangedEventArgs(const UITextChangedEventArgs::Settings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~UITextChangedEventArgs();


		/// <summary>Gets the new text entered into the UI control.</summary>
		/// <returns>Returns the newly entered text as a UTF-8 encoded string.</returns>
		const char* GetNewTextAsUtf8() const;

		/// <summary>Gets the new text entered into the UI control.</summary>
		/// <returns>Returns the newly entered text as a UTF-16 encoded string.</returns>
		const wchar_t* GetNewTextAsUtf16() const;

		/// <summary>Gets the previous text that was in the UI control before the text changed.</summary>
		/// <returns>Returns the previous text as a UTF-8 encoded string.</returns>
		const char* GetPreviousTextAsUtf8() const;

		/// <summary>Gets the previous text that was in the UI control before the text changed.</summary>
		/// <returns>Returns the previous text as a UTF-16 encoded string.</returns>
		const wchar_t* GetPreviousTextAsUtf16() const;

		/// <summary>
		///  <para>Gets the character start selection index before the text was changed.</para>
		///  <para>If no text was selected/highlighted, then this will be the cursor position.</para>
		/// </summary>
		/// <returns>Returns the character start selection index.</returns>
		int GetPreviousStartSelectionIndex() const;

		/// <summary>Gets the number of characters added to the UI control.</summary>
		/// <returns>Returns the number of characters added</returns>
		int GetAddedCharacterCount() const;

		/// <summary>Gets the number of characters deleted from the UI control.</summary>
		/// <returns>Returns the number of characters deleted.</returns>
		int GetDeletedCharacterCount() const;

	private:
		WinString fNewText;
		WinString fPreviousText;
		int fPreviousStartSelectionIndex;
		int fAddedCharacterCount;
		int fDeletedCharacterCount;
};

} }	// namespace Interop::UI

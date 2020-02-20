//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

/// <summary>Interface used to set up, display, and manage a message box.</summary>
public interface class IMessageBox
{
	#pragma region Events
	/// <summary>
	///  Raised when the message box has been closed by the end-user via a displayed button, back key,
	///  or if one of the Close() methods have been called.
	/// </summary>
	event Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ Closed;

	#pragma endregion


	#pragma region Methods/Properties
	/// <summary>Gets or sets the message box's optional title text.</summary>
	/// <remarks>
	///  You are expected to set this property before calling the <see cref="Show()"/> method.
	///  Any changes made to this property while the message box is shown will not be displayed until
	///  the next time the message box is shown.
	/// </remarks>
	/// <value>
	///  <para>The title text to be displayed at the top of the message box.</para>
	///  <para>Set to empty string to not display a title in the message box.</para>
	/// </value>
	property Platform::String^ Title;

	/// <summary>Gets or sets the main text to be displayed in the message box.</summary>
	/// <remarks>
	///  You are expected to set this property before calling the <see cref="Show()"/> method.
	///  Any changes made to this property while the message box is shown will not be displayed until
	///  the next time the message box is shown.
	/// </remarks>
	/// <value>
	///  <para>The main text to be displayed within the message box.</para>
	///  <para>Set to empty string to not display text within the message box.</para>
	/// </value>
	property Platform::String^ Message;

	/// <summary>Gets a modifiable collection of labels to be displayed as buttons in the message box.</summary>
	/// <remarks>
	///  You are expected to set this property before calling the <see cref="Show()"/> method.
	///  Any changes made to this property while the message box is shown will not be displayed until
	///  the next time the message box is shown.
	/// </remarks>
	/// <value>
	///  <para>Modifiable collection of button labels to be displayed as button in the message box.</para>
	///  <para>
	///   Can be empty, which means that the message box will not display any buttons and can only be closed
	///   by the end-user via the back key.
	///  </para>
	/// </value>
	property Windows::Foundation::Collections::IVector<Platform::String^>^ ButtonLabels
	{
		Windows::Foundation::Collections::IVector<Platform::String^>^ get();
	}

	/// <summary>Determines if the message box is currently being shown to the end-user.</summary>
	/// <value>
	///  <para>
	///   Set true if the message box is currently being shown via the <see cref="Show()"/> method.
	///   Note that this property will still be true if the message box is obscurred by any UI displayed on top of it.
	///  </para>
	///  <para>Set false if the message box has been closed or has not been shown yet.</para>
	/// </value>
	property bool IsShowing { bool get(); }

	/// <summary>Determines if the message box was closed via one of its displayed button.</summary>
	/// <value>
	///  <para>
	///   Set true if the message box was closed via a button press, in which case, property
	///   <see cref="PressedButtonIndex"/> will indicate which button was pressed. Note that this will be true
	///   if you call the <see cref="CloseWithButtonIndex"/> method as well.
	///  </para>
	///  <para>
	///   Set false if the message box was closed via the back key, if a Close() method was called,
	///   or if the message box hasn't been shown yet.
	///  </para>
	/// </value>
	property bool WasButtonPressed { bool get(); }

	/// <summary>
	///  <para>Indicates which button was pressed to close the message box.</para>
	///  <para>Note that this property is only applicable if the <see cref="WasButtonPressed"/> property is set true.</para>
	/// </summary>
	/// <value>
	///  <para>
	///   Zero based index of the button that was pressed to close the message box.
	///   This index matches a button label in the <see cref="ButtonLabels"/> collection.
	///  </para>
	///  <para>
	///   Set to -1 if a button was not pressed to close the message box, which is indicated by the
	///   <see cref="WasButtonPressed"/> property. In this case, the message box was closed via the back key
	///   or by one of the Close() methods.
	///  </para>
	/// </value>
	property int PressedButtonIndex { int get(); }

	/// <summary>Displays the message box using the assigned title, message, and button labels.</summary>
	/// <remarks>This method will do nothing if the message box is currently being shown.</remarks>
	void Show();

	/// <summary>
	///  <para>Closes the message box, if currently shown.</para>
	///  <para>The <see cref="WasButtonPressed"/> property will be set to false in this case.</para>
	/// </summary>
	void Close();

	/// <summary>Closes the message box, if currently shown, by simulating a button press.</summary>
	/// <param name="index">Zero based index of a button label in the <see cref="ButtonLabels"/> collection.</param>
	/// <returns>
	///  <para>Returns true if successfully closed the message box with the given button index.</para>
	///  <para>Returns false if the message box is not currently shown or if given an invalid index.</para>
	/// </returns>
	bool CloseWithButtonIndex(int index);

	#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI

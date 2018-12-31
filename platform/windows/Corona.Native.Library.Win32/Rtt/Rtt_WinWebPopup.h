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

#pragma once

#include "Core\Rtt_Build.h"
#include "Interop\UI\Control.h"
#include "Interop\UI\WebBrowser.h"
#include "Interop\Event.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_PlatformWebPopup.h"
#include "Rtt_MPlatform.h"
#include "WinString.h"


#pragma region Forward Declarations
extern "C"
{
	struct lua_State;
}

#pragma endregion


namespace Rtt
{

class WinWebPopup : public PlatformWebPopup
{
	Rtt_CLASS_NO_COPIES(WinWebPopup)

	public:
		typedef PlatformWebPopup Super;

		#pragma region Constructors/Destructors
		WinWebPopup(Interop::RuntimeEnvironment& environment);
		virtual ~WinWebPopup();

		#pragma endregion


		#pragma region Public Methods
		virtual void Show(const MPlatform& platform, const char* url) override;
		virtual bool Close() override;
		virtual void Reset() override;
		virtual void SetPostParams(const MKeyValueIterable& params) override;
		virtual int ValueForKey(lua_State *L, const char key[]) const override;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex) override;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Called when Corona's rendering surface has been resized.</summary>
		/// <param name="sender">The rendering surface that has raised this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnRenderSurfaceResized(Interop::UI::Control& sender, const Interop::EventArgs& arguments);

		/// <summary>Called just before the Corona runtime dispatches a Lua "enterFrame" event.</summary>
		/// <param name="sender">The RuntimeEnvironment instance that raised this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnRuntimeEnteringFrame(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);

		/// <summary>Called when the web browser control is about to load a URL.</summary>
		/// <param name="sender">The web browser control that has raised this event.</param>
		/// <param name="arguments">Provides the URL to be loaded and the ability to cancel it.</param>
		void OnNavigating(Interop::UI::WebBrowser& sender, Interop::UI::WebBrowserNavigatingEventArgs& arguments);

		/// <summary>Called when the web browser control has failed to load a URL.</summary>
		/// <param name="sender">The web browser control that has raised this event.</param>
		/// <param name="arguments">Provides error information about why it failed to load the URL.</param>
		void OnNavigationFailed(
				Interop::UI::WebBrowser& sender, const Interop::UI::WebBrowserNavigationFailedEventArgs& arguments);

		/// <summary>Called when a Windows message has been received by the message-only window.</summary>
		/// <param name="sender">Reference to the message-only window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments);

		/// <summary>
		///  <para>Delets all web browser controls stored in member variable "fClosedWebControlCollection".</para>
		///  <para>This method should only be called once it is safe to delete these controls.</para>
		/// </summary>
		void DeleteClosedWebControls();

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Reference to the Corona runtime environment that owns this object.</summary>
		Interop::RuntimeEnvironment& fEnvironment;

		/// <summary>Pointer to the Win32 Web Browser control this Corona popup object wraps.</summary>
		Interop::UI::WebBrowser* fWebBrowserPointer;

		/// <summary>
		///  <para>Stores a collection of web browser controls to be deleted by method DeleteClosedWebControls().</para>
		///  <para>
		///   This class' Close() method stores these controls to this collection instead of deleting them immediately.
		///   Deletion must be deferred in case Close() was called during a Lua "urlRequest" event. Avoids a crash.
		///  </para>
		/// </summary>
		std::vector<Interop::UI::WebBrowser*> fClosedWebControlCollection;

		/// <summary>
		///  <para>Stores the Corona sandboxed base directory type when loading a local HTML file.</para>
		///  <para>The URL given to this popup is expected to be a relative path to file in this case.</para>
		///  <para>Set to "kUnknownDir" if not assigned when showing a popup.</para>
		/// </summary>
		MPlatform::Directory fBaseDirectory;

		/// <summary>
		///  <para>Set to a base directory path loading local HTML files or a URL domain.</para>
		///  <para>
		///   If this is not an empty string, then the URL to be loaded is expected to be a relative path
		///   and will be appended to this base path.
		///  </para>
		/// </summary>
		WinString fBaseUrl;

		/// <summary>Set true if the popup's control needs to be resized during Corona's next render pass.</summary>
		bool fWasResizeRequested;

		/// <summary>
		///  <para>Unique windows message ID reserved for this handler via the runtime's message-only window.</para>
		///  <para>Used by the OnReceivedMessage() function to fetch the handler that has posted a message to itself.</para>
		/// </summary>
		UINT fReservedMessageId;

		/// <summary>Handler to be invoked when the "Resized" event has been raised by Corona's rendering surface.</summary>
		Interop::UI::Control::ResizedEvent::MethodHandler<WinWebPopup> fRenderSurfaceResizedEventHandler;

		/// <summary>Handler to be invoked when the "EnteringFrame" event has been raised by the Corona runtime.</summary>
		Interop::RuntimeEnvironment::EnteringFrameEvent::MethodHandler<WinWebPopup> fRuntimeEnteringFrameEventHandler;

		/// <summary>Handler to be invoked when the "Navigating" event has been raised by the browser.</summary>
		Interop::UI::WebBrowser::NavigatingEvent::MethodHandler<WinWebPopup> fNavigatingEventHandler;

		/// <summary>Handler to be invoked when the "NavigationFailed" event has been raised by the browser.</summary>
		Interop::UI::WebBrowser::NavigationFailedEvent::MethodHandler<WinWebPopup> fNavigationFailedEventHandler;

		/// <summary>
		///  Handler to be invoked when the "ReceivedMessage" event has been raised by the message-only window.
		/// </summary>
		Interop::UI::UIComponent::ReceivedMessageEvent::MethodHandler<WinWebPopup> fReceivedMessageEventHandler;

		#pragma endregion
};

} // namespace Rtt

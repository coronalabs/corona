//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Interop\UI\WebBrowser.h"
#include "Rtt_MPlatform.h"
#include "Rtt_WinDisplayObject.h"


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}
extern "C"
{
	struct lua_State;
}

#pragma endregion


namespace Rtt
{

class WinWebViewObject : public WinDisplayObject
{
	Rtt_CLASS_NO_COPIES(WinWebViewObject)

	public:
		typedef WinDisplayObject Super;

		#pragma region Constructors/Destructors
		WinWebViewObject(Interop::RuntimeEnvironment& environment, const Rect& bounds);
		virtual ~WinWebViewObject();

		#pragma endregion


		#pragma region Public Methods
		virtual bool Initialize() override;
		virtual Interop::UI::Control* GetControl() const override;
		virtual const LuaProxyVTable& ProxyVTable() const override;
		virtual int ValueForKey(lua_State *L, const char key[]) const override;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex) override;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Called when the web browser control is about to load a URL.</summary>
		/// <param name="sender">The web browser control that has raised this event.</param>
		/// <param name="arguments">Provides the URL to be loaded and the ability to cancel it.</param>
		void OnNavigating(Interop::UI::WebBrowser& sender, Interop::UI::WebBrowserNavigatingEventArgs& arguments);

		/// <summary>Called when the web browser control has finished loading a URL.</summary>
		/// <param name="sender">The web browser control that has raised this event.</param>
		/// <param name="arguments">Provides the URL that was loaded.</param>
		void OnNavigated(Interop::UI::WebBrowser& sender, const Interop::UI::WebBrowserNavigatedEventArgs& arguments);

		/// <summary>Called when the web browser control has failed to load a URL.</summary>
		/// <param name="sender">The web browser control that has raised this event.</param>
		/// <param name="arguments">Provides error information about why it failed to load the URL.</param>
		void OnNavigationFailed(
				Interop::UI::WebBrowser& sender, const Interop::UI::WebBrowserNavigationFailedEventArgs& arguments);

		#pragma endregion


		#pragma region Private Static Functions
		static int OnLoad(lua_State *L);
		static int OnRequest(lua_State *L);
		static int OnStop(lua_State *L);
		static int OnBack(lua_State *L);
		static int OnForward(lua_State *L);
		static int OnReload(lua_State *L);
		static int OnResize(lua_State *L);
		static int OnDeleteCookies(lua_State *L);
		static int InjectJS(lua_State *L );
		static int RegisterCallback(lua_State *L);
		static int On(lua_State *L);
		static int Send(lua_State *L);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Pointer to the Win32 Web Browser control this Corona display object wraps.</summary>
		Interop::UI::WebBrowser* fWebBrowserPointer;

		/// <summary>Handler to be invoked when the "Navigating" event has been raised by the browser.</summary>
		Interop::UI::WebBrowser::NavigatingEvent::MethodHandler<WinWebViewObject> fNavigatingEventHandler;

		/// <summary>Handler to be invoked when the "Navigated" event has been raised by the browser.</summary>
		Interop::UI::WebBrowser::NavigatedEvent::MethodHandler<WinWebViewObject> fNavigatedEventHandler;

		/// <summary>Handler to be invoked when the "NavigationFailed" event has been raised by the browser.</summary>
		Interop::UI::WebBrowser::NavigationFailedEvent::MethodHandler<WinWebViewObject> fNavigationFailedEventHandler;

		#pragma endregion
};

}	// namespace Rtt

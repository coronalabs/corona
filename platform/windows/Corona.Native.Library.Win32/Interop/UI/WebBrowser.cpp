//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WebBrowser.h"
#include "Core\Rtt_Assert.h"
#include "Interop\Storage\RegistryStoredPreferences.h"
#include "Interop\ApplicationServices.h"
#include <comutil.h>
#include <ExDispid.h>
#include <MsHTML.h>
#include <sstream>
#include <string>
#include <strsafe.h>
#include <stdlib.h>

// Microsoft Edge WebView2 SDK is provided by the "Microsoft.Web.WebView2" NuGet package.
// Guarded by __has_include so the file still compiles if the SDK is absent (in which case
// the IE-based ActiveX backend is always used).
#if defined(__has_include)
    #if __has_include(<WebView2.h>)
        #include <Windows.h>
		#include <winstring.h>
        #include <wrl.h>
        #include <wrl/event.h>
        #include <WebView2.h>
        #define Rtt_WIN_WEBVIEW2_ENABLED 1
    #endif
#endif

#ifndef Rtt_WIN_WEBVIEW2_ENABLED
    #define Rtt_WIN_WEBVIEW2_ENABLED 0
#endif


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
WebBrowser::WebBrowser(const WebBrowser::CreationSettings& settings)
:	Control(),
	fWebBrowserHandlerPointer(nullptr),
	fEdgeWebViewHandlerPointer(nullptr)
{
	// Store the Internet Explorer registy path, if given.
	if (settings.IEOverrideRegistryPath && (settings.IEOverrideRegistryPath[0] != L'\0'))
	{
		fIEOverrideRegistryPath = settings.IEOverrideRegistryPath;
	}

	// Create a control to be used as a container for the embedded web browser.
	// Both the IE ActiveX object and the Edge WebView2 controller parent themselves to this HWND.
	DWORD styles = WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	auto windowHandle = ::CreateWindowExW(
			WS_EX_CONTROLPARENT, L"BUTTON", L"", styles,
			settings.Bounds.left, settings.Bounds.top,
			settings.Bounds.right - settings.Bounds.left,
			settings.Bounds.bottom - settings.Bounds.top,
			settings.ParentWindowHandle, nullptr, ::GetModuleHandleW(nullptr), nullptr);
	if (windowHandle)
	{
		::BringWindowToTop(windowHandle);
	}

	// Store the window handle and start listening to its Windows message events.
	OnSetWindowHandle(windowHandle);

	// Prefer the Microsoft Edge WebView2 control when available on the system.
	// CreateAndAttachTo returns null if either the WebView2 SDK was not compiled in or
	// the Edge WebView2 Runtime is not installed; in that case fall back to the IE ActiveX control
	// to preserve the original behavior.
	fEdgeWebViewHandlerPointer = EdgeWebViewHandler::CreateAndAttachTo(this);
	if (!fEdgeWebViewHandlerPointer)
	{
		fWebBrowserHandlerPointer = MicrosoftWebBrowserHandler::CreateAndAttachTo(this);
	}
}

WebBrowser::~WebBrowser()
{
	// Detach the embedded Edge WebView2 control from this container, if active.
	// Done before window destruction so the controller can release its child window cleanly.
	if (fEdgeWebViewHandlerPointer)
	{
		fEdgeWebViewHandlerPointer->DetachFromControl();
		fEdgeWebViewHandlerPointer->Release();
		fEdgeWebViewHandlerPointer = nullptr;
	}

	// Detach the embedded ActiveX web browser from this control.
	if (fWebBrowserHandlerPointer)
	{
		fWebBrowserHandlerPointer->DetachFromControl();
		fWebBrowserHandlerPointer->Release();
		fWebBrowserHandlerPointer = nullptr;
	}

	// Fetch the control's window handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return;
	}

	// Detach the WndProc callback from the window.
	// Note: Must be done before destroying it.
	OnSetWindowHandle(nullptr);

	// Destroy the control.
	::DestroyWindow(windowHandle);
}

#pragma endregion


#pragma region Public Methods
WebBrowser::NavigatingEvent::HandlerManager& WebBrowser::GetNavigatingEventHandlers()
{
	return fNavigatingEvent.GetHandlerManager();
}

WebBrowser::NavigatedEvent::HandlerManager& WebBrowser::GetNavigatedEventHandlers()
{
	return fNavigatedEvent.GetHandlerManager();
}

WebBrowser::NavigationFailedEvent::HandlerManager& WebBrowser::GetNavigationFailedEventHandlers()
{
	return fNavigationFailedEvent.GetHandlerManager();
}

const wchar_t* WebBrowser::GetIEOverrideRegistryPath() const
{
	return fIEOverrideRegistryPath.empty() ? nullptr : fIEOverrideRegistryPath.c_str();
}

bool WebBrowser::CanNavigateBack()
{
	if (fEdgeWebViewHandlerPointer)
	{
		return fEdgeWebViewHandlerPointer->CanNavigateBack();
	}
	return fWebBrowserHandlerPointer ? fWebBrowserHandlerPointer->CanNavigateBack() : false;
}

bool WebBrowser::CanNavigateForward()
{
	if (fEdgeWebViewHandlerPointer)
	{
		return fEdgeWebViewHandlerPointer->CanNavigateForward();
	}
	return fWebBrowserHandlerPointer ? fWebBrowserHandlerPointer->CanNavigateForward() : false;
}

void WebBrowser::NavigateBack()
{
	if (fEdgeWebViewHandlerPointer)
	{
		fEdgeWebViewHandlerPointer->NavigateBack();
	}
	else if (fWebBrowserHandlerPointer)
	{
		fWebBrowserHandlerPointer->NavigateBack();
	}
}

void WebBrowser::NavigateForward()
{
	if (fEdgeWebViewHandlerPointer)
	{
		fEdgeWebViewHandlerPointer->NavigateForward();
	}
	else if (fWebBrowserHandlerPointer)
	{
		fWebBrowserHandlerPointer->NavigateForward();
	}
}

void WebBrowser::NavigateTo(const wchar_t* url)
{
	if (fEdgeWebViewHandlerPointer)
	{
		fEdgeWebViewHandlerPointer->NavigateTo(url);
	}
	else if (fWebBrowserHandlerPointer)
	{
		fWebBrowserHandlerPointer->NavigateTo(url);
	}
}

void WebBrowser::Reload()
{
	if (fEdgeWebViewHandlerPointer)
	{
		fEdgeWebViewHandlerPointer->Reload();
	}
	else if (fWebBrowserHandlerPointer)
	{
		fWebBrowserHandlerPointer->Reload();
	}
}

void WebBrowser::StopLoading()
{
	if (fEdgeWebViewHandlerPointer)
	{
		fEdgeWebViewHandlerPointer->StopLoading();
	}
	else if (fWebBrowserHandlerPointer)
	{
		fWebBrowserHandlerPointer->StopLoading();
	}
}

#pragma endregion


#pragma region MicrosoftWebBrowserHandler Methods
WebBrowser::MicrosoftWebBrowserHandler::MicrosoftWebBrowserHandler(WebBrowser* controlPointer)
:	fWebBrowserControlPointer(controlPointer),
	fWebBrowserOleObjectPointer(nullptr),
	fOleInPlaceObjectPointer(nullptr),
	fEventToken(0),
	fReferenceCount(1),
	fCanNavigateBack(false),
	fCanNavigateForward(false),
	fResizedEventHandler(this, &WebBrowser::MicrosoftWebBrowserHandler::OnResized)
{
	// Fetch the container control's window handle.
	HWND windowHandle = nullptr;
	if (fWebBrowserControlPointer)
	{
		windowHandle = fWebBrowserControlPointer->GetWindowHandle();
	}
	if (!windowHandle)
	{
		return;
	}

	// Set up the ActiveX object to use the newest version of Internet Explorer installed on the system.
	{
		// Set up our Windows registry reading object.
		Interop::Storage::RegistryStoredPreferences::CreationSettings settings{};
		settings.Wow64ViewType = Interop::Storage::RegistryStoredPreferences::Wow64ViewType::kDefault;
		settings.IsUsingForwardSlashAsPathSeparator = false;
		Interop::Storage::RegistryStoredPreferences registryStoredPreferences(settings);

		// Determine which version of Internet Explorer is installed on the system.
		U32 majorVersion = 0;
		auto readResult = registryStoredPreferences.Fetch("HKLM\\Software\\Microsoft\\Internet Explorer\\svcVersion");
		if (readResult.HasFailed())
		{
			readResult = registryStoredPreferences.Fetch("HKLM\\Software\\Microsoft\\Internet Explorer\\Version");
		}
		if (readResult.HasSucceeded())
		{
			auto valueResult = readResult.GetValue().ToUnsignedInt32();
			if (valueResult.HasSucceeded())
			{
				majorVersion = valueResult.GetValue();
			}
		}

		// Write to the registry enabling it to use the Internet Explorer version installed on the system.
		// Note: If we don't do this, then the ActiveX will use up to IE 7 instead, regardless of what's installed.
		//       We don't want this since IE 7 does not support HTML5. Which also means no HTML5 video either.
		if (majorVersion > 0)
		{
			majorVersion *= 1000;
			RttString keyName(
					L"HKCU\\Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION\\");
			keyName.Append(ApplicationServices::GetExeFileName());
			registryStoredPreferences.UpdateWith(Rtt::Preference(keyName.GetUTF8(), majorVersion));
		}
	}
	
	// Disable JavaScript error/debugging popups.
	// We an do this by writing to this app's own private IE override registry section.
	if (fWebBrowserControlPointer->fIEOverrideRegistryPath.empty() == false)
	{
		std::wstring baseRegistryPath;
		baseRegistryPath = L"HKCU\\";
		baseRegistryPath += fWebBrowserControlPointer->fIEOverrideRegistryPath;

		Interop::Storage::RegistryStoredPreferences::CreationSettings settings{};
		settings.BaseRegistryPath = baseRegistryPath.c_str();
		settings.Wow64ViewType = Interop::Storage::RegistryStoredPreferences::Wow64ViewType::kDefault;
		settings.IsUsingForwardSlashAsPathSeparator = false;
		Interop::Storage::RegistryStoredPreferences registryStoredPreferences(settings);
		registryStoredPreferences.UpdateWith(Rtt::Preference("Main\\Disable Script Debugger", "yes"));
		registryStoredPreferences.UpdateWith(Rtt::Preference("Main\\DisableScriptDebuggerIE", "yes"));
	}

	// Create the ActiveX web browser control and embed it into the given container control.
	::OleCreate(
			CLSID_WebBrowser, IID_IOleObject, OLERENDER_DRAW, nullptr,
			this, this, (void**)&fWebBrowserOleObjectPointer);
	if (!fWebBrowserOleObjectPointer)
	{
		return;
	}
	fWebBrowserOleObjectPointer->SetClientSite(this);
	::OleSetContainedObject(fWebBrowserOleObjectPointer, TRUE);
	auto bounds = fWebBrowserControlPointer->GetClientBounds();
	fWebBrowserOleObjectPointer->DoVerb(OLEIVERB_INPLACEACTIVATE, nullptr, this, -1, windowHandle, &bounds);
	{
		IWebBrowser2* browserPointer = nullptr;
		fWebBrowserOleObjectPointer->QueryInterface(&browserPointer);
		if (browserPointer)
		{
			browserPointer->put_RegisterAsDropTarget(VARIANT_FALSE);
			browserPointer->Release();
		}
	}

	// Add event handlers.
	fWebBrowserControlPointer->GetResizedEventHandlers().Add(&fResizedEventHandler);
	{
		IWebBrowser2* browserPointer = nullptr;
		fWebBrowserOleObjectPointer->QueryInterface(&browserPointer);
		if (browserPointer)
		{
			IConnectionPointContainer* connectionPointContainerPointer = nullptr;
			browserPointer->QueryInterface(&connectionPointContainerPointer);
			if (connectionPointContainerPointer)
			{
				IConnectionPoint* connectionPointPointer = nullptr;
				connectionPointContainerPointer->FindConnectionPoint(DIID_DWebBrowserEvents2, &connectionPointPointer);
				if (connectionPointPointer)
				{
					connectionPointPointer->Advise(static_cast<IDispatch*>(this), &fEventToken);
					connectionPointPointer->Release();
				}
				connectionPointContainerPointer->Release();
			}
			browserPointer->Release();
		}
	}
}

WebBrowser::MicrosoftWebBrowserHandler::~MicrosoftWebBrowserHandler()
{
	DetachFromControl();
}

WebBrowser::MicrosoftWebBrowserHandler* WebBrowser::MicrosoftWebBrowserHandler::CreateAndAttachTo(
	WebBrowser* controlPointer)
{
	if (!controlPointer)
	{
		return nullptr;
	}
	return new MicrosoftWebBrowserHandler(controlPointer);
}

void WebBrowser::MicrosoftWebBrowserHandler::DetachFromControl()
{
	// Remove event handlers.
	if (fWebBrowserControlPointer)
	{
		fWebBrowserControlPointer->GetResizedEventHandlers().Remove(&fResizedEventHandler);
	}
	if (fWebBrowserOleObjectPointer)
	{
		IWebBrowser2* browserPointer = nullptr;
		fWebBrowserOleObjectPointer->QueryInterface(&browserPointer);
		if (browserPointer)
		{
			IConnectionPointContainer* connectionPointContainerPointer = nullptr;
			browserPointer->QueryInterface(&connectionPointContainerPointer);
			if (connectionPointContainerPointer)
			{
				IConnectionPoint* connectionPointPointer = nullptr;
				connectionPointContainerPointer->FindConnectionPoint(DIID_DWebBrowserEvents2, &connectionPointPointer);
				if (connectionPointPointer)
				{
					connectionPointPointer->Unadvise(fEventToken);
					connectionPointPointer->Release();
					fEventToken = 0;
				}
				connectionPointContainerPointer->Release();
			}
			browserPointer->Release();
		}
	}

	// Null out our pointer to the container control.
	fWebBrowserControlPointer = nullptr;

	// Dispose the ActiveX web browser object.
	if (fOleInPlaceObjectPointer)
	{
		fOleInPlaceObjectPointer->Release();
		fOleInPlaceObjectPointer = nullptr;
	}
	if (fWebBrowserOleObjectPointer)
	{
		fWebBrowserOleObjectPointer->Close(OLECLOSE_NOSAVE);
		::OleSetContainedObject(fWebBrowserOleObjectPointer, FALSE);
		fWebBrowserOleObjectPointer->SetClientSite(nullptr);
		fWebBrowserOleObjectPointer->Release();
		fWebBrowserOleObjectPointer = nullptr;
	}

	// Flag that we can't navigate anymore.
	fCanNavigateBack = false;
	fCanNavigateForward = false;
}

bool WebBrowser::MicrosoftWebBrowserHandler::CanNavigateBack()
{
	return fCanNavigateBack;
}

bool WebBrowser::MicrosoftWebBrowserHandler::CanNavigateForward()
{
	return fCanNavigateForward;
}

void WebBrowser::MicrosoftWebBrowserHandler::NavigateBack()
{
	if (fWebBrowserOleObjectPointer)
	{
		IWebBrowser2* interfacePointer = nullptr;
		fWebBrowserOleObjectPointer->QueryInterface(&interfacePointer);
		if (interfacePointer)
		{
			auto result = interfacePointer->GoBack();
			interfacePointer->Release();
		}
	}
}

void WebBrowser::MicrosoftWebBrowserHandler::NavigateForward()
{
	if (fWebBrowserOleObjectPointer)
	{
		IWebBrowser2* interfacePointer = nullptr;
		fWebBrowserOleObjectPointer->QueryInterface(&interfacePointer);
		if (interfacePointer)
		{
			auto result = interfacePointer->GoForward();
			interfacePointer->Release();
		}
	}
}

void WebBrowser::MicrosoftWebBrowserHandler::NavigateTo(const wchar_t* url)
{
	// Validate,
	if (!fWebBrowserOleObjectPointer)
	{
		return;
	}
	if (!url || (L'\0' == url[0]))
	{
		return;
	}

	// Fetch an interface to the web browser ActiveX.
	IWebBrowser2* interfacePointer = nullptr;
	fWebBrowserOleObjectPointer->QueryInterface(&interfacePointer);
	if (!interfacePointer)
	{
		return;
	}

	// Load the given URL.
	BSTR oleUrl = ::SysAllocString(url);
	if (oleUrl)
	{
		VARIANT emptyVariant;
		::VariantInit(&emptyVariant);
		interfacePointer->Navigate(oleUrl, &emptyVariant, &emptyVariant, &emptyVariant, &emptyVariant);
		::SysFreeString(oleUrl);
	}
	interfacePointer->Release();
}

void WebBrowser::MicrosoftWebBrowserHandler::Reload()
{
	if (fWebBrowserOleObjectPointer)
	{
		IWebBrowser2* interfacePointer = nullptr;
		fWebBrowserOleObjectPointer->QueryInterface(&interfacePointer);
		if (interfacePointer)
		{
			auto result = interfacePointer->Refresh();
			interfacePointer->Release();
		}
	}
}

void WebBrowser::MicrosoftWebBrowserHandler::StopLoading()
{
	if (fWebBrowserOleObjectPointer)
	{
		IWebBrowser2* interfacePointer = nullptr;
		fWebBrowserOleObjectPointer->QueryInterface(&interfacePointer);
		if (interfacePointer)
		{
			auto result = interfacePointer->Stop();
			interfacePointer->Release();
		}
	}
}

void WebBrowser::MicrosoftWebBrowserHandler::OnResized(Interop::UI::Control& sender, const Interop::EventArgs& arguments)
{
	if (fWebBrowserControlPointer && fOleInPlaceObjectPointer)
	{
		auto bounds = fWebBrowserControlPointer->GetClientBounds();
		fOleInPlaceObjectPointer->SetObjectRects(&bounds, &bounds);
	}
}

bool WebBrowser::MicrosoftWebBrowserHandler::IsTopLevelBrowserDispatch(IDispatch* dispatchPointer) const
{
	// Check if the given web browser IDispatch object is for the main web page and not an <iframe>.
	// We know its the main page if it doesn't have a parent page.
	bool isTopLevel = false;
	if (dispatchPointer)
	{
		IWebBrowser2* browserPointer = nullptr;
		dispatchPointer->QueryInterface(&browserPointer);
		if (browserPointer)
		{
			IDispatch* parentDispatchPointer = nullptr;
			browserPointer->get_Parent(&parentDispatchPointer);
			if (!parentDispatchPointer || (parentDispatchPointer == dispatchPointer))
			{
				isTopLevel = true;
			}
			if (parentDispatchPointer)
			{
				parentDispatchPointer->Release();
			}
			browserPointer->Release();
		}
	}
	return isTopLevel;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::QueryInterface(
	REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	// Validate argument.
	if (!ppvObject)
	{
		return E_POINTER;
	}

	// If the given interface ID matches this object's interface, then pass the pointer back to the caller.
	// Note: Since this class uses multiple inheritance, we must use static_cast<> to return the right vtable.
	if (__uuidof(IOleClientSite) == riid)
	{
		*ppvObject = static_cast<IOleClientSite*>(this);
	}
	else if (__uuidof(IOleInPlaceSite) == riid)
	{
		*ppvObject = static_cast<IOleInPlaceSite*>(this);
	}
	else if (__uuidof(IOleWindow) == riid)
	{
		*ppvObject = static_cast<IOleWindow*>(this);
	}
	else if (__uuidof(IDocHostUIHandler2) == riid)
	{
		*ppvObject = static_cast<IDocHostUIHandler2*>(this);
	}
	else if (__uuidof(IDocHostUIHandler) == riid)
	{
		*ppvObject = static_cast<IDocHostUIHandler*>(this);
	}
	else if (__uuidof(IOleCommandTarget) == riid)
	{
		*ppvObject = static_cast<IOleCommandTarget*>(this);
	}
	else if (__uuidof(IDispatch) == riid)
	{
		*ppvObject = static_cast<IDispatch*>(this);
	}
	else if (__uuidof(IStorage) == riid)
	{
		*ppvObject = static_cast<IStorage*>(this);
	}
	else if (__uuidof(IUnknown) == riid)
	{
		*ppvObject = static_cast<IUnknown*>(static_cast<IOleClientSite*>(this));
	}
	else
	{
		return E_NOINTERFACE;
	}
	this->AddRef();
	return S_OK;
}

ULONG WebBrowser::MicrosoftWebBrowserHandler::AddRef(void)
{
	fReferenceCount++;
	return fReferenceCount;
}

ULONG WebBrowser::MicrosoftWebBrowserHandler::Release(void)
{
	// Decrement the reference count.
	fReferenceCount--;

	// Delete this object if this reference count is now zero.
	ULONG currentCount = fReferenceCount;
	if (currentCount <= 0)
	{
		delete this;
	}

	// Return the current reference count.
	return currentCount;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::SaveObject(void)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetMoniker(
	DWORD dwAssign, DWORD dwWhichMoniker, __RPC__deref_out_opt IMoniker **ppmk)
{
	if ((OLEGETMONIKER_ONLYIFTHERE == dwAssign) && (OLEWHICHMK_CONTAINER == dwWhichMoniker))
	{
		return E_FAIL;
	}
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetContainer(__RPC__deref_out_opt IOleContainer **ppContainer)
{
	return E_NOINTERFACE;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::ShowObject(void)
{
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnShowWindow(BOOL fShow)
{
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::RequestNewObjectLayout(void)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetWindow(__RPC__deref_out_opt HWND *phwnd)
{
	// Validate.
	if (!phwnd)
	{
		return E_POINTER;
	}

	// Fetch a handle to the control that's hosting the ActiveX web browser.
	HWND windowHandle = nullptr;
	if (fWebBrowserControlPointer)
	{
		windowHandle = fWebBrowserControlPointer->GetWindowHandle();
	}
	if (!windowHandle)
	{
		return E_FAIL;
	}

	// Return the requested window handle.
	*phwnd = windowHandle;
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::CanInPlaceActivate(void)
{
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnInPlaceActivate(void)
{
	// Validate.
	if (!fWebBrowserOleObjectPointer)
	{
		return E_FAIL;
	}

	// Release the last in-place object. (This should never happen.)
	if (fOleInPlaceObjectPointer)
	{
		fOleInPlaceObjectPointer->Release();
		fOleInPlaceObjectPointer = nullptr;
	}

	// Acquire a new in-place object used to access the embedded ActiveX object.
	::OleLockRunning(fWebBrowserOleObjectPointer, TRUE, FALSE);
	fWebBrowserOleObjectPointer->QueryInterface(&fOleInPlaceObjectPointer);
	if (!fOleInPlaceObjectPointer)
	{
		return E_UNEXPECTED;
	}

	// Resize the embedded ActiveX object to fill the container control.
	auto bounds = fWebBrowserControlPointer->GetClientBounds();
	fOleInPlaceObjectPointer->SetObjectRects(&bounds, &bounds);
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnUIActivate(void)
{
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetWindowContext(
	__RPC__deref_out_opt IOleInPlaceFrame **ppFrame, __RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
	__RPC__out LPRECT lprcPosRect, __RPC__out LPRECT lprcClipRect, __RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	if (ppFrame)
	{
		*ppFrame = nullptr;
	}
	if (ppDoc)
	{
		*ppDoc = nullptr;
	}
	if (fWebBrowserControlPointer)
	{
		auto bounds = fWebBrowserControlPointer->GetClientBounds();
		if (lprcPosRect)
		{
			*lprcPosRect = bounds;
		}
		if (lprcClipRect)
		{
			*lprcClipRect = bounds;
		}
		if (lpFrameInfo)
		{
			lpFrameInfo->hwndFrame = fWebBrowserControlPointer->GetWindowHandle();
			lpFrameInfo->fMDIApp = FALSE;
			lpFrameInfo->haccel = nullptr;
			lpFrameInfo->cAccelEntries = 0;
		}
	}
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::Scroll(SIZE scrollExtant)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnUIDeactivate(BOOL fUndoable)
{
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnInPlaceDeactivate(void)
{
	if (fOleInPlaceObjectPointer)
	{
		fOleInPlaceObjectPointer->Release();
		fOleInPlaceObjectPointer = nullptr;
	}
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::DiscardUndoState(void)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnPosRectChange(__RPC__in LPCRECT lprcPosRect)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::ShowContextMenu(
	DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::ShowUI(
	DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
	IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::HideUI(void)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::UpdateUI(void)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnDocWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OnFrameWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::ResizeBorder(
	LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::TranslateAccelerator(
	LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetOptionKeyPath(__out LPOLESTR *pchKey, DWORD dw)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetExternal(IDispatch **ppDispatch)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::TranslateUrl(
	DWORD dwTranslate, __in __nullterminated  OLECHAR *pchURLIn, __out OLECHAR **ppchURLOut)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::FilterDataObject(IDataObject *pDO, IDataObject **ppDORet)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetOverrideKeyPath(__deref_out LPOLESTR *pchKey, DWORD dw)
{
	// Do not continue if this COM interface has been detached from the control
	if (!fWebBrowserControlPointer)
	{
		return E_NOTIMPL;
	}

	// Do not continue if the owner of this browser control has not supplied a registry path.
	// In this case, we will not override Internet Explorer's default behaviors/settings.
	if (fWebBrowserControlPointer->fIEOverrideRegistryPath.empty())
	{
		return E_NOTIMPL;
	}

	// Validate argument.
	if (!pchKey)
	{
		return E_INVALIDARG;
	}

	// Copy the registry path to the given argument.
	size_t oleStringByteLength = 0;
	auto utf16RegistryPath = fWebBrowserControlPointer->fIEOverrideRegistryPath.c_str();
	::StringCbLengthW(utf16RegistryPath, STRSAFE_MAX_CCH * sizeof(wchar_t), &oleStringByteLength);
	if (oleStringByteLength <= 0)
	{
		return E_FAIL;
	}
	oleStringByteLength += sizeof(wchar_t);
	*pchKey = (LPOLESTR)::CoTaskMemAlloc(oleStringByteLength);
	if (!*pchKey)
	{
		return E_NOTIMPL;
	}
	return ::StringCbCopyW(*pchKey, oleStringByteLength, utf16RegistryPath);
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::QueryStatus(
	__RPC__in_opt const GUID *pguidCmdGroup, ULONG cCmds,
	__RPC__inout_ecount_full(cCmds) OLECMD prgCmds[],
	__RPC__inout_opt OLECMDTEXT *pCmdText)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::Exec(
	__RPC__in_opt const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
	__RPC__in_opt VARIANT *pvaIn, __RPC__inout_opt VARIANT *pvaOut)
{
	// If a JavaScript error has occurred, then don't let the browser display an error dialog.
	// We do this by returning S_OK to ignore all JavaScript errors.
	// Note: IE JavaScript errors are unfortunately very common with a lot of website on the Internet. (It's annoying.)
	if (pguidCmdGroup && ::IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
	{
		if (OLECMDID_SHOWSCRIPTERROR == nCmdID)
		{
			return S_OK;
		}
	}

	// Let the ActiveX web brower do its default handling.
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetTypeInfoCount(__RPC__out UINT *pctinfo)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetTypeInfo(
	UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::GetIDsOfNames(
	__RPC__in REFIID riid, __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
	__RPC__in_range(0, 16384) UINT cNames, LCID lcid,
	__RPC__out_ecount_full(cNames) DISPID *rgDispId)
{
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::Invoke(
	DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
	VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	// Handle the web browser's event.
	// Note: The "pDispParams->rgvarg[]" array stores event arguments in reverse order.
	//       For example, "pDispParams->rgvarg[0]" is the last argument.
	switch (dispIdMember)
	{
		case DISPID_BEFORENAVIGATE2:
		{
			// Notify the owner of the web browser control that we're about to load the main web page.
			// Note: This case statement is hit for each <iframe> in the web page as well.
			//       Ignore them by checking if they're not top-level frames via IsTopLevelBrowserDispatch() below.
			if (fWebBrowserControlPointer && pDispParams && (pDispParams->cArgs >= 7))
			{
				const wchar_t* url = V_BSTR(pDispParams->rgvarg[5].pvarVal);
				if (url && (url[0] != L'\0') && IsTopLevelBrowserDispatch(pDispParams->rgvarg[6].pdispVal))
				{
					WebBrowserNavigatingEventArgs eventArgs(url);
					fWebBrowserControlPointer->fNavigatingEvent.Raise(*fWebBrowserControlPointer, eventArgs);
					pDispParams->rgvarg[0].pvarVal->boolVal = eventArgs.WasCanceled() ? VARIANT_TRUE : VARIANT_FALSE;
					return S_OK;
				}
			}
			break;
		}
		case DISPID_NAVIGATECOMPLETE2:
		{
			// Notify the owner of the web browser that we've finished loading a web page.
			// Note: This case statement is hit for each <iframe> in the web page as well.
			//       Ignore them by checking if they're not top-level frames via IsTopLevelBrowserDispatch() below.
			if (fWebBrowserControlPointer && pDispParams && (pDispParams->cArgs >= 2))
			{
				const wchar_t* url = V_BSTR(pDispParams->rgvarg[0].pvarVal);
				if (url && (url[0] != L'\0') && IsTopLevelBrowserDispatch(pDispParams->rgvarg[1].pdispVal))
				{
					WebBrowserNavigatedEventArgs eventArgs(url);
					fWebBrowserControlPointer->fNavigatedEvent.Raise(*fWebBrowserControlPointer, eventArgs);
				}
			}
			break;
		}
		case DISPID_NAVIGATEERROR:
		{
			// Notify the owner of the web browser that we've failed to load a web page.
			if (fWebBrowserControlPointer && pDispParams && (pDispParams->cArgs >= 5))
			{
				const wchar_t* url = V_BSTR(pDispParams->rgvarg[3].pvarVal);
				if (url && (url[0] != L'\0'))
				{
					// Acquire the error code.
					int errorCode = V_I4(pDispParams->rgvarg[1].pvarVal);

					// Create an error message from the above error code.
					std::wstringstream stringStream;
					if (errorCode >= 100)
					{
						stringStream << L"Received HTTP Status Code: ";
						stringStream << errorCode;
					}
					else if (errorCode < 0)
					{
						LPWSTR utf16Buffer = nullptr;
						::FormatMessageW(
								FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
								::GetModuleHandleW(L"wininet.dll"), (DWORD)errorCode,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&utf16Buffer, 0, nullptr);
						if (utf16Buffer)
						{
							stringStream << utf16Buffer;
							::LocalFree(utf16Buffer);
						}
						else
						{
							::FormatMessageW(
									FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
									0, (DWORD)errorCode,
									MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&utf16Buffer, 0, nullptr);
							if (utf16Buffer)
							{
								stringStream << utf16Buffer;
								::LocalFree(utf16Buffer);
							}
						}
					}
					std::wstring errorMessage = stringStream.str();
					if (errorMessage.empty())
					{
						stringStream << L"Received unknown IWebBrowser2 error code: ";
						stringStream << errorCode;
						errorMessage = stringStream.str();
					}

					// Raise the failure events.
					WebBrowserNavigationFailedEventArgs eventArgs(url, errorCode, errorMessage.c_str());
					fWebBrowserControlPointer->fNavigationFailedEvent.Raise(*fWebBrowserControlPointer, eventArgs);
				}
			}
			break;
		}
		case DISPID_COMMANDSTATECHANGE:
		{
			// The enable/disable state of the web browser's hidden toolbar buttons has changed.
			// We can use this to determine if we can navigate back/forward.
			if (pDispParams && (2 == pDispParams->cArgs))
			{
				bool isEnabled = pDispParams->rgvarg[0].boolVal ? true : false;
				const auto state = (CommandStateChangeConstants)pDispParams->rgvarg[1].lVal;
				if (state == CSC_NAVIGATEBACK)
				{
					fCanNavigateBack = isEnabled;
				}
				else if (state == CSC_NAVIGATEFORWARD)
				{
					fCanNavigateForward = isEnabled;
				}
			}
			break;
		}
	}
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::CreateStream(
	__RPC__in_string const OLECHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2,
	__RPC__deref_out_opt IStream **ppstm)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OpenStream(
	const OLECHAR *pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::CreateStorage(
	__RPC__in_string const OLECHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2,
	__RPC__deref_out_opt IStorage **ppstg)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::OpenStorage(
	__RPC__in_opt_string const OLECHAR *pwcsName, __RPC__in_opt IStorage *pstgPriority,
	DWORD grfMode, __RPC__deref_opt_in_opt SNB snbExclude, DWORD reserved, __RPC__deref_out_opt IStorage **ppstg)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::CopyTo(
	DWORD ciidExclude, const IID *rgiidExclude, __RPC__in_opt  SNB snbExclude, IStorage *pstgDest)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::MoveElementTo(
	__RPC__in_string const OLECHAR *pwcsName, __RPC__in_opt IStorage *pstgDest,
	__RPC__in_string const OLECHAR *pwcsNewName, DWORD grfFlags)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::Revert(void)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::EnumElements(
	DWORD reserved1, void *reserved2, DWORD reserved3, IEnumSTATSTG **ppenum)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::DestroyElement(__RPC__in_string const OLECHAR *pwcsName)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::RenameElement(
	__RPC__in_string const OLECHAR *pwcsOldName, __RPC__in_string const OLECHAR *pwcsNewName)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::SetElementTimes(
	__RPC__in_opt_string const OLECHAR *pwcsName, __RPC__in_opt const FILETIME *pctime,
	__RPC__in_opt const FILETIME *patime, __RPC__in_opt const FILETIME *pmtime)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::SetClass(__RPC__in REFCLSID clsid)
{
	return S_OK;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
	return E_NOTIMPL;
}

HRESULT WebBrowser::MicrosoftWebBrowserHandler::Stat(__RPC__out STATSTG *pstatstg, DWORD grfStatFlag)
{
	return E_NOTIMPL;
}

#pragma endregion


#pragma region EdgeWebViewHandler Methods
#if Rtt_WIN_WEBVIEW2_ENABLED

WebBrowser::EdgeWebViewHandler::EdgeWebViewHandler(WebBrowser* controlPointer)
:	fWebBrowserControlPointer(controlPointer),
	fEnvironmentPointer(nullptr),
	fControllerPointer(nullptr),
	fWebViewPointer(nullptr),
	fNavigationStartingToken{},
	fNavigationCompletedToken{},
	fSourceChangedToken{},
	fHistoryChangedToken{},
	fReferenceCount(1),
	fIsDetached(false),
	fIsReady(false),
	fHasPendingUrl(false),
	fCanNavigateBack(false),
	fCanNavigateForward(false),
	fResizedEventHandler(this, &EdgeWebViewHandler::OnResized)
{
}

WebBrowser::EdgeWebViewHandler::~EdgeWebViewHandler()
{
	DetachFromControl();
}

bool WebBrowser::EdgeWebViewHandler::IsRuntimeAvailable()
{
	// Returns success only when the Edge WebView2 Runtime is installed on the host machine.
	// We probe synchronously here so the caller can fall back to the IE backend without spinning
	// up any async work.
	LPWSTR versionString = nullptr;
	HRESULT hr = ::GetAvailableCoreWebView2BrowserVersionString(nullptr, &versionString);
	bool available = SUCCEEDED(hr) && (versionString != nullptr) && (versionString[0] != L'\0');
	if (versionString)
	{
		::CoTaskMemFree(versionString);
	}
	return available;
}

WebBrowser::EdgeWebViewHandler* WebBrowser::EdgeWebViewHandler::CreateAndAttachTo(WebBrowser* controlPointer)
{
	if (!controlPointer || !controlPointer->GetWindowHandle())
	{
		return nullptr;
	}
	if (!IsRuntimeAvailable())
	{
		return nullptr;
	}

	auto handler = new EdgeWebViewHandler(controlPointer);
	if (!handler->BeginAsyncCreation())
	{
		// Synchronous failure starting WebView2 environment creation. Tear down and let the caller
		// fall back to the IE ActiveX backend so the WebView still works (just on the legacy stack).
		handler->fIsDetached = true;
		handler->Release();
		return nullptr;
	}

	// Listen for container resizes so we can keep the WebView2 controller's bounds in sync.
	controlPointer->GetResizedEventHandlers().Add(&handler->fResizedEventHandler);
	return handler;
}

bool WebBrowser::EdgeWebViewHandler::BeginAsyncCreation()
{
	// WebView2 requires a writable user-data folder. Use a per-app subfolder under %LOCALAPPDATA%.
	// Falls back to %TEMP% in unusual environments where LOCALAPPDATA is unset.
	wchar_t userDataFolder[MAX_PATH] = { 0 };
	wchar_t baseFolder[MAX_PATH] = { 0 };
	DWORD len = ::GetEnvironmentVariableW(L"LOCALAPPDATA", baseFolder, MAX_PATH);
	if ((len == 0) || (len >= MAX_PATH))
	{
		len = ::GetEnvironmentVariableW(L"TEMP", baseFolder, MAX_PATH);
	}
	if ((len > 0) && (len < MAX_PATH))
	{
		const wchar_t* exeName = ApplicationServices::GetExeFileName();
		if (!exeName || (exeName[0] == L'\0'))
		{
			exeName = L"Corona";
		}
		::StringCchPrintfW(userDataFolder, MAX_PATH, L"%s\\%s\\WebView2", baseFolder, exeName);
	}
	else
	{
		::StringCchCopyW(userDataFolder, MAX_PATH, L"WebView2Data");
	}

	// Capture self via a ComPtr so the handler stays alive until the async completion fires.
	Microsoft::WRL::ComPtr<EdgeWebViewHandler> selfRef(this);
	auto envHandler = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
		[selfRef](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
		{
			return selfRef->OnEnvironmentCreated(result, env);
		});
	if (!envHandler)
	{
		return false;
	}

	HRESULT hr = ::CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder, nullptr, envHandler.Get());
	return SUCCEEDED(hr);
}

HRESULT WebBrowser::EdgeWebViewHandler::OnEnvironmentCreated(
	HRESULT result, ICoreWebView2Environment* environmentPointer)
{
	// If the host has been torn down before this async callback fired, do nothing.
	// The handler is still alive (held by the captured ComPtr) but the WebBrowser is gone.
	if (fIsDetached || !fWebBrowserControlPointer)
	{
		return S_OK;
	}
	if (FAILED(result) || !environmentPointer)
	{
		return S_OK;
	}

	fEnvironmentPointer = environmentPointer;
	fEnvironmentPointer->AddRef();

	HWND parentHwnd = fWebBrowserControlPointer->GetWindowHandle();
	if (!parentHwnd)
	{
		return S_OK;
	}

	Microsoft::WRL::ComPtr<EdgeWebViewHandler> selfRef(this);
	auto controllerHandler = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
		[selfRef](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
		{
			return selfRef->OnControllerCreated(result, controller);
		});
	if (!controllerHandler)
	{
		return E_FAIL;
	}

	return fEnvironmentPointer->CreateCoreWebView2Controller(parentHwnd, controllerHandler.Get());
}

HRESULT WebBrowser::EdgeWebViewHandler::OnControllerCreated(
	HRESULT result, ICoreWebView2Controller* controllerPointer)
{
	if (fIsDetached || !fWebBrowserControlPointer)
	{
		return S_OK;
	}
	if (FAILED(result) || !controllerPointer)
	{
		return S_OK;
	}

	fControllerPointer = controllerPointer;
	fControllerPointer->AddRef();

	HRESULT hr = fControllerPointer->get_CoreWebView2(&fWebViewPointer);
	if (FAILED(hr) || !fWebViewPointer)
	{
		return S_OK;
	}

	// Apply default settings comparable to what we configured on the IE ActiveX backend.
	{
		ICoreWebView2Settings* settingsPointer = nullptr;
		if (SUCCEEDED(fWebViewPointer->get_Settings(&settingsPointer)) && settingsPointer)
		{
			settingsPointer->put_IsScriptEnabled(TRUE);
			settingsPointer->put_AreDefaultScriptDialogsEnabled(TRUE);
			settingsPointer->put_AreDefaultContextMenusEnabled(TRUE);
			settingsPointer->put_AreDevToolsEnabled(FALSE);
			settingsPointer->put_IsStatusBarEnabled(FALSE);
			settingsPointer->put_IsZoomControlEnabled(TRUE);
			settingsPointer->Release();
		}
	}

	UpdateBounds();

	// NavigationStarting → maps to WebBrowser::NavigatingEvent (cancellable).
	{
		Microsoft::WRL::ComPtr<EdgeWebViewHandler> selfRef(this);
		auto navStartHandler = Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
			[selfRef](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT
			{
				if (selfRef->fIsDetached || !selfRef->fWebBrowserControlPointer || !args)
				{
					return S_OK;
				}
				LPWSTR uri = nullptr;
				if (FAILED(args->get_Uri(&uri)) || !uri)
				{
					return S_OK;
				}
				WebBrowserNavigatingEventArgs eventArgs(uri);
				selfRef->fWebBrowserControlPointer->fNavigatingEvent.Raise(
						*selfRef->fWebBrowserControlPointer, eventArgs);
				if (eventArgs.WasCanceled())
				{
					args->put_Cancel(TRUE);
				}
				::CoTaskMemFree(uri);
				return S_OK;
			});
		if (navStartHandler)
		{
			fWebViewPointer->add_NavigationStarting(navStartHandler.Get(), &fNavigationStartingToken);
		}
	}

	// NavigationCompleted → maps to NavigatedEvent (success) or NavigationFailedEvent (failure).
	{
		Microsoft::WRL::ComPtr<EdgeWebViewHandler> selfRef(this);
		auto navCompleteHandler = Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
			[selfRef](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
			{
				if (selfRef->fIsDetached || !selfRef->fWebBrowserControlPointer || !args || !sender)
				{
					return S_OK;
				}
				BOOL isSuccess = FALSE;
				args->get_IsSuccess(&isSuccess);
				LPWSTR sourceUrl = nullptr;
				sender->get_Source(&sourceUrl);
				const wchar_t* url = sourceUrl ? sourceUrl : L"";
				if (isSuccess)
				{
					WebBrowserNavigatedEventArgs eventArgs(url);
					selfRef->fWebBrowserControlPointer->fNavigatedEvent.Raise(
							*selfRef->fWebBrowserControlPointer, eventArgs);
				}
				else
				{
					COREWEBVIEW2_WEB_ERROR_STATUS status = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
					args->get_WebErrorStatus(&status);
					std::wstringstream stringStream;
					stringStream << L"WebView2 navigation failed (web error status ";
					stringStream << static_cast<int>(status);
					stringStream << L")";
					std::wstring errorMessage = stringStream.str();
					selfRef->RaiseNavigationFailed(url, static_cast<int>(status), errorMessage.c_str());
				}
				if (sourceUrl)
				{
					::CoTaskMemFree(sourceUrl);
				}
				return S_OK;
			});
		if (navCompleteHandler)
		{
			fWebViewPointer->add_NavigationCompleted(navCompleteHandler.Get(), &fNavigationCompletedToken);
		}
	}

	// HistoryChanged → updates the cached canGoBack / canGoForward flags.
	{
		Microsoft::WRL::ComPtr<EdgeWebViewHandler> selfRef(this);
		auto historyHandler = Microsoft::WRL::Callback<ICoreWebView2HistoryChangedEventHandler>(
			[selfRef](ICoreWebView2* sender, IUnknown* /*args*/) -> HRESULT
			{
				if (selfRef->fIsDetached || !sender)
				{
					return S_OK;
				}
				BOOL canBack = FALSE;
				BOOL canForward = FALSE;
				sender->get_CanGoBack(&canBack);
				sender->get_CanGoForward(&canForward);
				selfRef->fCanNavigateBack = canBack ? true : false;
				selfRef->fCanNavigateForward = canForward ? true : false;
				return S_OK;
			});
		if (historyHandler)
		{
			fWebViewPointer->add_HistoryChanged(historyHandler.Get(), &fHistoryChangedToken);
		}
	}

	fControllerPointer->put_IsVisible(TRUE);
	fIsReady = true;

	// If a navigation was requested before WebView2 finished initializing, dispatch it now.
	DispatchPendingNavigation();
	return S_OK;
}

void WebBrowser::EdgeWebViewHandler::DetachFromControl()
{
	if (fIsDetached)
	{
		return;
	}
	fIsDetached = true;

	if (fWebBrowserControlPointer)
	{
		fWebBrowserControlPointer->GetResizedEventHandlers().Remove(&fResizedEventHandler);
	}

	if (fWebViewPointer)
	{
		fWebViewPointer->remove_NavigationStarting(fNavigationStartingToken);
		fWebViewPointer->remove_NavigationCompleted(fNavigationCompletedToken);
		fWebViewPointer->remove_HistoryChanged(fHistoryChangedToken);
		fWebViewPointer->Release();
		fWebViewPointer = nullptr;
	}

	if (fControllerPointer)
	{
		// Close() detaches the WebView2 from the parent HWND synchronously and stops further events.
		fControllerPointer->Close();
		fControllerPointer->Release();
		fControllerPointer = nullptr;
	}

	if (fEnvironmentPointer)
	{
		fEnvironmentPointer->Release();
		fEnvironmentPointer = nullptr;
	}

	fWebBrowserControlPointer = nullptr;
	fIsReady = false;
	fHasPendingUrl = false;
	fPendingUrl.clear();
	fCanNavigateBack = false;
	fCanNavigateForward = false;
}

bool WebBrowser::EdgeWebViewHandler::CanNavigateBack()
{
	return fCanNavigateBack;
}

bool WebBrowser::EdgeWebViewHandler::CanNavigateForward()
{
	return fCanNavigateForward;
}

void WebBrowser::EdgeWebViewHandler::NavigateBack()
{
	if (fIsDetached || !fWebViewPointer)
	{
		return;
	}
	BOOL canGoBack = FALSE;
	fWebViewPointer->get_CanGoBack(&canGoBack);
	if (canGoBack)
	{
		fWebViewPointer->GoBack();
	}
}

void WebBrowser::EdgeWebViewHandler::NavigateForward()
{
	if (fIsDetached || !fWebViewPointer)
	{
		return;
	}
	BOOL canGoForward = FALSE;
	fWebViewPointer->get_CanGoForward(&canGoForward);
	if (canGoForward)
	{
		fWebViewPointer->GoForward();
	}
}

void WebBrowser::EdgeWebViewHandler::NavigateTo(const wchar_t* url)
{
	if (fIsDetached)
	{
		return;
	}
	if (!url || (url[0] == L'\0'))
	{
		return;
	}

	// WebView2 init is asynchronous. If a Lua call to native.newWebView():request() arrives
	// before the controller is ready, queue the URL and load it once init completes.
	if (!fIsReady || !fWebViewPointer)
	{
		fPendingUrl = url;
		fHasPendingUrl = true;
		return;
	}

	fWebViewPointer->Navigate(url);
}

void WebBrowser::EdgeWebViewHandler::Reload()
{
	if (fIsDetached || !fWebViewPointer)
	{
		return;
	}
	fWebViewPointer->Reload();
}

void WebBrowser::EdgeWebViewHandler::StopLoading()
{
	if (fIsDetached || !fWebViewPointer)
	{
		return;
	}
	fWebViewPointer->Stop();
}

void WebBrowser::EdgeWebViewHandler::OnResized(
	Interop::UI::Control& sender, const Interop::EventArgs& arguments)
{
	UpdateBounds();
}

void WebBrowser::EdgeWebViewHandler::UpdateBounds()
{
	if (fIsDetached || !fControllerPointer || !fWebBrowserControlPointer)
	{
		return;
	}
	RECT bounds = fWebBrowserControlPointer->GetClientBounds();
	fControllerPointer->put_Bounds(bounds);
}

void WebBrowser::EdgeWebViewHandler::DispatchPendingNavigation()
{
	if (!fHasPendingUrl || !fIsReady || !fWebViewPointer)
	{
		return;
	}
	std::wstring url = fPendingUrl;
	fHasPendingUrl = false;
	fPendingUrl.clear();
	fWebViewPointer->Navigate(url.c_str());
}

void WebBrowser::EdgeWebViewHandler::RaiseNavigationFailed(
	const wchar_t* url, int errorCode, const wchar_t* message)
{
	if (!fWebBrowserControlPointer)
	{
		return;
	}
	WebBrowserNavigationFailedEventArgs eventArgs(
			url ? url : L"", errorCode, message ? message : L"");
	fWebBrowserControlPointer->fNavigationFailedEvent.Raise(*fWebBrowserControlPointer, eventArgs);
}

HRESULT WebBrowser::EdgeWebViewHandler::QueryInterface(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
	{
		return E_POINTER;
	}
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG WebBrowser::EdgeWebViewHandler::AddRef(void)
{
	return ++fReferenceCount;
}

ULONG WebBrowser::EdgeWebViewHandler::Release(void)
{
	ULONG count = --fReferenceCount;
	if (count == 0)
	{
		delete this;
	}
	return count;
}

#else  // Rtt_WIN_WEBVIEW2_ENABLED == 0

// Stub implementations used when the Microsoft.Web.WebView2 SDK is unavailable at compile time.
// CreateAndAttachTo always returns null, so the WebBrowser ctor falls through to the IE backend
// and these member functions are never actually invoked at runtime — but the linker still needs
// definitions for the methods declared in the header.

WebBrowser::EdgeWebViewHandler::EdgeWebViewHandler(WebBrowser* /*controlPointer*/)
:	fWebBrowserControlPointer(nullptr),
	fEnvironmentPointer(nullptr),
	fControllerPointer(nullptr),
	fWebViewPointer(nullptr),
	fNavigationStartingToken{},
	fNavigationCompletedToken{},
	fSourceChangedToken{},
	fHistoryChangedToken{},
	fReferenceCount(1),
	fIsDetached(true),
	fIsReady(false),
	fHasPendingUrl(false),
	fCanNavigateBack(false),
	fCanNavigateForward(false),
	fResizedEventHandler(this, &EdgeWebViewHandler::OnResized)
{
}

WebBrowser::EdgeWebViewHandler::~EdgeWebViewHandler() {}

bool WebBrowser::EdgeWebViewHandler::IsRuntimeAvailable() { return false; }

WebBrowser::EdgeWebViewHandler* WebBrowser::EdgeWebViewHandler::CreateAndAttachTo(WebBrowser*)
{
	return nullptr;
}

bool WebBrowser::EdgeWebViewHandler::BeginAsyncCreation() { return false; }
HRESULT WebBrowser::EdgeWebViewHandler::OnEnvironmentCreated(HRESULT, ICoreWebView2Environment*) { return E_NOTIMPL; }
HRESULT WebBrowser::EdgeWebViewHandler::OnControllerCreated(HRESULT, ICoreWebView2Controller*) { return E_NOTIMPL; }

void WebBrowser::EdgeWebViewHandler::DetachFromControl() {}
bool WebBrowser::EdgeWebViewHandler::CanNavigateBack() { return false; }
bool WebBrowser::EdgeWebViewHandler::CanNavigateForward() { return false; }
void WebBrowser::EdgeWebViewHandler::NavigateBack() {}
void WebBrowser::EdgeWebViewHandler::NavigateForward() {}
void WebBrowser::EdgeWebViewHandler::NavigateTo(const wchar_t*) {}
void WebBrowser::EdgeWebViewHandler::Reload() {}
void WebBrowser::EdgeWebViewHandler::StopLoading() {}
void WebBrowser::EdgeWebViewHandler::OnResized(Interop::UI::Control&, const Interop::EventArgs&) {}
void WebBrowser::EdgeWebViewHandler::UpdateBounds() {}
void WebBrowser::EdgeWebViewHandler::DispatchPendingNavigation() {}
void WebBrowser::EdgeWebViewHandler::RaiseNavigationFailed(const wchar_t*, int, const wchar_t*) {}

HRESULT WebBrowser::EdgeWebViewHandler::QueryInterface(REFIID, void** ppvObject)
{
	if (ppvObject) { *ppvObject = nullptr; }
	return E_NOINTERFACE;
}

ULONG WebBrowser::EdgeWebViewHandler::AddRef(void) { return ++fReferenceCount; }

ULONG WebBrowser::EdgeWebViewHandler::Release(void)
{
	ULONG count = --fReferenceCount;
	if (count == 0) { delete this; }
	return count;
}

#endif  // Rtt_WIN_WEBVIEW2_ENABLED
#pragma endregion

} }	// namespace Interop::UI

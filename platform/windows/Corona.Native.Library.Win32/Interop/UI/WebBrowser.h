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
#include "Interop\Event.h"
#include "Interop\ScopedOleInitializer.h"
#include "Control.h"
#include "WebBrowserNavigatedEventArgs.h"
#include "WebBrowserNavigatingEventArgs.h"
#include "WebBrowserNavigationFailedEventArgs.h"
#include <ExDisp.h>
#include <MsHtmHst.h>
#include <string>
#include <Windows.h>


namespace Interop { namespace UI {

/// <summary>Microsoft Web Browser ActiveX control.</summary>
class WebBrowser : public Control
{
	Rtt_CLASS_NO_COPIES(WebBrowser)

	public:
		#pragma region Public Event Types
		/// <summary>
		///  Defines the "Navigating" event type which is raised when the control is about to load a URL.
		/// </summary>
		typedef Event<WebBrowser&, WebBrowserNavigatingEventArgs&> NavigatingEvent;

		/// <summary>
		///  Defines the "Navigated" event type which is raised when the control finishes loading a URL.
		/// </summary>
		typedef Event<WebBrowser&, const WebBrowserNavigatedEventArgs&> NavigatedEvent;

		/// <summary>
		///  Defines the "NavigationFailed" event type which is raised when the control fails to load a URL.
		/// </summary>
		typedef Event<WebBrowser&, const WebBrowserNavigationFailedEventArgs&> NavigationFailedEvent;

		#pragma endregion


		#pragma region CreationSettings Structure
		/// <summary>Settings needed to create a new WebBrowser class instances.</summary>
		struct CreationSettings
		{
			/// <summary>Handle to the window or control that will be the parent for the new Web Browser control.</summary>
			HWND ParentWindowHandle;

			/// <summary>The parent window/control's client coordinates to display the Web Browser at upon creation.</summary>
			RECT Bounds;

			/// <summary>
			///  <para>
			///   Registry path without the hive prefix used to store custom Internet Explorer settings that will
			///   only apply to this application. These settings override IE's default settings.
			///  </para>
			///  <para>Set to null or empty string to use Internet Explorer's default settings.</para>
			/// </summary>
			const wchar_t* IEOverrideRegistryPath;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new Web Browser control.</summary>
		/// <param name="settings">Provides the settings needed to initialize the Web Browser control.</param>
		WebBrowser(const CreationSettings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~WebBrowser();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets an object used to add or remove an event handler for the "Navigating" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		NavigatingEvent::HandlerManager& GetNavigatingEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Navigated" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		NavigatedEvent::HandlerManager& GetNavigatedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "NavigationFailed" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		NavigationFailedEvent::HandlerManager& GetNavigationFailedEventHandlers();

		/// <summary>
		///  <para>
		///   Gets the Internet Explorer registry path passed to this object's constructor via the
		///   "CreationSettings.IEOverrideRegistryPath" field.
		///  </para>
		///  <para>
		///   If the path is set, then it is used to store custom Internet Explorer settings which will only
		///   apply to this app. (Does not impact IE's default settings.)
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a registry path without the hive name prefix.</para>
		///  <para>Returns null if a registry path was not provided to this object's constructor.</para>
		/// </returns>
		const wchar_t* GetIEOverrideRegistryPath() const;

		/// <summary>Determins if the web browser can navigate back in its history.</summary>
		/// <returns>Returns true if can navigate back. Returns false if not.</returns>
		bool CanNavigateBack();

		/// <summary>Determins if the web browser can navigate forward in its history.</summary>
		/// <returns>Returns true if can navigate forward. Returns false if not.</returns>
		bool CanNavigateForward();

		/// <summary>Commands the web browser to navigate back in its history list.</summary>
		void NavigateBack();

		/// <summary>Commands the web browser to navigate forward in its history list.</summary>
		void NavigateForward();

		/// <summary>Commands the web browser to navigate to the given URL or file path.</summary>
		/// <param name="url">The URL of local file path to navigate to. Will be ignored if null or empty.</param>
		void NavigateTo(const wchar_t* url);

		/// <summary>Commands the web browser to reload the current page.</summary>
		void Reload();

		/// <summary>Commands the web browser to stop loading the current page, if loading.</summary>
		void StopLoading();

		#pragma endregion

	private:
		#pragma region MicrosoftWebBrowserHandler Class
		/// <summary>Handler which creates and interfaces with the Internet Explorer ActiveX object.</summary>
		class MicrosoftWebBrowserHandler
		:	public IOleClientSite, public IOleInPlaceSite, public IDocHostUIHandler2, public IOleCommandTarget,
			public IDispatch, public IStorage
		{
			public:
				#pragma region IUnknown Methods
				virtual HRESULT STDMETHODCALLTYPE QueryInterface(
							REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
				virtual ULONG STDMETHODCALLTYPE AddRef(void);
				virtual ULONG STDMETHODCALLTYPE Release(void);

				#pragma endregion


				#pragma region IOleClientSite Methods
				virtual HRESULT STDMETHODCALLTYPE SaveObject(void);
				virtual HRESULT STDMETHODCALLTYPE GetMoniker(
							DWORD dwAssign, DWORD dwWhichMoniker, __RPC__deref_out_opt IMoniker **ppmk);
				virtual HRESULT STDMETHODCALLTYPE GetContainer(__RPC__deref_out_opt IOleContainer **ppContainer);
				virtual HRESULT STDMETHODCALLTYPE ShowObject(void);
				virtual HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL fShow);
				virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout(void);

				#pragma endregion


				#pragma region IOleWindow Methods
				virtual HRESULT STDMETHODCALLTYPE GetWindow(__RPC__deref_out_opt HWND *phwnd);
				virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode);

				#pragma endregion


				#pragma region IOleInPlaceSite Methods
				virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate(void);
				virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate(void);
				virtual HRESULT STDMETHODCALLTYPE OnUIActivate(void);
				virtual HRESULT STDMETHODCALLTYPE GetWindowContext(
							__RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
							__RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
							__RPC__out LPRECT lprcPosRect,
							__RPC__out LPRECT lprcClipRect,
							__RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo);
				virtual HRESULT STDMETHODCALLTYPE Scroll(SIZE scrollExtant);
				virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL fUndoable);
				virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate(void);
				virtual HRESULT STDMETHODCALLTYPE DiscardUndoState(void);
				virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo(void);
				virtual HRESULT STDMETHODCALLTYPE OnPosRectChange(__RPC__in LPCRECT lprcPosRect);

				#pragma endregion


				#pragma region IDocHostUIHandler
				virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(
							DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);
				virtual HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO *pInfo);
				virtual HRESULT STDMETHODCALLTYPE ShowUI(
							DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
							IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc);
				virtual HRESULT STDMETHODCALLTYPE HideUI(void);
				virtual HRESULT STDMETHODCALLTYPE UpdateUI(void);
				virtual HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable);
				virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL fActivate);
				virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL fActivate);
				virtual HRESULT STDMETHODCALLTYPE ResizeBorder(
							LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow);
				virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(
							LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);
				virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath(__out LPOLESTR *pchKey, DWORD dw);
				virtual HRESULT STDMETHODCALLTYPE GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget);
				virtual HRESULT STDMETHODCALLTYPE GetExternal(IDispatch **ppDispatch);
				virtual HRESULT STDMETHODCALLTYPE TranslateUrl(
							DWORD dwTranslate, __in __nullterminated  OLECHAR *pchURLIn, __out OLECHAR **ppchURLOut);
				virtual HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject *pDO, IDataObject **ppDORet);

				#pragma endregion


				#pragma region IDocHostUIHandler2
				virtual HRESULT STDMETHODCALLTYPE GetOverrideKeyPath(__deref_out LPOLESTR *pchKey, DWORD dw);

				#pragma endregion


				#pragma region IOleCommandTarget
				virtual HRESULT STDMETHODCALLTYPE QueryStatus(
							__RPC__in_opt const GUID *pguidCmdGroup, ULONG cCmds,
							__RPC__inout_ecount_full(cCmds) OLECMD prgCmds[],
							__RPC__inout_opt OLECMDTEXT *pCmdText);
				virtual HRESULT STDMETHODCALLTYPE Exec(
							__RPC__in_opt const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
							__RPC__in_opt VARIANT *pvaIn, __RPC__inout_opt VARIANT *pvaOut);

				#pragma endregion


				#pragma region IDispatch Methods
				virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(__RPC__out UINT *pctinfo);
				virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(
							UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo);
				virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(
							__RPC__in REFIID riid, __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
							__RPC__in_range(0,16384) UINT cNames, LCID lcid,
							__RPC__out_ecount_full(cNames) DISPID *rgDispId);
				virtual HRESULT STDMETHODCALLTYPE Invoke(
							DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
							VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

				#pragma endregion


				#pragma region IStorage Methods
				virtual HRESULT STDMETHODCALLTYPE CreateStream(
							__RPC__in_string const OLECHAR *pwcsName, DWORD grfMode,
							DWORD reserved1, DWORD reserved2,
							__RPC__deref_out_opt IStream **ppstm);
				virtual HRESULT STDMETHODCALLTYPE OpenStream(
							const OLECHAR *pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm);
				virtual HRESULT STDMETHODCALLTYPE CreateStorage(
							__RPC__in_string const OLECHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2,
							__RPC__deref_out_opt IStorage **ppstg);
				virtual HRESULT STDMETHODCALLTYPE OpenStorage(
							__RPC__in_opt_string const OLECHAR *pwcsName,
							__RPC__in_opt IStorage *pstgPriority,
							DWORD grfMode, __RPC__deref_opt_in_opt SNB snbExclude,
							DWORD reserved, __RPC__deref_out_opt IStorage **ppstg);
				virtual HRESULT STDMETHODCALLTYPE CopyTo(
							DWORD ciidExclude, const IID *rgiidExclude,
							__RPC__in_opt  SNB snbExclude, IStorage *pstgDest);
				virtual HRESULT STDMETHODCALLTYPE MoveElementTo(
							__RPC__in_string const OLECHAR *pwcsName, __RPC__in_opt IStorage *pstgDest,
							__RPC__in_string const OLECHAR *pwcsNewName, DWORD grfFlags);
				virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
				virtual HRESULT STDMETHODCALLTYPE Revert(void);
				virtual HRESULT STDMETHODCALLTYPE EnumElements(
							DWORD reserved1, void *reserved2, DWORD reserved3, IEnumSTATSTG **ppenum);
				virtual HRESULT STDMETHODCALLTYPE DestroyElement(__RPC__in_string const OLECHAR *pwcsName);
				virtual HRESULT STDMETHODCALLTYPE RenameElement(
							__RPC__in_string const OLECHAR *pwcsOldName, __RPC__in_string const OLECHAR *pwcsNewName);
				virtual HRESULT STDMETHODCALLTYPE SetElementTimes(
							__RPC__in_opt_string const OLECHAR *pwcsName, __RPC__in_opt const FILETIME *pctime,
							__RPC__in_opt const FILETIME *patime, __RPC__in_opt const FILETIME *pmtime);
				virtual HRESULT STDMETHODCALLTYPE SetClass(__RPC__in REFCLSID clsid);
				virtual HRESULT STDMETHODCALLTYPE SetStateBits(DWORD grfStateBits, DWORD grfMask);
				virtual HRESULT STDMETHODCALLTYPE Stat(__RPC__out STATSTG *pstatstg, DWORD grfStatFlag);

				#pragma endregion


				#pragma region Pubic Methods
				void DetachFromControl();
				bool CanNavigateBack();
				bool CanNavigateForward();
				void NavigateBack();
				void NavigateForward();
				void NavigateTo(const wchar_t* url);
				void Reload();
				void StopLoading();

				#pragma endregion


				#pragma region Public Static Functions
				static MicrosoftWebBrowserHandler* CreateAndAttachTo(WebBrowser* controlPointer);

				#pragma endregion

			private:
				#pragma region Constructors/Destructors
				MicrosoftWebBrowserHandler(WebBrowser* controlPointer);
				MicrosoftWebBrowserHandler(const MicrosoftWebBrowserHandler&) = delete;
				virtual ~MicrosoftWebBrowserHandler();

				#pragma endregion


				#pragma region Private Methods
				void OnResized(Interop::UI::Control& sender, const Interop::EventArgs& arguments);
				bool IsTopLevelBrowserDispatch(IDispatch* dispatchPointer) const;

				#pragma endregion


				#pragma region Private Member Variables
				WebBrowser* fWebBrowserControlPointer;
				IOleObject* fWebBrowserOleObjectPointer;
				IOleInPlaceObject* fOleInPlaceObjectPointer;
				DWORD fEventToken;
				ULONG fReferenceCount;
				bool fCanNavigateBack;
				bool fCanNavigateForward;
				Control::ResizedEvent::MethodHandler<MicrosoftWebBrowserHandler> fResizedEventHandler;

				#pragma endregion
		};

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Manages the "Navigating" event.</summary>
		NavigatingEvent fNavigatingEvent;

		/// <summary>Manages the "Navigated" event.</summary>
		NavigatedEvent fNavigatedEvent;

		/// <summary>Manages the "NavigationFailed" event.</summary>
		NavigationFailedEvent fNavigationFailedEvent;

		/// <summary>
		///  <para>Initializes Microsoft OLE/COM upon construction and unitializes OLE/COM upon destruction.</para>
		///  <para>OLE is needed to support ActiveX controls, drag & drop, and clipboard copy/paste.</para>
		///  <para>Note: The Internet Explorer WebBrowser control is an ActiveX control which needs OLE support.</para>
		/// </summary>
		ScopedOleInitializer fScopedOleInitializer;

		/// <summary>Handler which creates and interfaces with the Internet Explorer ActiveX object.</summary>
		MicrosoftWebBrowserHandler* fWebBrowserHandlerPointer;
		
		/// <summary>
		///  <para>
		///   Registry path without the hive prefix used to store custom Internet Explorer settings that will
		///   only apply to this application. These settings override IE's default settings.
		///  </para>
		///  <para>Set to empty string to use Internet Explorer's default settings.</para>
		/// </summary>
		std::wstring fIEOverrideRegistryPath;

		#pragma endregion
};

} }	// namespace Interop::UI

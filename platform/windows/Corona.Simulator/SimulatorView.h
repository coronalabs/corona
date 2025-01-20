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
#include "Core\Rtt_Array.h"
#include "Interop\ScopedComInitializer.h"
#include "Interop\SimulatorRuntimeEnvironment.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_WinSimulatorServices.h"


#pragma region Forward Declarations
namespace Rtt
{
	struct RecentProjectInfo;
}
namespace Gdiplus
{
	class Bitmap;
}
class CMessageDlg;
class CProgressWnd;
class CSimulatorDoc;

#pragma endregion


class CSimulatorView : public CView
{
	protected:
		CSimulatorView();
		DECLARE_DYNCREATE(CSimulatorView)

	public:
		virtual ~CSimulatorView();

		virtual void OnInitialUpdate();
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
#endif
		void StartSimulation();
		void RestartSimulation();
		void SuspendResumeSimulationWithOverlay(bool showOverlay, bool sendSystemEvents);
		void StopSimulation();
		bool IsShowingInternalScreen() { return mIsShowingInternalScreen; }
		bool IsSimulationSuspended() const;
		CSimulatorDoc* GetDocument() const;
		CString GetDisplayName()   { return mDisplayName; }
		const Rtt::PlatformSimulator::Config& GetDeviceConfig() { return mDeviceConfig; }
		int GetRotation() { return mRotation; }
		Gdiplus::Bitmap *GetSkinBitmap() { return mpSkinBitmap; }
		void UpdateSimulatorSkin();  // zoom functions in CMainFrame call this
		Interop::SimulatorRuntimeEnvironment* GetRuntimeEnvironment()  { return mRuntimeEnvironmentPointer; }
		void GetRecentDocs(Rtt::LightPtrArray<Rtt::RecentProjectInfo> *list);
		void PostOpenWithPath(CString fileName);

		/// <summary>
		///  <para>
		///   Determines if all plugins for the currently running project have been successfully downloaded and acquired.
		///  </para>
		///  <para>Will display a message box if at least 1 plugin the project depends on was not found</para>
		///  <para>Intended to be called by the Win32 build dialog if at least 1 plugin was missing.</para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns true if all plugins for the currently running project have been acquired
		///   or if the project the does not depend on any plugins.
		///  </para>
		///  <para>Returns false if at least 1 plugin the projct depends on was not found.</para>
		/// </returns>
		bool VerifyAllPluginsAcquired();

		/// <summary>
		///  <para>Custom control used by CSimulatorView to render Corona's content.</para>
		///  <para>This container is a CStatic (ie: label control) that can be used to display a suspended message.</para>
		///  <para>
		///   This container's child control, accessed via GetCoronaControl(), is expected to be used by Corona
		///   to render content to.
		///  </para>
		///  <para>
		///   This child Corona control is expected to be hidden in order to reveal this container's suspended message.
		///  </para>
		/// </summary>
		class CCoronaControlContainer : public CStatic
		{
			public:
				/// <summary>Fetches the container's child Corona control used to render Corona's content.</summary>
				/// <returns>Returns a reference to this contain's child Corona control.</returns>
				CWnd& GetCoronaControl();

			protected:
				DECLARE_MESSAGE_MAP()
				afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
				afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
				afx_msg void OnSize(UINT nType, int cx, int cy);

			private:
				CButton mCoronaControl;
		};

	protected:
		virtual void OnDraw(CDC* pDC);
		virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject *pHint);
		virtual void OnTimer(UINT nIDEvent);

		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy();
		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnAppAbout();
		afx_msg void OnHelp();
		afx_msg void OnPreferences();
		afx_msg void OnViewHomeScreen();
		afx_msg void OnViewConsole();
		afx_msg void OnViewShake();
		afx_msg void OnViewSuspend();
		afx_msg void OnViewRotateLeft();
		afx_msg void OnViewRotateRight();
		afx_msg void OnViewNavigateBack();
		afx_msg void OnFileMRU1();
		afx_msg void OnFileNew();
		afx_msg void OnFileOpen();
		afx_msg void OnFileOpenSampleProject();
		afx_msg void OnBuildForAndroid();
		afx_msg void OnBuildForWeb();
		afx_msg void OnBuildForLinux();
		afx_msg void OnBuildForNxS();
		afx_msg void OnBuildForWin32();
		afx_msg void OnFileOpenInEditor();
		afx_msg void OnFileRelaunch();
		afx_msg void OnShowProjectFiles();
		afx_msg void OnShowProjectSandbox();
		afx_msg void OnClearProjectSandbox();
		afx_msg void OnFileClose();
		afx_msg void OnClose();
		afx_msg void OnUpdateViewSuspend(CCmdUI *pCmdUI);
		afx_msg void OnWindowViewAs( UINT nID );
		afx_msg void OnUpdateWindowViewAs( CCmdUI *pCmdUI );
		afx_msg void OnUpdateViewNavigateBack(CCmdUI *pCmdUI);
		afx_msg void OnUpdateFileRelaunch(CCmdUI *pCmdUI);
		afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
		afx_msg void OnUpdateShowProjectFiles(CCmdUI *pCmdUI);
		afx_msg void OnUpdateShowProjectSandbox(CCmdUI *pCmdUI);
		afx_msg void OnUpdateBuildMenuItem(CCmdUI *pCmdUI);
		afx_msg void OnUpdateFileOpenInEditor(CCmdUI *pCmdUI);
		afx_msg LRESULT OnActivityIndicator(WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnNativeAlert(WPARAM wParam, LPARAM lParam);

	private:
		void RunCoronaProject();
		void RunCoronaProject(CString &projectPath);
		void ScaleRect(CRect& rect, float scale);
		Rtt::TargetDevice::Skin SkinIDFromMenuID(UINT nID);
		int GetAnalyticsSkinId();
		bool InitSkin(Rtt::TargetDevice::Skin skinId);
		bool InitializeSimulation(Rtt::TargetDevice::Skin skinId);
		void SetRotation(int rotation);
		bool ValidateOpenGL();
		bool HasApplicationChanged();
		bool LoadSkinResources();
		void GetFilePaths(LPCTSTR pattern, CStringArray& filepaths);
		void OnRuntimeLoaded(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);
		void RemoveUnauthorizedMenuItemsFrom(CMenu* menuPointer);


		CCoronaControlContainer mCoronaContainerControl;
		Interop::SimulatorRuntimeEnvironment* mRuntimeEnvironmentPointer;
		Interop::SimulatorRuntimeEnvironment::LoadedEvent::MethodHandler<CSimulatorView> mRuntimeLoadedEventHandler;
		Interop::ScopedComInitializer mScopedComInitializer;
		Rtt::WinSimulatorServices mSimulatorServices;
		CMessageDlg* mMessageDlgPointer;
		CProgressWnd* mProgressDlgPointer;
		CString mDisplayName;
		int mRotation;  // 0, 90, 180, 270
		Gdiplus::Bitmap *mpSkinBitmap;
		Rtt::PlatformSimulator::Config mDeviceConfig;
		bool mIsShowingInternalScreen;	// Set true when showing the Home Screen or Demo Screen.
		HANDLE mAppChangeHandle;		// Filesystem notifications handle
		int mShakeAmount;
		int mShakeReps;
		int mShakeNum;
		CRect mShakeOriginRect;
		Rtt::TargetDevice::Skin m_nSkinId;
		CString mCustomSkinFileName;
		CString mSystemSkinsDir;
		CString mUserSkinsDir;
		int mRelaunchCount;
};

#ifndef _DEBUG  // debug version in SimulatorView.cpp
inline CSimulatorDoc* CSimulatorView::GetDocument() const
   { return reinterpret_cast<CSimulatorDoc*>(m_pDocument); }
#endif

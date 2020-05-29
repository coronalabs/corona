//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


class CMainFrame : public CFrameWnd
{
	protected:
		CMainFrame();
		DECLARE_DYNCREATE(CMainFrame)

	public:
		virtual ~CMainFrame();
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
#endif
		void CopyUnscaledDeviceWidthHeightTo(int *widthPointer, int *heightPointer);
		CPoint GetMinTrackSize()  { return m_ptMinTrackSize; }
		int GetZoom()    { return mZoom; }
		void SetZoom(int zoom);
		void AdjustZoom(int width, int height);
		float CalcZoomFactor();
		CRect SizeToClient(CRect rectClient);

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnClose();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
		afx_msg void OnWindowZoomIn();
		afx_msg void OnWindowZoomOut();
		afx_msg void OnUpdateWindowZoomOut(CCmdUI *pCmdUI);
		afx_msg void OnUpdateWindowZoomIn(CCmdUI *pCmdUI);

		void OnUpdateFrameTitle(BOOL bAddToTitle);
		int CalculateNewHeight(float factor, float height);
		int CalculateNewWidth(float factor, float width);

	private:
		CPoint m_ptMinTrackSize;
		int mZoom;      // 0 is normal, -1 is zoomed out once, 2 is zoomed in twice
		static const int ZOOM_IN_LIMIT = 0;
		static const int ZOOM_OUT_LIMIT = -3;
};

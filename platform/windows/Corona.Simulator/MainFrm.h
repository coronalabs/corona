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

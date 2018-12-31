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

#include "stdafx.h"
#include <gdiplus.h>
using namespace Gdiplus;

#include "Simulator.h"
#include "MainFrm.h"
#include "math.h"     // floor
#include "float.h"
#include "Core/Rtt_Build.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_RenderingStream.h"
#include "Rtt_Runtime.h"
#include "SimulatorDoc.h"
#include "SimulatorView.h"

#define log2f(x) (logf(x)/logf(2.0f))

// The #includes must go above this line because this macro will override the "new" operator
// which will cause compiler errors with header files belonging to other libraries.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_WINDOW_ZOOMIN, &CMainFrame::OnWindowZoomIn)
	ON_COMMAND(ID_WINDOW_ZOOMOUT, &CMainFrame::OnWindowZoomOut)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ZOOMOUT, &CMainFrame::OnUpdateWindowZoomOut)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ZOOMIN, &CMainFrame::OnUpdateWindowZoomIn)
END_MESSAGE_MAP()


// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
    m_ptMinTrackSize( 0, 0 )
{
    mZoom = 0;
	// http://software.intel.com/en-us/articles/fast-floating-point-to-integer-conversions/
//	unsigned int control_word;
//	_controlfp_s( &control_word, 0, 0 );
//	_controlfp_s( &control_word, _RC_CHOP, _MCW_RC );
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

// PreCreateWindow - remove maximize box and resize frame
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= WS_CLIPCHILDREN;  // added for CView smaller than MainFrame.

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

    // Can't maximize or resize window
	cs.style &= ~(WS_MAXIMIZEBOX|WS_THICKFRAME);

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

// CMainFrame message handlers

// OnClose - Save position of window at closing for next invocation 
void CMainFrame::OnClose()
{
	CSimulatorApp *app=(CSimulatorApp *)AfxGetApp();
    CSimulatorView *pView = (CSimulatorView *)GetActiveView();
	WINDOWPLACEMENT wp;

    // Store windows position, zoom size, and rotation.
	this->GetWindowPlacement(&wp);
	app->PutWP(wp);
    app->PutZoom( GetZoom() );
    app->PutRotation( pView->GetRotation() );
    app->PutDisplayName( pView->GetDisplayName() );

	// Stop simulation. This also posts user feedback/analytics to the sever if enabled.
	pView->StopSimulation();

	// Close the window.
	CFrameWnd::OnClose();
}

/// Updates the title bar text on the window according to the currently selected document.
/// @param bAddToTitle Set TRUE to show the document name in the title.
///                    Set FALSE to only show the application name in the title.
void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// Override the title bar text to only show the application name if we are currently
	// showing an internal screen such as the home screen or demos screen.
	CSimulatorView *viewPointer = (CSimulatorView*)GetActiveView();
	if (viewPointer && viewPointer->IsShowingInternalScreen())
	{
		bAddToTitle = FALSE;
	}
	
	// Do not display an document name in title bar if document does not have a title.
	// This works-around issue where MFC shows "- Corona Simulator" in this case.
	CDocument *documentPointer = GetActiveDocument();
	if (documentPointer && documentPointer->GetTitle().IsEmpty())
	{
		bAddToTitle = FALSE;
	}
	
	// Update the title bar text.
	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
}

// OnSize - override because we don't want to call CFrameWnd implementation
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    // Skip CFrameWnd version, which ruins our layout
	CWnd::OnSize(nType, cx, cy);

	// We need to repaint the menu bar when the window grows in size
	// or else white space will be left behind.
	DrawMenuBar();
}

// OnGetMinMaxInfo - store the current min and max tracking sizes for zoom limits
// Max window sizes are limited by screen size, and min sizes are limited by menu size
void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// Increase the maximum size of the window to the biggest possible value.
	lpMMI->ptMaxTrackSize.x = LONG_MAX;
	lpMMI->ptMaxTrackSize.y = LONG_MAX;
    m_ptMinTrackSize = lpMMI->ptMinTrackSize;
}

// OnUpdateWindowZoomOut - can zoom out further if Windows allows the
// window to get that small, and if we haven't passed our own zoom limit
void CMainFrame::OnUpdateWindowZoomOut(CCmdUI *pCmdUI)
{
	CPoint ptMin = GetMinTrackSize();

	int nDeviceWidth = 0;
	int nDeviceHeight = 0;
	CopyUnscaledDeviceWidthHeightTo(&nDeviceWidth, &nDeviceHeight);

	bool canZoom = false;
	if ((nDeviceWidth > 0) && (nDeviceHeight > 0))
	{
		bool bWindowSmallest = false;
		float factor = 0.5f * CalcZoomFactor();
		int nNewWidth = CalculateNewWidth(factor, (float)nDeviceWidth);
		int nNewHeight = CalculateNewHeight(factor, (float)nDeviceHeight);
		if ((nNewWidth < ptMin.x) || (nNewHeight < ptMin.y))
		{
			bWindowSmallest = true;
		}
		canZoom = !bWindowSmallest && (GetZoom() > ZOOM_OUT_LIMIT);
	}
	pCmdUI->Enable(canZoom ? TRUE : FALSE);
}

// OnUpdateWindowZoomIn - can zoom in further if Windows allows the
// window to get that big, and if we haven't passed our own zoom limit
void CMainFrame::OnUpdateWindowZoomIn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetZoom() < ZOOM_IN_LIMIT);
}

// OnWindowZoomIn - increase zoom factor (make window twice as big)
// and update the skin
void CMainFrame::OnWindowZoomIn()
{
	SetZoom( GetZoom() + 1 ); 

	CSimulatorView *pView = (CSimulatorView *)GetActiveView();
    pView->UpdateSimulatorSkin();

	CSimulatorApp *applicationPointer = (CSimulatorApp*)AfxGetApp();
	applicationPointer->SaveZoomToRegistry(GetZoom());
}

// OnWindowZoomOut - decrease zoom factor (make window twice as small)
// and update the skin
void CMainFrame::OnWindowZoomOut()
{
	SetZoom( GetZoom() - 1 ); 

	CSimulatorView *pView = (CSimulatorView *)GetActiveView();
    pView->UpdateSimulatorSkin();

	CSimulatorApp *applicationPointer = (CSimulatorApp*)AfxGetApp();
	applicationPointer->SaveZoomToRegistry(GetZoom());
}

// SetZoom - save zoom, ensuring it is within limits.
void CMainFrame::SetZoom( int zoom )
{
	if (zoom > ZOOM_IN_LIMIT)
	{
		zoom = ZOOM_IN_LIMIT;
	}
	if (zoom < ZOOM_OUT_LIMIT)
	{
		zoom = ZOOM_OUT_LIMIT;
	}
    mZoom = zoom;
}

/// Copies the simulator's device width and height to the given arguments.
/// @param widthPointer Pointer to the integer that the width will be copied to.
/// @param heightPointer Pointer to the integer that the height will be copied to.
void CMainFrame::CopyUnscaledDeviceWidthHeightTo(int *widthPointer, int *heightPointer)
{
	int width = 0;
	int height = 0;

	// Fetch the view.
	CSimulatorView *viewPointer = (CSimulatorView *)GetActiveView();
	if (NULL == viewPointer)
	{
		return;
	}

	// Fetch the skin's bitmap.
	Gdiplus::Bitmap *bitmapPointer = viewPointer->GetSkinBitmap();

	// Fetch the device's unscaled with and height.
	if (bitmapPointer)
	{
		// Get the skin's width and height.
		// Note: The simulator will rotate the bitmap. So, we do not need to flip width/height here.
		width = bitmapPointer->GetWidth();
		height = bitmapPointer->GetHeight();
	}
	else
	{
		// The simulator is in skinless mode.
		// Fetch the width and height from the simulated device configuration.
		const Rtt::PlatformSimulator::Config &deviceConfig = viewPointer->GetDeviceConfig();
		if (deviceConfig.supportsScreenRotation)
		{
			bool isPortrait = ((viewPointer->GetRotation() == 0) || (viewPointer->GetRotation() == 180));
			width = (int)((isPortrait ? deviceConfig.screenWidth : deviceConfig.screenHeight) + 0.5f);
			height = (int)((isPortrait ? deviceConfig.screenHeight : deviceConfig.screenWidth) + 0.5f);
		}
		else
		{
			bool isPortrait = deviceConfig.isUprightOrientationPortrait;
			width = (int)((isPortrait ? deviceConfig.screenWidth : deviceConfig.screenHeight) + 0.5f);
			height = (int)((isPortrait ? deviceConfig.screenHeight :  deviceConfig.screenWidth) + 0.5f);
		}
	}

	// Copy the width and height to the given arguments.
	if (widthPointer)
	{
		*widthPointer = width;
	}
	if (heightPointer)
	{
		*heightPointer = height;
	}
}

// CalcZoomFactor - multiply initial skin size by 2^zoom
float CMainFrame::CalcZoomFactor()
{
    return pow( 2.0f, GetZoom() );
}

// AdjustZoom - take requested window size and adjust zoom factor if it is
// too big or too small.  Used from CSimulatorView::UpdateSimulatorSkin, since
// some skin images are bigger than the vertical screen size allows.  In that
// case, zoom is initially set to -1.
void CMainFrame::AdjustZoom( int width, int height )
{
	if ((width <= 0) || (height <= 0))
	{
		return;
	}

	int zoomFactor = GetZoom();
    float factor = CalcZoomFactor();

	int nNewWidth = CalculateNewWidth(factor, (float)width);
	int nNewHeight = CalculateNewHeight(factor, (float)height);

    CPoint ptMin = GetMinTrackSize();

    int nZoomX = 0, nZoomY = 0;

    if ((nNewWidth < ptMin.x) || (nNewHeight < ptMin.y))
	{
		nZoomX = (int)ceil( log2f( (float) ptMin.x / (float) width ));
		nZoomY = (int)ceil( log2f( (float) ptMin.y / (float) height ));
		int zoom = Rtt::Max( nZoomX, nZoomY );
		SetZoom( zoom );
	}
}

// SizeToClient - resize the window to have the requested client size.
// CalcWindowRect is not reliable, so do the calculations here
// Don't redraw until later to avoid flickering (FALSE arg to MoveWindow).
// Return previous window rect, to be erased/redrawn later
CRect CMainFrame::SizeToClient( CRect rectNew )
{
	CRect rectWindow, rectClient;
    GetWindowRect( rectWindow );
    GetClientRect( rectClient );

    CRect rectPrevious = rectWindow;

	// Calculate non-client width & height (menu, frame, titlebar)
    int ncWidth = rectWindow.Width() - rectClient.Width();
    int ncHeight = rectWindow.Height() - rectClient.Height();

    rectWindow.right = rectWindow.left + ncWidth + rectNew.Width();
    rectWindow.bottom = rectWindow.top + ncHeight + rectNew.Height();

	MoveWindow( rectWindow, FALSE );

    GetWindowRect( rectWindow );
    GetClientRect( rectClient );

	// Again, calculate non-client width & height (menu, frame, titlebar)
    int ncNewWidth = rectWindow.Width() - rectClient.Width();
    int ncNewHeight = rectWindow.Height() - rectClient.Height();

    // If non-client area changed size (menu on 2 lines), try again
    if ((ncNewHeight != ncHeight) || (ncNewWidth != ncWidth))
	{
        // return largest possible previous rect to be erased/redrawn later
        rectPrevious.UnionRect( rectPrevious, rectWindow );

		rectWindow.right = rectWindow.left + ncNewWidth + rectNew.Width();
		rectWindow.bottom = rectWindow.top + ncNewHeight + rectNew.Height();

		MoveWindow( rectWindow, FALSE );
	}

    return rectPrevious;
}

int CMainFrame::CalculateNewHeight(float factor, float height)
{
	CRect rectWindow, rectClient;
	GetWindowRect( rectWindow );
	GetClientRect( rectClient );

	// Add non-client area of window to calculations
	// This ignores that small window sizes have extra menu height
	int ncHeight = rectWindow.Height() - rectClient.Height();
	int nNewHeight = (int)floor(factor * height) + ncHeight;

	return nNewHeight;
}

int CMainFrame::CalculateNewWidth(float factor, float width)
{
	CRect rectWindow, rectClient;
	GetWindowRect( rectWindow );
	GetClientRect( rectClient );

	// Add non-client area of window to calculations
	// This ignores that small window sizes have extra menu height
	int ncWidth = rectWindow.Width() - rectClient.Width();
	int nNewWidth = (int)floor(factor * width) + ncWidth;

	return nNewWidth;
}
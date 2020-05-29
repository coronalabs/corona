//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"


// CustomDeviceDlg dialog

class CustomDeviceDlg : public CDialog
{
	DECLARE_DYNAMIC(CustomDeviceDlg)

public:
	CustomDeviceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CustomDeviceDlg();

	BOOL CustomDeviceDlg::OnInitDialog();
	void OnOK();

	CString GetCustomDeviceName() { return mCustomDeviceName; };
	CString GetCustomDevicePlatform() { return mCustomDevicePlatform; };
	int GetCustomDeviceWidth() { return mCustomDeviceWidth; };
	int GetCustomDeviceHeight() { return mCustomDeviceHeight; };

	void SetCustomDeviceName(CString value) { mCustomDeviceName = value; };
	void SetCustomDevicePlatform(CString value) { mCustomDevicePlatform = value; };
	void SetCustomDeviceWidth(int value) { mCustomDeviceWidth = value; };
	void SetCustomDeviceHeight(int value) { mCustomDeviceHeight = value; };

// Dialog Data
	enum { IDD = IDD_CUSTOM_DEVICE };

protected:
	CString mCustomDeviceName;
	int mCustomDeviceWidth;
	int mCustomDeviceHeight;
	CString mCustomDevicePlatform;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnEnChangeCdDeviceName();
	afx_msg void OnEnChangeCdScreenWidth();
	afx_msg void OnEnChangeCdScreenHeight();
	afx_msg void OnCbnSelchangeCdPlatform();
};

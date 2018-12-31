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
#include "resource.h"
#include "Simulator.h"
#include "CustomDeviceDlg.h"

// Keep these two arrays in sync
LPCTSTR CustomDevicePlatformNames[] = {
	L"Android",
	L"iOS",
	L"macOS",
	L"tvOS",
	L"Windows",
	L"Windows Phone",
	NULL
};

LPCTSTR CustomDevicePlatformTags[] = {
	L"android",
	L"ios",
	L"macos",
	L"tvos",
	L"win32",
	L"winphone",
	NULL
};


// CustomDeviceDlg dialog

IMPLEMENT_DYNAMIC(CustomDeviceDlg, CDialog)

CustomDeviceDlg::CustomDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CustomDeviceDlg::IDD, pParent)
{

}

CustomDeviceDlg::~CustomDeviceDlg()
{
}

BOOL CustomDeviceDlg::OnInitDialog()
{
	if (mCustomDeviceName.IsEmpty())
	{
		// If we have no settings already, load previous settings (if any) from the registry
		SetCustomDeviceName(AfxGetApp()->GetProfileStringW(REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_NAME, REGISTRY_CD_NAME_DEFAULT));
		SetCustomDevicePlatform(AfxGetApp()->GetProfileStringW(REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_PLATFORM, REGISTRY_CD_PLATFORM_DEFAULT));
		SetCustomDeviceWidth(AfxGetApp()->GetProfileInt(REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_WIDTH, REGISTRY_CD_WIDTH_DEFAULT));
		SetCustomDeviceHeight(AfxGetApp()->GetProfileInt(REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_HEIGHT, REGISTRY_CD_HEIGHT_DEFAULT));
	}

	SetDlgItemText(IDC_CD_DEVICE_NAME, mCustomDeviceName);
	SetDlgItemInt(IDC_CD_SCREEN_WIDTH, mCustomDeviceWidth);
	SetDlgItemInt(IDC_CD_SCREEN_HEIGHT, mCustomDeviceHeight);

	CComboBox *platformComboBox = (CComboBox*)GetDlgItem(IDC_CD_PLATFORM);
	ASSERT(platformComboBox != NULL);
	ASSERT((sizeof(CustomDevicePlatformNames) / sizeof(LPCTSTR)) == (sizeof(CustomDevicePlatformTags) / sizeof(LPCTSTR)));

	int platformNum = 0;
	while (CustomDevicePlatformNames[platformNum] != NULL)
	{
		int platformBoxIndex = platformComboBox->AddString(CustomDevicePlatformNames[platformNum]);
		if (platformBoxIndex >= 0)
		{
			platformComboBox->SetItemDataPtr(platformBoxIndex, (void*)platformNum);

			if (mCustomDevicePlatform == CustomDevicePlatformTags[platformNum])
			{
				platformComboBox->SetCurSel(platformNum);
			}
		}

		++platformNum;
	}

	return TRUE;
}

void CustomDeviceDlg::OnOK()
{
	printf("CustomDeviceDlg::OnOK\n");

	AfxGetApp()->WriteProfileString( REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_NAME, GetCustomDeviceName());
	AfxGetApp()->WriteProfileString(REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_PLATFORM, GetCustomDevicePlatform());
	AfxGetApp()->WriteProfileInt(REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_WIDTH, GetCustomDeviceWidth());
	AfxGetApp()->WriteProfileInt(REGISTRY_SECTION, REGISTRY_CUSTOM_DEVICE_HEIGHT, GetCustomDeviceHeight());

	CDialog::OnOK();
}

BEGIN_MESSAGE_MAP(CustomDeviceDlg, CDialog)
	ON_EN_CHANGE(IDC_CD_DEVICE_NAME, &CustomDeviceDlg::OnEnChangeCdDeviceName)
	ON_EN_CHANGE(IDC_CD_SCREEN_WIDTH, &CustomDeviceDlg::OnEnChangeCdScreenWidth)
	ON_EN_CHANGE(IDC_CD_SCREEN_HEIGHT, &CustomDeviceDlg::OnEnChangeCdScreenHeight)
	ON_CBN_SELCHANGE(IDC_CD_PLATFORM, &CustomDeviceDlg::OnCbnSelchangeCdPlatform)
END_MESSAGE_MAP()


// CustomDeviceDlg message handlers

void CustomDeviceDlg::OnEnChangeCdDeviceName()
{
	 GetDlgItemText(IDC_CD_DEVICE_NAME, mCustomDeviceName);
}

void CustomDeviceDlg::OnEnChangeCdScreenWidth()
{
	mCustomDeviceWidth = GetDlgItemInt(IDC_CD_SCREEN_WIDTH);
}

void CustomDeviceDlg::OnEnChangeCdScreenHeight()
{
	mCustomDeviceHeight = GetDlgItemInt(IDC_CD_SCREEN_HEIGHT);
}

void CustomDeviceDlg::OnCbnSelchangeCdPlatform()
{
	CComboBox *platformComboBox = (CComboBox*)GetDlgItem(IDC_CD_PLATFORM);
	int currSelection = platformComboBox->GetCurSel();

	mCustomDevicePlatform = CustomDevicePlatformTags[currSelection];
}

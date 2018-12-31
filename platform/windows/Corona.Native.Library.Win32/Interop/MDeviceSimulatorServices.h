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

#include "Rtt_DeviceOrientation.h"
#include "Rtt_MPlatform.h"


#pragma region Forward Declarations
namespace Rtt
{
	class LuaResource;
}

#pragma endregion


namespace Interop {

class MDeviceSimulatorServices
{
	public:
		virtual ~MDeviceSimulatorServices() {}

		virtual const char* GetManufacturerName() const = 0;
		virtual const char* GetModelName() const = 0;
		virtual bool IsLuaExitAllowed() const = 0;
		virtual bool IsScreenRotationSupported() const = 0;
		virtual bool IsMouseSupported() const = 0;
		virtual bool AreInputDevicesSupported() const = 0;
		virtual bool AreKeyEventsSupported() const = 0;
		virtual bool AreKeyEventsFromKeyboardSupported() const = 0;
		virtual bool IsBackKeySupported() const = 0;
		virtual bool AreExitRequestsSupported() const = 0;
		virtual bool AreMultipleAlertsSupported() const = 0;
		virtual bool IsAlertButtonOrderRightToLeft() const = 0;
		virtual double GetZoomScale() const = 0;
		virtual Rtt::DeviceOrientation::Type GetOrientation() const = 0;
		virtual void SetOrientation(Rtt::DeviceOrientation::Type value) = 0;
		virtual POINT GetSimulatedPointFromClient(const POINT& value) = 0;
		virtual double GetDefaultFontSize() const = 0;
		virtual int GetScreenWidthInPixels() const = 0;
		virtual int GetScreenHeightInPixels() const = 0;
		virtual int GetAdaptiveScreenWidthInPixels() const = 0;
		virtual int GetAdaptiveScreenHeightInPixels() const = 0;
		virtual const char* GetStatusBarImageFilePathFor(Rtt::MPlatform::StatusBarMode value) const = 0;
		virtual const char* GetAuthorizationTicketString() const = 0;
		virtual void RotateClockwise() = 0;
		virtual void RotateCounterClockwise() = 0;
		virtual void* ShowNativeAlert(
						const char *title, const char *message, const char **buttonLabels,
						int buttonCount, Rtt::LuaResource* resource) = 0;
		virtual void CancelNativeAlert(void* alertReference) = 0;
		virtual void SetActivityIndicatorVisible(bool value) = 0;
		virtual void RequestRestart() = 0;
		virtual void RequestTerminate() = 0;
		virtual void Shake() = 0;
		virtual const char* GetOSName() const = 0;
		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const = 0;
		virtual Rtt::MPlatform::StatusBarMode GetStatusBar() const = 0;
		virtual void SetStatusBar(Rtt::MPlatform::StatusBarMode newValue) = 0;

};

}	// namespace Interop

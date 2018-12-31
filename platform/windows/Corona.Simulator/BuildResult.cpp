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

#include "StdAfx.h"
#include "BuildResult.h"
#include "Core/Rtt_Build.h"
#include "Rtt_WebServicesSession.h"


#pragma region Constructor/Destructor
/// Creates a result object indicating if a build succeeded or failed.
/// @param errorCode Unique integer ID indicating the result of the build.
///                  Must be assigned a value from the enum in class WebServicesSession.
///                  Assign it to "Rtt::WebServicesSession::kNoError" if the build succeeded.
/// @param statusMessage Localized message to be displayed to the user. If the build failed,
///                      then this message should indicate exactly what went wrong.
CBuildResult::CBuildResult(int errorCode, CString& statusMessage)
{
	fErrorCode = errorCode;
	fStatusMessage = statusMessage;
}

/// Destructor.
CBuildResult::~CBuildResult()
{
}

#pragma endregion


#pragma region Public Functions
/// Determines if the build has succeeded.
/// @return Returns true if the app build succeeded. Returns false if the build failed.
bool CBuildResult::HasSucceeded()
{
	return (fErrorCode == Rtt::WebServicesSession::kNoError);
}

/// Determines if the build has failed.
/// @return Returns true if the build failed. Returns true if the build succeeded.
bool CBuildResult::HasFailed()
{
	return !HasSucceeded();
}

/// Gets the unique integer ID indicating what specifically went wrong during the build.
/// To be called if this object's HasSucceeded() or HasFailed() function indicate a failure.
/// @return Returns an integer ID matching the enum in class WebServicesSession.
///         Returns "Rtt::WebServicesSession::kNoError" if the build succeeded.
///         Any other value returned indicates a specific failure code.
int CBuildResult::GetErrorCode()
{
	return fErrorCode;
}

/// Gets a localized status message providing details about the buld.
/// Expected to be called on failure to provide details on why the build failed.
/// @return Returns a localized string providing details about the build.
CString& CBuildResult::GetStatusMessage()
{
	return fStatusMessage;
}

#pragma endregion

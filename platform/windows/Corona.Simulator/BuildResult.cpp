//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BuildResult.h"
#include "Core/Rtt_Build.h"

#pragma region Constructor/Destructor
/// Creates a result object indicating if a build succeeded or failed.
/// @param errorCode Unique integer ID indicating the result of the build.
///                  Must be assigned a value from the enum in class WebServicesSession.
///                  Assign it to "0" if the build succeeded.
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
	return (fErrorCode == 0);
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
///         Returns "0" if the build succeeded.
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

//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


/// Result object indicating if an app build succeeded or failed. Provides addition detail
/// if the build failed, such as a numeric error code and a status message.
/// Instances of this class are immutable.
class CBuildResult
{
public:
	CBuildResult(int errorCode, CString& statusMessage);
	virtual ~CBuildResult();

	bool HasSucceeded();
	bool HasFailed();
	int GetErrorCode();
	CString& GetStatusMessage();

private:
	int fErrorCode;
	CString fStatusMessage;
};

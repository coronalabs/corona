//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

class CBrowseDirDialog
{
public:
	// Browses for directory. First parameter is default directory if not empty
	bool static Browse(CString &dir, UINT titleId = 0);
private:
	static int CALLBACK DefaultDirCallback(HWND, UINT, LPARAM, LPARAM);
};


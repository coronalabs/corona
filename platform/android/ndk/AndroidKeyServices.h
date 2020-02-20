//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidKeyServices_H__
#define _AndroidKeyServices_H__


class AndroidKeyServices
{
	private:
		AndroidKeyServices();

	public:
		static const char* GetNameFromKeyCode(int keyCode);
		static bool IsKeyCodeKnown(int keyCode);
};

#endif // _AndroidKeyServices_H__

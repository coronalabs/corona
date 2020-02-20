//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidFontSettings_H__
#define _AndroidFontSettings_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_String.h"


/// Stores font configuration such as font name, bold, size, etc.
class AndroidFontSettings
{
	public:
		AndroidFontSettings(Rtt_Allocator *allocatorPointer);
		virtual ~AndroidFontSettings();

		void SetName(const char *name);
		const char* GetName() const;
		void SetSize(float size);
		float GetSize() const;
		void SetIsBold(bool value);
		bool IsBold() const;

	private:
		Rtt::String fName;
		float fSize;
		bool fIsBold;
};

#endif // _AndroidFontSettings_H__

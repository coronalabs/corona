//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VisualTheme.h"
#include "resource.h"
#include <exception>
#include <list>


#pragma region Static Member Variables
static std::list<VisualTheme*> sThemeCollection;

const VisualTheme VisualTheme::kSystem(ID_THEME_SYSTEM, _T("system"));
const VisualTheme VisualTheme::kDark(ID_THEME_DARK, _T("dark"));
const VisualTheme VisualTheme::kBlue(ID_THEME_BLUE, _T("blue"));

#pragma endregion


#pragma region Constructors/Destructors
VisualTheme::VisualTheme(const VisualTheme& theme)
{
	throw std::exception();
}

VisualTheme::VisualTheme(UINT resourceIntegerId, const TCHAR* stringId)
:	fResourceIntegerId(resourceIntegerId),
	fStringId(stringId)
{
	sThemeCollection.push_back(this);
}

VisualTheme::~VisualTheme()
{
}

#pragma endregion


#pragma region Public Methods
UINT VisualTheme::GetResourceIntegerId() const
{
	return fResourceIntegerId;
}

const TCHAR* VisualTheme::GetStringId() const
{
	return fStringId;
}

#pragma endregion


#pragma region Public Static Functions
const VisualTheme* VisualTheme::FromResourceIntegerId(UINT integerId)
{
	for (auto&& themePointer : sThemeCollection)
	{
		if (integerId == themePointer->fResourceIntegerId)
		{
			return themePointer;
		}
	}
	return nullptr;
}

const VisualTheme* VisualTheme::FromStringId(const TCHAR* stringId)
{
	if (stringId && stringId[0])
	{
		for (auto&& themePointer : sThemeCollection)
		{
			if (themePointer->fStringId.Compare(stringId) == 0)
			{
				return themePointer;
			}
		}
	}
	return nullptr;
}

#pragma endregion

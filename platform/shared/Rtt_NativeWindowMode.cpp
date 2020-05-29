//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_NativeWindowMode.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include <list>


namespace Rtt
{

typedef std::list<const NativeWindowMode*> NativeWindowModeCollection;
static NativeWindowModeCollection sWindowModeCollection;

const NativeWindowMode NativeWindowMode::kNormal("normal");
const NativeWindowMode NativeWindowMode::kMinimized("minimized");
const NativeWindowMode NativeWindowMode::kMaximized("maximized");
const NativeWindowMode NativeWindowMode::kFullscreen("fullscreen");


NativeWindowMode::NativeWindowMode()
{
}

NativeWindowMode::NativeWindowMode(const char *stringId)
:	fStringId(stringId)
{
	// Add this window mode to the global collection.
	sWindowModeCollection.push_back(this);
}

NativeWindowMode::~NativeWindowMode()
{
}

const char* NativeWindowMode::GetStringId() const
{
	return fStringId;
}

bool NativeWindowMode::Equals(const NativeWindowMode &value) const
{
	return (this == &value);
}

bool NativeWindowMode::NotEquals(const NativeWindowMode &value) const
{
	return (this != &value);
}

bool NativeWindowMode::operator==(const NativeWindowMode &value) const
{
	return this->Equals(value);
}

bool NativeWindowMode::operator!=(const NativeWindowMode &value) const
{
	return this->NotEquals(value);
}

const NativeWindowMode* NativeWindowMode::FromStringId(const char *stringId)
{
	// Attempt to fetch the given string ID's matching pre-defined window mode object.
	if (!Rtt_StringIsEmpty(stringId))
	{
		NativeWindowModeCollection::iterator iter;
		for (iter = sWindowModeCollection.begin(); iter != sWindowModeCollection.end(); iter++)
		{
			if (Rtt_StringCompare((*iter)->fStringId, stringId) == 0)
			{
				return *iter;
			}
		}
	}

	// The given string ID was not recognized.
	return NULL;
}

} // namespace Rtt

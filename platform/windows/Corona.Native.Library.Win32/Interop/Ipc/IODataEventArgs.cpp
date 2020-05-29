//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IODataEventArgs.h"


namespace Interop { namespace Ipc {

#pragma region Constructors/Destructors
IODataEventArgs::IODataEventArgs(const IODataEventArgs::Data& data)
:	fData(data)
{
}

IODataEventArgs::~IODataEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
DateTime IODataEventArgs::GetTimestamp() const
{
	return fData.Timestamp;
}

std::shared_ptr<const std::string> IODataEventArgs::GetText() const
{
	return fData.Text;
}

#pragma endregion

} }	// namespace Interop::Ipc

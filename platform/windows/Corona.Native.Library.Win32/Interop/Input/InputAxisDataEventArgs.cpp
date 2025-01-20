//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputAxisDataEventArgs.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputAxisDataEventArgs::InputAxisDataEventArgs(const InputAxisDataEventArgs::Data& data)
:	EventArgs(),
	fData(data)
{
}

InputAxisDataEventArgs::~InputAxisDataEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
unsigned int InputAxisDataEventArgs::GetAxisIndex() const
{
	return fData.AxisIndex;
}

int32_t InputAxisDataEventArgs::GetRawValue() const
{
	return fData.RawValue;
}

InputAxisDataEventArgs::Data InputAxisDataEventArgs::ToData() const
{
	return fData;
}

#pragma endregion


#pragma region Public Static Functions
InputAxisDataEventArgs InputAxisDataEventArgs::From(const InputAxisDataEventArgs::Data& data)
{
	return InputAxisDataEventArgs(data);
}

#pragma endregion

} }	// namespace Interop::Input

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

#include "Rtt_Preference.h"
#include "Core/Rtt_SharedStringPtr.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Public Constants
// ----------------------------------------------------------------------------

const char Preference::kCategoryNameApp[] = "app";
const char Preference::kCategoryNameLocale[] = "locale";
const char Preference::kCategoryNameSimulator[] = "simulator";
const char Preference::kCategoryNameUI[] = "ui";

	
// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

Preference::Preference(const char* keyName)
:	fKeyName(Rtt_MakeSharedConstStdStringPtr(keyName ? keyName : ""))
{
}

Preference::Preference(const Rtt::SharedConstStdStringPtr& keyName)
:	fKeyName(keyName.NotNull() ? keyName : Rtt_MakeSharedConstStdStringPtr(""))
{
}

Preference::Preference(const char* keyName, const Rtt::PreferenceValue& value)
:	fKeyName(Rtt_MakeSharedConstStdStringPtr(keyName ? keyName : "")),
	fValue(value)
{
}

Preference::Preference(const Rtt::SharedConstStdStringPtr& keyName, const Rtt::PreferenceValue& value)
:	fKeyName(keyName.NotNull() ? keyName : Rtt_MakeSharedConstStdStringPtr("")),
	fValue(value)
{
}

Preference::~Preference()
{
}


// ----------------------------------------------------------------------------
// Public Methods
// ----------------------------------------------------------------------------

const char* Preference::GetKeyName() const
{
	return fKeyName->c_str();
}

Rtt::SharedConstStdStringPtr Preference::GetKeyNameAsSharedPointer() const
{
	return fKeyName;
}

Rtt::PreferenceValue Preference::GetValue() const
{
	return fValue;
}

void Preference::SetValue(const Rtt::PreferenceValue& value)
{
	fValue = value;
}


// ----------------------------------------------------------------------------
// ReadValueResult Inner Class Members
// ----------------------------------------------------------------------------

const Preference::ReadValueResult Preference::ReadValueResult::kPreferenceNotFound(
		false, "Preference not found.", PreferenceValue(), true);

Preference::ReadValueResult::ReadValueResult(
	bool hasSucceeded, const char* utf8Message, PreferenceValue value, bool hadPreferenceNotFoundError)
:	ValueResult<PreferenceValue>(hasSucceeded, utf8Message, value),
	fHadPreferenceNotFoundError(hadPreferenceNotFoundError)
{
}

Preference::ReadValueResult::ReadValueResult(
	bool hasSucceeded, const Rtt::SharedConstStdStringPtr& utf8Message,
	PreferenceValue value, bool hadPreferenceNotFoundError)
:	ValueResult<PreferenceValue>(hasSucceeded, utf8Message, value),
	fHadPreferenceNotFoundError(hadPreferenceNotFoundError)
{
}

Preference::ReadValueResult::~ReadValueResult()
{
}

bool Preference::ReadValueResult::HadPreferenceNotFoundError() const
{
	return fHadPreferenceNotFoundError;
}

Preference::ReadValueResult Preference::ReadValueResult::SucceededWith(PreferenceValue value)
{
	return ReadValueResult(true, (const char*)NULL, value, false);
}

Preference::ReadValueResult Preference::ReadValueResult::FailedWith(const char* utf8Message)
{
	return ReadValueResult(false, utf8Message, PreferenceValue(), false);
}

Preference::ReadValueResult Preference::ReadValueResult::FailedWith(const Rtt::SharedConstStdStringPtr& utf8Message)
{
	return ReadValueResult(false, utf8Message, PreferenceValue(), false);
}

} // namespace Rtt

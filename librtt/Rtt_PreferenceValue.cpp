//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_PreferenceValue.h"
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_String.h"
#include <float.h>
#include <locale>
#include <math.h>
#include <stdint.h>
#include <sstream>
extern "C"
{
#	include "lua.h"
}


namespace Rtt
{

// ----------------------------------------------------------------------------
// TypeNameProvider Template Specializations
// ----------------------------------------------------------------------------

template<>
const char PreferenceValue::TypeNameProvider<bool>::kName[] = "boolean";

template<>
const char PreferenceValue::TypeNameProvider<S8>::kName[] = "8-bit signed integer";

template<>
const char PreferenceValue::TypeNameProvider<S16>::kName[] = "16-bit signed integer";

template<>
const char PreferenceValue::TypeNameProvider<S32>::kName[] = "32-bit signed integer";

template<>
const char PreferenceValue::TypeNameProvider<S64>::kName[] = "64-bit signed integer";

template<>
const char PreferenceValue::TypeNameProvider<U8>::kName[] = "8-bit unsigned integer";

template<>
const char PreferenceValue::TypeNameProvider<U16>::kName[] = "16-bit unsigned integer";

template<>
const char PreferenceValue::TypeNameProvider<U32>::kName[] = "32-bit unsigned integer";

template<>
const char PreferenceValue::TypeNameProvider<U64>::kName[] = "64-bit unsigned integer";

template<>
const char PreferenceValue::TypeNameProvider<float>::kName[] = "single precision float";

template<>
const char PreferenceValue::TypeNameProvider<double>::kName[] = "double precision float";


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

PreferenceValue::PreferenceValue()
:	fType(PreferenceValue::kTypeBoolean)
{
	fValue.Boolean = false;
}

PreferenceValue::PreferenceValue(bool value)
:	fType(PreferenceValue::kTypeBoolean)
{
	fValue.Boolean = value;
}

PreferenceValue::PreferenceValue(S8 value)
:	fType(PreferenceValue::kTypeSignedInt8)
{
	fValue.SignedInt8 = value;
}

PreferenceValue::PreferenceValue(S16 value)
:	fType(PreferenceValue::kTypeSignedInt16)
{
	fValue.SignedInt16 = value;
}

PreferenceValue::PreferenceValue(S32 value)
:	fType(PreferenceValue::kTypeSignedInt32)
{
	fValue.SignedInt32 = value;
}

PreferenceValue::PreferenceValue(S64 value)
:	fType(PreferenceValue::kTypeSignedInt64)
{
	fValue.SignedInt64 = value;
}

PreferenceValue::PreferenceValue(U8 value)
:	fType(PreferenceValue::kTypeUnsignedInt8)
{
	fValue.UnsignedInt8 = value;
}

PreferenceValue::PreferenceValue(U16 value)
:	fType(PreferenceValue::kTypeUnsignedInt16)
{
	fValue.UnsignedInt16 = value;
}

PreferenceValue::PreferenceValue(U32 value)
:	fType(PreferenceValue::kTypeUnsignedInt32)
{
	fValue.UnsignedInt32 = value;
}

PreferenceValue::PreferenceValue(U64 value)
:	fType(PreferenceValue::kTypeUnsignedInt64)
{
	fValue.UnsignedInt64 = value;
}

PreferenceValue::PreferenceValue(float value)
:	fType(PreferenceValue::kTypeFloatSingle)
{
	fValue.FloatSingle = value;
}

PreferenceValue::PreferenceValue(double value)
:	fType(PreferenceValue::kTypeFloatDouble)
{
	fValue.FloatDouble = value;
}

PreferenceValue::PreferenceValue(const char* utf8String)
:	fType(PreferenceValue::kTypeString)
{
	if (!utf8String)
	{
		utf8String = "";
	}
	fStringPointer = Rtt_MakeSharedConstStdStringPtr(utf8String);
}

PreferenceValue::PreferenceValue(const Rtt::SharedConstStdStringPtr& utf8String)
:	fType(PreferenceValue::kTypeString),
	fStringPointer(utf8String.NotNull() ? utf8String : Rtt_MakeSharedConstStdStringPtr(""))
{
}

PreferenceValue::~PreferenceValue()
{
}


// ----------------------------------------------------------------------------
// Private Template Functions
// Note: These must be defined before the non-template method definitions.
// ----------------------------------------------------------------------------

template<typename TValueType, typename TRangeType>
static bool IsSignedIntInRange(TValueType value)
{
	if (std::numeric_limits<TRangeType>::is_integer)
	{
		if (std::numeric_limits<TRangeType>::is_signed)
		{
			if (std::numeric_limits<TRangeType>::digits < std::numeric_limits<TValueType>::digits)
			{
				if ((value < (TValueType)std::numeric_limits<TRangeType>::min()) ||
				    (value > (TValueType)std::numeric_limits<TRangeType>::max()))
				{
					return false;
				}
			}
		}
		else
		{
			if (value < 0)
			{
				return false;
			}
			else if (std::numeric_limits<TRangeType>::digits < std::numeric_limits<TValueType>::digits)
			{
				if (value > (TValueType)std::numeric_limits<TRangeType>::max())
				{
					return false;
				}
			}
		}
	}
	else
	{
		if (std::numeric_limits<TRangeType>::digits10 < std::numeric_limits<TValueType>::digits10)
		{
			TValueType maxValue = 0;
			for (int digitCount = std::numeric_limits<TRangeType>::digits10; digitCount > 0; digitCount--)
			{
				maxValue = (maxValue * 10) + 9;
			}
			TValueType minValue = -maxValue;
			if ((value < minValue) || (value > maxValue))
			{
				return false;
			}
		}
	}
	return true;
}

template<typename TValueType, typename TRangeType>
static bool IsUnsignedIntInRange(TValueType value)
{
	if (std::numeric_limits<TRangeType>::is_integer)
	{
		if (std::numeric_limits<TRangeType>::digits < std::numeric_limits<TValueType>::digits)
		{
			if (value > (TValueType)std::numeric_limits<TRangeType>::max())
			{
				return false;
			}
		}
	}
	else
	{
		if (std::numeric_limits<TRangeType>::digits10 < std::numeric_limits<TValueType>::digits10)
		{
			TValueType maxValue = 0;
			for (int digitCount = std::numeric_limits<TRangeType>::digits10; digitCount > 0; digitCount--)
			{
				maxValue = (maxValue * 10) + 9;
			}
			if (value > maxValue)
			{
				return false;
			}
		}
	}
	return true;
}

template<typename TValue, class TTypeNameProvider>
Rtt::ValueResult<TValue> PreferenceValue::ToNumber() const
{
	switch (fType)
	{
		case PreferenceValue::kTypeBoolean:
		{
			return ValueResult<TValue>::SucceededWith(fValue.Boolean ? (TValue)1 : (TValue)0);
		}
		case PreferenceValue::kTypeSignedInt8:
		{
			if (IsSignedIntInRange<S8, TValue>(fValue.SignedInt8))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.SignedInt8);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeSignedInt16:
		{
			if (IsSignedIntInRange<S16, TValue>(fValue.SignedInt16))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.SignedInt16);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeSignedInt32:
		{
			if (IsSignedIntInRange<S32, TValue>(fValue.SignedInt32))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.SignedInt32);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeSignedInt64:
		{
			if (IsSignedIntInRange<S64, TValue>(fValue.SignedInt64))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.SignedInt64);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeUnsignedInt8:
		{
			if (IsUnsignedIntInRange<U8, TValue>(fValue.UnsignedInt8))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.UnsignedInt8);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeUnsignedInt16:
		{
			if (IsUnsignedIntInRange<U16, TValue>(fValue.UnsignedInt16))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.UnsignedInt16);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeUnsignedInt32:
		{
			if (IsUnsignedIntInRange<U32, TValue>(fValue.UnsignedInt32))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.UnsignedInt32);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeUnsignedInt64:
		{
			if (IsUnsignedIntInRange<U64, TValue>(fValue.UnsignedInt64))
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.UnsignedInt64);
			}
			return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
		}
		case PreferenceValue::kTypeFloatSingle:
		{
			if (std::numeric_limits<TValue>::is_integer)
			{
				// Round the float to the nearest integer and make sure it doesn't exceeds the integer's limits.
				// Note: If the compiler does not support C99 (such as WP8), then we have to round it ourselves.
#if (defined(_MSC_VER) && (_MSC_VER < 1800)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ < 199901L))
				float roundedValue = fValue.FloatSingle;
				if (roundedValue >= 0.0f)
				{
					roundedValue = floorf(roundedValue + 0.5f);
				}
				else
				{
					roundedValue = ceilf(roundedValue - 0.5f);
				}
#else
				float roundedValue = roundf(fValue.FloatSingle);
#endif
				if ((roundedValue >= (float)std::numeric_limits<TValue>::min()) &&
				    (roundedValue <= (float)std::numeric_limits<TValue>::max()))
				{
					return ValueResult<TValue>::SucceededWith((TValue)roundedValue);
				}
				return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
			}
			else
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.FloatSingle);
			}
		}
		case PreferenceValue::kTypeFloatDouble:
		{
			if (std::numeric_limits<TValue>::is_integer)
			{
				// Round the float to the nearest integer and make sure it doesn't exceeds the integer's limits.
				// Note: If the compiler does not support C99 (such as WP8), then we have to round it ourselves.
#if (defined(_MSC_VER) && (_MSC_VER < 1800)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ < 199901L))
				double roundedValue = fValue.FloatDouble;
				if (roundedValue >= 0.0)
				{
					roundedValue = floor(roundedValue + 0.5);
				}
				else
				{
					roundedValue = ceil(roundedValue - 0.5);
				}
#else
				double roundedValue = round(fValue.FloatDouble);
#endif
				if ((roundedValue >= (double)std::numeric_limits<TValue>::min()) &&
				    (roundedValue <= (double)std::numeric_limits<TValue>::max()))
				{
					return ValueResult<TValue>::SucceededWith((TValue)roundedValue);
				}
				return ValueResult<TValue>::FailedWith(CreateOutOfBoundsMessageForType(TTypeNameProvider::kName));
			}
			else
			{
				return ValueResult<TValue>::SucceededWith((TValue)fValue.FloatDouble);
			}
		}
		case PreferenceValue::kTypeString:
		{
			TValue value = 0;
			if (fStringPointer.NotNull() && !fStringPointer->empty())
			{
				std::stringstream stringStream;
				stringStream.imbue(std::locale::classic());
				stringStream << *fStringPointer;
				stringStream >> value;
				if (stringStream.fail())
				{
					return ValueResult<TValue>::FailedWith(
							CreateCannotConvertStringMessageForType(TTypeNameProvider::kName));
				}
			}
			return ValueResult<TValue>::SucceededWith(value);
		}
	}
	Rtt_ASSERT(0);
	return ValueResult<TValue>::FailedWith(CreateCannotConvertUnknownValueMessageForType(TTypeNameProvider::kName));
}

template<typename TValue>
Rtt::ValueResult<TValue> PreferenceValue::ToNumber() const
{
	return ToNumber< TValue, TypeNameProvider<TValue> >();
}

template<typename TValue>
static PreferenceValue::StringResult StringResultFromNumber(TValue value, const std::locale& locale)
{
	std::stringstream stringStream;
	stringStream.imbue(locale);
	stringStream << value;
	std::string stringResult = stringStream.str();
	if (stringResult.c_str() == NULL)
	{
		stringResult = "";
	}
	return PreferenceValue::StringResult::SucceededWith(Rtt_MakeSharedConstStdStringPtr(stringResult));
}

template<typename TValue>
static Rtt::ValueResult<PreferenceValue> PreferenceValueResultFrom(const Rtt::ValueResult<TValue>& result)
{
	if (result.HasFailed())
	{
		return ValueResult<PreferenceValue>::FailedWith(result.GetUtf8MessageAsSharedPointer());
	}
	return ValueResult<PreferenceValue>::SucceededWith(PreferenceValue(result.GetValue()));
}


// ----------------------------------------------------------------------------
// Public Methods
// ----------------------------------------------------------------------------

PreferenceValue::Type PreferenceValue::GetType() const
{
	return fType;
}

Rtt::ValueResult<bool> PreferenceValue::ToBoolean() const
{
	switch (fType)
	{
		case PreferenceValue::kTypeBoolean:
		{
			return ValueResult<bool>::SucceededWith(fValue.Boolean);
		}
		case PreferenceValue::kTypeSignedInt8:
		{
			return ValueResult<bool>::SucceededWith(fValue.SignedInt8 != 0);
		}
		case PreferenceValue::kTypeSignedInt16:
		{
			return ValueResult<bool>::SucceededWith(fValue.SignedInt16 != 0);
		}
		case PreferenceValue::kTypeSignedInt32:
		{
			return ValueResult<bool>::SucceededWith(fValue.SignedInt32 != 0);
		}
		case PreferenceValue::kTypeSignedInt64:
		{
			return ValueResult<bool>::SucceededWith(fValue.SignedInt64 != 0);
		}
		case PreferenceValue::kTypeUnsignedInt8:
		{
			return ValueResult<bool>::SucceededWith(fValue.UnsignedInt8 != 0);
		}
		case PreferenceValue::kTypeUnsignedInt16:
		{
			return ValueResult<bool>::SucceededWith(fValue.UnsignedInt16 != 0);
		}
		case PreferenceValue::kTypeUnsignedInt32:
		{
			return ValueResult<bool>::SucceededWith(fValue.UnsignedInt32 != 0);
		}
		case PreferenceValue::kTypeUnsignedInt64:
		{
			return ValueResult<bool>::SucceededWith(fValue.UnsignedInt64 != 0);
		}
		case PreferenceValue::kTypeFloatSingle:
		{
			bool isNonZero = (fValue.FloatSingle > FLT_EPSILON) || (fValue.FloatSingle < -FLT_EPSILON);
			return ValueResult<bool>::SucceededWith(isNonZero);
		}
		case PreferenceValue::kTypeFloatDouble:
		{
			bool isNonZero = (fValue.FloatDouble > DBL_EPSILON) || (fValue.FloatDouble < -DBL_EPSILON);
			return ValueResult<bool>::SucceededWith(isNonZero);
		}
		case PreferenceValue::kTypeString:
		{
			if (fStringPointer.NotNull() && !fStringPointer->empty())
			{
				if (!Rtt_StringCompareNoCase(fStringPointer->c_str(), "true") ||
				    !Rtt_StringCompareNoCase(fStringPointer->c_str(), "yes"))
				{
					return ValueResult<bool>::SucceededWith(true);
				}
				else if (!Rtt_StringCompareNoCase(fStringPointer->c_str(), "false") ||
				         !Rtt_StringCompareNoCase(fStringPointer->c_str(), "no"))
				{
					return ValueResult<bool>::SucceededWith(false);
				}
			}
			return ValueResult<bool>::FailedWith(CreateCannotConvertStringMessageForType(TypeNameProvider<bool>::kName));
		}
		default:
			break;
	}
	Rtt_ASSERT(0);
	return ValueResult<bool>::FailedWith(CreateCannotConvertUnknownValueMessageForType(TypeNameProvider<bool>::kName));
}

Rtt::ValueResult<S8> PreferenceValue::ToSignedInt8() const
{
	return ToNumber<S8>();
}

Rtt::ValueResult<S16> PreferenceValue::ToSignedInt16() const
{
	return ToNumber<S16>();
}

Rtt::ValueResult<S32> PreferenceValue::ToSignedInt32() const
{
	return ToNumber<S32>();
}

Rtt::ValueResult<S64> PreferenceValue::ToSignedInt64() const
{
	return ToNumber<S64>();
}

Rtt::ValueResult<U8> PreferenceValue::ToUnsignedInt8() const
{
	return ToNumber<U8>();
}

Rtt::ValueResult<U16> PreferenceValue::ToUnsignedInt16() const
{
	return ToNumber<U16>();
}

Rtt::ValueResult<U32> PreferenceValue::ToUnsignedInt32() const
{
	return ToNumber<U32>();
}

Rtt::ValueResult<U64> PreferenceValue::ToUnsignedInt64() const
{
	return ToNumber<U64>();
}

Rtt::ValueResult<float> PreferenceValue::ToFloatSingle() const
{
	return ToNumber<float>();
}

Rtt::ValueResult<double> PreferenceValue::ToFloatDouble() const
{
	return ToNumber<double>();
}

PreferenceValue::StringResult PreferenceValue::ToString() const
{
	return ToStringWithLocale(std::locale::classic());
}

PreferenceValue::StringResult PreferenceValue::ToStringWithLocale(const std::locale& locale) const
{
	switch (fType)
	{
		case PreferenceValue::kTypeBoolean:
		{
			const char* valueString = fValue.Boolean ? "true" : "false";
			return StringResult::SucceededWith(Rtt_MakeSharedConstStdStringPtr(valueString));
		}
		case PreferenceValue::kTypeSignedInt8:
		{
			return StringResultFromNumber<S8>(fValue.SignedInt8, locale);
		}
		case PreferenceValue::kTypeSignedInt16:
		{
			return StringResultFromNumber<S16>(fValue.SignedInt16, locale);
		}
		case PreferenceValue::kTypeSignedInt32:
		{
			return StringResultFromNumber<S32>(fValue.SignedInt32, locale);
		}
		case PreferenceValue::kTypeSignedInt64:
		{
			return StringResultFromNumber<S64>(fValue.SignedInt64, locale);
		}
		case PreferenceValue::kTypeUnsignedInt8:
		{
			return StringResultFromNumber<U8>(fValue.UnsignedInt8, locale);
		}
		case PreferenceValue::kTypeUnsignedInt16:
		{
			return StringResultFromNumber<U16>(fValue.UnsignedInt16, locale);
		}
		case PreferenceValue::kTypeUnsignedInt32:
		{
			return StringResultFromNumber<U32>(fValue.UnsignedInt32, locale);
		}
		case PreferenceValue::kTypeUnsignedInt64:
		{
			return StringResultFromNumber<U64>(fValue.UnsignedInt64, locale);
		}
		case PreferenceValue::kTypeFloatSingle:
		{
			return StringResultFromNumber<float>(fValue.FloatSingle, locale);
		}
		case PreferenceValue::kTypeFloatDouble:
		{
			return StringResultFromNumber<double>(fValue.FloatDouble, locale);
		}
		case PreferenceValue::kTypeString:
		{
			return StringResult::SucceededWith(fStringPointer);
		}
	}
	Rtt_ASSERT(0);
	return StringResult::FailedWith(CreateCannotConvertUnknownValueMessageForType("string"));
}

Rtt::ValueResult<PreferenceValue> PreferenceValue::ToValueType(PreferenceValue::Type type) const
{
	switch (type)
	{
		case PreferenceValue::kTypeBoolean:
		{
			return PreferenceValueResultFrom<bool>(ToBoolean());
		}
		case PreferenceValue::kTypeSignedInt8:
		{
			return PreferenceValueResultFrom<S8>(ToSignedInt8());
		}
		case PreferenceValue::kTypeSignedInt16:
		{
			return PreferenceValueResultFrom<S16>(ToSignedInt16());
		}
		case PreferenceValue::kTypeSignedInt32:
		{
			return PreferenceValueResultFrom<S32>(ToSignedInt32());
		}
		case PreferenceValue::kTypeSignedInt64:
		{
			return PreferenceValueResultFrom<S64>(ToSignedInt64());
		}
		case PreferenceValue::kTypeUnsignedInt8:
		{
			return PreferenceValueResultFrom<U8>(ToUnsignedInt8());
		}
		case PreferenceValue::kTypeUnsignedInt16:
		{
			return PreferenceValueResultFrom<U16>(ToUnsignedInt16());
		}
		case PreferenceValue::kTypeUnsignedInt32:
		{
			return PreferenceValueResultFrom<U32>(ToUnsignedInt32());
		}
		case PreferenceValue::kTypeUnsignedInt64:
		{
			return PreferenceValueResultFrom<U64>(ToUnsignedInt64());
		}
		case PreferenceValue::kTypeFloatSingle:
		{
			return PreferenceValueResultFrom<float>(ToFloatSingle());
		}
		case PreferenceValue::kTypeFloatDouble:
		{
			return PreferenceValueResultFrom<double>(ToFloatDouble());
		}
		case PreferenceValue::kTypeString:
		{
			return PreferenceValueResultFrom<Rtt::SharedConstStdStringPtr>(ToString());
		}
		default:
		{
			break;
		}
	}
	Rtt_ASSERT(0);
	const char kMessage[] = "PreferenceValue::ToValueType() method was given an unknown type to convert to.";
	return ValueResult<PreferenceValue>::FailedWith(Rtt_MakeSharedConstStdStringPtr(kMessage));
}

Rtt::ValueResult<PreferenceValue> PreferenceValue::ToClosestValueTypeIn(const PreferenceValue::TypeSet& typeSet) const
{
	// Do not continue if given an empty type set.
	if (typeSet.GetCount() <= 0)
	{
		return ValueResult<PreferenceValue>::FailedWith("Given an empty type set.");
	}

	// First, check if this preference value's type exactly matches a type in the given collection.
	// If so, then return a copy of this preference's value as-is. No conversion is necessary.
	if (typeSet.Contains(fType))
	{
		return ValueResult<PreferenceValue>::SucceededWith(*this);
	}

	// Attempt to convert this object's preference value to the closest value type in the caller's type collection.
	ValueResult<PreferenceValue> result = ValueResult<PreferenceValue>::FailedWith(NULL);
	switch (fType)
	{
		case PreferenceValue::kTypeBoolean:
		{
			// Attempt to convert the boolean value to an integer type supported by the caller.
			// Note: If any of these fail, then we'll fallback to convert to a string or float way down below.
			if (typeSet.Contains(PreferenceValue::kTypeSignedInt8))
			{
				result = ToValueType(PreferenceValue::kTypeSignedInt8);
			}
			else if (typeSet.Contains(PreferenceValue::kTypeUnsignedInt8))
			{
				result = ToValueType(PreferenceValue::kTypeUnsignedInt8);
			}
			else if (typeSet.Contains(PreferenceValue::kTypeSignedInt16))
			{
				result = ToValueType(PreferenceValue::kTypeSignedInt16);
			}
			else if (typeSet.Contains(PreferenceValue::kTypeUnsignedInt16))
			{
				result = ToValueType(PreferenceValue::kTypeUnsignedInt16);
			}
			else if (typeSet.Contains(PreferenceValue::kTypeSignedInt32))
			{
				result = ToValueType(PreferenceValue::kTypeSignedInt32);
			}
			else if (typeSet.Contains(PreferenceValue::kTypeUnsignedInt32))
			{
				result = ToValueType(PreferenceValue::kTypeUnsignedInt32);
			}
			else if (typeSet.Contains(PreferenceValue::kTypeSignedInt64))
			{
				result = ToValueType(PreferenceValue::kTypeSignedInt64);
			}
			else if (typeSet.Contains(PreferenceValue::kTypeUnsignedInt64))
			{
				result = ToValueType(PreferenceValue::kTypeUnsignedInt64);
			}
			break;
		}
		case PreferenceValue::kTypeSignedInt8:
		case PreferenceValue::kTypeSignedInt16:
		case PreferenceValue::kTypeSignedInt32:
		case PreferenceValue::kTypeSignedInt64:
		case PreferenceValue::kTypeUnsignedInt8:
		case PreferenceValue::kTypeUnsignedInt16:
		case PreferenceValue::kTypeUnsignedInt32:
		case PreferenceValue::kTypeUnsignedInt64:
		{
			// *** We're converting an integer type. ***

			// First, attempt to convert this object's int value to an int type of the same byte size.
			// Will only happen if switching from signed to unsigned or vice-versa.
			switch (fType)
			{
				case PreferenceValue::kTypeSignedInt8:
				case PreferenceValue::kTypeUnsignedInt8:
				{
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt8))
					{
						result = ToValueType(PreferenceValue::kTypeSignedInt8);
					}
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt8))
					{
						result = ToValueType(PreferenceValue::kTypeUnsignedInt8);
					}
					break;
				}
				case PreferenceValue::kTypeSignedInt16:
				case PreferenceValue::kTypeUnsignedInt16:
				{
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt16))
					{
						result = ToValueType(PreferenceValue::kTypeSignedInt16);
					}
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt16))
					{
						result = ToValueType(PreferenceValue::kTypeUnsignedInt16);
					}
					break;
				}
				case PreferenceValue::kTypeSignedInt32:
				case PreferenceValue::kTypeUnsignedInt32:
				{
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt32))
					{
						result = ToValueType(PreferenceValue::kTypeSignedInt32);
					}
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt32))
					{
						result = ToValueType(PreferenceValue::kTypeUnsignedInt32);
					}
					break;
				}
				case PreferenceValue::kTypeSignedInt64:
				case PreferenceValue::kTypeUnsignedInt64:
				{
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt64))
					{
						result = ToValueType(PreferenceValue::kTypeSignedInt64);
					}
					if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt64))
					{
						result = ToValueType(PreferenceValue::kTypeUnsignedInt64);
					}
					break;
				}
				default:
					break;
			}

			// If unable to convert the value to an integer of the same byte size,
			// then try to convert to it to any other integer type supported by the caller.
			if (result.HasFailed())
			{
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt8))
				{
					result = ToValueType(PreferenceValue::kTypeSignedInt8);
				}
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt8))
				{
					result = ToValueType(PreferenceValue::kTypeUnsignedInt8);
				}
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt16))
				{
					result = ToValueType(PreferenceValue::kTypeSignedInt16);
				}
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt16))
				{
					result = ToValueType(PreferenceValue::kTypeUnsignedInt16);
				}
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt32))
				{
					result = ToValueType(PreferenceValue::kTypeSignedInt32);
				}
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt32))
				{
					result = ToValueType(PreferenceValue::kTypeUnsignedInt32);
				}
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt64))
				{
					result = ToValueType(PreferenceValue::kTypeSignedInt64);
				}
				if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt64))
				{
					result = ToValueType(PreferenceValue::kTypeUnsignedInt64);
				}
			}
			break;
		}
		case PreferenceValue::kTypeFloatSingle:
		case PreferenceValue::kTypeFloatDouble:
		{
			// If we're here, then we're likely trying to convert a single precision float to a double.
			if (typeSet.Contains(PreferenceValue::kTypeFloatDouble))
			{
				result = ToValueType(PreferenceValue::kTypeFloatDouble);
			}
			break;
		}
		case PreferenceValue::kTypeString:
		{
			// Do the following if we have to convert a string to a numeric or boolean type.
			// That is, the caller doesn't want a string type returned.
			if (fStringPointer.NotNull() && (typeSet.Contains(PreferenceValue::kTypeString) == false))
			{
				// First, check if the string can be converted to a boolean value, if supported by the caller.
				// Note: This will only succeed if it contains strings "true", "false", "yes", or "no".
				if (typeSet.Contains(PreferenceValue::kTypeBoolean))
				{
					result = ToValueType(PreferenceValue::kTypeBoolean);
				}

				// If unable to convert to a boolean, then try to convert the string to an integer or float.
				if (result.HasFailed())
				{
					bool hasNegativeSign = (fStringPointer->find('-') != std::string::npos);
					bool hasDecimalPoint = (fStringPointer->find('.') != std::string::npos);
					bool hasExponent =
							(fStringPointer->find('E') != std::string::npos) ||
							(fStringPointer->find('e') != std::string::npos);
					if (hasDecimalPoint || hasExponent)
					{
						// *** The string might be a decimal number. ***

						// Attempt to convert it to a float, if supported by the caller.
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeFloatSingle))
						{
							result = ToValueType(PreferenceValue::kTypeFloatSingle);
						}
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeFloatDouble))
						{
							result = ToValueType(PreferenceValue::kTypeFloatDouble);
						}

						// If we were unable to convert to a float, then try to convert it to an integer type.
						if (result.HasFailed())
						{
							// First, try to convert the string to a float.
							// Note: We do this because we know the string contains a decimal point or exponent
							//       and the C/C++ string-to-int functions ignore these characters.
							ValueResult<PreferenceValue> numericResult = ToValueType(PreferenceValue::kTypeFloatDouble);
							if (numericResult.HasSucceeded())
							{
								// Convert the float to an integer type that is supported by the caller.
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt8))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeSignedInt8);
								}
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt8))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeUnsignedInt8);
								}
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt16))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeSignedInt16);
								}
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt16))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeUnsignedInt16);
								}
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt32))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeSignedInt32);
								}
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt32))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeUnsignedInt32);
								}
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt64))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeSignedInt64);
								}
								if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeUnsignedInt64))
								{
									result = numericResult.GetValue().ToValueType(PreferenceValue::kTypeUnsignedInt64);
								}
							}
						}
					}
					else
					{
						// No floating point characters (decimal points or exponents) were found in the string.
						// Attempt to convert the string to an integer or float, starting with integer types.
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt8))
						{
							result = ToValueType(PreferenceValue::kTypeSignedInt8);
						}
						if (result.HasFailed() && !hasNegativeSign && typeSet.Contains(PreferenceValue::kTypeUnsignedInt8))
						{
							result = ToValueType(PreferenceValue::kTypeUnsignedInt8);
						}
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt16))
						{
							result = ToValueType(PreferenceValue::kTypeSignedInt16);
						}
						if (result.HasFailed() && !hasNegativeSign && typeSet.Contains(PreferenceValue::kTypeUnsignedInt16))
						{
							result = ToValueType(PreferenceValue::kTypeUnsignedInt16);
						}
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt32))
						{
							result = ToValueType(PreferenceValue::kTypeSignedInt32);
						}
						if (result.HasFailed() && !hasNegativeSign && typeSet.Contains(PreferenceValue::kTypeUnsignedInt32))
						{
							result = ToValueType(PreferenceValue::kTypeUnsignedInt32);
						}
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeSignedInt64))
						{
							result = ToValueType(PreferenceValue::kTypeSignedInt64);
						}
						if (result.HasFailed() && !hasNegativeSign && typeSet.Contains(PreferenceValue::kTypeUnsignedInt64))
						{
							result = ToValueType(PreferenceValue::kTypeUnsignedInt64);
						}
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeFloatSingle))
						{
							result = ToValueType(PreferenceValue::kTypeFloatSingle);
						}
						if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeFloatDouble))
						{
							result = ToValueType(PreferenceValue::kTypeFloatDouble);
						}
					}
				}
			}
			break;
		}
		default:
		{
			Rtt_ASSERT(0);
			break;
		}
	}
	if (result.HasSucceeded())
	{
		return result;
	}

	// If unable to convert the value to a numeric/boolean type,
	// then convert it to a string if supported by the caller.
	if (typeSet.Contains(PreferenceValue::kTypeString))
	{
		return ToValueType(PreferenceValue::kTypeString);
	}

	// Worst Case Scenario:
	// The caller doesn't support converting the value to a string or integer.
	// So, attempt to convert it to a float, if supported by the caller.
	if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeFloatSingle))
	{
		result = ToValueType(PreferenceValue::kTypeFloatSingle);
	}
	if (result.HasFailed() && typeSet.Contains(PreferenceValue::kTypeFloatDouble))
	{
		result = ToValueType(PreferenceValue::kTypeFloatDouble);
	}

	// We've failed to convert this preference value to a type given by the caller.
	// Note: This should never happen if the caller has given us a string type.
	const char kMessage[] = "Failed to convert preference value to an accepted type.";
	return ValueResult<PreferenceValue>::FailedWith(kMessage);
}

bool PreferenceValue::Equals(const PreferenceValue& value) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (&value == this)
	{
		return true;
	}

	// Check if the value types equal.
	if (value.fType != fType)
	{
		return false;
	}

	// Check if the values equal.
	switch (fType)
	{
		case PreferenceValue::kTypeBoolean:
		{
			return (value.fValue.Boolean == fValue.Boolean);
		}
		case PreferenceValue::kTypeSignedInt8:
		{
			return (value.fValue.SignedInt8 == fValue.SignedInt8);
		}
		case PreferenceValue::kTypeSignedInt16:
		{
			return (value.fValue.SignedInt16 == fValue.SignedInt16);
		}
		case PreferenceValue::kTypeSignedInt32:
		{
			return (value.fValue.SignedInt32 == fValue.SignedInt32);
		}
		case PreferenceValue::kTypeSignedInt64:
		{
			return (value.fValue.SignedInt64 == fValue.SignedInt64);
		}
		case PreferenceValue::kTypeUnsignedInt8:
		{
			return (value.fValue.UnsignedInt8 == fValue.UnsignedInt8);
		}
		case PreferenceValue::kTypeUnsignedInt16:
		{
			return (value.fValue.UnsignedInt16 == fValue.UnsignedInt16);
		}
		case PreferenceValue::kTypeUnsignedInt32:
		{
			return (value.fValue.UnsignedInt32 == fValue.UnsignedInt32);
		}
		case PreferenceValue::kTypeUnsignedInt64:
		{
			return (value.fValue.UnsignedInt64 == fValue.UnsignedInt64);
		}
		case PreferenceValue::kTypeFloatSingle:
		{
			float delta = value.fValue.FloatSingle - fValue.FloatSingle;
			return ((delta <= FLT_EPSILON) && (delta >= -FLT_EPSILON));
		}
		case PreferenceValue::kTypeFloatDouble:
		{
			double delta = value.fValue.FloatDouble - fValue.FloatDouble;
			return ((delta <= DBL_EPSILON) && (delta >= -DBL_EPSILON));
		}
		case PreferenceValue::kTypeString:
		{
			if (value.fStringPointer == fStringPointer)
			{
				return true;
			}
			if (value.fStringPointer.IsNull() || fStringPointer.IsNull())
			{
				return false;
			}
			return (fStringPointer->compare(*value.fStringPointer) == 0);
		}
		default:
		{
			break;
		}
	}
	Rtt_ASSERT(0);
	return false;
}

bool PreferenceValue::NotEquals(const PreferenceValue& value) const
{
	return !Equals(value);
}

bool PreferenceValue::operator==(const PreferenceValue& value) const
{
	return Equals(value);
}

bool PreferenceValue::operator!=(const PreferenceValue& value) const
{
	return !Equals(value);
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

Rtt::ValueResult<PreferenceValue> PreferenceValue::From(lua_State* luaStatePointer, int index)
{
	// Validate.
	if (!luaStatePointer)
	{
		return ValueResult<PreferenceValue>::FailedWith("Lua state pointer is null.");
	}

	// Attempt to box and return the indexed Lua value within a PreferenceValue object.
	switch (lua_type(luaStatePointer, index))
	{
		case LUA_TBOOLEAN:
		{
			return ValueResult<PreferenceValue>::SucceededWith(
					Rtt::PreferenceValue(lua_toboolean(luaStatePointer, index) != 0));
		}
		case LUA_TNUMBER:
		{
			return ValueResult<PreferenceValue>::SucceededWith(
					Rtt::PreferenceValue(lua_tonumber(luaStatePointer, index)));
		}
		case LUA_TSTRING:
		{
			size_t length = 0;
			const char* stringPointer = lua_tolstring(luaStatePointer, index, &length);
			if (stringPointer && (length > 0))
			{
				Rtt::SharedConstStdStringPtr sharedStringPointer = Rtt_MakeSharedConstStdStringPtr(stringPointer, length);
				if (sharedStringPointer.NotNull())
				{
					return ValueResult<PreferenceValue>::SucceededWith(Rtt::PreferenceValue(sharedStringPointer));
				}
			}
			return ValueResult<PreferenceValue>::SucceededWith(Rtt::PreferenceValue((const char*)NULL));
		}
		case LUA_TNIL:
		{
			return ValueResult<PreferenceValue>::FailedWith("You cannot store nil to a preference.");
		}
		case LUA_TTABLE:
		{
			return ValueResult<PreferenceValue>::FailedWith("You cannot store a Lua table to a preference.");
		}
		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
		{
			return ValueResult<PreferenceValue>::FailedWith("You cannot store Lua \"user data\" to a preference.");
		}
		case LUA_TFUNCTION:
		{
			return ValueResult<PreferenceValue>::FailedWith("You cannot store a Lua function to a preference.");
		}
		case LUA_TTHREAD:
		{
			return ValueResult<PreferenceValue>::FailedWith("You cannot store a Lua thread to a preference.");
		}
		case LUA_TNONE:
		{
			return ValueResult<PreferenceValue>::FailedWith("Invalid Lua stack index.");
		}
		default:
		{
			break;
		}
	}
	return ValueResult<PreferenceValue>::FailedWith("Unknown Lua value type cannot be stored to a preference.");
}


// ----------------------------------------------------------------------------
// Private Static Functions
// ----------------------------------------------------------------------------

Rtt::SharedConstStdStringPtr PreferenceValue::CreateCannotConvertUnknownValueMessageForType(const char* typeName)
{
	std::stringstream stringStream;
	stringStream << "Cannot convert to ";
	stringStream << (typeName ? typeName : "type");
	stringStream << ". Preference value type is unknown.";
	return Rtt_MakeSharedConstStdStringPtr(stringStream.str());
}

Rtt::SharedConstStdStringPtr PreferenceValue::CreateCannotConvertStringMessageForType(const char* typeName)
{
	std::stringstream stringStream;
	stringStream << "Preference's string value cannot be converted to ";
	stringStream << (typeName ? typeName : "type");
	stringStream << ".";
	return Rtt_MakeSharedConstStdStringPtr(stringStream.str());
}

Rtt::SharedConstStdStringPtr PreferenceValue::CreateOutOfBoundsMessageForType(const char* typeName)
{
	std::stringstream stringStream;
	stringStream << "Cannot convert to ";
	stringStream << (typeName ? typeName : "type");
	stringStream << ". Preference value exceeds its bounds.";
	return Rtt_MakeSharedConstStdStringPtr(stringStream.str());
}


// ----------------------------------------------------------------------------
// TypeSet Inner Class Methods
// ----------------------------------------------------------------------------

PreferenceValue::TypeSet::TypeSet()
{
}

PreferenceValue::TypeSet::~TypeSet()
{
}

void PreferenceValue::TypeSet::Add(PreferenceValue::Type type)
{
	fSet.insert(type);
}

void PreferenceValue::TypeSet::Remove(PreferenceValue::Type type)
{
	fSet.erase(type);
}

void PreferenceValue::TypeSet::Clear()
{
	fSet.clear();
}

bool PreferenceValue::TypeSet::Contains(PreferenceValue::Type type) const
{
	return (fSet.find(type) != fSet.end());
}

bool PreferenceValue::TypeSet::ContainsIntegerType() const
{
	for (CollectionType::const_iterator iter = fSet.begin(); iter != fSet.end(); iter++)
	{
		switch (*iter)
		{
			case PreferenceValue::kTypeSignedInt8:
			case PreferenceValue::kTypeSignedInt16:
			case PreferenceValue::kTypeSignedInt32:
			case PreferenceValue::kTypeSignedInt64:
			case PreferenceValue::kTypeUnsignedInt8:
			case PreferenceValue::kTypeUnsignedInt16:
			case PreferenceValue::kTypeUnsignedInt32:
			case PreferenceValue::kTypeUnsignedInt64:
				return true;
			default:
				break;
		}
	}
	return false;
}

bool PreferenceValue::TypeSet::ContainsFloatType() const
{
	for (CollectionType::const_iterator iter = fSet.begin(); iter != fSet.end(); iter++)
	{
		switch (*iter)
		{
			case PreferenceValue::kTypeFloatSingle:
			case PreferenceValue::kTypeFloatDouble:
				return true;
			default:
				break;
		}
	}
	return false;
}

int PreferenceValue::TypeSet::GetCount() const
{
	return (int)fSet.size();
}

Rtt::ValueResult<PreferenceValue::Type> PreferenceValue::TypeSet::GetByIndex(int index) const
{
	if ((index >= 0) && (index < GetCount()))
	{
		for (CollectionType::const_iterator iter = fSet.begin(); iter != fSet.end(); iter++)
		{
			if (index == 0)
			{
				return ValueResult<PreferenceValue::Type>::SucceededWith(*iter);
			}
			index--;
		}
	}
	return ValueResult<PreferenceValue::Type>::FailedWith("Index out of range.");
}

} // namespace Rtt

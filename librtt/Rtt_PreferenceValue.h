//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PreferenceValue_H__
#define _Rtt_PreferenceValue_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_OperationResult.h"
#include "Core/Rtt_SharedPtr.h"
#include "Core/Rtt_SharedStringPtr.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_ValueResult.h"
#include <locale>
#include <set>
#include <string>
extern "C"
{
	struct lua_State;
}


namespace Rtt
{

/**
  Stores a single preference's value in variant form.

  Provides an easy means of converting the value to string or other value types.

  This class was designed to be efficiently passed by value.
 */
class PreferenceValue
{
	public:
		/** Indicates the type of value stored by a preference such as kTypeBoolean, kTypeFloatDouble, kTypeString, etc. */
		enum Type
		{
			kTypeBoolean,
			kTypeSignedInt8,
			kTypeSignedInt16,
			kTypeSignedInt32,
			kTypeSignedInt64,
			kTypeUnsignedInt8,
			kTypeUnsignedInt16,
			kTypeUnsignedInt32,
			kTypeUnsignedInt64,
			kTypeFloatSingle,
			kTypeFloatDouble,
			kTypeString
		};

		/**
		  Stores a collection of value types such as kTypeBoolean, kTypeFloatDouble, kTypeString, etc.
		  Intended to be passed to the PreferenceValue::ToClosestValueTypeIn() method.
		 */
		class TypeSet
		{
			public:
				TypeSet();
				virtual ~TypeSet();

				void Add(PreferenceValue::Type type);
				void Remove(PreferenceValue::Type type);
				void Clear();
				bool Contains(PreferenceValue::Type type) const;
				bool ContainsIntegerType() const;
				bool ContainsFloatType() const;
				int GetCount() const;
				Rtt::ValueResult<PreferenceValue::Type> GetByIndex(int index) const;

			private:
				typedef std::set<PreferenceValue::Type> CollectionType;
				CollectionType fSet;
		};

		/**
		  Type returned by the PreferenceValue::ToString() method, indicating if the preference value was
		  successfully converted to a string. If successful, then the GetValue() method will return that string.
		 */
		typedef Rtt::ValueResult<Rtt::SharedConstStdStringPtr> StringResult;


		/** Creates a new preference value object initialized to boolean flag "false". */
		PreferenceValue();

		/**
		  Creates a new preference value assigned the given boolean flag.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(bool value);

		/**
		  Creates a new preference value assigned the given 8-bit signed integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(S8 value);

		/**
		  Creates a new preference value assigned the given 16-bit signed integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(S16 value);

		/**
		  Creates a new preference value assigned the given 32-bit signed integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(S32 value);

		/**
		  Creates a new preference value assigned the given 64-bit signed integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(S64 value);

		/**
		  Creates a new preference value assigned the given 8-bit unsigned integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(U8 value);

		/**
		  Creates a new preference value assigned the given 16-bit unsigned integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(U16 value);

		/**
		  Creates a new preference value assigned the given 32-bit unsigned integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(U32 value);

		/**
		  Creates a new preference value assigned the given 64-bit unsigned integer value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(U64 value);

		/**
		  Creates a new preference value assigned the given single precision floating point value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(float value);

		/**
		  Creates a new preference value assigned the given double precision floating point value.
		  @param value The value to be assigned to the preference.
		 */
		PreferenceValue(double value);

		/**
		  Creates a new preference value assigned the given UTF-8 encoded string.
		  @param utf8String The UTF-8 encoded string to be assigned to the preference. Can be null or empty.
		 */
		PreferenceValue(const char* utf8String);

		/**
		  Creates a new preference value assigned the given UTF-8 encoded string.
		  @param utf8String The UTF-8 encoded string to be assigned to the preference. Can be null or empty.
		                    String can also contain embedded null characters.
		 */
		PreferenceValue(const Rtt::SharedConstStdStringPtr& utf8String);

		/** Destroys this object. */
		virtual ~PreferenceValue();

		/**
		  Gets the type of value stored by the preference such as kTypeBoolean, kTypeFloatDouble, kTypeString, etc.
		  @return Returns the type of value stored by the preference.
		 */
		PreferenceValue::Type GetType() const;

		/**
		  Gets or converts the preference's value to a boolean, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to a boolean.
		          The result object's GetValue() method will return the requested boolean value if successful.

		          Returns a failure result if the preference's value cannot be converted to a boolean.
		          The result object's GetValue() method will always return false in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<bool> ToBoolean() const;

		/**
		  Gets or converts the preference's value to an 8-bit signed integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<S8> ToSignedInt8() const;

		/**
		  Gets or converts the preference's value to a 16-bit signed integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<S16> ToSignedInt16() const;

		/**
		  Gets or converts the preference's value to a 32-bit signed integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<S32> ToSignedInt32() const;

		/**
		  Gets or converts the preference's value to a 64-bit signed integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<S64> ToSignedInt64() const;

		/**
		  Gets or converts the preference's value to an 8-bit unsigned integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<U8> ToUnsignedInt8() const;

		/**
		  Gets or converts the preference's value to a 16-bit unsigned integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<U16> ToUnsignedInt16() const;

		/**
		  Gets or converts the preference's value to a 32-bit unsigned integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<U32> ToUnsignedInt32() const;

		/**
		  Gets or converts the preference's value to a 64-bit unsigned integer, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to an integer.
		          The result object's GetValue() method will return the requested integer value if successful.

		          Returns a failure result if the preference's value cannot be converted to an integer.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<U64> ToUnsignedInt64() const;

		/**
		  Gets or converts the preference's value to a single precision float, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to a float.
		          The result object's GetValue() method will return the requested float value if successful.

		          Returns a failure result if the preference's value cannot be converted to a float.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<float> ToFloatSingle() const;

		/**
		  Gets or converts the preference's value to a double precision float, if possible.
		  @return Returns a success result if the preference's value is stored or convertible to a float.
		          The result object's GetValue() method will return the requested float value if successful.

		          Returns a failure result if the preference's value cannot be converted to a float.
		          The result object's GetValue() method will always return zero in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<double> ToFloatDouble() const;

		/**
		  Gets or converts the preference's value to a UTF-8 encoded string, if possible.

		  If converting a numeric value, the returned string will not be localized.
		  @return Returns a success result if the preference's value is stored or convertible to a string.
		          The result object's GetValue() method will return the requested string if successful.

		          Returns a failure result if the preference's value cannot be converted to a string.
		          The result object's GetValue() method will always return an empty string in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		PreferenceValue::StringResult ToString() const;

		/**
		  Gets or converts the preference's value to a UTF-8 encoded string, if possible.

		  If converting a numeric value, the given locale will be used to generate a localized string.
		  @param locale The locale to be used to convert the preference value to a localized string.
		  @return Returns a success result if the preference's value is stored or convertible to a string.
		          The result object's GetValue() method will return the requested string if successful.

		          Returns a failure result if the preference's value cannot be converted to a string.
		          The result object's GetValue() method will always return an empty string in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		PreferenceValue::StringResult ToStringWithLocale(const std::locale& locale) const;

		/**
		  Converts the preference's value to the given type and returns it as a new PreferenceValue object, if possible.
		  @param type The value type to convert to such as kTypeSignedInt32, kTypeString, etc.
		  @return Returns a success result if the preference's value is stored or convertible to the given type.
		          The result object's GetValue() method will return the requested type if successful.

		          Returns a failure result if the preference's value cannot be converted to the given type.
		          The result object's GetValue() method will return an invalid value in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<PreferenceValue> ToValueType(PreferenceValue::Type type) const;

		/**
		  Converts the preference's value to the best matching type in the given type set, if possible,
		  and returns it as a new preference value object.
		  @param typeSet
		  A collection of value types supported by the caller.
		  This method will only attempt to convert to one of the types in this collection.
		  It is highly recommended to add a kTypeString to this collection, because all types are convertible to string.
		  @return Returns a success result if the preference's value is convertible to one of the given types.
		          The result object's GetValue() method will return the converted value if successful.

		          Returns a failure result if the preference's value cannot be converted to any of the given types.
		          The result object's GetValue() method will return an invalid value in this case.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		Rtt::ValueResult<PreferenceValue> ToClosestValueTypeIn(const PreferenceValue::TypeSet& typeSet) const;

		/**
		  Determines if this preference value matches the given value.
		  @param value Reference to the preference value to compare against.
		  @return Returns true if the objects match. Returns false if not.
		 */
		bool Equals(const PreferenceValue& value) const;

		/**
		  Determines if this preference value does not match the given value.
		  @param value Reference to the preference value to compare against.
		  @return Returns true if the objects do not match. Returns false if they do match.
		 */
		bool NotEquals(const PreferenceValue& value) const;

		/**
		  Determines if this preference value matches the given value.
		  @param value Reference to the preference value to compare against.
		  @return Returns true if the objects match. Returns false if not.
		 */
		bool operator==(const PreferenceValue& value) const;

		/**
		  Determines if this preference value does not match the given value.
		  @param value Reference to the preference value to compare against.
		  @return Returns true if the objects do not match. Returns false if they do match.
		 */
		bool operator!=(const PreferenceValue& value) const;

		/**
		  Fetches the indexed Lua value and returns it in boxed form in a PreferenceValue object, if possible.
		  @param luaStatePointer Pointer to the Lua state that the "luaStackIndex" argument references.
		  @param luaStackIndex Index to the Lua value to read. Expected to reference a Lua boolean, number, or string.
		  @return Returns success result if the Lua value was successfully read and boxed into a PreferenceValue object.
		          The result object's GetValue() method will return the read value if successful.

		          Returns a failure result if given invalid arguments or if the reference Lua object cannot be
				  converted to a type supported by the PreferenceValue class.
		          The result object's GetUtf8Message() method will return details why this function failed.
		 */
		static ValueResult<PreferenceValue> From(lua_State* luaStatePointer, int luaStackIndex);

	private:
		/**
		  Converts the preference's value to a numeric type specified by template type "TValue" such as S32, S64, etc.
		  
		  Template argument type "TTypeNameProvider" is expected to be set to a TypeNameProvider class template
		  specialization, which is used to provide this ToNumber() method the name of the value type it's converting to.
		  @return Returns success result if the preference value was successfully converted to the "TValue" type.
		          The result object's GetValue() method will return the converted value if successful.

		          Returns a failure result if the preference's value cannot be converted the "TValue" type.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		template<typename TValue, class TTypeNameProvider>
		Rtt::ValueResult<TValue> ToNumber() const;

		/**
		  Converts the preference's value to a numeric type specified by template type "TValue" such as S32, S64, etc.
		  @return Returns success result if the preference value was successfully converted to the "TValue" type.
		          The result object's GetValue() method will return the converted value if successful.

		          Returns a failure result if the preference's value cannot be converted the "TValue" type.
		          The result object's GetUtf8Message() method will return a string explaining why the conversion failed.
		 */
		template<typename TValue>
		Rtt::ValueResult<TValue> ToNumber() const;

		/** Creates an unknown conversion error message string for the given type name. */
		static Rtt::SharedConstStdStringPtr CreateCannotConvertUnknownValueMessageForType(const char* typeName);

		/** Creates a string conversion error message string for the given type name. */
		static Rtt::SharedConstStdStringPtr CreateCannotConvertStringMessageForType(const char* typeName);

		/** Creates a value type out-of-bounds error message string for the given type name. */
		static Rtt::SharedConstStdStringPtr CreateOutOfBoundsMessageForType(const char* typeName);


		/**
		  Private class used by the templatized PreferenceValue::ToNumber() method to provide the value type name
		  that method is converting to, such as "boolean", "32-bit signed integer" etc.
		  Requires template specialization to be used.
		 */
		template<typename T>
		class TypeNameProvider
		{
			private:
				TypeNameProvider() {};
			public:
				static const char kName[];
		};

		/** Variant type used to store non-string values such as a boolean, integer, or float. */
		union VariantValue
		{
			bool Boolean;
			S8 SignedInt8;
			S16 SignedInt16;
			S32 SignedInt32;
			S64 SignedInt64;
			U8 UnsignedInt8;
			U16 UnsignedInt16;
			U32 UnsignedInt32;
			U64 UnsignedInt64;
			float FloatSingle;
			double FloatDouble;
		};


		/**
		  Indicates the type of value this object is storing.

		  If set to kString, then member variable "fStringPointer" stores the preference's string value.

		  If set to any other type, then the value is stored by member variable "fValue" under that union's
		  respective field. For example, type kSignedInt32 would stored under VariantValue::SignedInt32.
		 */
		PreferenceValue::Type fType;

		/**
		  Stores either a boolean, integer, or floating point value via a union for memory efficiency.
		  Only applicable when member variable "fType" is not set kString.
		 */
		VariantValue fValue;

		/** Stores a preference's string value. Only applicable if member variable "fType" is set to kString. */
		Rtt::SharedConstStdStringPtr fStringPointer;
};

} // namespace Rtt

#endif // _Rtt_PreferenceValue_H__

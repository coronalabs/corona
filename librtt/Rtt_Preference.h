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

#ifndef _Rtt_Preference_H__
#define _Rtt_Preference_H__

#include "Core/Rtt_SharedStringPtr.h"
#include "Rtt_PreferenceValue.h"


namespace Rtt
{

/**
  Stores a single preference's key-value pair.

  A preference is used to store a single setting or piece of data for an application. Instances of this class
  are intended to be used to interop between Corona and the platform's native preferences-like feature such
  as Apple's NSUserDefaults, Android's SharedPreferences, and Win32's registry.

  This class was designed to be efficiently passed by value.
 */
class Preference
{
	public:
		/**
		  Defines the "app" category name used by Corona's Lua preference APIs.
		  Used to access the Corona built application's custom preferences as defined by the Corona developer.
		 */
		static const char kCategoryNameApp[];

		/**
		  Defines the legacy "locale" category name used by Corona's Lua preference APIs.
		  Used to fetch read-only "System Preferences" to acquire current locale information.
		 */
		static const char kCategoryNameLocale[];

		/** 
		  Defines the hidden "simulator" category name used by Corona's Lua preference APIs.
		  Used to access the Corona Simulator application's preferences, such as by the "shell.lua" script.
		 */
		static const char kCategoryNameSimulator[];

		/**
		  Defines the legacy "ui" category name used by Corona's Lua preference APIs.
		  Used to fetch read-only "System Preferences" to acquire current UI language and display information.
		 */
		static const char kCategoryNameUI[];


		class ReadValueResult : public ValueResult<PreferenceValue>
		{
			protected:
				/**
				  Creates a new result object with the given information.

				  This is a protected constructor that is only expected to be called by this class' static
				  SucceededWith() and FailedWith() functions or by derived versions of this class.
				  @param hasSucceeded Set to true if the operation succeeded. Set to false if failed.
				  @param utf8Message
				  Message providing details about the operation's final result. Can be null or empty string.
				  If the operation failed, then this message is expected to explain why.
				  @param value The value to be returned by the GetValue() method.
				  @param hadPreferenceNotFoundError
				  Set true if a "Preference not found error" occurred. Set false if this specific error did not occur.
				 */
				ReadValueResult(
						bool hasSucceeded, const char* utf8Message, PreferenceValue value,
						bool hadPreferenceNotFoundError);

				/**
				  Creates a new result object with the given information.

				  This is a protected constructor that is only expected to be called by this class' static
				  SucceededWith() and FailedWith() functions or by derived versions of this class.
				  @param hasSucceeded Set to true if the operation succeeded. Set to false if failed.
				  @param utf8Message
				  Message providing details about the final results of the operation. Can be null or empty string.
				  If the operation failed, then this message is expected to explain why.
				  @param value The value to be returned by the GetValue() method.
				  @param hadPreferenceNotFoundError
				  Set true if a "Preference not found error" occurred. Set false if this specific error did not occur.
				 */
				ReadValueResult(
						bool hasSucceeded, const Rtt::SharedConstStdStringPtr& utf8Message, PreferenceValue value,
						bool hadPreferenceNotFoundError);

			public:
				/** Destroys this object. */
				virtual ~ReadValueResult();

				/**
				  Determines if a "Preference not found" error occurred.
				  Will only be true for the ReadValueResult::kPreferenceNotFound constant result object.
				  @return
				  Returns true if the requested preference was not found.
				  Returns false if this specific error did not occur.
				 */
				bool HadPreferenceNotFoundError() const;

				/**
				  Gets a reusable ReadValueResult object used to indicate that preferences were accessible,
				  but the requested preference key to be read/fetched was not found.
				 */
				static const ReadValueResult kPreferenceNotFound;

				/**
				  Creates a new success result object providing the given value.
				  @param value Value to be returned by the result object's GetValue() method.
				  @return Returns a new success result object with the given info.
				 */
				static ReadValueResult SucceededWith(PreferenceValue value);

				/**
				  Creates a new result object set to failed and with the given message.
				  @param utf8Message Message explaining why the operation failed. Can be null or empty.
				  @return Returns a new result object set to failed and with the given message.
				 */
				static ReadValueResult FailedWith(const char* utf8Message);

				/**
				  Creates a new result object set to failed and with the given message.
				  @param utf8Message Message explaining why the operation failed. Can be null or empty.
				  @return Returns a new result object set to failed and with the given message.
				 */
				static ReadValueResult FailedWith(const Rtt::SharedConstStdStringPtr& utf8Message);

			private:
				bool fHadPreferenceNotFoundError;
		};


		/**
		  Creates a new preference with the given name and value initialized to a false boolean flag.
		  @param keyName Unique name of the preference in UTF-8 form. Can be null or empty.
		 */
		Preference(const char* keyName);

		/**
		  Creates a new preference with the given name and value initialized to a false boolean flag.
		  @param keyName Unique name of the preference in UTF-8 form. Can be null or empty.
		 */
		Preference(const Rtt::SharedConstStdStringPtr& keyName);

		/**
		  Creates a new preference with the given name and value.
		  @param keyName Unique name of the preference in UTF-8 form. Can be null or empty.
		  @param value The value to be assigned to the preference.
		 */
		Preference(const char* keyName, const Rtt::PreferenceValue& value);

		/**
		  Creates a new preference with the given name and value.
		  @param keyName Unique name of the preference in UTF-8 form. Can be null or empty.
		  @param value The value to be assigned to the preference.
		 */
		Preference(const Rtt::SharedConstStdStringPtr& keyName, const Rtt::PreferenceValue& value);

		/** Destroys this object. */
		virtual ~Preference();

		/**
		  Gets the preference's unique name in UTF-8 form.
		  @return Returns the preference's unique name. Can be empty string, but never null.
		 */
		const char* GetKeyName() const;

		/**
		  Gets the preference's unique name in UTF-8 form as a shared string pointer.
		  @return Returns a shared string pointer to the preference's unique name.
		          Can be empty string, but never null.
		 */
		Rtt::SharedConstStdStringPtr GetKeyNameAsSharedPointer() const;

		/**
		  Gets a copy of the preference's assigned value.
		  @return Returns the a copy of the preference's value.
		 */
		Rtt::PreferenceValue GetValue() const;

		/**
		  Assigns the given value to this preference.
		  @param value The value to be assigned to this preference.
		 */
		void SetValue(const Rtt::PreferenceValue& value);

	private:
		/** Assignment operator made private to prevent members from being overwritten. */
		void operator=(const Preference& value) {}


		/** The preference's unique string ID in UTF-8 form. */
		Rtt::SharedConstStdStringPtr fKeyName;

		/** The preference's assigned value. */
		Rtt::PreferenceValue fValue;
};

} // namespace Rtt

#endif // _Rtt_Preference_H__

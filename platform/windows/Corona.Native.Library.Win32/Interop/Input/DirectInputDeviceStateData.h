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

#pragma once

#include "Interop\OperationResult.h"
#include "Interop\ValueResult.h"
#include <Windows.h>


#pragma region Forward Declarations
namespace Interop { namespace Input {
	class InputDeviceInfo;
} }
extern "C" {
	struct _DIDATAFORMAT;
	typedef struct _DIDATAFORMAT *LPDIDATAFORMAT;
}

#pragma endregion


namespace Interop { namespace Input {

/// <summary>
///  <para>Class used to create a DirectInput data format and data buffer to store button, axis, and hat data to.</para>
/// </summary>
class DirectInputDeviceStateData
{
	public:
		#pragma region Public Constants
		/// <summary>The number of bytes 1 axis value consumes in the buffer by DirectInput.</summary>
		static const size_t kAxisValueByteCount;

		/// <summary>The number of bytes 1 button value consumes in the buffer by DirectInput.</summary>
		static const size_t kButtonValueByteCount;

		/// <summary>The number of bytes 1 hat switch (aka: POV) value consumes in the buffer by DirectInput.</summary>
		static const size_t kHatValueByteCount;

		#pragma endregion


		#pragma region Public FormatSettings Class
		/// <summary>
		///  <para>Provides all of the object instance IDs to collect data for from one DirectInput device.</para>
		///  <para>Intended to be passed into the SetFormatWith() method to set up the DIDATAFORMAT object.</para>
		/// </summary>
		struct FormatSettings
		{
			/// <summary>
			///  <para>Pointer to an array of axis instance IDs to collect data for. Can be null.</para>
			///  <para>Order matters! Axis data will be buffered and indexed according to the order of this array.</para>
			/// </summary>
			const WORD* AxisInstanceIdArray;

			/// <summary>Number of elements stored by the "AxisInstanceIdArray" field. Can be zero.</summary>
			size_t AxisInstanceIdArraySize;

			/// <summary>
			///  <para>Pointer to an array of button instance IDs to collect data for. Can be null.</para>
			///  <para>Order matters! Button data will be buffered and indexed according to the order of this array.</para>
			/// </summary>
			const WORD* ButtonInstanceIdArray;

			/// <summary>Number of elements stored by the "ButtonInstanceIdArray" field. Can be zero.</summary>
			size_t ButtonInstanceIdArraySize;

			/// <summary>
			///  <para>Pointer to an array of hat switch (aka: POV) instance IDs to collect data for. Can be null.</para>
			///  <para>Order matters! Hat data will be buffered and indexed according to the order of this array.</para>
			/// </summary>
			const WORD* HatInstanceIdArray;

			/// <summary>Number of elements stored by the "HatInstanceIdArray" field. Can be zero.</summary>
			size_t HatInstanceIdArraySize;
		};

		#pragma endregion


		#pragma region Public BufferOffsetInfoResult Class
		/// <summary>
		///  Provides the input type (axis, button, or hat) and input index referenced by the data byte buffer index
		///  passed into the DirectInputDeviceStateData::GetInfoFromBufferOffset() method.
		/// </summary>
		class BufferOffsetInfoResult : public Interop::OperationResult
		{
			private:
				/// <summary>Creates an error result object with the given message.</summary>
				/// <remarks>Made private to force the caller to use the static FailedWith() function.</remarks>
				/// <param name="errorMessage">Error message describing what is wrong. Can be null or empty string.</param>
				BufferOffsetInfoResult(const wchar_t* errorMessage);

			public:
				/// <summary>Identifies the input type such as axis, button, or hat switch.</summary>
				enum class InputType
				{
					/// <summary>Indicates that the input type is unknown or invalid.</summary>
					kUnknown,

					/// <summary>Identifies an axis input type.</summary>
					kAxis,

					/// <summary>Identifies a button input type.</summary>
					kButton,

					/// <summary>Identifies a hat switch (aka: POV) input type.</summary>
					kHat
				};

				/// <summary>Creates a result object with the given information.</summary>
				/// <param name="inputType">
				///  <para>The input type such as kAxis, kButton, or kHat.</para>
				///  <para>Result object will be flagged as failed if set to kUnknown.</para>
				/// </param>
				/// <param name="inputIndex">
				///  <para>Zero based index of the input.</para>
				///  <para>Result object will be flagged as failed if the index is less than zero.</para>
				/// </param>
				BufferOffsetInfoResult(BufferOffsetInfoResult::InputType inputType, int inputIndex);

				/// <summary>Destroys this object.</summary>
				virtual ~BufferOffsetInfoResult();

				/// <summary>Determines if the referenced byte in the buffer is for storing an axis' data.</summary>
				/// <returns>Returns true if referencing an axis input's bytes. Returns false if not.</returns>
				bool IsAxis() const;

				/// <summary>Determines if the referenced byte in the buffer is for storing a button's data.</summary>
				/// <returns>Returns true if referencing a button input's bytes. Returns false if not.</returns>
				bool IsButton() const;

				/// <summary>Determines if the referenced byte in the buffer is for storing a hat switch's data.</summary>
				/// <returns>Returns true if referencing a hat switch input's bytes. Returns false if not.</returns>
				bool IsHat() const;

				/// <summary>
				///  <para>
				///   Gets a zero based index of the input type (axis, button, or hat) that was reference byte offset.
				///  </para>
				///  <para>
				///   This index correlates to the input's associated array element passed into the
				///   DirectInputDeviceStateData::SetFormatUsing() method.
				///  </para>
				/// </summary>
				/// <returns>
				///  <para>Returns the referenced input type's zero based index.</para>
				///  <para>Returns -1 for a failure result.</para>
				/// </returns>
				int GetIndex() const;

				/// <summary>Creates a failure result object with the given error message.</summary>
				/// <param name="errorMessage">Message describing why the operation failed. Can be null or empty.</param>
				/// <returns>Returns a failure result object providing the given error message.</returns>
				static BufferOffsetInfoResult FailedWith(const wchar_t* errorMessage);

			private:
				/// <summary>
				///  <para>The referenced input type such as kAxis, kButton, or kHat.</para>
				///  <para>Set to kUnknown for failure result objects.</para>
				/// </summary>
				BufferOffsetInfoResult::InputType fInputType;

				/// <summary>
				///  <para>The referenced input's zero based index.</para>
				///  <para>Set to -1 for failure result objects.</para>
				/// </summary>
				int fInputIndex;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>
		///  <para>
		///   Creates a new object used to generate and manage a DirectInput data format and byte buffer for storing data.
		///  </para>
		///  <para>You are expected to call this object's SetFormatUsing() method right afterwards.</para>
		/// </summary>
		DirectInputDeviceStateData();

		/// <summary>Destroys this object data format and data buffer.</summary>
		virtual ~DirectInputDeviceStateData();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Creates a DIDATAFORMAT object and byte buffer needed to collect data from a DirectInput device.</para>
		///  <para>
		///   The generated DIDATAFORMAT object will be made available via the GetFormat() method which is expected
		///   to be passed to the IDirectInputDevice8::SetDataFormat() method before acquiring the device.
		///  </para>
		///  <para>
		///   The generated byte buffer will be made available via the GetBufferPointer() method and is expected to
		///   be passed to the IDirectInputDevice8::GetDeviceState() method to fetch data.
		///  </para>
		///  <para>If this method was called once before, then its prior format settings will be cleared.</para>
		/// </summary>
		/// <param name="settings">
		///  Provides the DirectInput instance IDs of all axes, buttons, and hat switch (aka: POV) to collect data for.
		/// </param>
		void SetFormatUsing(const DirectInputDeviceStateData::FormatSettings& settings);

		/// <summary>
		///  <para>Gets a DirectInput data format object generated by the SetFormatUsing() method.</para>
		///  <para>
		///   This is expected to be passed to the IDirectInputDevice8::SetDataFormat() method before acquiring the device.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the DirectInput data format generated by the SetFormatUsing() method.</para>
		///  <para>Returns null if a data format was not generated yet or it has been cleared.</para>
		/// </returns>
		LPDIDATAFORMAT GetFormat() const;

		/// <summary>
		///  <para>Deletes the last DirectInput data format generated by the SetFormatUsing() method call.</para>
		///  <para>This causes the GetFormat() method to return null afterwards.</para>
		/// </summary>
		void ClearFormat();

		/// <summary>
		///  <para>
		///   Gets a byte buffer generated by the SetFormatUsing() method used to store data for all configured
		///   axes, buttons, and hat switches.
		///  </para>
		///  <para>Intended to be passed to the IDirectInputDevice8::GetDeviceState() method to fetch input data.</para>
		///  <para>
		///   After fetching data from the device, you can retrived the fetched data via the GetAxisValueByIndex(),
		///   GetButtonValueByIndex(), and GetHatValueByIndex() methods.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns a byte buffer pointer expected to be passed to the IDirectInputDevice8::GetDeviceState() method.
		///  </para>
		///  <para>Returns null if a data buffer was not generated yet or it has been cleared.</para>
		/// </returns>
		BYTE* GetBufferPointer() const;

		/// <summary>
		///  Gets the number of bytes available in the byte buffer returned by the GetBufferPointer() method.
		/// </summary>
		/// <returns>
		///  <para>Returns the number of bytes available in this object's byte buffer.</para>
		///  <para>
		///   Returns zero if the buffer hasn't been generated by the SetFormatUinsg() method yet
		///   or if ClearFormat() has been called.
		///  </para>
		/// </returns>
		size_t GetBufferSize() const;

		/// <summary>Gets the number of axes the DirectInput data format was set up to collect data for.</summary>
		/// <returns>
		///  <para>Returns the number of axes the data format was set up to collect data for.</para>
		///  <para>Returns zero if not set up to collect axes data.</para>
		/// </returns>
		int GetAxisCount() const;

		/// <summary>
		///  <para>Fetches the indexed axis' data from this object's byte buffer.</para>
		///  <para>
		///   You are expected to call this method after passing this object's byte buffer to the
		///   IDirectInputDevice8::GetDeviceState() method.
		///  </para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the axis to fetch data for.</para>
		///  <para>This index correlates to the axis in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the indexed data via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<LONG> GetAxisValueByIndex(int index) const;

		/// <summary>
		///  <para>Fetches the indexed axis' DirectInput instance ID.</para>
		///  <para>This is the same instance ID passed into this object's SetFormatUsing() method.</para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the axis to fetch the instance ID for.</para>
		///  <para>This index correlates to the axis in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the requested data via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<WORD> GetAxisInstanceIdByIndex(int index) const;

		/// <summary>Fetches a zero based index of the axis assigned the given DirectInput instance ID.</summary>
		/// <param name="instanceId">
		///  <para>The instance ID to search for.</para>
		///  <para>This is the same instance ID passed into this object's SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>
		///   Returns a result object providing a zero based index of the axis assigned the given instance ID
		///   if flagged succeessful. The index is provided via the GetValue() method.
		///  </para>
		///  <para>
		///   Returns a failure result if the given instance ID was not found or if SetFormatUsing() hasn't been called yet.
		///  </para>
		/// </returns>
		ValueResult<int> GetAxisIndexByInstanceId(WORD instanceId) const;

		/// <summary>
		///  <para>Gets the indexed axis' position in the byte buffer for storing data.</para>
		///  <para>
		///   This offset can be added to the pointer returned by GetBufferPointer() to access this axis' bytes.
		///  </para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the axis to fetch the byte buffer offset for.</para>
		///  <para>This index correlates to the axis in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the requested offset via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<int> GetAxisBufferOffsetByIndex(int index) const;

		/// <summary>
		///  <para>Fetches an axis' position in the byte buffer by its DirectInput instance ID.</para>
		///  <para>
		///   This offset can be added to the pointer returned by GetBufferPointer() to access this axis' bytes.
		///  </para>
		/// </summary>
		/// <param name="instanceId">
		///  <para>The instance ID to search for.</para>
		///  <para>This is the same instance ID passed into this object's SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the requested offset via GetValue() if successful.</para>
		///  <para>
		///   Returns a failure result if the given instance ID was not found or if SetFormatUsing() hasn't been called yet.
		///  </para>
		/// </returns>
		ValueResult<int> GetAxisBufferOffsetByInstanceId(WORD instanceId) const;

		/// <summary>Gets the number of buttons the DirectInput data format was set up to collect data for.</summary>
		/// <returns>
		///  <para>Returns the number of buttons the data format was set up to collect data for.</para>
		///  <para>Returns zero if not set up to collect button data.</para>
		/// </returns>
		int GetButtonCount() const;

		/// <summary>
		///  <para>Fetches the indexed buttons's data from this object's byte buffer.</para>
		///  <para>
		///   You are expected to call this method after passing this object's byte buffer to the
		///   IDirectInputDevice8::GetDeviceState() method.
		///  </para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the button to fetch data for.</para>
		///  <para>This index correlates to the button in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the indexed data via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<BYTE> GetButtonValueByIndex(int index) const;

		/// <summary>
		///  <para>Fetches the indexed button's DirectInput instance ID.</para>
		///  <para>This is the same instance ID passed into this object's SetFormatUsing() method.</para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the button to fetch the instance ID for.</para>
		///  <para>This index correlates to the button in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the requested data via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<WORD> GetButtonInstanceIdByIndex(int index) const;

		/// <summary>
		///  <para>Gets the indexed button's position in the byte buffer for storing data.</para>
		///  <para>
		///   This offset can be added to the pointer returned by GetBufferPointer() to access this button's byte.
		///  </para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the button to fetch the byte buffer offset for.</para>
		///  <para>This index correlates to the button in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the requested offset via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<int> GetButtonBufferOffsetByIndex(int index) const;

		/// <summary>Gets the number of hat switches the DirectInput data format was set up to collect data for.</summary>
		/// <returns>
		///  <para>Returns the number of hat switches the data format was set up to collect data for.</para>
		///  <para>Returns zero if not set up to collect hat switch data.</para>
		/// </returns>
		int GetHatCount() const;

		/// <summary>
		///  <para>Fetches the indexed hat switch's data from this object's byte buffer.</para>
		///  <para>
		///   You are expected to call this method after passing this object's byte buffer to the
		///   IDirectInputDevice8::GetDeviceState() method.
		///  </para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the hat switch to fetch data for.</para>
		///  <para>This index correlates to the hat in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the indexed data via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<DWORD> GetHatValueByIndex(int index) const;

		/// <summary>
		///  <para>Fetches the indexed hat switch's DirectInput instance ID.</para>
		///  <para>This is the same instance ID passed into this object's SetFormatUsing() method.</para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the hat switch to fetch the instance ID for.</para>
		///  <para>This index correlates to the hat in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the requested data via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<WORD> GetHatInstanceIdByIndex(int index) const;

		/// <summary>
		///  <para>Gets the indexed hat switch's position in the byte buffer for storing data.</para>
		///  <para>
		///   This offset can be added to the pointer returned by GetBufferPointer() to access this hat's bytes.
		///  </para>
		/// </summary>
		/// <param name="index">
		///  <para>Zero based index to the hat switch to fetch the byte buffer offset for.</para>
		///  <para>This index correlates to the hat in the array passed into the SetFormatUsing() method.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the requested offset via GetValue() if successful.</para>
		///  <para>Returns a failure result if given an invalid index or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		ValueResult<int> GetHatBufferOffsetByIndex(int index) const;

		/// <summary>
		///  <para>Gets information about the input object referenced by the given byte buffer offset.</para>
		///  <para>
		///   Information such as if it stores data for an axis, button, or hat switch and the index of that input type.
		///  </para>
		///  <para>
		///   This method is expected to be used when calling the IDirectInputDevice8::GetDeviceData()
		///   to identify which input object has collected data.
		///  </para>
		/// </summary>
		/// <param name="offset">
		///  <para>Offset to an axis, button, or hat's data within the byte buffer.</para>
		///  <para>This offset is an index to a byte within byte buffer returned by the GetBufferPointer() method.</para>
		/// </para>
		/// <returns>
		///  <para>Returns information about the referenced input object if successful.</para>
		///  <para>Returns a failure result if given an invalid offset or if SetFormatUsing() hasn't been called yet.</para>
		/// </returns>
		DirectInputDeviceStateData::BufferOffsetInfoResult GetInfoFromBufferOffset(int offset) const;

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>Pointer to a DirectInput data format object created by the SetFormatUsing() method.</summary>
		LPDIDATAFORMAT fDataFormatPointer;

		/// <summary>Number of axes the DirectInput data format is set up to collect data for.</summary>
		int fAxisCount;

		/// <summary>Number of buttons the DirectInput data format is set up to collect data for.</summary>
		int fButtonCount;

		/// <summary>Number of hat switches the DirectInput data format is set up to collect data for.</summary>
		int fHatCount;

		/// <summary>
		///  <para>Pointer to a byte buffer used to store data for all axes, buttons, and hat switches.</para>
		///  <para>Intended to be passed to the IDirectInputDevice8::GetDeviceState() method to fetch input data.</para>
		///  <para>
		///   This byte buffer is created and sized according to the settings provided to the SetFormatUsing() method.
		///  </para>
		/// </summary>
		BYTE* fBufferPointer;

		/// <summary>
		///  <para>Number of bytes that byte buffer "fBufferPointer" has allocated.</para>
		///  <para>Set to zero if "fBufferPointer" is null.</para>
		/// </summary>
		size_t fBufferSize;

		#pragma endregion
};

} }	// namespace Interop::Input

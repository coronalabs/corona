//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\DateTime.h"
#include "Interop\EventArgs.h"
#include <memory>
#include <string>


namespace Interop { namespace Ipc {

/// <summary>
///  <para>Event arguments providing the text received from an I/O source and time it was retrieved.</para>
///  <para>It's up to the caller to assume what the encoding of the text is. (Corona assumes it to be UTF-8.)</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
class IODataEventArgs : public EventArgs
{
	public:
		/// <summary>
		///  <para>Stores the data received from an I/O source.</para>
		///  <para>This data structure is used to create an IODataEventArgs instance.</para>
		/// </summary>
		struct Data
		{
			/// <summary>The date and time the text I/O was received.</summary>
			DateTime Timestamp;

			/// <summary>Immutable shared pointer of the text received from the I/O source.</summary>
			std::shared_ptr<const std::string> Text;
		};

		/// <summary>Creates a new event arguments object with the given data.</summary>
		/// <param name="data">The data received from the I/O source.</param>
		IODataEventArgs(const IODataEventArgs::Data& data);

		/// <summary>Destroys this object.</summary>
		virtual ~IODataEventArgs();

		/// <summary>Gets the date and time the text I/O was received.</summary>
		/// <returns>Returns the date and time the text I/O was received.</returns>
		DateTime GetTimestamp() const;

		/// <summary>Gets the text I/O received.</summary>
		/// <returns>Returns a shared pointer of the text received as an immutable string.</returns>
		std::shared_ptr<const std::string> GetText() const;

	private:
		/// <summary>Assignment operator made private to make this object immutable.</summary>
		void operator=(const IODataEventArgs& value) {}

		/// <summary>Stores the I/O event's data.</summary>
		const IODataEventArgs::Data fData;
};

} }	// namespace Interop::Ipc

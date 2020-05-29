//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

#pragma region Forward Declarations
namespace Graphics
{
	interface class IImageServices;
}
namespace Input
{
	interface class IInputDeviceServices;
}
namespace Networking
{
	interface class INetworkServices;
}
namespace Storage
{
	interface class IResourceServices;
}
namespace UI
{
	interface class IUserInterfaceServices;
}
interface class IApplicationServices;
interface class ITimerServices;
ref class CoronaInteropSettings;

#pragma endregion


/// <summary>
///  <para>Provides cross-platform and cross-language feature implementations to the C/C++ side of Corona.</para>
///  <para>This is needed if a feature is only available in .NET, such as Windows Phone 8.0's .NET Xaml framework.</para>
///  <para>
///   This is also needed to implement features between Windows Phone 8.0 and Windows Store Universal app
///   which use different class frameworks.
///  </para>
/// </summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class InteropServices sealed
{
	public:
		/// <summary>Creates a new interop services object using the given settings.</summary>
		/// <param name="settings">
		///  <para>Settings providing the interop objects that the new InteropServices object will use.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		///  <para>
		///   All of the setting's properties must be set or else an exception will be thrown,
		///   except for the UserIntefaceServices and WeakCoronaControlReference properties which are allowed to be null.
		///  </para>
		/// </param>
		InteropServices(CoronaInteropSettings^ settings);

		/// <summary>Gets an object used to access the currently running application's information and functionality.</summary>
		/// <value>Reference to an object used to access the application's information and functionality.</value>
		property IApplicationServices^ ApplicationServices { IApplicationServices^ get(); }

		/// <summary>Gets an object used to decode/encode image files and generate text bitmaps.</summary>
		/// <value>Reference to an object that can decode and encode images.</value>
		property Graphics::IImageServices^ ImageServices { Graphics::IImageServices^ get(); }

		/// <summary>Gets an object which provides input events from the touchscreen, keyboard, and other devices.</summary>
		/// <value>Reference to an object which provides input events and input device related properties.</value>
		property Input::IInputDeviceServices^ InputDeviceServices { Input::IInputDeviceServices^ get(); }

		/// <summary>Gets or sets an object which provides access to network status and commmunications features.</summary>
		/// <value>Reference to an object which provides networking features.</value>
		property Networking::INetworkServices^ NetworkServices { Networking::INetworkServices^ get(); }

		/// <summary>Gets an object used to access embedded resources such as Corona's widget library images.</summary>
		/// <value>Reference to an object which provides access to embedded resources.</value>
		property Storage::IResourceServices^ ResourceServices { Storage::IResourceServices^ get(); };

		/// <summary>Gets an object used to create timers.</summary>
		/// <value>Reference to an object used to create timers.</value>
		property ITimerServices^ TimerServices { ITimerServices^ get(); }

		/// <summary>Gets an objects used to create UI controls.</summary>
		/// <value>
		///  <para>Reference to an object used to create UI controls.</para>
		///  <para>Set to null if the Corona runtime will not be rendering anything or interacting with the UI.</para>
		/// </value>
		property UI::IUserInterfaceServices^ UserInterfaceServices { UI::IUserInterfaceServices^ get(); }

	private:
		/// <summary>Copy of the interop settings that this class provides read-only access to.</summary>
		CoronaInteropSettings^ fSettings;
};

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop

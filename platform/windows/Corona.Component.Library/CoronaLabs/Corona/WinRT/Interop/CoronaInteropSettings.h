// ----------------------------------------------------------------------------
// 
// CoronaInteropSettings.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
interface class IReadOnlyWeakReference;
interface class ITimerServices;

#pragma endregion


/// <summary>
///  <para>Provides settable cross-platform and cross-language feature implementations to the C/C++ side of Corona.</para>
///  <para>This is needed if a feature is only available in .NET, such as Windows Phone 8.0's .NET Xaml framework.</para>
///  <para>
///   This is also needed to implement features between Windows Phone 8.0 and Windows Store Universal app
///   which use different class frameworks.
///  </para>
///  <para>An instance of this class is intended to be given to the CoronaRuntime's constructor.</para>
/// </summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class CoronaInteropSettings sealed
{
	public:
		/// <summary>Creates a new settings object initialized to its defaults.</summary>
		CoronaInteropSettings();

		/// <summary>Copies the given settings to this object.</summary>
		/// <param name="settings">
		///  <para>Reference to the settings object to copy from.</para>
		///  <para>Will be ignored if set to null.</para>
		/// </param>
		void CopyFrom(CoronaInteropSettings^ settings);

		/// <summary>Gets or sets an object that will provide access to the currently running application's features.</summary>
		/// <value>
		///  <para>Reference to an object used to access the currently running application information and features.</para>
		///  <para>Set to null (the default) if this property has not been assigned yet.</para>
		/// </value>
		property IApplicationServices^ ApplicationServices;

		/// <summary>
		///  <para>Gets or sets an object that will decode and encode image files.</para>
		///  <para>Corona needs this to load image files, save screen captures to file, and generate text bitmaps.</para>
		/// </summary>
		/// <value>
		///  <para>Reference to an object that can decode and encode images.</para>
		///  <para>Set to null (the default) if this property has not been assigned yet.</para>
		/// </value>
		property Graphics::IImageServices^ ImageServices;

		/// <summary>
		///  Gets or sets an object which provides input events from the touchscreen, keyboard, and other devices.
		/// </summary>
		/// <value>
		///  <para>Reference to an object which provides input events and input device related properties.</para>
		///  <para>Set to null (the default) if this property has not been assigned yet.</para>
		/// </value>
		property Input::IInputDeviceServices^ InputDeviceServices;

		/// <summary>Gets or sets an object which provides access to network status and commmunications features.</summary>
		/// <value>
		///  <para>Reference to an object which provides networking features.</para>
		///  <para>Set to null (the default) if this property has not been assigned yet.</para>
		/// </value>
		property Networking::INetworkServices^ NetworkServices;

		/// <summary>Gets or sets an object used to access embedded resources such as Corona's widget library images.</summary>
		/// <value>
		///  <para>Reference to an object which provides access to embedded resources.</para>
		///  <para>Set to null (the default) if this property has not been assigned yet.</para>
		/// </value>
		property Storage::IResourceServices^ ResourceServices;

		/// <summary>Gets or sets the object used to create timers.</summary>
		/// <value>
		///  <para>Reference to an object used to create timers.</para>
		///  <para>Set to null (the default) if this property has not been assigned yet.</para>
		/// </value>
		property ITimerServices^ TimerServices;

		/// <summary>Gets or sets an objects used to create UI controls.</summary>
		/// <value>
		///  <para>Reference to an object used to create UI controls.</para>
		///  <para>Set to null (the default) if this property has not been assigned yet.</para>
		/// </value>
		property UI::IUserInterfaceServices^ UserInterfaceServices;

		/// <summary>Gets or sets a weak reference to the control that Corona will be rendering to.</summary>
		/// <remarks>
		///  <para>
		///   This is a weak reference to the control so that the control can be garbage collected when it is no longer
		///   attached to a page and all references to it has been dropped. That is, this prevents this settings object
		///   from keeping the control alive with a strong reference.
		///  </para>
		///  <para>
		///   This property stores the control reference as type "object" so that this property can support both the
		///   the Windows Phone and Windows Universal Xaml frameworks. The control object is expected to be of a type
		///   that can be wrapped by Corona's "ICoronaControlAdapter" interface, which allows the CoronaRuntime to
		///   render to the control's surface.
		///  </para>
		///  <para>
		///   On Windows Phone, this property is expected to reference a "CoronaPanel" object that owns the CoronaRuntime.
		///  </para>
		/// </remarks>
		/// <value>
		///  <para>Weak reference to the control that the CoronaRuntime will be rendering to.</para>
		///  <para>Set to null if this property has not been set yet.</para>
		///  <para>Leaving this property null is allowed, but will prevent Corona from rendering to the screen.</para>
		/// </value>
		property IReadOnlyWeakReference^ WeakCoronaControlReference;
};

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop

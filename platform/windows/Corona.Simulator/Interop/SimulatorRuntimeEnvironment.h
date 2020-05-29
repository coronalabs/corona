//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Interop\LuaMethodCallback.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_WinInputDeviceManager.h"
#include <vector>
#include <Windows.h>


#pragma region Forward Declarations
namespace Rtt
{
	class WinSimulatorServices;
}

#pragma endregion


namespace Interop {

/// <summary>Provides information and services for one Corona runtime that is simulating a device.</summary>
class SimulatorRuntimeEnvironment : public RuntimeEnvironment
{
	Rtt_CLASS_NO_COPIES(SimulatorRuntimeEnvironment)

	public:
		#pragma region CreationSettings Structure
		/// <summary>Provides settings to be passed into the RuntimeEnvironment class' constructor.</summary>
		struct CreationSettings : public RuntimeEnvironment::CreationSettings
		{
			/// <summary>
			///  <para>Pointer to a device configuration to be simulated by the runtime.</para>
			///  <para>Set to null to not simulate a device. In this case, the app runs like a Win32 application.</para>
			/// </summary>
			Rtt::PlatformSimulator::Config* DeviceConfigPointer;

			/// <summary>
			///  <para>Pointer to the Corona Simulator app's features, such as the ability to create and run project.</para>
			///  <para>Expected to only be set when running the Corona Simulator welcom window project.</para>
			///  <para>Set to null to not provide access to the Corona Smulator app's features.</para>
			/// </summary>
			Rtt::WinSimulatorServices* CoronaSimulatorServicesPointer;

			/// <summary>Creates a new settings object initialized to its defaults.</summary>
			CreationSettings()
			:	RuntimeEnvironment::CreationSettings(),
				DeviceConfigPointer(nullptr),
				CoronaSimulatorServicesPointer(nullptr)
			{
			}
		};

		#pragma endregion


		#pragma region CreationResult
		/// <summary>
		///  <para>Type returned by the SimulatorRuntimeEnvironment class' CreateUsing() static function.</para>
		///  <para>Determines if the CreateUsing() function successfully created a new runtime environment object.</para>
		///  <para>If successful, the GetPointer() method will provide the newly created runtime environment object.</para>
		/// </summary>
		typedef PointerResult<SimulatorRuntimeEnvironment> CreationResult;

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  Gets a pointer to an optional interface providing device simulation services for the Corona Simulator.
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to a device simulation interface if running under the Corona Simulator.</para>
		///  <para>Returns null if the Corona runtime is not simulating a device.</para>
		/// </returns>
		virtual MDeviceSimulatorServices* GetDeviceSimulatorServices() const override;

		/// <summary>
		///  <para>Adds a mouse cursor rollover region effect to the top of the rendering surface.</para>
		///  <para>If regions overlap, then the last one added wins.</para>
		/// </summary>
		/// <param name="style">
		///  Mouse cursor style to be displayed while the mouse is hovering over the given region.
		/// </param>
		/// <param name="region">The region in Corona scaled content coordinates.</param>
		void AddMouseCursorRegion(Rtt::WinInputDeviceManager::CursorStyle style, const RECT& region);

		/// <summary>
		///  Removes all regions that were given to the AddMouseCursorRegion() method that exactly match the given region.
		/// </summary>
		/// <param name="region">A region in Corona scaled content coordinates.</param>
		void RemoveMouseCursorRegion(const RECT& region);

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>
		///  <para>Creates a new Corona runtime environment using the given creation/project settings.</para>
		///  <para>
		///   The Corona runtime will start running immediately upon return, provided that the
		///   "IsRuntimeCreationEnabled" setting is set to true.
		///  </para>
		///  <para>
		///   The environment's "Loaded" event will be raised before this function returns,
		///   provided that the project was successfully loaded.
		///  </para>
		///  <para>
		///   To delete the returned runtime environment object, you must pass it into this class' static Destroy() function.
		///  </para>
		/// </summary>
		/// <param name="settings">
		///  <para>Provides Corona project settings such as directory paths, the window to render to, etc.</para>
		///  <para>
		///   If the "IsRuntimeCreationEnabled" field is set false, then only an Rtt::Platform object will be
		///   created and a Corona project will never be loaded/started.
		///  </para>
		/// </param>
		/// <returns>
		///  <para>
		///   Returns a success result and a pointer to a new runtime environment object if creation was
		///   successful and was able to load the given Corona project if applicable.
		///  </para>
		///  <para>
		///   Returns a failure result if creation or project loading failed. The result's GetMessage() method
		///   will provide details as to what went wrong.
		///  </para>
		/// </returns>
		static SimulatorRuntimeEnvironment::CreationResult CreateUsing(
				const SimulatorRuntimeEnvironment::CreationSettings& settings);
		
		/// <summary>
		///  <para>
		///   Destroys the runtime environment object that was returned by this class' static CreateUsing() function.
		///  </para>
		///  <para>This will automatically terminate the runtime if it is currently running.</para>
		/// </summary>
		/// <param name="environmentPointer">
		///  <para>Pointer to the runtime enivornment to be deleted/destroyed.</para>
		///  <para>A null pointer will be safely ignored.</para>
		/// </para>
		static void Destroy(SimulatorRuntimeEnvironment* environmentPointer);

		#pragma endregion

	protected:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new Corona Simulator runtime environment with the given settings.</summary>
		/// <param name="settings">
		///  <para>Provides launch settings and interop layer settings to be copied to the new environment object.</para>
		///  <para>Will throw an exception if any of the settings are invalid.</para>
		/// </param>
		SimulatorRuntimeEnvironment(const SimulatorRuntimeEnvironment::CreationSettings& settings);

		/// <summary>Terminates the runtime, if running, and deletes resources consumed by the Corona environment.</summary>
		virtual ~SimulatorRuntimeEnvironment();

		#pragma endregion


	private:
		#pragma region DeviceSimulatorServices Class
		class DeviceSimulatorServices : public MDeviceSimulatorServices
		{
			public:
				DeviceSimulatorServices(
						SimulatorRuntimeEnvironment* environmentPointer,
						const Rtt::PlatformSimulator::Config* deviceConfigPointer);

				const Rtt::PlatformSimulator::Config* GetDeviceConfig() const;
				virtual const char* GetManufacturerName() const override;
				virtual const char* GetModelName() const override;
				virtual bool IsLuaExitAllowed() const override;
				virtual bool IsScreenRotationSupported() const override;
				virtual bool IsMouseSupported() const override;
				virtual bool AreInputDevicesSupported() const override;
				virtual bool AreKeyEventsSupported() const override;
				virtual bool AreKeyEventsFromKeyboardSupported() const override;
				virtual bool IsBackKeySupported() const override;
				virtual bool AreExitRequestsSupported() const override;
				virtual bool AreMultipleAlertsSupported() const override;
				virtual bool IsAlertButtonOrderRightToLeft() const override;
				virtual double GetZoomScale() const override;
				virtual Rtt::DeviceOrientation::Type GetOrientation() const override;
				virtual void SetOrientation(Rtt::DeviceOrientation::Type value) override;
				virtual POINT GetSimulatedPointFromClient(const POINT& value) override;
				virtual double GetDefaultFontSize() const override;
				virtual int GetScreenWidthInPixels() const override;
				virtual int GetScreenHeightInPixels() const override;
				virtual int GetAdaptiveScreenWidthInPixels() const override;
				virtual int GetAdaptiveScreenHeightInPixels() const override;
				virtual const char* GetStatusBarImageFilePathFor(Rtt::MPlatform::StatusBarMode value) const override;
				virtual void RotateClockwise() override;
				virtual void RotateCounterClockwise() override;
				virtual void* ShowNativeAlert(
								const char *title, const char *message, const char **buttonLabels,
								int buttonCount, Rtt::LuaResource* resource) override;
				virtual void CancelNativeAlert(void* alertReference) override;
				virtual void SetActivityIndicatorVisible(bool value) override;
				virtual void RequestRestart() override;
				virtual void RequestTerminate() override;
				virtual void Shake() override;
				virtual const char* GetOSName() const override;
				virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const override;
				virtual Rtt::MPlatform::StatusBarMode GetStatusBar() const override;
				virtual void SetStatusBar(Rtt::MPlatform::StatusBarMode newValue) override;

			private:
				SimulatorRuntimeEnvironment* fEnvironmentPointer;
				const Rtt::PlatformSimulator::Config* fDeviceConfigPointer;
				Rtt::DeviceOrientation::Type fCurrentOrientation;
				Rtt::MPlatform::StatusBarMode fCurrentStatusBar;
		};

		#pragma endregion


		#pragma region MouseCursorRegion Structure
		/// <summary>
		///  Private structure used to store a mouse cursor rollover region for the Corona Simulator's welcome window.
		/// </summary>
		struct MouseCursorRegion
		{
			/// <summary>The mouse cursor icon to be used when rolling over this object's region.</summary>
			Rtt::WinInputDeviceManager::CursorStyle CursorStyle;

			/// <summary>The mouse rollover region in Corona scaled content coordinates.</summary>
			RECT CoronaContentBounds;
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>
		///  <para>
		///   Called when the Corona runtime finished loading the "config.lua" and just before the runtime
		///   executes the "main.lua".
		///  </para>
		///  <para>This is this object's opportunity to register Corona Simulator APIs into Lua.</para>
		/// </summary>
		/// <param name="sender">The Corona runtime environment that raised this event.</para>
		/// <param name="arguments">Empty event arguments.</param>
		void OnRuntimeLoaded(RuntimeEnvironment& sender, const EventArgs& arguments);

		/// <summary>Called when the Corona runtime is about to be terminated.</summary>
		/// <param name="sender">The Corona runtime environment that raised this event.</para>
		/// <param name="arguments">Empty event arguments.</param>
		void OnRuntimeTerminating(RuntimeEnvironment& sender, const EventArgs& arguments);

		/// <summary>Called when a Lua "mouse" event has been received from the Corona runtime.</summary>
		/// <param name="luaStatePointer">Pointer to the Lua state that invoked this method.</param>
		/// <returns>
		///  <para>Returns the number of values pushed to the Lua stack as Lua return values.</para>
		///  <para>Returns zero if this function is not returning any values to Lua.</para>
		/// </returns>
		int OnLuaMouseEventReceived(lua_State* luaStatePointer);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Handler to be invoked when the "Loaded" event has been raised.</summary>
		RuntimeEnvironment::LoadedEvent::MethodHandler<SimulatorRuntimeEnvironment> fLoadedEventHandler;

		/// <summary>Handler to be invoked when the "Terminating" event has been raised.</summary>
		RuntimeEnvironment::LoadedEvent::MethodHandler<SimulatorRuntimeEnvironment> fTerminatingEventHandler;

		/// <summary>Registers the OnLuaMouseEventReceived() method as a Lua "mouse" event listener.</summary>
		LuaMethodCallback<SimulatorRuntimeEnvironment> fLuaMouseEventCallback;

		/// <summary>Interface providing device simulation features for the Corona Simulator.</summary>
		DeviceSimulatorServices* fDeviceSimulatorServicesPointer;

		/// <summary>
		///  <para>Pointer to the Corona Simulator's welcome screen and main application features and services.</para>
		///  <para>Should only be set if running the welcom screen's Corona project.</para>
		///  <para>Will be null if running a device simulation or a Win32 desktop app.</para>
		/// </summary>
		Rtt::WinSimulatorServices* fCoronaSimulatorServicesPointer;

		/// <summary>
		///  Stores a collection of mouse cursor rollover regions to be ued by the simulator's welcome window.
		/// </summary>
		std::vector<MouseCursorRegion> fCursorRegionCollection;

		#pragma endregion
};

}	// namespace Interop

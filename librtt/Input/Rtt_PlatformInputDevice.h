//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformInputDevice_H__
#define _Rtt_PlatformInputDevice_H__

#include "Rtt_InputAxisType.h"
#include "Rtt_InputAxisCollection.h"
#include "Rtt_InputDeviceType.h"
#include "Rtt_InputDeviceConnectionState.h"
#include "Rtt_InputDeviceDescriptor.h"
#include "Rtt_ReadOnlyInputAxisCollection.h"


// Forward declarations.
struct Rtt_Allocator;
extern "C"
{
	struct lua_State;
}
namespace Rtt
{
	class InputAxisDescriptor;
	class PlatformInputAxis;
}


namespace Rtt
{

/// Provides access to one input device's information and functionality.
/// Also provides a Lua interface that can be pushed into the Lua state.
class PlatformInputDevice
{
	public:
		PlatformInputDevice(const InputDeviceDescriptor &descriptor);
		virtual ~PlatformInputDevice();

		static const char kMetatableName[];

		Rtt_Allocator* GetAllocator() const;
		InputDeviceDescriptor& GetDescriptor();
		virtual const char* GetProductName();
		virtual const char* GetDisplayName();
		virtual const char* GetPermanentStringId();
		virtual const char* GetDriverName();
		bool HasPlayerNumber();
		virtual int GetPlayerNumber();
		virtual InputDeviceConnectionState GetConnectionState();
		virtual bool CanVibrate();
		virtual void Vibrate();
		const ReadOnlyInputAxisCollection& GetAxes() const;
		PlatformInputAxis* AddAxis();
		void RemoveAllAxes();
		void PushTo(lua_State *L);

// Note: KeyEvent and MouseEvent objects will call these functions if they have a pointer to this object.
//		void Dispatch(const KeyEvent &event);
//		void Dispatch(const MouseEvent &event);

	protected:
		virtual PlatformInputAxis* OnCreateAxisUsing(const InputAxisDescriptor &descriptor);
		virtual void OnDestroyAxis(PlatformInputAxis *axisPointer);
		virtual int OnAccessingField(lua_State *L, const char fieldName[]);
		virtual int OnAssigningField(lua_State *L, const char fieldName[], int valueIndex);

	private:
		// Assignment operator made private to prevent instances from being overwritten.
		void operator=(const PlatformInputDevice &device) { };

		static void AddMetatableTo(lua_State *L);
		static int OnAccessingField(lua_State *L);
		static int OnAssigningField(lua_State *L);
		static int OnFinalizing(lua_State *L);
		static int OnVibrate(lua_State *L);
		static int OnGetAxes(lua_State *L);


		/// Descriptor used to uniquely identify this device.
		InputDeviceDescriptor fDescriptor;

		/// Stores information about all input axes provided by the device.
		InputAxisCollection fAxisCollection;

		/// Read-only collection which wraps the mutable "fAxisCollection" collection.
		/// This is the collection that gets exposed outside of this class.
		ReadOnlyInputAxisCollection fReadOnlyAxisCollection;

		/// Lua reference key to this device's light user data in the Lua registry.
		/// Intended to be re-used for all input events coming from this device.
		int fLuaReferenceKey;

//		LightPointerArray<LuaResource*> fKeyEventListeners;
//		LightPointerArray<LuaResource*> fMouseEventListeners;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_PlatformInputDevice_H__

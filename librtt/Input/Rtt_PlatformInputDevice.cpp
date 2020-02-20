//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_InputAxisDescriptor.h"
#include "Rtt_PlatformInputAxis.h"
#include "Rtt_PlatformInputDevice.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include <string.h>


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

/// Uniquely identifies the InputDevice type in Lua.
const char PlatformInputDevice::kMetatableName[] = "InputDevice";


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new interface to an input device.
/// @param descriptor Unique descriptor used to identify this input device.
PlatformInputDevice::PlatformInputDevice(const InputDeviceDescriptor &descriptor)
:	fDescriptor(descriptor),
	fAxisCollection(descriptor.GetAllocator()),
	fReadOnlyAxisCollection(&fAxisCollection),
	fLuaReferenceKey(LUA_NOREF)
{
}

/// Destroys this input device and its resources.
PlatformInputDevice::~PlatformInputDevice()
{
	RemoveAllAxes();
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the allocator that this device object uses to create input axis objects.
/// @return Returns a pointer to the allocator this input device uses.
Rtt_Allocator* PlatformInputDevice::GetAllocator() const
{
	return fDescriptor.GetAllocator();
}

/// Gets the descriptor used to uniquely identify this device.
/// Provides information such as device type, device number, and its unique descriptor name.
/// @return Returns this device's descriptor by reference.
InputDeviceDescriptor& PlatformInputDevice::GetDescriptor()
{
	return fDescriptor;
}

/// Gets the name of the device as assigned by the manufacturer.
/// <br>
/// If the device is a gamepad or joystick, then this name is typically used to determined
/// which axis inputs and key/button inputs are what on the device.  Especially when determining
/// which axis inputs make up the right thumbstick on a gamepad since there is no standard.
/// @return Returns the device's name as assigned to it by the manufacturer.
///         <br>
///         Returns NULL if the product name could not be obtained.
const char* PlatformInputDevice::GetProductName()
{
	return NULL;
}

/// Gets a descriptive name assigned to the device by the end-user or by the system.
/// <br>
/// This is typically the product name if the display/alias name could not be obtained.
/// @return Returns the device's display name.
///         <br>
///         Returns NULL if the display name could not be obtained.
const char* PlatformInputDevice::GetDisplayName()
{
	return NULL;
}

/// Gets a unique string ID assigned to the device that can be saved to file.
/// <br>
/// Unlike the device's descriptor name or native device ID, this string ID will always
/// identify this specific device. Even after restarting the app or rebooting the device.
/// @return Returns the device's unique string ID.
///         <br>
///         Returns NULL if the device does not have a permanent string ID assigned to it.
const char* PlatformInputDevice::GetPermanentStringId()
{
	return NULL;
}

/// Gets which driver/backend controller uses.
/// <br>
/// Some drivers have fixed layouts, which may be helpful to be able to identify
/// @return controller driver/backend string or NULL if none specified.
const char* PlatformInputDevice::GetDriverName()
{
	return NULL;
}
	
/// Determines if a "player number" has been assigned to the input device by the system.
/// If true, then the GetPlayerNumber() will return a valid value.
/// <br>
/// This is typically supported by game controllers for Microsoft XInput, iOS, and Android.
/// @return Returns true if a player number has been assigned to the device by the system. Returns false if not.
bool PlatformInputDevice::HasPlayerNumber()
{
	return (GetPlayerNumber() >= 1);
}

/// Gets a one based player number assigned to the device by the system.
/// @return Returns a 1 based number assigned to the device.
///         <br>
///         Returns a value less than 1 if a number was not assigned to the device.
int PlatformInputDevice::GetPlayerNumber()
{
	return 0;
}

/// Gets the input device's current connection state such as kConnected, kDisconnected, etc.
/// @return Returns the input device's current connection state.
InputDeviceConnectionState PlatformInputDevice::GetConnectionState()
{
	return InputDeviceConnectionState::kDisconnected;
}

/// Determines if the input device supports vibrate/rumble functionality.
/// @return Returns true if the device support vibration feedback. Returns false if not.
bool PlatformInputDevice::CanVibrate()
{
	return false;
}

/// Causes the controller to vibrate/rumble, if supported.
/// <br>
/// You can determine if the device supports vibration feedback if function CanVibrate() returns true.
void PlatformInputDevice::Vibrate()
{
}

/// Fetches a collection of all input axes provided by this input device.
/// @return Returns a read-only collection of all input axes.
///         The collection will be empty if the device does not have any input axes.
const ReadOnlyInputAxisCollection& PlatformInputDevice::GetAxes() const
{
	return fReadOnlyAxisCollection;
}

/// Adds a new input axis to the device.
/// @return Returns a pointer to the new input axis that was created.
///         <br>
///         Returns NULL if failed to create the axis object.
PlatformInputAxis* PlatformInputDevice::AddAxis()
{
	// Generate a unique axis number and descriptor for the new axis.
	S32 axisNumber = fAxisCollection.GetCount() + 1;
	InputAxisDescriptor axisDescriptor(fDescriptor, axisNumber);

	// Create the axis and add it to the end of the collection.
	PlatformInputAxis *axisPointer = OnCreateAxisUsing(axisDescriptor);
	fAxisCollection.Add(axisPointer);

	// Return the new axis so that the caller can finish configuring it.
	return axisPointer;
}

/// Removes all input axes from this input device.
void PlatformInputDevice::RemoveAllAxes()
{
	for (S32 index = fAxisCollection.GetCount() - 1; index >= 0; index--)
	{
		Rtt_DELETE(fAxisCollection.GetByIndex(index));
	}
	fAxisCollection.Clear();
}

/// Pushes this object as Lua userdata into the given Lua state.
/// @param L The Lua state to push this object into.
void PlatformInputDevice::PushTo(lua_State *L)
{
	// Validate.
	if (NULL == L)
	{
		return;
	}

	// Push this object as userdata into the Lua registry, if not done already.
	// This Lua userdata will be re-used for all input events.
	if (LUA_NOREF == fLuaReferenceKey)
	{
		PlatformInputDevice::AddMetatableTo(L);
		Lua::PushUserdata(L, this, PlatformInputDevice::kMetatableName);
		fLuaReferenceKey = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	// Fetch this object's userdata from the Lua registry and push it to the top of the stack.
	lua_rawgeti(L, LUA_REGISTRYINDEX, fLuaReferenceKey);
}


// ----------------------------------------------------------------------------
// Protected Member Functions
// ----------------------------------------------------------------------------

/// Called when the AddAxis() function has been called.
/// Creates a new PlatformInputAxis object using the given descriptor.
/// <br>
/// Can be overriden by a derived class to create and return a different type of input axis object
/// in case platform specific settings need to be added to it.
/// @param descriptor Unique descriptor used to identify the new input axis.
/// @return Returns a pointer to the new input axis object that was created.
PlatformInputAxis* PlatformInputDevice::OnCreateAxisUsing(const InputAxisDescriptor &descriptor)
{
	return Rtt_NEW(GetAllocator(), PlatformInputAxis(descriptor));
}

/// Called when this device object needs to delete an input axis object.
/// <br>
/// Should be overriden by a derived class if the OnCreateAxisUsing() function is overriden.
/// The intent is to delete the object in the same class that it was created in.
/// This is important on platforms such as Windows where each module/library has its own
/// heap and you must delete an object in the same module that it was created in.
/// @param axisPointer Pointer to the input axis to be deleted.
void PlatformInputDevice::OnDestroyAxis(PlatformInputAxis *axisPointer)
{
	Rtt_DELETE(axisPointer);
}

/// Called when Lua is attempting to access the input device object's property or function.
/// @param L The Lua state that is performing the access operation.
/// @param fieldName The name of the field being accessed such as "type", "productName", etc.
/// @return Returns the number of values being returned to Lua.
int PlatformInputDevice::OnAccessingField(lua_State *L, const char fieldName[])
{
	// Handle the accessed field.
	int result = 0;
	if (strcmp("descriptor", fieldName) == 0)
	{
		lua_pushstring(L, fDescriptor.GetInvariantName());
		result = 1;
	}
	else if (strcmp("type", fieldName) == 0)
	{
		lua_pushstring(L, fDescriptor.GetDeviceType().GetStringId());
		result = 1;
	}
	else if (strcmp("productName", fieldName) == 0)
	{
		const char* name = GetProductName();
		if (name)
		{
			lua_pushstring(L, name);
		}
		else
		{
			lua_pushnil(L);
		}
		result = 1;
	}
	else if (strcmp(fieldName, "displayName") == 0)
	{
		const char* name = GetDisplayName();
		if (name)
		{
			lua_pushstring(L, name);
		}
		else
		{
			lua_pushnil(L);
		}
		result = 1;
	}
	else if (strcmp("permanentId", fieldName) == 0)
	{
		const char* stringId = GetPermanentStringId();
		if (stringId)
		{
			lua_pushstring(L, stringId);
		}
		else
		{
			lua_pushnil(L);
		}
		result = 1;
	}
	else if (strcmp("playerNumber", fieldName) == 0)
	{
		if (HasPlayerNumber())
		{
			lua_pushinteger(L, GetPlayerNumber());
		}
		else
		{
			lua_pushnil(L);
		}
		result = 1;
	}
	else if (strcmp("driver", fieldName) == 0)
	{
		const char* stringDriver = GetDriverName();
		if (stringDriver)
		{
			lua_pushstring(L, stringDriver);
		}
		else
		{
			lua_pushnil(L);
		}
		result = 1;
	}
	else if (strcmp("canVibrate", fieldName) == 0)
	{
		bool value = CanVibrate();
		lua_pushboolean(L, value ? 1 : 0);
		result = 1;
	}
	else if (strcmp("vibrate", fieldName) == 0)
	{
		lua_pushcfunction(L, OnVibrate);
		result = 1;
	}
	else if (strcmp("getAxes", fieldName) == 0)
	{
		lua_pushcfunction(L, OnGetAxes);
		result = 1;
	}
	else if (strcmp("connectionState", fieldName) == 0)
	{
		lua_pushstring(L, GetConnectionState().GetStringId());
		result = 1;
	}
	else if (strcmp("isConnected", fieldName) == 0)
	{
		bool value = GetConnectionState().IsConnected();
		lua_pushboolean(L, value ? 1 : 0);
		result = 1;
	}
	else if (strcmp("addEventListener", fieldName) == 0)
	{
	}
	else if (strcmp("removeEventListener", fieldName) == 0)
	{
	}

	// Return the number of Lua values returned by this field.
	return result;
}

/// Called when Lua is attempting to write to an input device object's property.
/// @param L The Lua state that is performing the assignment operation.
/// @param fieldName The name of the field being written to.
/// @param valueIndex Index to the Lua value that is intended to replace the field's value with.
/// @return Returns the number of values being returned to Lua.
int PlatformInputDevice::OnAssigningField(lua_State *L, const char fieldName[], int valueIndex)
{
	return 0;
}


// ----------------------------------------------------------------------------
// Static Functions
// ----------------------------------------------------------------------------

/// Creates a new metatable for this input device and adds it to the Lua registry.
/// This metatable is used to provide properties and functions the device's userdata object in Lua.
/// @param L The Lua state to add the metatable to.
void PlatformInputDevice::AddMetatableTo(lua_State *L)
{
	// Validate argument.
	if (NULL == L)
	{
		return;
	}

	// Create the input device's metatable.
	// Note: Will do nothing if the metatable already exists in the Lua registry.
	Rtt_LUA_STACK_GUARD(L);
	const luaL_Reg kVTable[] =
	{
		{ "__index", PlatformInputDevice::OnAccessingField },
		{ "__newindex", PlatformInputDevice::OnAssigningField },
		{ "__gc", PlatformInputDevice::OnFinalizing },
		{ NULL, NULL }
	};
	Lua::InitializeMetatable(L, PlatformInputDevice::kMetatableName, kVTable);
}

/// Called when Lua is attempting to access an input device object's property or function.
/// @param L The Lua state that is performing the access operation.
/// @return Returns the number of values being returned to Lua.
int PlatformInputDevice::OnAccessingField(lua_State *L)
{
	// Validate.
	if (NULL == L)
	{
		return 0;
	}

	// Fetch the input device object that was accessed from Lua.
	PlatformInputDevice *devicePointer;
	devicePointer = (PlatformInputDevice*)Lua::CheckUserdata(L, 1, PlatformInputDevice::kMetatableName);
	if (NULL == devicePointer)
	{
		return 0;
	}

	// Fetch the name of the field that was accessed in Lua.
	const char *fieldName = luaL_checkstring(L, 2);
	if (Rtt_StringIsEmpty(fieldName))
	{
		return 0;
	}

	// Access the requested field.
	return devicePointer->OnAccessingField(L, fieldName);
}

/// Called when Lua is attempting to write to an input device object's property.
/// @param L The Lua state that is performing the assignment operation.
/// @return Returns the number of values being returned to Lua.
int PlatformInputDevice::OnAssigningField(lua_State *L)
{
	// Validate.
	if (NULL == L)
	{
		return 0;
	}

	// Fetch the input device object that was accessed from Lua.
	PlatformInputDevice *devicePointer;
	devicePointer = (PlatformInputDevice*)Lua::CheckUserdata(L, 1, PlatformInputDevice::kMetatableName);
	if (NULL == devicePointer)
	{
		return 0;
	}

	// Fetch the name of the field that the assignment operation is being executed on in Lua.
	const char *fieldName = luaL_checkstring(L, 2);
	if (Rtt_StringIsEmpty(fieldName))
	{
		return 0;
	}

	// Write to the requested field.
	return devicePointer->OnAssigningField(L, fieldName, 3);
}

/// Called when this object's Lua userdata is being garbage collected by Lua.
/// @param L The Lua state that the userdata is being garbage collected in.
/// @return Returns the number of return values. Should always be zero for this function.
int PlatformInputDevice::OnFinalizing(lua_State *L)
{
	// Validate.
	if (NULL == L)
	{
		return 0;
	}

	// Fetch the input device object.
	PlatformInputDevice *devicePointer;
	devicePointer = (PlatformInputDevice*)Lua::CheckUserdata(L, 1, PlatformInputDevice::kMetatableName);
	if (NULL == devicePointer)
	{
		return 0;
	}

	// Clear the object's Lua registry reference.
	devicePointer->fLuaReferenceKey = LUA_NOREF;
	return 0;
}

/// Called when this object's Lua userdata has had its vibrate() function called.
/// Calls the C++ object's virtual Vibrate() function to be handled by the derived class.
/// @param L The Lua state the vibrate() function was called in.
/// @return Returns the number of return values. Should always be zero for this function.
int PlatformInputDevice::OnVibrate(lua_State *L)
{
	// Validate.
	if (NULL == L)
	{
		return 0;
	}

	// Fetch the input device object.
	PlatformInputDevice *devicePointer;
	devicePointer = (PlatformInputDevice*)Lua::CheckUserdata(L, 1, PlatformInputDevice::kMetatableName);
	if (NULL == devicePointer)
	{
		return 0;
	}

	// Vibrate the device.
	devicePointer->Vibrate();
	return 0;
}

/// Called when this object's Lua userdata has had its getAxes() function called.
/// @param L The Lua state the vibrate() function was called in.
/// @return Returns the number of return values. Should be 1 for this function.
int PlatformInputDevice::OnGetAxes(lua_State *L)
{
	// Validate.
	if (NULL == L)
	{
		return 0;
	}

	// Fetch the input device object.
	PlatformInputDevice *devicePointer;
	devicePointer = (PlatformInputDevice*)Lua::CheckUserdata(L, 1, PlatformInputDevice::kMetatableName);
	if (NULL == devicePointer)
	{
		return 0;
	}

	// Return all axis information as a Lua array/table.
	lua_createtable(L, devicePointer->fAxisCollection.GetCount(), 0);
	for (S32 index = 0; index < devicePointer->fAxisCollection.GetCount(); index++)
	{
		PlatformInputAxis *axisPointer = devicePointer->fAxisCollection.GetByIndex(index);
		if (axisPointer)
		{
			axisPointer->PushTo(L);
			lua_rawseti(L, -2, (int)index + 1);
		}
	}
	return 1;
}

} // namespace Rtt

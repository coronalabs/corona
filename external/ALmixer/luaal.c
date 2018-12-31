/*
	LuaOpenAL
	Copyright (C) 2010  Eric Wing <ewing . public @ playcontrol.net>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

*/
#include "al.h" 
/* #include "alc.h" */

#include "lua.h"
#include "lauxlib.h"

#define LUAAL_VERSION "1.1"

typedef struct luaopenal_Enum  
{
  const char* stringName;
  ALenum alValue;
} luaopenal_Enum;


void luaopenal_initenum(lua_State* lua_state, const luaopenal_Enum* al_enums)
{
	for( ; al_enums->stringName; al_enums++) 
	{
		lua_pushstring(lua_state, al_enums->stringName);
		lua_pushinteger(lua_state, al_enums->alValue);
		lua_settable(lua_state, -3);
	}
}


static const luaopenal_Enum s_luaALEnum[] = {
	{ "INVALID",						AL_INVALID },
	{ "NONE",							AL_NONE },
	{ "FALSE",							AL_FALSE },
	{ "TRUE",							AL_TRUE },
	{ "SOURCE_RELATIVE",				AL_SOURCE_RELATIVE },
	{ "CONE_INNER_ANGLE",				AL_CONE_INNER_ANGLE },
	{ "CONE_OUTER_ANGLE",				AL_CONE_OUTER_ANGLE },
	{ "PITCH",							AL_PITCH },
	{ "POSITION",						AL_POSITION },
	{ "DIRECTION",						AL_DIRECTION },
	{ "VELOCITY",						AL_VELOCITY },
	{ "LOOPING",						AL_LOOPING },
	{ "BUFFER",							AL_BUFFER },
	{ "GAIN",							AL_GAIN },
	{ "MIN_GAIN",						AL_MIN_GAIN },
	{ "MAX_GAIN",						AL_MAX_GAIN },
	{ "ORIENTATION",					AL_ORIENTATION },
/*	{ "CHANNEL_MASK",					AL_CHANNEL_MASK }, */
	{ "SOURCE_STATE",					AL_SOURCE_STATE },
	{ "INITIAL",						AL_INITIAL },
	{ "PLAYING",						AL_PLAYING },
	{ "PAUSED",							AL_PAUSED },
	{ "STOPPED",						AL_STOPPED },
	{ "BUFFERS_QUEUED",					AL_BUFFERS_QUEUED },
	{ "BUFFERS_PROCESSED",				AL_BUFFERS_PROCESSED },
	{ "SEC_OFFSET",						AL_SEC_OFFSET },
	{ "SAMPLE_OFFSET",					AL_SAMPLE_OFFSET },
	{ "BYTE_OFFSET",					AL_BYTE_OFFSET },
	{ "SOURCE_TYPE",					AL_SOURCE_TYPE },
	{ "STATIC",							AL_STATIC },
	{ "STREAMING",						AL_STREAMING },
	{ "UNDETERMINED",					AL_UNDETERMINED },
	{ "FORMAT_MONO8",					AL_FORMAT_MONO8 },
	{ "FORMAT_MONO16",					AL_FORMAT_MONO16 },
	{ "FORMAT_STEREO8",					AL_FORMAT_STEREO8 },
	{ "FORMAT_STEREO16",				AL_FORMAT_STEREO16 },
	{ "REFERENCE_DISTANCE",				AL_REFERENCE_DISTANCE },
	{ "ROLLOFF_FACTOR",					AL_ROLLOFF_FACTOR },
	{ "CONE_OUTER_GAIN",				AL_CONE_OUTER_GAIN },
	{ "MAX_DISTANCE",					AL_MAX_DISTANCE },
	{ "FREQUENCY",						AL_FREQUENCY },
	{ "BITS",							AL_BITS },
	{ "CHANNELS",						AL_CHANNELS },
	{ "SIZE",							AL_SIZE },
/*
	{ "UNUSED",							AL_UNUSED },
	{ "PENDING",						AL_PENDING },
	{ "PROCESSED",						AL_PROCESSED },
*/
	{ "NO_ERROR",						AL_NO_ERROR },
	{ "INVALID_NAME",					AL_INVALID_NAME },
	{ "ILLEGAL_ENUM",					AL_ILLEGAL_ENUM },
	{ "INVALID_VALUE",					AL_INVALID_VALUE },
	{ "ILLEGAL_COMMAND",				AL_ILLEGAL_COMMAND },
	{ "INVALID_OPERATION",				AL_INVALID_OPERATION },
	{ "OUT_OF_MEMORY",					AL_OUT_OF_MEMORY },
	{ "VENDOR",							AL_VENDOR },
	{ "VERSION",						AL_VERSION },
	{ "RENDERER",						AL_RENDERER },
	{ "EXTENSIONS",						AL_EXTENSIONS },
	{ "DOPPLER_FACTOR",					AL_DOPPLER_FACTOR },
	{ "DOPPLER_VELOCITY",				AL_DOPPLER_VELOCITY },
	{ "SPEED_OF_SOUND",					AL_SPEED_OF_SOUND },
	{ "DISTANCE_MODEL",					AL_DISTANCE_MODEL },
	{ "INVERSE_DISTANCE",				AL_INVERSE_DISTANCE },
	{ "INVERSE_DISTANCE_CLAMPED",		AL_INVERSE_DISTANCE_CLAMPED },
	{ "LINEAR_DISTANCE",				AL_LINEAR_DISTANCE },
	{ "LINEAR_DISTANCE_CLAMPED",		AL_LINEAR_DISTANCE_CLAMPED },
	{ "EXPONENT_DISTANCE",				AL_EXPONENT_DISTANCE },
	{ "EXPONENT_DISTANCE_CLAMPED",		AL_EXPONENT_DISTANCE_CLAMPED },
	{ 0, 0 }
};


#define LUAOPENAL_BOOL_TYPE 1
#define LUAOPENAL_INT_TYPE 2
#define LUAOPENAL_FLOAT_TYPE 3


#define LUAOPENAL_INT_3_TYPE 4
#define LUAOPENAL_FLOAT_3_TYPE 5
#define LUAOPENAL_FLOAT_6_TYPE 6
#define LUAOPENAL_STRING_TYPE 7


static int lua_getTypeForEnum(ALenum enum_parameter)
{
	switch(enum_parameter)
	{
		case AL_SOURCE_RELATIVE:
		case AL_LOOPING:
		{
			return LUAOPENAL_BOOL_TYPE;
			break;
		}

		case AL_SOURCE_STATE:
		case AL_SOURCE_TYPE:
		case AL_BUFFER:
		case AL_BUFFERS_QUEUED:
		case AL_BUFFERS_PROCESSED:
		{
			return LUAOPENAL_INT_TYPE;
			break;
		}
		case AL_GAIN:
		case AL_MIN_GAIN:
		case AL_MAX_GAIN:
		case AL_REFERENCE_DISTANCE: /* could be integer */
		case AL_ROLLOFF_FACTOR: /* could be integer */
		case AL_MAX_DISTANCE: /* could be integer */
		case AL_PITCH:
		case AL_CONE_INNER_ANGLE: /* could be integer */
		case AL_CONE_OUTER_ANGLE: /* could be integer */
		case AL_CONE_OUTER_GAIN: /* interesting...only f,fv according to spec */
		case AL_SAMPLE_OFFSET: /* could be integer */
		case AL_BYTE_OFFSET: /* could be integer */
		case AL_SEC_OFFSET: /* could be integer */

		case AL_DOPPLER_FACTOR:
		case AL_DOPPLER_VELOCITY:
		case AL_SPEED_OF_SOUND:
		{
			return LUAOPENAL_FLOAT_TYPE;
			break;
		}

		case AL_POSITION: /* could be integer */
		case AL_VELOCITY: /* could be integer */
		case AL_DIRECTION: /* could be integer */
		{
			return LUAOPENAL_FLOAT_3_TYPE;
			break;
		}

		case AL_ORIENTATION: /* could be integer */
		{
			return LUAOPENAL_FLOAT_6_TYPE;
			break;
		}

		case AL_NO_ERROR:
		case AL_INVALID_NAME:
		case AL_ILLEGAL_ENUM:
/*		case AL_INVALID_ENUM: */ /* same as AL_ILLEGAL_ENUM */
		case AL_INVALID_VALUE:
		case AL_ILLEGAL_COMMAND:
/*		case AL_INVALID_OPERATION: */ /* same as AL_ILLEGAL_COMMAND */
		case AL_OUT_OF_MEMORY:
		case AL_VENDOR:
		case AL_VERSION:
		case AL_RENDERER:
		case AL_EXTENSIONS:
		{
			return LUAOPENAL_STRING_TYPE;
			break;
		}

	}
	return 0;
}

static int lua_alEnable(lua_State* lua_state)
{
	ALenum enum_parameter;
	enum_parameter = (int) lua_tointeger(lua_state, 1);
	alEnable(enum_parameter);
	return 0;
}

static int lua_alDisable(lua_State* lua_state)
{
	ALenum enum_parameter;
	enum_parameter = (int) lua_tointeger(lua_state, 1);
	alDisable(enum_parameter);
	return 0;
}

static int lua_alIsEnabled(lua_State* lua_state)
{
	ALenum enum_parameter;
	ALboolean ret_val;
	enum_parameter = (int) lua_tointeger(lua_state, 1);
	ret_val = alIsEnabled(enum_parameter);
	lua_pushboolean(lua_state, ret_val);
	return 1;
}

static int lua_alGetError(lua_State* lua_state)
{
	ALenum ret_val;
	ret_val = alGetError();
	lua_pushinteger(lua_state, ret_val);
	return 1;
}

static int lua_alIsExtensionPresent(lua_State* lua_state)
{
	const ALchar* string_name;
	ALboolean ret_val;
	string_name = lua_tostring(lua_state, 1);
	ret_val = alIsExtensionPresent(string_name);
	lua_pushboolean(lua_state, ret_val);
	return 1;
}

/* TODO: alGetProcAddress */

static int lua_alGetEnumValue(lua_State* lua_state)
{
	const ALchar* string_name;
	ALenum ret_val;
	string_name = lua_tostring(lua_state, 1);
	ret_val = alGetEnumValue(string_name);
	lua_pushinteger(lua_state, ret_val);
	return 1;
}




static int lua_alGetListener(lua_State* lua_state)
{
	ALenum enum_parameter;
	int openal_primitive_type;

	enum_parameter = (int) lua_tointeger(lua_state, 1);

	openal_primitive_type = lua_getTypeForEnum(enum_parameter);

	switch(openal_primitive_type)
	{
		case LUAOPENAL_BOOL_TYPE:
		{
			ALint ret_val;
			alGetListeneri(enum_parameter, &ret_val);
			lua_pushboolean(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_INT_TYPE:
		{
			ALint ret_val;
			alGetListeneri(enum_parameter, &ret_val);
			lua_pushinteger(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_FLOAT_TYPE:
		{
			ALfloat ret_val;
			alGetListenerf(enum_parameter, &ret_val);
			lua_pushnumber(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_INT_3_TYPE:
		{
			ALint ret_val[3];
			alGetListeneriv(enum_parameter, ret_val);
			lua_pushinteger(lua_state, ret_val[0]);
			lua_pushinteger(lua_state, ret_val[1]);
			lua_pushinteger(lua_state, ret_val[2]);
			return 3;
			break;	
		}
		case LUAOPENAL_FLOAT_3_TYPE:
		{
			ALfloat ret_val[3];
			alGetListenerfv(enum_parameter, ret_val);
			lua_pushnumber(lua_state, ret_val[0]);
			lua_pushnumber(lua_state, ret_val[1]);
			lua_pushnumber(lua_state, ret_val[2]);
			return 3;
			break;			
		}
		case LUAOPENAL_FLOAT_6_TYPE:
		{
			ALfloat ret_val[6];
			alGetListenerfv(enum_parameter, ret_val);
			lua_pushnumber(lua_state, ret_val[0]);
			lua_pushnumber(lua_state, ret_val[1]);
			lua_pushnumber(lua_state, ret_val[2]);
			lua_pushnumber(lua_state, ret_val[3]);
			lua_pushnumber(lua_state, ret_val[4]);
			lua_pushnumber(lua_state, ret_val[5]);
			return 6;
			break;			
		}
		default:
		{
			/* TODO: Figure out how to handle OpenAL extensions. */
			luaL_error(lua_state, "Unhandled parameter type for alGetSource*");
		}
	}
	return 0;
}

static int lua_alListener(lua_State* lua_state)
{
	ALenum enum_parameter;
	int openal_primitive_type;

	enum_parameter = (int) (int) lua_tointeger(lua_state, 1);


	openal_primitive_type = lua_getTypeForEnum(enum_parameter);

	switch(openal_primitive_type)
	{
		case LUAOPENAL_BOOL_TYPE:
		case LUAOPENAL_INT_TYPE:
		{
			alListeneri(enum_parameter, (int) lua_tointeger(lua_state, 2));
			break;
		}
		case LUAOPENAL_FLOAT_TYPE:
		{
			alListenerf(enum_parameter, lua_tonumber(lua_state, 2));
			break;
		}
		case LUAOPENAL_INT_3_TYPE:
		{
			alListener3i(enum_parameter, (int) lua_tointeger(lua_state, 2), (int) lua_tointeger(lua_state, 3),  (int) lua_tointeger(lua_state, 4));
			break;			
		}
		case LUAOPENAL_FLOAT_3_TYPE:
		{
			alListener3f(enum_parameter, lua_tonumber(lua_state, 2),  lua_tonumber(lua_state, 3),  lua_tonumber(lua_state, 4));
			break;			
		}
		case LUAOPENAL_FLOAT_6_TYPE:
		{
			ALfloat val_array[6] = 
			{
				lua_tonumber(lua_state, 2),
				lua_tonumber(lua_state, 3),
				lua_tonumber(lua_state, 4),
				lua_tonumber(lua_state, 5),
				lua_tonumber(lua_state, 6),
				lua_tonumber(lua_state, 7)
			};

			alListenerfv(enum_parameter, val_array);
			break;			
		}
		default:
		{
			/* TODO: Figure out how to handle OpenAL extensions. */
			luaL_error(lua_state, "Unhandled parameter type for alSource*");
		}
	}
	return 0;
}




static int lua_alGetSource(lua_State* lua_state)
{
	ALuint source_id;
	ALenum enum_parameter;
	int openal_primitive_type;

	source_id = (int) lua_tointeger(lua_state, 1);
	enum_parameter = (int) lua_tointeger(lua_state, 2);

	openal_primitive_type = lua_getTypeForEnum(enum_parameter);

	switch(openal_primitive_type)
	{
		case LUAOPENAL_BOOL_TYPE:
		{
			ALint ret_val;
			alGetSourcei(source_id, enum_parameter, &ret_val);
			lua_pushboolean(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_INT_TYPE:
		{
			ALint ret_val;
			alGetSourcei(source_id, enum_parameter, &ret_val);
			lua_pushinteger(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_FLOAT_TYPE:
		{
			ALfloat ret_val;
			alGetSourcef(source_id, enum_parameter, &ret_val);
			lua_pushnumber(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_INT_3_TYPE:
		{
			ALint ret_val[3];
			alGetSourceiv(source_id, enum_parameter, ret_val);
			lua_pushinteger(lua_state, ret_val[0]);
			lua_pushinteger(lua_state, ret_val[1]);
			lua_pushinteger(lua_state, ret_val[2]);
			return 3;
			break;	
		}
		case LUAOPENAL_FLOAT_3_TYPE:
		{
			ALfloat ret_val[3];
			alGetSourcefv(source_id, enum_parameter, ret_val);
			lua_pushnumber(lua_state, ret_val[0]);
			lua_pushnumber(lua_state, ret_val[1]);
			lua_pushnumber(lua_state, ret_val[2]);
			return 3;
			break;			
		}
		default:
		{
			/* TODO: Figure out how to handle OpenAL extensions. */
			luaL_error(lua_state, "Unhandled parameter type for alGetSource*");
		}
	}
	return 0;
}


static int lua_alSource(lua_State* lua_state)
{
	ALuint source_id;
	ALenum enum_parameter;
	int openal_primitive_type;

	source_id = (int) lua_tointeger(lua_state, 1);
	enum_parameter = (int) lua_tointeger(lua_state, 2);

	openal_primitive_type = lua_getTypeForEnum(enum_parameter);

	switch(openal_primitive_type)
	{
		case LUAOPENAL_BOOL_TYPE:
		case LUAOPENAL_INT_TYPE:
		{
			alSourcei(source_id, enum_parameter, (int) lua_tointeger(lua_state, 3));
			break;
		}
		case LUAOPENAL_FLOAT_TYPE:
		{
			alSourcef(source_id, enum_parameter, lua_tonumber(lua_state, 3));
			break;
		}
		case LUAOPENAL_INT_3_TYPE:
		{
			alSource3i(source_id, enum_parameter, (int) lua_tointeger(lua_state, 3), (int) lua_tointeger(lua_state, 4),  (int) lua_tointeger(lua_state, 5));
			break;			
		}
		case LUAOPENAL_FLOAT_3_TYPE:
		{
			alSource3f(source_id, enum_parameter, lua_tonumber(lua_state, 3),  lua_tonumber(lua_state, 4),  lua_tonumber(lua_state, 5));
			break;			
		}
		default:
		{
			/* TODO: Figure out how to handle OpenAL extensions. */
			luaL_error(lua_state, "Unhandled parameter type for alSource*");
		}
	}
	return 0;
}


static int lua_alDopplerFactor(lua_State* lua_state)
{
	ALfloat the_value;
	the_value = lua_tonumber(lua_state, 1);
	alDopplerFactor(the_value);
	return 0;
}

static int lua_alDopplerVelocity(lua_State* lua_state)
{
	ALfloat the_value;
	the_value = lua_tonumber(lua_state, 1);
	alDopplerVelocity(the_value);
	return 0;
}

static int lua_alSpeedOfSound(lua_State* lua_state)
{
	ALfloat the_value;
	the_value = lua_tonumber(lua_state, 1);
	alSpeedOfSound(the_value);
	return 0;
}

static int lua_alDistanceModel(lua_State* lua_state)
{
	ALenum enum_parameter;
	enum_parameter = (int) lua_tointeger(lua_state, 1);
	alDistanceModel(enum_parameter);
	return 0;
}

static int lua_alGet(lua_State* lua_state)
{
	ALenum enum_parameter;
	int openal_primitive_type;

	enum_parameter = (int) lua_tointeger(lua_state, 1);

	openal_primitive_type = lua_getTypeForEnum(enum_parameter);

	switch(openal_primitive_type)
	{
		case LUAOPENAL_BOOL_TYPE:
		{
			ALboolean ret_val;
			ret_val = alGetBoolean(enum_parameter);
			lua_pushboolean(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_INT_TYPE:
		{
			ALint ret_val;
			ret_val = alGetInteger(enum_parameter);
			lua_pushinteger(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_FLOAT_TYPE:
		{
			ALfloat ret_val;
			ret_val = alGetFloat(enum_parameter);
			lua_pushnumber(lua_state, ret_val);
			return 1;
			break;
		}
		case LUAOPENAL_INT_3_TYPE:
		{
			ALint ret_val[3];
			alGetIntegerv(enum_parameter, ret_val);
			lua_pushinteger(lua_state, ret_val[0]);
			lua_pushinteger(lua_state, ret_val[1]);
			lua_pushinteger(lua_state, ret_val[2]);
			return 3;
			break;	
		}
		case LUAOPENAL_FLOAT_3_TYPE:
		{
			ALfloat ret_val[3];
			alGetFloatv(enum_parameter, ret_val);
			lua_pushnumber(lua_state, ret_val[0]);
			lua_pushnumber(lua_state, ret_val[1]);
			lua_pushnumber(lua_state, ret_val[2]);
			return 3;
			break;			
		}
		case LUAOPENAL_FLOAT_6_TYPE:
		{
			ALfloat ret_val[6];
			alGetFloatv(enum_parameter, ret_val);
			lua_pushnumber(lua_state, ret_val[0]);
			lua_pushnumber(lua_state, ret_val[1]);
			lua_pushnumber(lua_state, ret_val[2]);
			lua_pushnumber(lua_state, ret_val[3]);
			lua_pushnumber(lua_state, ret_val[4]);
			lua_pushnumber(lua_state, ret_val[5]);
			return 6;
			break;			
		}
		case LUAOPENAL_STRING_TYPE:
		{
			const ALchar* ret_val;
			ret_val = alGetString(enum_parameter);
			lua_pushstring(lua_state, ret_val);
			return 1;
			break;
		}
		default:
		{
			/* TODO: Figure out how to handle OpenAL extensions. */
			luaL_error(lua_state, "Unhandled parameter type for alGetSource*");
		}
	}
	return 0;
}

static const luaL_reg luaALFuncs[] =
{
	{ "Enable", 								lua_alEnable },
	{ "Disable", 								lua_alDisable },
	{ "IsEnabled",								lua_alIsEnabled },
	{ "Get", 									lua_alGet },
	{ "GetError",								lua_alGetError },
	{ "IsExtensionPresent", 					lua_alIsExtensionPresent },
	{ "GetEnumValue",							lua_alGetEnumValue },
	{ "Listener",								lua_alListener },
	{ "GetListener", 							lua_alGetListener },
	{ "Source", 								lua_alSource },
	{ "GetSource",								lua_alGetSource },
	{ "DopplerFactor",							lua_alDopplerFactor },
	{ "DopplerVelocity", 						lua_alDopplerVelocity },
	{ "SpeedOfSound",							lua_alSpeedOfSound },
	{ "DistanceModel",							lua_alDistanceModel },
	{ "DopplerVelocity", 						lua_alDopplerVelocity },
	{NULL, NULL}
};


int luaopen_luaal(lua_State *L) 
{
  luaL_register(L, "al", luaALFuncs);

  luaopenal_initenum(L, s_luaALEnum);

  lua_pushstring(L, "_VERSION");
  lua_pushstring(L, LUAAL_VERSION);
  lua_settable(L,-3);

  return 1;
}



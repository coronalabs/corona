//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_EmscriptenJSPluginLoader.h"
#include "Rtt_EmscriptenJSPluginLoader.h"
#include "Corona/CoronaLua.h"
#include "Rtt_Lua.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"


#pragma region JSON

int Rtt::EmscriptenJSPluginLoader::json_decode(lua_State* L, const char* json)
{
	Rtt_LUA_STACK_GUARD(L, 1);
	
	int top = lua_gettop(L);
	bool success = false;

	lua_getglobal(L, "require");
	lua_pushstring(L, "dkjson");
	if (CoronaLuaDoCall(L, 1, 1) == 0)
	{
		lua_getfield(L, -1, "decode");
		lua_remove(L, -2); // remove dkjson
		lua_pushstring(L, json);	
		if (CoronaLuaDoCall(L, 1, 1) == 0)
		{
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "data");
				lua_remove(L, -2); // remove decoded table
				success = true;
			}
		}
	}
	if(!success) {
		lua_pushnil(L);
	}
	return 1;
}

int Rtt::EmscriptenJSPluginLoader::EncodeException(lua_State*L)
{	
	Rtt_LUA_STACK_GUARD(L, 1);

	if(lua_isfunction(L,2) && lua_istable(L, lua_upvalueindex(1)))
	{
		lua_pushvalue(L,2);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		lua_pushinteger(L, ref);
		lua_rawseti(L, lua_upvalueindex(1), 1+lua_objlen(L, lua_upvalueindex(1)));
		lua_pushinteger(L, ref);
	}
	else
	{
		lua_pushstring(L, "null");
	}

	lua_tostring(L, -1);
	return 1;
}

const char* Rtt::EmscriptenJSPluginLoader::json_encode(lua_State* L, int valueIndex, bool storeFunctions)
{
	Rtt_LUA_STACK_GUARD(L);

	static std::string json;
	json = "{}";
	int top = lua_gettop(L);

	if(storeFunctions)
	{
		lua_newtable(L);
	}
	else
	{
		lua_pushnil(L);
	}
	int exceptionUpvalueIndex = lua_gettop(L);

	lua_getglobal(L, "require");
	lua_pushstring(L, "dkjson");
	if (CoronaLuaDoCall(L, 1, 1) == 0)
	{
		lua_getfield(L, -1, "encode");
		
		lua_createtable(L, 0, 1);
		lua_pushvalue(L, valueIndex);
		lua_setfield(L, -2, "data");

		lua_createtable(L, 0, 1);
		lua_pushvalue(L, exceptionUpvalueIndex);
		lua_pushcclosure(L, &EmscriptenJSPluginLoader::EncodeException, 1);
		lua_setfield(L, -2, "exception");

		if (CoronaLuaDoCall(L, 2, 1) == 0)
		{
			json = lua_tostring(L, -1);
			lua_pop(L, 1);		
		}

		if(lua_objlen(L, exceptionUpvalueIndex) > 0)
		{
			lua_getfield(L, -1, "encode");
			
			lua_createtable(L, 0, 1);
			lua_pushvalue(L, exceptionUpvalueIndex);
			lua_setfield(L, -2, "functions");

			if (CoronaLuaDoCall(L, 1, 1) == 0)
			{
				std::string functions = lua_tostring(L, -1);
				lua_pop(L, 1);
				functions[0] = ',';
				json.pop_back();
				json += functions; // skip first character
			}
		}
	}
	lua_pop(L, 2);

	return json.c_str();
}

#pragma endregion


int Rtt::EmscriptenJSPluginLoader::JSFunctionWrapper(lua_State *L)
{
	Rtt_LUA_STACK_GUARD(L, 1);

	int nArgs = lua_gettop(L);
	lua_createtable(L, nArgs, 0);
	for(int n=1; n<=nArgs; n++)
	{
		lua_pushvalue(L, n);
		lua_rawseti(L, -2, n);
	}
	const char *packageName = lua_tostring(L, lua_upvalueindex(1));
	const char *property = lua_tostring(L, lua_upvalueindex(2));
	const char *arguments = json_encode(L, nArgs+1, true);
	lua_pop(L, 1);
	
	char *str = (char *)EM_ASM_INT({
		var module = UTF8ToString($0);
		var property = UTF8ToString($1);
		var args = UTF8ToString($2);
		var args = JSON.parse(args);
		var arguments = args.data || [];
		var functions = args.functions;
		var object = window[module];
		var func = object[property];

		var result = null; 
		try {
			result = func.apply(object, arguments);
		} catch(e) {
			Module.printErr("Error while calling '" + module + "." + property + "()': ", e);
		}

		if(functions) {
			for(var i=0; i<functions.length; i++) {
				window.LuaReleaseFunction(functions[i]);
			}
		}
		
		var str = JSON.stringify({data:result});
        var lengthBytes = lengthBytesUTF8(str)+1;
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes+1);
        return stringOnWasmHeap;
	}, packageName, property, arguments);

	json_decode(L, str);
	free(str);
	return 1;
}


int Rtt::EmscriptenJSPluginLoader::Index(lua_State *L)
{
	Rtt_LUA_STACK_GUARD(L, 1);

	const char *packageName = lua_tostring(L, lua_upvalueindex(1));
	const char *property = lua_tostring(L, 2);
 
	char *str = (char *)EM_ASM_INT({
		var module = UTF8ToString($0);
		var property = UTF8ToString($1);	
		var property = window[module][property];
		var str = typeof property;
		if(str != 'function') {
			str = JSON.stringify({data:property});
		}
        var lengthBytes = lengthBytesUTF8(str)+1;
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes+1);
        return stringOnWasmHeap;
	}, packageName, property);

	if(strncmp("function", str, 9) == 0) {
		lua_pushstring(L, packageName);
		lua_pushstring(L, property);
		lua_pushcclosure(L, &EmscriptenJSPluginLoader::JSFunctionWrapper, 2);
	} else {
		json_decode(L, str);
	}
	free(str);
	return 1;
}


int Rtt::EmscriptenJSPluginLoader::NewIndex(lua_State *L) 
{
	Rtt_LUA_STACK_GUARD(L);

	const char *packageName = lua_tostring(L, lua_upvalueindex(1));
	const char *key = lua_tostring(L, 2);

	EM_ASM({
		var module = UTF8ToString($0);
		var key = UTF8ToString($1);
		if(typeof window[module][key] == 'function' && window.LuaIsFunction(window[module][key])) {
			window.LuaReleaseFunction(window[module][key]);
		}
	}, packageName, key);
	
	if(lua_isfunction(L, 3))
	{
		lua_pushvalue(L, 3);
		int dispatcherRef = luaL_ref(L, LUA_REGISTRYINDEX);
		EM_ASM({
			var module = UTF8ToString($0);
			var key = UTF8ToString($1);
			var refId = $2;
			window[module][key] = window.LuaCreateFunction(refId);
		}, packageName, key, dispatcherRef);
		luaL_unref(L, LUA_REGISTRYINDEX, dispatcherRef);
	}
	else
	{
		EM_ASM({
			var module = UTF8ToString($0);
			var key = UTF8ToString($1);
			var jsonData = UTF8ToString($2);
			var value = JSON.parse(jsonData).data;
			window[module][key] = value;
		}, packageName, key, json_encode(L, 3, false));
	}
	return 0; 
}


int Rtt::EmscriptenJSPluginLoader::Open(lua_State *L)
{
	Rtt_LUA_STACK_GUARD(L, 1);

	const char* packageName = luaL_checkstring(L, 1);
	packageName = luaL_gsub( L, packageName, ".", "_" );
	lua_remove(L, 1);

	static const luaL_Reg kVTable[] = {{ NULL, NULL }};
	luaL_openlib( L, packageName, kVTable, 0 );

	lua_createtable(L, 0, 2);
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -3);

	lua_pushvalue(L, 1);
	lua_pushcclosure(L, &EmscriptenJSPluginLoader::Index, 1);
	lua_setfield(L, 3, "__index");

	lua_pushvalue(L, 1);
	lua_pushcclosure(L, &EmscriptenJSPluginLoader::NewIndex, 1);
	lua_setfield(L, 3, "__newindex");

	lua_pop(L,1);
	return 1;
}


extern "C"
{
	int EMSCRIPTEN_KEEPALIVE JSLuaFunctionRef(lua_State *L, int action, int refId, const char* data)
	{
		Rtt_LUA_STACK_GUARD(L);

		lua_rawgeti(L, LUA_REGISTRYINDEX, refId);
		if(!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);
			return 0;
		}
		if(action == 1) // if reference funciton
		{
			lua_pop(L, 1);
			return 1;
		}
		if(action == 2) // create new reference
			return luaL_ref(L, LUA_REGISTRYINDEX);
		if(action == 3) // release reference
		{
			lua_pop(L, 1);
			luaL_unref(L, LUA_REGISTRYINDEX, refId);
		}
		return 0;
	}

	const char* EMSCRIPTEN_KEEPALIVE JSLuaFunctionWrapper(lua_State *L, int refId, const char* data)
	{
		Rtt_LUA_STACK_GUARD(L);
		
		const char *ret = "{\"data\":null}";
		lua_rawgeti(L, LUA_REGISTRYINDEX, refId);
		if(!lua_isfunction(L, -1))
			return 0;
		
		Rtt::EmscriptenJSPluginLoader::json_decode(L, data);
		int argArray = lua_gettop(L);		
		size_t nArgs = lua_objlen(L, -1);
		for(size_t i=1; i<=nArgs; i++)
		{
			lua_rawgeti(L, argArray, i);
		}
		lua_remove(L, argArray);

		if(CoronaLuaDoCall(L, nArgs, 1) == 0) {
			ret = Rtt::EmscriptenJSPluginLoader::json_encode(L, lua_gettop(L), true);
			lua_pop(L, 1);
		}
		return ret;
	}
}


int Rtt::EmscriptenJSPluginLoader::Loader(lua_State *L)
{
	Rtt_LUA_STACK_GUARD(L, 1);

	const char* packageName = luaL_checkstring(L, 1);
	std::string subdirModule(luaL_gsub( L, packageName, ".", "/" ));
	lua_pop(L, 1);
	packageName = luaL_gsub( L, packageName, ".", "_" );
	lua_remove(L, 1);

	std::string module(packageName);

	std::string jsFileName = module + ".js"; 
	FILE* fi = fopen(jsFileName.c_str(), "rb");
	if (fi == NULL)
	{
		jsFileName = subdirModule + ".js"; 
		fi = fopen(jsFileName.c_str(), "rb");
	}

	if (fi == NULL)
	{
		lua_pushfstring(L, "\n\tno file '%s.js' or '%s.js'", module.c_str(), subdirModule.c_str());
		return 1;
	}

	fseek(fi, 0, SEEK_END);
	int size = ftell(fi);
	fseek(fi, 0, SEEK_SET);
	char* js = (char*) malloc(size + 1);		// +1 for eol
	fread(js, 1, size, fi);
	fclose(fi);
	js[size] = 0;

	bool loaded = EM_ASM_INT({
		var src = UTF8ToString($0);
		var module = UTF8ToString($1);
		var L = $2;
		var loaded = 0;
		var script = document.createElement('script');
		script.setAttribute('type', 'text/javascript');
		script.setAttribute('charset', 'utf-8');
		script.appendChild(document.createTextNode(src));
		document.head.appendChild(script);
		if(!window.hasOwnProperty('LuaCreateFunction')) {
			var luaFunctionReferences = [];
			var JSLuaFunctionRef = Module.cwrap('JSLuaFunctionRef', 'number', ['number', 'number', 'number', 'string']);
			var JSLuaFunctionWrapper = Module.cwrap('JSLuaFunctionWrapper', 'string', ['number', 'number', 'string']);
			window.LuaIsFunction = function(fnc) {
				var refId = (fnc || {})._refId || fnc || 0;
				if( typeof refId == 'number' && refId !== 0) {
					return (JSLuaFunctionRef(L, 1, refId, null)!=0);
				};
				return false;
			};
			window.LuaCreateFunction = function(fnc) {
				var refId = (fnc || {})._refId || fnc || 0;
				if( typeof refId == 'number' && refId !== 0) {
					var newRefId = JSLuaFunctionRef(L, 2, refId, null);
					if (newRefId == 0)
						return null;
					var ret=(function(){
						var f = function() {
							if(typeof f._action === 'function') {
								return f._action.apply(null, arguments);
							} else {
								Module.print("Warning: trying to invoke released Lua Function!");
							}
						};
						f._refId = newRefId;
						f._action = function() {
							var args = JSON.stringify({data:Array.prototype.slice.call(arguments)});
							var strRet = JSLuaFunctionWrapper(L, f._refId, args);
							var luaReturns = JSON.parse(strRet);
							var retData = luaReturns.data;
							var functions = luaReturns.functions;

							// TODO: this is very sketchy place. This is used when Lua function is returned from Lua function
							for(var i=0; i<luaFunctionReferences.length; i++) {
								window.LuaReleaseFunction(luaFunctionReferences[i]);
							}
							if(functions && functions.length) {
								luaFunctionReferences = functions;
							}
							return retData;
						};
						return f;
					})();
					return ret;
				};
				return null;
			};
			window.LuaReleaseFunction = function(fnc) {
				var refId = (fnc || {})._refId || fnc || 0;
				if( typeof refId == 'number' && refId !== 0) {
					JSLuaFunctionRef(L, 3, refId, null);
				};
				if(fnc) {
					delete fnc._refId;
					delete fnc._action;
				}
			};
		}
		return window.hasOwnProperty(module);
	}, js, module.c_str(), L);

	free(js);

	if(loaded)
	{
		lua_pushcfunction(L, &EmscriptenJSPluginLoader::Open);
	} 
	else
	{
		lua_pushfstring(L, "\n\tJS library is loaded, but object '%s' is not found!", module.c_str());
	}
	return 1;
}


#else
int Rtt::EmscriptenJSPluginLoader::Loader(lua_State *L) { return 0; }
int Rtt::EmscriptenJSPluginLoader::Open(lua_State *L) { return 0; }
#endif

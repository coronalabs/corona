
if( CMAKE_BUILD_TYPE STREQUAL "Release" )
	set(BUILD_CONFIG RELEASE)
elseif( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	set(BUILD_CONFIG DEBUG)
	set(BUILD_CONFIG_WIN "/debug")
else()
	message(FATAL_ERROR "No valid CMAKE_BUILD_TYPE (${CMAKE_BUILD_TYPE})" )
endif()

set(Lua2CppOutputDir "${CMAKE_CURRENT_BINARY_DIR}/generated_src")
file(MAKE_DIRECTORY "${Lua2CppOutputDir}")

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")

	function(lua_2_cpp LUA_FILE)
		if(DEFINED ARGV1)
			SET(MODULE_COMMAND1 "/moduleName")
			SET(MODULE_COMMAND2 "${ARGV1}")
		endif()
		get_filename_component(F "${LUA_FILE}" NAME)
		string(REPLACE ".lua" "_luaload.cpp" Fout "${F}")
		string(REPLACE ".lua" ".cpp" F "${F}")
		add_custom_command(
			OUTPUT "${Lua2CppOutputDir}/${F}"
			MAIN_DEPENDENCY "${LUA_FILE}"
			COMMAND "${CORONA_ROOT}/platform/windows/Build.Tools/LuaToCppFile.bat" "${CORONA_ROOT}/bin/win" "${Lua2CppOutputDir}" "${LUA_FILE}" "${Lua2CppOutputDir}" ${MODULE_COMMAND1} ${MODULE_COMMAND2}
			COMMAND ${CMAKE_COMMAND} -E copy "${Lua2CppOutputDir}/${Fout}" "${Lua2CppOutputDir}/${F}"
		)
	endfunction()

	function(lua_2_c LUA_FILE)
		get_filename_component(F "${LUA_FILE}" NAME)
		string(REPLACE ".lua" "_luaload.cpp" Fout "${F}")
		string(REPLACE ".lua" ".c" F "${F}")

		add_custom_command(
			OUTPUT "${Lua2CppOutputDir}/${F}"
			MAIN_DEPENDENCY "${LUA_FILE}"
			COMMAND "${CORONA_ROOT}/platform/windows/Build.Tools/LuaToCppFile.bat" "${CORONA_ROOT}/bin/win" "${Lua2CppOutputDir}" "${LUA_FILE}" "${Lua2CppOutputDir}" ${MODULE_COMMAND1} ${MODULE_COMMAND2}
			COMMAND ${CMAKE_COMMAND} -E copy "${Lua2CppOutputDir}/${Fout}" "${Lua2CppOutputDir}/${F}"
		)
	endfunction()


elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")

	function(lua_2_cpp LUA_FILE)
		if(DEFINED ARGV1)
			SET(MODULE_COMMAND1 "-m")
			SET(MODULE_COMMAND2 "${ARGV1}")
		endif()
		get_filename_component(F "${LUA_FILE}" NAME)
		string(REPLACE ".lua" ".cpp" F "${F}")
		add_custom_command(
			OUTPUT "${Lua2CppOutputDir}/${F}"
			MAIN_DEPENDENCY "${LUA_FILE}"
			COMMAND ${CMAKE_COMMAND} -E env BUILD_CONFIG="${BUILD_CONFIG}" "${CORONA_ROOT}/platform/android/ndk/lua_to_native.sh" ${MODULE_COMMAND1} ${MODULE_COMMAND2} "${LUA_FILE}" "${Lua2CppOutputDir}/"
		)
	endfunction()


	function(lua_2_c LUA_FILE)
		get_filename_component(F "${LUA_FILE}" NAME)
		string(REPLACE ".lua" ".c" F "${F}")
		add_custom_command(
			OUTPUT "${Lua2CppOutputDir}/${F}"
			MAIN_DEPENDENCY "${LUA_FILE}"
			COMMAND "${CORONA_ROOT}/bin/mac/lua2c.sh" "${LUA_FILE}" "${Lua2CppOutputDir}/" "${BUILD_CONFIG}" "${CORONA_ROOT}/bin/mac"
		)
	endfunction()

elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")

	function(lua_2_cpp LUA_FILE)
		if(DEFINED ARGV1)
			SET(MODULE_COMMAND1 "-m")
			SET(MODULE_COMMAND2 "${ARGV1}")
		endif()
		get_filename_component(F "${LUA_FILE}" NAME)
		string(REPLACE ".lua" ".cpp" F "${F}")
		add_custom_command(
			OUTPUT "${Lua2CppOutputDir}/${F}"
			MAIN_DEPENDENCY "${LUA_FILE}"
			COMMAND ${CMAKE_COMMAND} -E env BUILD_CONFIG="${BUILD_CONFIG}" "${CORONA_ROOT}/platform/linux/lua_to_native.sh" ${MODULE_COMMAND1} ${MODULE_COMMAND2} "${LUA_FILE}" "${Lua2CppOutputDir}/"
		)
	endfunction()


	function(lua_2_c LUA_FILE)
		get_filename_component(F "${LUA_FILE}" NAME)
		string(REPLACE ".lua" ".c" F "${F}")
		add_custom_command(
			OUTPUT "${Lua2CppOutputDir}/${F}"
			MAIN_DEPENDENCY "${LUA_FILE}"
			COMMAND "${CORONA_ROOT}/bin/linux/lua2c.sh" "${LUA_FILE}" "${Lua2CppOutputDir}/" "${BUILD_CONFIG}" "${CORONA_ROOT}/bin/linux"
		)
	endfunction()

else()
	message(FATAL_ERROR "CMakeGenLua .cmake is not ported to current OS: ${CMAKE_HOST_SYSTEM_NAME}")
endif()
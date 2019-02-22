
if( CMAKE_BUILD_TYPE STREQUAL "Release" )
	set(BUILD_CONFIG RELEASE)
else()
	set(BUILD_CONFIG DEBUG)
endif()


function(lua_2_cpp LUA_FILE)
	if(DEFINED ARGV1)
		SET(MODULE_COMMAND1 "-m")
		SET(MODULE_COMMAND2 "${ARGV1}")
	endif()
	get_filename_component(F "${LUA_FILE}" NAME)
	string(REPLACE ".lua" ".cpp" F "${F}")
	add_custom_command(
		OUTPUT "${LibNdkGenSharedPath}/${F}"
		MAIN_DEPENDENCY "${LUA_FILE}"
		COMMAND ${CMAKE_COMMAND} -E env BUILD_CONFIG="${BUILD_CONFIG}" "${CORONA_ROOT}/platform/android/ndk/lua_to_native.sh" ${MODULE_COMMAND1} ${MODULE_COMMAND2} "${LUA_FILE}" "${LibNdkGenSharedPath}/"
	)
endfunction()




function(lua_2_c LUA_FILE)
	get_filename_component(F "${LUA_FILE}" NAME)
	string(REPLACE ".lua" ".c" F "${F}")
	add_custom_command(
		OUTPUT "${LibNdkGenSharedPath}/${F}"
		MAIN_DEPENDENCY "${LUA_FILE}"
		COMMAND "${CORONA_ROOT}/bin/mac/lua2c.sh" "${LUA_FILE}" "${LibNdkGenSharedPath}/" "${BUILD_CONFIG}" "${CORONA_ROOT}/bin/mac"
	)
endfunction()

# - Check if the function exists.
# CHECK_LIBRARY_EXISTS (LIBRARY FUNCTION LOCATION VARIABLE)
#
#  LIBRARY  - the name of the library you are looking for
#  FUNCTION - the name of the function
#  ARGCOUNT - number of arguments for stdcall functions
#  LOCATION - location where the library should be found
#  VARIABLE - variable to store the result
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link

MACRO(CHECK_SHARED_LIBRARY_EXISTS LIBRARY FUNCTION ARGCOUNT LOCATION VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(MACRO_CHECK_LIBRARY_EXISTS_DEFINITION 
      "-DCHECK_SHARED_FUNCTION_EXISTS=${FUNCTION} ${CMAKE_REQUIRED_FLAGS}")
    IF(WIN32)
        IF(${ARGCOUNT} GREATER 0)
            SET(ARGSTACK "void*")
            SET(CALLSTACK "NULL")
            SET(CURARG 1)
            WHILE(${ARGCOUNT} GREATER ${CURARG})
                SET(ARGSTACK "${ARGSTACK},void*")
                SET(CALLSTACK "${CALLSTACK},NULL")
                MATH(EXPR CURARG "${CURARG} + 1")
            ENDWHILE(${ARGCOUNT} GREATER ${CURARG})
        ENDIF(${ARGCOUNT} GREATER 0)
        SET(MACRO_CHECK_LIBRARY_EXISTS_DEFINITION 
            "-D_WIN32 -DARGSTACK=\"${ARGSTACK}\" -DCALLSTACK=\"${CALLSTACK}\" ${MACRO_CHECK_LIBRARY_EXISTS_DEFINITION}")
    ENDIF(WIN32)
    MESSAGE(STATUS "Looking for ${FUNCTION} in ${LIBRARY}")
    SET(CHECK_LIBRARY_EXISTS_LIBRARIES ${LIBRARY})
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_LIBRARY_EXISTS_LIBRARIES 
        ${CHECK_LIBRARY_EXISTS_LIBRARIES} ${CMAKE_REQUIRED_LIBRARIES})
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_SOURCE_DIR}/cmake/CheckSharedFunctionExists.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS 
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_LIBRARY_EXISTS_DEFINITION}
      -DLINK_DIRECTORIES:STRING=${LOCATION}
      "-DLINK_LIBRARIES:STRING=${CHECK_LIBRARY_EXISTS_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)

    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for ${FUNCTION} in ${LIBRARY} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have library ${LIBRARY}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Determining if the function ${FUNCTION} exists in the ${LIBRARY} "
        "passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${FUNCTION} in ${LIBRARY} - not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have library ${LIBRARY}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining if the function ${FUNCTION} exists in the ${LIBRARY} "
        "failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_SHARED_LIBRARY_EXISTS)

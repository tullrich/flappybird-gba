#See https://www.vtk.org/Wiki/CMake_Cross_Compiling
set(DEVKITARM $ENV{DEVKITARM})
set(DEVKITPRO $ENV{DEVKITPRO})

if(NOT DEVKITARM)
	message(FATAL_ERROR "DEVKITARM environment variable missing.")
endif()

if(NOT DEVKITPRO)
	message(FATAL_ERROR "DEVKITPRO environment variable missing.")
endif()

set(CMAKE_SYSTEM_NAME "none")
set(CMAKE_SYSTEM_PROCESSOR "arm7tdmi")

set(CMAKE_ASM_COMPILER ${DEVKITARM}/bin/arm-none-eabi-as)
set(CMAKE_C_COMPILER ${DEVKITARM}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${DEVKITARM}/bin/arm-none-eabi-g++)
set(GBAFIX_EXECUTABLE ${DEVKITARM}/bin/gbafix)

set(CMAKE_FIND_ROOT_PATH ${DEVKITARM}/arm-none-eabi)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY LAST)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE LAST)

set(CMAKE_FIND_LIBRARY_PREFIXES lib)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

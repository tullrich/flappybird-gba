cmake_minimum_required(VERSION 2.8)


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Toolchain-devkitARM.cmake)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu99 -fomit-frame-pointer -ffast-math -fno-common -mcpu=arm7tdmi -mtune=arm7tdmi -mthumb -mthumb-interwork -Wall -save-temps -O3" )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=gba.specs")

project(FlappyBird C ASM)

add_executable(FlappyBird
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/video.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/io.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/irq.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/audio.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/sin_lut.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/systemcall.s

	${CMAKE_CURRENT_SOURCE_DIR}/assets/flappy_background.c
	${CMAKE_CURRENT_SOURCE_DIR}/assets/flappy_foreground.c
	${CMAKE_CURRENT_SOURCE_DIR}/assets/flappy_sprites.c
	${CMAKE_CURRENT_SOURCE_DIR}/assets/sfx_die.o
	${CMAKE_CURRENT_SOURCE_DIR}/assets/sfx_wing.o
	${CMAKE_CURRENT_SOURCE_DIR}/assets/sfx_hit.o
	${CMAKE_CURRENT_SOURCE_DIR}/assets/sfx_swooshing.o
	${CMAKE_CURRENT_SOURCE_DIR}/assets/sfx_point.o
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/assets
	${GBA_INCLUDE_DIRS}
)
link_libraries(${GBA_LIBRARY})
set(_TARGET_BIN_LOCATION $<TARGET_FILE:FlappyBird>.gba)
add_custom_command(
	TARGET FlappyBird
	POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} --output-format=binary $<TARGET_FILE:FlappyBird> ${_TARGET_BIN_LOCATION}
	COMMAND ${GBAFIX_EXECUTABLE} ${_TARGET_BIN_LOCATION}
)

cmake_minimum_required(VERSION 3.0)
project("spa2txt" C)

if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
		set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
	endif ()
else ()
	set (CMAKE_C_STANDARD 99)
endif ()

add_executable(spa2txt spa2txt.c)

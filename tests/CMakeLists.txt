cmake_minimum_required(VERSION 3.2)
project(MCUsim_Tests)

include(CheckFunctionExists)

#
# Set flags here
#
if (CMAKE_COMPILER_IS_GNUCC OR
    "${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1999")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
endif()

#
# Set libraries minimum versions here
#
#set(WXWIDGETS_MIN_VERSION "3.0.0")

#
# Set sources here
#
set(TSC_TEST_SOURCES	../arch/x86/tsc.c
			../tools/arch/x86/util/tsc.c
			minunit.c
			tsc_test.c)

#
# Look for libraries, packages, etc.
#
#find_package(wxWidgets ${WXWIDGETS_MIN_VERSION} REQUIRED core base)

#
# Define includes here
#
include_directories("../include/")
#include(${wxWidgets_USE_FILE})

#
# Set target libraries here
#
#set(TARGET_LIBS "${wxWidgets_LIBRARIES}")

#
# Check functions exist here
#
#if (NOT MSVC)
#	CHECK_FUNCTION_EXISTS(fmax RESULT)
#	if(NOT RESULT)
#		unset(RESULT)
#		list(APPEND TARGET_LIBS m)
#		message(STATUS "Math library added to the TARGET_LIBS")
#	endif()
#endif()

add_executable(tsc_test ${TSC_TEST_SOURCES})

#target_link_libraries(tsc_test ${TARGET_LIBS})

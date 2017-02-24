#
# mcusim - Interactive simulator for microcontrollers.
# Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.2)
project(MCUsim_Tests)

include(CheckFunctionExists)

#
# Set flags here
#
if (CMAKE_COMPILER_IS_GNUCC OR
    "${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -g")
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
set(TSC_TEST_SRC	../arch/x86/tsc.c
			../tools/arch/x86/util/tsc.c
			../tools/math/math.c
			minunit.c
			tsc_test.c)
set(MATH_TEST_SRC	../tools/math/math.c
			minunit.c
			math_test.c)
set(M8A_TEST_SRC	../avr/simm8a.c
			../tools/gis/ihex.c
			minunit.c
			m8a_test.c)

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
set(TARGET_LIBS "")

#
# Check functions exist here
#
if (NOT MSVC)
	CHECK_FUNCTION_EXISTS(fmax RESULT)
	if(NOT RESULT)
		unset(RESULT)
		list(APPEND TARGET_LIBS m)
		message(STATUS "Math library added to the TARGET_LIBS")
	endif()
endif()

add_executable(tsc_test ${TSC_TEST_SRC})
add_executable(math_test ${MATH_TEST_SRC})
add_executable(m8a_test ${M8A_TEST_SRC})

target_link_libraries(math_test ${TARGET_LIBS})

#
# Prepare files required for tests
#
file(COPY "../samples/avr-progmem.hex" DESTINATION ${CMAKE_BINARY_DIR})

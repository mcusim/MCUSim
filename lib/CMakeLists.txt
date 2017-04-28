#
# MCUSim - Interactive simulator for microcontrollers.
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
project(MCUSim_Library)

include(CheckFunctionExists)

#
# Set library name
#
set(LIB_NAME "mcusim")

#add_custom_target(sources_tar
#			COMMAND tar -zcvf mcusim.tar.gz
#				arch/ avr/ include/ tests/ tools/
#				CMakeLists.txt COPYING
#			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

#
# Set flags here
#
if (CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "(PROJECT) Release version of MCUSim library will be built.")

	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wmissing-prototypes -Wconversion")
	endif()
else()
	message(STATUS "(PROJECT) Debug version of MCUSim library will be built by default.")
	message(STATUS "(PROJECT) Set CMAKE_BUILD_TYPE=Release to build a release version.")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")

	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
	endif()
endif()

#
# Set sources here
#
set(LIB_SRCS		./avr/simcore.c
			./avr/simm8a.c
			./clock/x86/tsc.c
			./clock/x86/util.c
			./hex/ihex.c
			./math/math.c
			./datastruct/queue.c
			./thread/pool.c)

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
		list(APPEND TARGET_LIBS -lm)
		message(STATUS "(PROJECT) Math library added to the TARGET_LIBS")
	endif()
endif()

#
# Define includes here
#
include_directories("./include/")

add_library(${LIB_NAME} STATIC ${LIB_SRCS})
target_link_libraries(${LIB_NAME} ${TARGET_LIBS})

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
project(MCUSim C)

include(CheckFunctionExists)

# Set application name
set(AVRSIM "avrsim")

#
# Set flags here
#
if (CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "(PROJECT) Release version of MCUSim will be built.")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic -std=iso9899:1990")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wmissing-prototypes -Wconversion")
	endif()
else()
	message(STATUS "(PROJECT) Debug version of MCUSim will be built by default.")
	message(STATUS "(PROJECT) Set CMAKE_BUILD_TYPE=Release to build a release version.")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1990")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
	endif()
endif()

#
# Set sources here
#
set(AVRSIM_SRC		avrsim/simcore.c
			avrsim/simm8a.c
			avrsim/simm328p.c
			avrsim/simm328.c
			avrsim/simm2560.c
			avrsim/ihex.c
			avrsim/cli.c
			avrsim/getopt_long.c
			avrsim/avrsim.c)

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
include_directories("avrsim/include/")

add_executable(${AVRSIM} ${AVRSIM_SRC})

target_link_libraries(${AVRSIM} ${TARGET_LIBS})

file(COPY "samples/avr-progmem.hex" DESTINATION ${CMAKE_BINARY_DIR})

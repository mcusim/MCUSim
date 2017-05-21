#
# AVRSim - Simulator for AVR microcontrollers.
# This software is a part of MCUSim, interactive simulator for
# microcontrollers.
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
project(AVRSim C)

include(CheckFunctionExists)

# Set application name
set(SIM_NAME "avrsim")
set(STAT_NAME "avrstat")

# Set build options here
option(MSIM_TEXT_MODE "Print simulator messages to stdout" ON)
option(MSIM_IPC_MODE_QUEUE "Use queues to communicate with the simulator" OFF)
option(MSIM_PRINT_INST "Print next AVR instruction to be decoded (It generates a lot of status messages!)" OFF)

#
# Set flags here
#
if (CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "(PROJECT) Release version of AVRSim will be built.")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wmissing-prototypes -Wconversion")
	endif()
else()
	message(STATUS "(PROJECT) Debug version of AVRSim will be built by default.")
	message(STATUS "(PROJECT) Set CMAKE_BUILD_TYPE=Release to build a release version.")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
	endif()
endif()
if (MSIM_TEXT_MODE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMSIM_TEXT_MODE")
endif()
if (MSIM_IPC_MODE_QUEUE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMSIM_IPC_MODE_QUEUE")
endif()
if (MSIM_PRINT_INST)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMSIM_PRINT_INST")
endif()

#
# Set sources here
#
set(SIM_SRCS		simcore.c
			simm8a.c
			ihex.c
			ipc.c
			avrsim.c)

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
include_directories("include/")

add_executable(${SIM_NAME} ${SIM_SRCS})
add_executable(${STAT_NAME} avrstat.c)

target_link_libraries(${SIM_NAME} ${TARGET_LIBS})
target_link_libraries(${STAT_NAME} ${TARGET_LIBS})

file(COPY "../samples/avr-progmem.hex" DESTINATION ${CMAKE_BINARY_DIR})

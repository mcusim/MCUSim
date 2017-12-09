#
# MCUSim - Simulator for microcontrollers.
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
include(CheckIncludeFiles)

# Version
set(MSIM_VERSION "0.1.26")
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

# Add subprojects here
add_subdirectory(handbook)

# Set application name
set(AVRSIM "avrsim")

# Set flags here
if (CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "Release version of MCUSim ${MSIM_VERSION} will be built.")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic -std=iso9899:1990")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wmissing-prototypes -Wconversion")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wsign-compare")
	endif()
else()
	message(STATUS "Debug version of MCUSim ${MSIM_VERSION} will be built by default.")
	message(STATUS "Set CMAKE_BUILD_TYPE=Release to build a release version.")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1990")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-compare")
	endif()
endif()

# Set sources here
set(AVRSIM_SRC		avrsim/simcore.c
			avrsim/simm8a.c
			avrsim/simm328p.c
			avrsim/simm328.c
			avrsim/simm2560.c
			avrsim/ihex.c
			avrsim/cli.c
			avrsim/getopt_long.c
			avrsim/peripheral_lua.c
			avrsim/decoder.c
			avrsim/avrsim.c
			avrsim/gdb_rsp.c
			avrsim/vcd_dump.c)

# Look for libraries, packages, etc.
find_package(Lua51)

# Set target libraries here
set(TARGET_LIBS "")
if (LUA51_FOUND)
	add_definitions(-DLUA51_FOUND=1)
	set(TARGET_LIBS ${TARGET_LIBS} ${LUA_LIBRARIES})
else()
	message(STATUS "Support for Lua-based peripherals will be disabled")
endif()

#
# Check functions, include files here
#
if (NOT MSVC)
	check_function_exists(fmax RESULT)
	if (NOT RESULT)
		unset(RESULT)
		list(APPEND TARGET_LIBS -lm)
		message(STATUS "Math library added to the TARGET_LIBS")
	endif()
endif()
check_include_files(netdb.h HAVE_NETDB_H)
check_include_files("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(poll.h HAVE_POLL_H)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)
if (NOT CYGWIN)
	check_include_files(netinet/tcp.h HAVE_NETINET_TCP_H)
	if (HAVE_NETDB_H AND
	    HAVE_SYS_SOCKET_H AND
	    HAVE_STRING_H AND
	    HAVE_FCNTL_H AND
	    HAVE_UNISTD_H AND
	    HAVE_ERRNO_H AND
	    HAVE_POLL_H AND
	    HAVE_NETINET_IN_H AND
	    HAVE_NETINET_TCP_H)
		add_definitions(-DMSIM_POSIX=1)
	endif()
else()
	# CMake cannot find netinet/tcp.h under CYGWIN and it's not
	# required, to be honest. Let me ignore this header silently.
	if (HAVE_NETDB_H AND
	    HAVE_SYS_SOCKET_H AND
	    HAVE_STRING_H AND
	    HAVE_FCNTL_H AND
	    HAVE_UNISTD_H AND
	    HAVE_ERRNO_H AND
	    HAVE_POLL_H AND
	    HAVE_NETINET_IN_H)
		add_definitions(-DMSIM_POSIX=1)
		add_definitions(-DMSIM_POSIX_CYGWIN=1)
	endif()
endif()
#
# END Check functions, include files here
#

# Define includes here
include_directories("avrsim/include/")
if (LUA51_FOUND)
	include_directories(${LUA_INCLUDE_DIR})
endif()

add_executable(${AVRSIM} ${AVRSIM_SRC})
target_link_libraries(${AVRSIM} ${TARGET_LIBS})

# Copy required files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/rom/avr-progmem.hex DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/peripheral/avr/examples/io.lua DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS ${AVRSIM} DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/avrsim/man/avrsim.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)

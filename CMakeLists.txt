#
# Copyright (c) 2017, 2018, The MCUSim Contributors
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the MCUSim or its parts nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# MCUSim build configuration file.
#
cmake_minimum_required(VERSION 3.2)
project(MCUSim C)

# Program version
set(MSIM_VERSION "0.1.97")

# Program name
set(MCUSIM "mcusim")
set(MCUSIM_LIB "libmcusim")

# Load modules from our source tree too
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")
add_subdirectory(scripts)	# Helper scripts and script-based MCUSim models
add_subdirectory(tests)		# MCUSim simulation tests
add_subdirectory(misra)		# Configuration to check MISRA C rules
add_subdirectory(xspice)	# Compile MCUSim as XSPICE .cm library

include(CheckFunctionExists)
include(CheckIncludeFiles)

# Set flags here
if (CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "[OPTION] Building MCUSim ${MSIM_VERSION}, debug version")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-compare")
	endif()
else()
	message(STATUS "[OPTION] Building MCUSim ${MSIM_VERSION}, release version")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wmissing-prototypes -Wconversion")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wsign-compare")
	endif()
endif()

# Set sources here
set(MCUSIM_LIB_SRC	src/avr/avr_simcore.c
			src/avr/avr_m8a.c
			src/avr/avr_m328p.c
			src/avr/avr_m328.c
			src/avr/avr_m2560.c
			src/avr/avr_lua.c
			src/avr/avr_luaapi.c
			src/avr/avr_decoder.c
			src/avr/avr_gdb.c
			src/avr/avr_vcd.c
			src/ihex.c
			src/getopt_long.c)

# Look for libraries, packages, etc.
if (WITH_LUA)
	find_package(Lua REQUIRED)
else()
	find_package(Lua)
endif()

# Set target libraries here
set(TARGET_LIBS "")
if (LUA_FOUND AND
    ((LUA_VERSION_MAJOR EQUAL 5) OR (LUA_VERSION_MAJOR GREATER 5)) AND
    ((LUA_VERSION_MINOR EQUAL 2) OR (LUA_VERSION_MINOR GREATER 2)))
	add_definitions(-DLUA_FOUND=1)
	set(TARGET_LIBS ${TARGET_LIBS} ${LUA_LIBRARIES})
	message(STATUS "[OPTION] Building with Lua-based models")
else()
	if (WITH_LUA)
		message(FATAL_ERROR "[OPTION] Building without Lua-based models")
	else()
		message(STATUS "[OPTION] Building without Lua-based models")
	endif()
endif()

# Check functions, include files here
if (NOT MSVC)
	check_function_exists(fmax RESULT)
	if (NOT RESULT)
		unset(RESULT)
		list(APPEND TARGET_LIBS -lm)
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

# Define include files and directories here
include_directories("include/")
if (LUA_FOUND)
	include_directories(${LUA_INCLUDE_DIR})
endif()

# Compile MCUSim library and executable
add_library(${MCUSIM_LIB} STATIC ${MCUSIM_LIB_SRC})
add_executable(${MCUSIM} src/main.c)
set_target_properties(${MCUSIM_LIB} PROPERTIES OUTPUT_NAME "mcusim")
if ((WITH_FPIC MATCHES FALSE) OR
    (WITH_FPIC MATCHES false) OR
    (WITH_FPIC MATCHES False))
	message(STATUS "[OPTION] Building without position-independent code (-fPIC) option")
	set_target_properties(${MCUSIM_LIB} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
else()
	message(STATUS "[OPTION] Building with position-independent code (-fPIC) option")
	set_target_properties(${MCUSIM_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Link MCUSim executable
target_link_libraries(${MCUSIM} ${MCUSIM_LIB})
target_link_libraries(${MCUSIM} ${TARGET_LIBS})

# Install MCUSim executable
install(TARGETS ${MCUSIM} DESTINATION bin)

file(COPY ${CMAKE_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR})

#
# Copyright 2017-2019 The MCUSim Project.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the MCUSim or its parts nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
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
# Configuration file of the MCUSim build system.
#
# NOTE: Verbose output of the build process can be activated
# using 'make VERBOSE=1'.
#
cmake_minimum_required(VERSION 3.2)
project(MCUSim C)

# Program version
set(MSIM_VERSION "0.1.170")
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

# Program name
set(MCUSIM "mcusim")
set(MCUSIM_LIB_NAME "msim")
set(MCUSIM_LIB "lib${MCUSIM_LIB_NAME}")

# Load modules from our source tree too
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Filename.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Subdirlist.cmake)

# -----------------------------------------------------------------------------
# Paths to install MCUSim to
# -----------------------------------------------------------------------------
if (NOT DEFINED MSIM_BIN_DIR)
	set(MSIM_BIN_DIR "bin")
endif()
if (NOT DEFINED MSIM_LIB_DIR)
	set(MSIM_LIB_DIR "lib")
endif()
if (NOT DEFINED MSIM_SLIB_DIR)
	set(MSIM_SLIB_DIR "lib")
endif()
if (NOT DEFINED MSIM_H_DIR)
	set(MSIM_H_DIR "include")
endif()
if (NOT DEFINED MSIM_CONF_DIR)
	set(MSIM_CONF_DIR "etc/mcusim")
endif()
if (NOT DEFINED MSIM_SCRIPT_DIR)
	set(MSIM_SCRIPT_DIR "share/mcusim/scripts")
endif()
if (NOT DEFINED MSIM_EXAMPLE_DIR)
	set(MSIM_EXAMPLE_DIR "share/mcusim/examples")
endif()

message(STATUS "Install to: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "    binaries: ${CMAKE_INSTALL_PREFIX}/${MSIM_BIN_DIR}")
message(STATUS "    shared libraries: ${CMAKE_INSTALL_PREFIX}/${MSIM_LIB_DIR}")
message(STATUS "    static libraries: ${CMAKE_INSTALL_PREFIX}/${MSIM_SLIB_DIR}")
message(STATUS "    headers: ${CMAKE_INSTALL_PREFIX}/${MSIM_H_DIR}")
message(STATUS "    config: ${CMAKE_INSTALL_PREFIX}/${MSIM_CONF_DIR}")
message(STATUS "    scripts: ${CMAKE_INSTALL_PREFIX}/${MSIM_SCRIPT_DIR}")
message(STATUS "    examples: ${CMAKE_INSTALL_PREFIX}/${MSIM_EXAMPLE_DIR}")

# -----------------------------------------------------------------------------
# Parts of the project
# -----------------------------------------------------------------------------
add_subdirectory(scripts)	# Scripts and lua models
add_subdirectory(examples)	# Example circuits
add_subdirectory(tests)		# Simulation tests
add_subdirectory(misra)		# Configuration to check MISRA C rules
add_subdirectory(xspice)	# Compile MCUSim as XSPICE library

include(CheckFunctionExists)
include(CheckIncludeFiles)

# -----------------------------------------------------------------------------
# Set flags here
# -----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "Building DEBUG version of MCUSim ${MSIM_VERSION}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC OR
			${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		add_definitions(" -g -Wall -pedantic -std=iso9899:1999")
		add_definitions(" -Wshadow -Wpointer-arith -Wcast-qual")
		add_definitions(" -Wcast-align -Wstrict-prototypes")
		add_definitions(" -Wmissing-prototypes -Wconversion")
		add_definitions(" -Wsign-compare")
	endif()
else()
	message(STATUS "Building RELEASE version of MCUSim ${MSIM_VERSION}")
	if (CMAKE_COMPILER_IS_GNUCC OR
			${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		add_definitions(" -Wall -pedantic -std=iso9899:1999 -O3")
		add_definitions(" -Wshadow -Wpointer-arith -Wcast-qual")
		add_definitions(" -Wcast-align -Wstrict-prototypes")
		add_definitions(" -Wmissing-prototypes -Wconversion")
		add_definitions(" -Wsign-compare")
	endif()
endif()

# -----------------------------------------------------------------------------
# Set sources here
# -----------------------------------------------------------------------------
set(MCUSIM_LIB_SOURCES	src/avr/avr_simcore.c
			src/avr/avr_m8a.c
			src/avr/avr_m328p.c
			src/avr/avr_m328.c
			#src/avr/avr_m2560.c
			src/avr/avr_lua.c
			src/avr/avr_luaapi.c
			src/avr/avr_decoder.c
			src/avr/avr_gdb.c
			src/avr/avr_vcd.c
			src/msim_config.c
			src/msim_getopt.c
			src/msim_ihex.c
			src/msim_log.c
			src/msim_pty.c
			src/msim_tsq.c)

# -----------------------------------------------------------------------------
# Look for libraries, packages, etc.
# -----------------------------------------------------------------------------
set(TARGET_LIBS "")

# Check LuaJIT or Lua installed
unset(LUA_FOUND CACHE)
unset(LUA_INCLUDE_DIR CACHE)
unset(LUA_LIBRARY CACHE)
if (WITH_LUA)
	set(LUA_TYPE "Lua")
	find_package(Lua REQUIRED)
else()
	set(LUA_TYPE "LuaJIT")
	find_package(LuaJIT REQUIRED)
endif()
if (LUA_FOUND)
	set(TARGET_LIBS ${TARGET_LIBS} ${LUA_LIBRARIES})
endif()

# Check POSIX threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
	set(TARGET_LIBS ${TARGET_LIBS} Threads::Threads)
endif()

# Check math library
if (NOT MSVC)
	check_function_exists(fmax RESULT)
	if (NOT RESULT)
		unset(RESULT)
		list(APPEND TARGET_LIBS -lm)
	endif()
endif()

# Check basic POSIX headers.
check_include_files(netdb.h HAVE_NETDB_H)
check_include_files("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(poll.h HAVE_POLL_H)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)
check_include_files(signal.h HAVE_SIGNAL_H)
if (NOT CYGWIN)
	check_include_files(netinet/tcp.h HAVE_NETINET_TCP_H)
	if (HAVE_NETDB_H AND HAVE_SYS_SOCKET_H AND HAVE_STRING_H AND
	    HAVE_FCNTL_H AND HAVE_UNISTD_H AND HAVE_ERRNO_H AND HAVE_POLL_H AND
	    HAVE_NETINET_IN_H AND HAVE_NETINET_TCP_H AND HAVE_SIGNAL_H AND
	    CMAKE_USE_PTHREADS_INIT)
		add_definitions(-DWITH_POSIX=1)
	else()
		message(STATUS "WITH_POSIX undefined!")
	endif()
else()
	# CMake cannot find netinet/tcp.h under CYGWIN and it's not
	# required. Let it be ignored silently.
	if (HAVE_NETDB_H AND HAVE_SYS_SOCKET_H AND HAVE_STRING_H AND
	    HAVE_FCNTL_H AND HAVE_UNISTD_H AND HAVE_ERRNO_H AND HAVE_POLL_H AND
	    HAVE_NETINET_IN_H AND CMAKE_USE_PTHREADS_INIT)
		add_definitions(-DWITH_POSIX=1)
		add_definitions(-DWITH_POSIX_CYGWIN=1)
	else()
		message(STATUS "WITH_POSIX undefined!")
		message(STATUS "WITH_POSIX_CYGWIN undefined!")
	endif()
endif()

# Check whether we will be able to use pseudo-terminals or not.
check_function_exists(posix_openpt HAVE_POSIX_OPENPT)
check_function_exists(grantpt HAVE_POSIX_GRANTPT)
check_function_exists(ptsname HAVE_POSIX_PTSNAME)
check_function_exists(unlockpt HAVE_POSIX_UNLOCKPT)
check_function_exists(open HAVE_POSIX_OPEN)
check_function_exists(close HAVE_POSIX_CLOSE)
check_function_exists(read HAVE_POSIX_READ)
check_function_exists(write HAVE_POSIX_WRITE)
if (HAVE_POSIX_OPENPT AND HAVE_POSIX_GRANTPT AND HAVE_POSIX_PTSNAME AND
    HAVE_POSIX_UNLOCKPT AND HAVE_POSIX_OPEN AND HAVE_POSIX_CLOSE AND
    HAVE_POSIX_READ AND HAVE_POSIX_WRITE)
	add_definitions(-DWITH_POSIX_PTY=1)
else()
	message(STATUS "WITH_POSIX_PTY undefined!")
endif()

# -----------------------------------------------------------------------------
# Prepare headers
# -----------------------------------------------------------------------------
file(COPY ${CMAKE_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR}
     FILES_MATCHING PATTERN "*.h")
configure_file(${CMAKE_SOURCE_DIR}/include/mcusim/config.h.in
               ${CMAKE_BINARY_DIR}/include/mcusim/config.h @ONLY)

# -----------------------------------------------------------------------------
# Define include files and directories here
# -----------------------------------------------------------------------------
include_directories("${CMAKE_BINARY_DIR}/include/")
if (LUA_FOUND)
	include_directories(${LUA_INCLUDE_DIR})
endif()

# -----------------------------------------------------------------------------
# Compile MCUSim
# -----------------------------------------------------------------------------
add_library(objlib OBJECT ${MCUSIM_LIB_SOURCES})
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(${MCUSIM_LIB} SHARED $<TARGET_OBJECTS:objlib>)
add_library("${MCUSIM_LIB}-static" STATIC $<TARGET_OBJECTS:objlib>)

add_executable(${MCUSIM} src/msim_main.c)
set_target_properties(${MCUSIM_LIB} PROPERTIES OUTPUT_NAME ${MCUSIM_LIB_NAME})
set_target_properties("${MCUSIM_LIB}-static" PROPERTIES OUTPUT_NAME ${MCUSIM_LIB_NAME})

define_filename_for_sources(${MCUSIM_LIB})
define_filename_for_sources(${MCUSIM})

# -----------------------------------------------------------------------------
# Link MCUSim
# -----------------------------------------------------------------------------
target_link_libraries(${MCUSIM_LIB} ${TARGET_LIBS})
target_link_libraries(${MCUSIM} ${MCUSIM_LIB})
if (APPLE AND CMAKE_SIZEOF_VOID_P EQUAL 8 AND LUA_TYPE MATCHES "LuaJIT")
	# Add LuaJIT-specific flags for 64-bit build on macOS
	message(STATUS "Linking MCUSim with LuaJIT-specific flags on macOS with 64-bit build")
	target_link_libraries(${MCUSIM} "-pagezero_size 10000")
	target_link_libraries(${MCUSIM} "-image_base 100000000")
endif()

# -----------------------------------------------------------------------------
# Install MCUSim executable, library and headers
# -----------------------------------------------------------------------------
install(TARGETS ${MCUSIM} ${MCUSIM_LIB} "${MCUSIM_LIB}-static"
	RUNTIME DESTINATION ${MSIM_BIN_DIR}
	LIBRARY DESTINATION ${MSIM_LIB_DIR}
	ARCHIVE DESTINATION ${MSIM_SLIB_DIR})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/mcusim DESTINATION ${MSIM_H_DIR}
        FILES_MATCHING PATTERN "*.h")

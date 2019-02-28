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
set(MSIM_VERSION "0.1.0151")
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

# Program name
set(MCUSIM "mcusim")
set(MCUSIM_LIB "libmsim")
set(MCUSIM_LIB_NAME "msim")

# Load modules from our source tree too
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Filename.cmake)

add_subdirectory(scripts)	# Scripts and script-based MCUSim models
add_subdirectory(tests)		# MCUSim simulation tests
add_subdirectory(misra)		# Configuration to check MISRA C rules
add_subdirectory(xspice)	# Compile MCUSim as XSPICE library

include(CheckFunctionExists)
include(CheckIncludeFiles)

# Set flags here
if (CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "Building a debug version of MCUSim ${MSIM_VERSION}")
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
	message(STATUS "Building a release version of MCUSim ${MSIM_VERSION}")
	if (CMAKE_COMPILER_IS_GNUCC OR
			${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		add_definitions(" -Wall -pedantic -std=iso9899:1999 -O3")
		add_definitions(" -Wshadow -Wpointer-arith -Wcast-qual")
		add_definitions(" -Wcast-align -Wstrict-prototypes")
		add_definitions(" -Wmissing-prototypes -Wconversion")
		add_definitions(" -Wsign-compare")
	endif()
endif()

# Set sources here
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

# Look for libraries, packages, etc.
option(LUA_USE_LUAJIT "Use LuaJIT instead of default Lua" ON)
unset(LUA_FOUND CACHE)
unset(LUA_INCLUDE_DIR CACHE)
unset(LUA_LIBRARY CACHE)
find_package(Lua REQUIRED)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Set target libraries here
set(TARGET_LIBS "")
if (LUA_FOUND)
	set(TARGET_LIBS ${TARGET_LIBS} ${LUA_LIBRARIES})
	message(STATUS "Building with Lua-based models")
else()
	message(FATAL_ERROR "Building without Lua-based models")
endif()

if (CMAKE_USE_PTHREADS_INIT)
	set(TARGET_LIBS ${TARGET_LIBS} Threads::Threads)
endif()

# Check functions, include files here
if (NOT MSVC)
	check_function_exists(fmax RESULT)
	if (NOT RESULT)
		unset(RESULT)
		list(APPEND TARGET_LIBS -lm)
	endif()
endif()
check_function_exists(posix_openpt HAVE_POSIX_OPENPT)
check_function_exists(grantpt HAVE_POSIX_GRANTPT)
check_function_exists(ptsname HAVE_POSIX_PTSNAME)
check_function_exists(unlockpt HAVE_POSIX_UNLOCKPT)
check_function_exists(open HAVE_POSIX_OPEN)
check_function_exists(close HAVE_POSIX_CLOSE)
check_function_exists(read HAVE_POSIX_READ)
check_function_exists(write HAVE_POSIX_WRITE)
if (WITH_POSIX_PTY MATCHES True)
	message(STATUS "POSIX pseudo-terminals strictly required")
endif()
if (WITH_POSIX_PTY MATCHES False)
	message(STATUS "POSIX pseudo-terminals strictly not required")
endif()
if (HAVE_POSIX_OPENPT AND
    HAVE_POSIX_GRANTPT AND
    HAVE_POSIX_PTSNAME AND
    HAVE_POSIX_UNLOCKPT AND
    HAVE_POSIX_OPEN AND
    HAVE_POSIX_CLOSE AND
    HAVE_POSIX_READ AND
    HAVE_POSIX_WRITE)
	if (WITH_POSIX_PTY MATCHES False)
		message(STATUS "Building without POSIX pseudo-terminals")
	else(WITH_POSIX_PTY MATCHES False)
		message(STATUS "Building with POSIX pseudo-terminals")
		add_definitions(-DMSIM_POSIX_PTY=1)
	endif(WITH_POSIX_PTY MATCHES False)
else()
	if (WITH_POSIX_PTY)
		message(FATAL_ERROR "Building without POSIX pseudo-terminals")
	else()
		message(STATUS "Building without POSIX pseudo-terminals")
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
check_include_files(signal.h HAVE_SIGNAL_H)
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
	    HAVE_NETINET_TCP_H AND
	    HAVE_SIGNAL_H AND
	    CMAKE_USE_PTHREADS_INIT)
		add_definitions(-DMSIM_POSIX=1)
		message(STATUS "Building with MSIM_POSIX defined")
	else()
		message(STATUS "Building without MSIM_POSIX defined")
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
	    HAVE_NETINET_IN_H AND
	    CMAKE_USE_PTHREADS_INIT)
		add_definitions(-DMSIM_POSIX=1)
		add_definitions(-DMSIM_POSIX_CYGWIN=1)
		message(STATUS "Building with MSIM_POSIX defined")
		message(STATUS "Building with MSIM_POSIX_CYGWIN defined")
	else()
		message(STATUS "Building without MSIM_POSIX defined")
		message(STATUS "Building without MSIM_POSIX_CYGWIN defined")
	endif()
endif()

# Prepare headers.
file(COPY ${CMAKE_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR}
     FILES_MATCHING PATTERN "*.h")
configure_file(${CMAKE_SOURCE_DIR}/include/mcusim/config.h.in
               ${CMAKE_BINARY_DIR}/include/mcusim/config.h @ONLY)

# Define include files and directories here.
include_directories("${CMAKE_BINARY_DIR}/include/")
if (LUA_FOUND)
	include_directories(${LUA_INCLUDE_DIR})
endif()

# Compile MCUSim library and executable
add_library(${MCUSIM_LIB} STATIC ${MCUSIM_LIB_SOURCES})
add_executable(${MCUSIM} src/msim_main.c)
set_target_properties(${MCUSIM_LIB} PROPERTIES OUTPUT_NAME ${MCUSIM_LIB_NAME})
if ((WITH_FPIC MATCHES FALSE) OR
    (WITH_FPIC MATCHES false) OR
    (WITH_FPIC MATCHES False))
	message(STATUS "Building without position-independent code (-fPIC) option")
	set_target_properties(${MCUSIM_LIB} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
else()
	message(STATUS "Building with position-independent code (-fPIC) option")
	set_target_properties(${MCUSIM_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
define_filename_for_sources(${MCUSIM_LIB})
define_filename_for_sources(${MCUSIM})

# Link MCUSim executable
target_link_libraries(${MCUSIM} ${MCUSIM_LIB})
target_link_libraries(${MCUSIM} ${TARGET_LIBS})
# Add LuaJIT-specific flags for 64-bit build on macOS
if (APPLE AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "Linking MCUSim with LuaJIT-specific flags on macOS with 64-bit build")
	target_link_libraries(${MCUSIM} "-pagezero_size 10000")
	target_link_libraries(${MCUSIM} "-image_base 100000000")
endif()

# Install MCUSim executable, library and headers.
install(TARGETS ${MCUSIM} DESTINATION bin)
install(TARGETS ${MCUSIM_LIB} DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/mcusim DESTINATION include
        FILES_MATCHING PATTERN "*.h")

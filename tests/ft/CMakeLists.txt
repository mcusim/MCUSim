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
# Configuration file for real-world firmware tests based on 'cmocka' library.
#
cmake_minimum_required(VERSION 3.2)
project(MCUSim-unit-tests C)

# Version
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

if (WITH_FIRMWARE_TESTS)
# -----------------------------------------------------------------------------
# Look for libraries, packages, etc.
# -----------------------------------------------------------------------------
	set(TARGET_LIBS ${MCUSIM_LIB})

	# Check cmocka library
	pkg_search_module(CMOCKA REQUIRED cmocka)
	if (CMOCKA_FOUND)
		message(STATUS "cmocka ldflags: ${CMOCKA_LDFLAGS}")
		set(TARGET_LIBS ${TARGET_LIBS} ${CMOCKA_LDFLAGS})
	endif()

# -----------------------------------------------------------------------------
# Define include files and directories here
# -----------------------------------------------------------------------------
	include_directories("${CMAKE_BINARY_DIR}/include/")
	if (CMOCKA_FOUND)
		message(STATUS "cmocka cflags: ${CMOCKA_CFLAGS}")
		add_definitions(${CMOCKA_CFLAGS})
	endif()

# -----------------------------------------------------------------------------
# Compile unit tests
# -----------------------------------------------------------------------------
	add_executable(XlingFirmware.ft XlingFirmware.ft.c)

# -----------------------------------------------------------------------------
# Link unit tests
# -----------------------------------------------------------------------------
	target_link_libraries(XlingFirmware.ft ${TARGET_LIBS})

# -----------------------------------------------------------------------------
# Prepare files in the current binary directory
# -----------------------------------------------------------------------------
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests.cmake.in
	               ${CMAKE_CURRENT_BINARY_DIR}/tests.cmake @ONLY)

	file(GLOB_RECURSE FLS "@CMAKE_CURRENT_SOURCE_DIR@/files/*")
	foreach(F ${FLS})
		file(COPY ${F} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	endforeach()

# -----------------------------------------------------------------------------
# Run unit tests by 'make check'
# -----------------------------------------------------------------------------
	add_custom_command(OUTPUT UNIT-TESTS
		COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tests.cmake)
	add_custom_target(check DEPENDS UNIT-TESTS)

endif() # WITH_UNIT_TESTS

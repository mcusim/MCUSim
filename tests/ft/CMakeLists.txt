#
# This file is part of MCUSim, an XSPICE library with microcontrollers.
#
# Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
#
# MCUSim is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# MCUSim is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

# Configuration file for firmware checks (run by 'make check').
cmake_minimum_required(VERSION 3.2)
project(MCUSim-unit-tests C)

# Version
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

if (WITH_CHECKS)
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
	add_executable(XlingFirmware_1.ft XlingFirmware_1.ft.c)

# -----------------------------------------------------------------------------
# Link unit tests
# -----------------------------------------------------------------------------
	target_link_libraries(XlingFirmware.ft ${TARGET_LIBS})
	target_link_libraries(XlingFirmware_1.ft ${TARGET_LIBS})

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

endif() # WITH_CHECKS

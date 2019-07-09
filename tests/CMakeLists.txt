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

# Configuration file for MCUSim simulation tests (run by 'make tests').
cmake_minimum_required(VERSION 3.2)
project(MCUSim-tests C)

# Version
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

# -----------------------------------------------------------------------------
# Additional tests
# -----------------------------------------------------------------------------
add_subdirectory(ft)		# Firmware checks

# -----------------------------------------------------------------------------
# Prepare files in the current binary directory
# -----------------------------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/tests.cmake @ONLY)

subdirlist(TEST_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(TEST_DIR ${TEST_DIRS})
	file(COPY ${TEST_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

# -----------------------------------------------------------------------------
# Run tests by 'make tests'
# -----------------------------------------------------------------------------
add_custom_command(OUTPUT SIMULATION-TESTS
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tests.cmake)
add_custom_target(tests DEPENDS SIMULATION-TESTS)
